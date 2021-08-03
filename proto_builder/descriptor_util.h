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

#ifndef PROTO_BUILDER_DESCRIPTOR_UTIL_H_
#define PROTO_BUILDER_DESCRIPTOR_UTIL_H_

#include <string>
#include <tuple>
#include <vector>

#include "proto_builder/oss/sourcefile_database.h"
#include "google/protobuf/descriptor.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace proto_builder {

enum class MessageSearchMode {
  kExplicit = 0,            // Only exlicit messages.
  kAllTopLevel = 1,         // ALL messages at the top level,
  kTransitiveRepeated = 2,  // All top level and all used in repeated fields.
  kTransitiveAll = 3,       // All messages (full transitive closure).
};

absl::StatusOr<std::vector<const ::google::protobuf::Descriptor*>> GetDescriptorsFrom(
    const ::google::protobuf::FileDescriptor& file_desc, MessageSearchMode search_mode);

class DescriptorUtil {
 public:
  enum class DefaultWillNotWork { kDefaultWillNotWork = 0 };

  static absl::StatusOr<DescriptorUtil> Load(
      absl::string_view proto_flag, std::vector<std::string> proto_files,
      std::vector<std::string> proto_paths);

  explicit DescriptorUtil(DefaultWillNotWork) {}
  DescriptorUtil(DescriptorUtil&&) = default;

  const MessageSearchMode search_mode() const { return search_mode_; }
  const std::vector<const ::google::protobuf::Descriptor*>& descriptors() const {
    return descriptors_;
  }

  std::set<std::string> GetFullNames() const;

 private:
  DescriptorUtil() = default;

  absl::Status LoadDescriptors(absl::string_view proto_flag,
                               std::vector<std::string> proto_files,
                               std::vector<std::string> proto_paths);

  std::unique_ptr<const oss::SourceFileDatabase> proto_db_;
  MessageSearchMode search_mode_;
  std::vector<const ::google::protobuf::Descriptor*> descriptors_;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_DESCRIPTOR_UTIL_H_
