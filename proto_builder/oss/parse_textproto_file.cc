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

#include <memory>
#include <string>
#include <vector>

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/parse_text_proto.h"
#include "proto_builder/oss/source_location.h"
#include "proto_builder/oss/sourcefile_database.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "gtest/gtest.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"

ABSL_FLAG(std::string, proto_type, "", "Proto message type.");
ABSL_FLAG(std::string, textproto, "", "File with textproto data.");

namespace proto_builder::oss {

void Load(int argc, char** argv) {
  // Prepare
  std::vector<std::string> proto_paths =
      oss::SourceFileDatabase::GetProtoPathsFlag();

  // Load proto database and construct a message.
  std::vector<std::string> proto_files;
  for (int i = 1; i < argc; ++i) {
    absl::string_view file = argv[i];
    QCHECK(absl::EndsWith(file, ".proto")) << file;
    proto_files.emplace_back(file);
  }
  const auto sfdb = SourceFileDatabase::New(proto_files, proto_paths);
  QCHECK(sfdb != nullptr);
  QCHECK(sfdb->LoadedSuccessfully()) << absl::StrJoin(sfdb->GetErrors(), "\n");
  const google::protobuf::DescriptorPool* pool = PBCC_DIE_IF_NULL(sfdb->pool());
  const google::protobuf::Descriptor* descriptor = PBCC_DIE_IF_NULL(
      pool->FindMessageTypeByName(absl::GetFlag(FLAGS_proto_type)));
  google::protobuf::DynamicMessageFactory factory(pool);
  const google::protobuf::Message* message_prototype =
      PBCC_DIE_IF_NULL(factory.GetPrototype(descriptor));
  std::unique_ptr<google::protobuf::Message> message(message_prototype->New());

  // Load textproto file
  const std::string textproto_file = absl::GetFlag(FLAGS_textproto);
  QCHECK(absl::EndsWith(textproto_file, ".textproto")) << textproto_file;
  std::string proto_text;
  QCHECK_OK(file::oss::GetContents(textproto_file, &proto_text))
      << textproto_file;
  const SourceLocation source(1, textproto_file.c_str());

  // Parse
  QCHECK_OK(internal::ParseTextInternal(proto_text, message.get(), source));
}

}  // namespace proto_builder::oss

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage(argv[0]);
  std::vector<char*> remaining = absl::ParseCommandLine(argc, argv);
  argc = remaining.size();
  QCHECK_GE(argc, 1);
  proto_builder::oss::Load(argc, remaining.data());
  return 0;
}
