// Copyright 2021 The CPP Proto Builder Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// READ: https://google.github.io/cpp-proto-builder

#include "proto_builder/descriptor_util.h"

#include <limits>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/sourcefile_database.h"
#include "proto_builder/oss/util.h"
#include "proto_builder/template_builder.h"
#include "google/protobuf/descriptor.h"
#include "absl/algorithm/container.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace proto_builder {

absl::StatusOr<std::vector<const ::google::protobuf::Descriptor*>> GetDescriptorsFrom(
    const ::google::protobuf::FileDescriptor& file_desc, MessageSearchMode search_mode) {
  std::vector<const ::google::protobuf::Descriptor*> descriptors(file_desc.message_type_count());
  std::queue<const ::google::protobuf::Descriptor*> queue;
  // Collect the top level messages of each file. When 'recursive' is true,
  // these messages serve as the starting node to traverse.
  for (int i = 0; i < file_desc.message_type_count(); ++i) {
    const ::google::protobuf::Descriptor* msg_desc = file_desc.message_type(i);
    if (!msg_desc) {
      return absl::InternalError(absl::StrCat(
          "Descriptor (", i, ") is nullptr: ", file_desc.DebugString()));
    }
    descriptors[i] = msg_desc;
    queue.emplace(msg_desc);
  }
  if (search_mode == MessageSearchMode::kExplicit ||
      search_mode == MessageSearchMode::kAllTopLevel) {
    return descriptors;
  }
  // Using BFS, traverse from the top level messages to collect all message
  // types.
  while (!queue.empty()) {
    const ::google::protobuf::Descriptor* msg = queue.front();
    queue.pop();
    for (int i = 0; i < msg->field_count(); ++i) {
      const FieldDescriptor* field_desc = PBCC_DIE_IF_NULL(msg->field(i));
      if (!field_desc) {
        return absl::InternalError(absl::StrCat(
            "FieldDescriptor (", i, ") is nullptr: ", msg->full_name()));
      }
      if (field_desc->type() != FieldDescriptor::Type::TYPE_MESSAGE) {
        continue;
      }
      const bool is_map = field_desc->is_map();
      const bool is_repeated = field_desc->is_repeated();
      if (is_map) {
        // Maps are represented as messages. The key cannot be a message, so
        // only value entry types are checked. Note maps are not recursive.
        // As a result, they are not traversed further.
        const ::google::protobuf::Descriptor* message_type = field_desc->message_type();
        if (!message_type) {
          return absl::InternalError(absl::StrCat(
              "Descriptor for map is nullptr: ", field_desc->full_name()));
        }
        const FieldDescriptor* value_type = message_type->field(1);
        if (!value_type) {
          return absl::InternalError(
              absl::StrCat("FieldDescriptor for map value is nullptr: ",
                           field_desc->full_name()));
        }
        if (value_type->type() != FieldDescriptor::Type::TYPE_MESSAGE) {
          continue;
        }
        // Switch to value type but do not change is_map or is_repeated.
        field_desc = value_type;
      }
      const ::google::protobuf::Descriptor* msg_desc = field_desc->message_type();
      if (!msg_desc || absl::c_linear_search(descriptors, msg_desc)) {
        continue;
      }
      if (msg_desc->file() != &file_desc) {
        continue;  // Do not generate builders for imported messages.
      }
      if (is_map || is_repeated ||
          search_mode == MessageSearchMode::kTransitiveAll) {
        descriptors.push_back(msg_desc);
      }
      queue.emplace(msg_desc);
    }
  }
  return descriptors;
}

absl::Status DescriptorUtil::LoadDescriptors(
    absl::string_view proto_flag, std::vector<std::string> proto_files,
    std::vector<std::string> proto_paths) {
  // proto.first contains the message list for which builder(s) will be
  // generated. A "*" signifies a message list containing the top level messages
  // from the first file. A "**" signifies a message list containing all
  // messages reachable from the top level messages in the first file including
  // the top level messages.
  const std::pair<std::string, std::string> proto =
      absl::StrSplit(proto_flag, absl::MaxSplits(':', 1));
  search_mode_ = [proto] {
    if (proto.first == "**") {
      return MessageSearchMode::kTransitiveAll;
    }
    if (proto.first == "*+") {
      return MessageSearchMode::kTransitiveRepeated;
    }
    if (proto.first == "*") {
      return MessageSearchMode::kAllTopLevel;
    }
    return MessageSearchMode::kExplicit;
  }();
  // Add the files from the proto flag and check they can be loaded.
  for (const auto& proto_file :
       absl::StrSplit(proto.second, ',', absl::SkipEmpty())) {
    if (!file::oss::Readable(proto_file).ok()) {
      return absl::NotFoundError(
          absl::StrCat("Proto file not readable: '", proto_file, "'"));
    }
    proto_files.emplace_back(proto_file);
  }
  proto_db_ = oss::SourceFileDatabase::New(proto_files, proto_paths);
  if (!proto_db_ || !proto_db_->LoadedSuccessfully()) {
    return absl::NotFoundError(absl::StrCat(
        "Could not load proto_db: (", absl::StrJoin(proto_files, ","), ")"));
  }
  if (search_mode_ != MessageSearchMode::kExplicit) {
    // We explicitly only generate builders for descriptors in the first file in
    // the proto file list, as it simplifies the builder story if all the
    // descriptor types are in the same package and we can't guarantee that
    // different files all use the same package.
    if (proto_files.empty()) {
      return absl::InvalidArgumentError(
          "At least one proto_files required, none given");
    }
    const std::string stripped = proto_files[0];
    const ::google::protobuf::FileDescriptor* file_desc =
        proto_db_->pool()->FindFileByName(stripped);
    if (!file_desc) {
      return absl::NotFoundError(
          absl::StrCat("FileDescriptor is nullptr for: '", stripped, "'"));
    }
    if (auto [status, descriptors] =
            UnpackStatusOrDefault(GetDescriptorsFrom(*file_desc, search_mode_));
        !status.ok()) {
      return status;
    } else {
      descriptors_ = std::move(descriptors);
      return absl::OkStatus();
    }
  } else {
    std::vector<const ::google::protobuf::Descriptor*> descriptors;
    for (auto name : absl::StrSplit(proto.first, ',')) {
      const ::google::protobuf::Descriptor* descriptor =
          proto_db_->pool()->FindMessageTypeByName(std::string(name));
      if (!descriptor) {
        return absl::NotFoundError(
            absl::StrCat("FieldDescriptor not found for: '", name, "'"));
      }
      descriptors.push_back(descriptor);
    }
    descriptors_ = std::move(descriptors);
    return absl::OkStatus();
  }
}

absl::StatusOr<DescriptorUtil> DescriptorUtil::Load(
    absl::string_view proto_flag, std::vector<std::string> proto_files,
    std::vector<std::string> proto_paths) {
  DescriptorUtil result;
  absl::Status s = result.LoadDescriptors(proto_flag, proto_files, proto_paths);
  if (!s.ok()) {
    return s;
  }
  return result;
}

std::set<std::string> DescriptorUtil::GetFullNames() const {
  std::set<std::string> result;
  for (const ::google::protobuf::Descriptor* descriptor : descriptors_) {
    result.emplace(PBCC_DIE_IF_NULL(descriptor)->full_name());
  }
  return result;
}

}  // namespace proto_builder
