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

#include <set>
#include <string>
#include <vector>

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/util.h"
#include "proto_builder/tests/test_import_message.pb.h"
#include "google/protobuf/descriptor.h"
#include "gmock/gmock.h"
// Placeholder for testing header.
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace proto_builder {
namespace {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;
using ::testing::status::oss::IsOkAndHolds;
using ::testing::status::oss::StatusIs;

class DescriptorUtilTest : public ::testing::Test {
 public:
  // LoadDescriptors() returns ::google::protobuf::Descriptor*s from its own SourceFileDatabase.
  // Instead of giving tests access to that DB, we rather make sure the returned
  // descriptors are for the expected messages. For that we simply return their
  // full_name()s.
  static absl::StatusOr<std::pair<std::set<std::string>, MessageSearchMode>>
  TestLoadDescriptors(absl::string_view proto,
                      const std::vector<std::string>& proto_files) {
    auto [status, descriptor_util] = UnpackStatusOrDefault(
        DescriptorUtil::Load(
            proto, proto_files,
            {file::oss::JoinPath(getenv("TEST_SRCDIR"), "")}),
        DescriptorUtil::DefaultWillNotWork::kDefaultWillNotWork);
    if (!status.ok()) {
      return status;
    }
    return std::make_pair(descriptor_util.GetFullNames(),
                          descriptor_util.search_mode());
  }

  static absl::StatusOr<std::pair<std::set<std::string>, MessageSearchMode>>
  TestLoadDescriptors(absl::string_view proto) {
    return TestLoadDescriptors(
        proto,
        {"proto_builder/tests/test_import_message.proto"});
  }
};

TEST_F(DescriptorUtilTest, GetDescriptorsFrom) {
  EXPECT_THAT(
      GetDescriptorsFrom(*ImportImportMessage::descriptor()->file(),
                         MessageSearchMode::kExplicit),
      IsOkAndHolds(UnorderedElementsAre(ImportImportMessage::descriptor())));
  EXPECT_THAT(
      GetDescriptorsFrom(*ImportImportMessage::descriptor()->file(),
                         MessageSearchMode::kAllTopLevel),
      IsOkAndHolds(UnorderedElementsAre(ImportImportMessage::descriptor())));
  EXPECT_THAT(GetDescriptorsFrom(*ImportImportMessage::descriptor()->file(),
                                 MessageSearchMode::kTransitiveRepeated),
              IsOkAndHolds(UnorderedElementsAre(
                  ImportImportMessage::descriptor(),
                  ImportImportMessage::Rep::descriptor(),
                  ImportImportMessage::Value::descriptor())));
  EXPECT_THAT(GetDescriptorsFrom(*ImportImportMessage::descriptor()->file(),
                                 MessageSearchMode::kTransitiveAll),
              IsOkAndHolds(UnorderedElementsAre(
                  ImportImportMessage::descriptor(),
                  ImportImportMessage::Rep::descriptor(),
                  ImportImportMessage::Sub::descriptor(),
                  ImportImportMessage::Value::descriptor())));
}

TEST_F(DescriptorUtilTest, LoadDescriptorsErrors) {
  // Load errors
  EXPECT_THAT(TestLoadDescriptors("NA", {}),
              StatusIs(absl::StatusCode::kNotFound,
                       "FieldDescriptor not found for: 'NA'"));

  // Errors from SourceFileDatabase::New
  EXPECT_THAT(TestLoadDescriptors("NA", {"na.proto"}),
              StatusIs(absl::StatusCode::kNotFound,
                       "Could not load proto_db: (na.proto)"));
  EXPECT_THAT(TestLoadDescriptors("*", {"na.proto"}),
              StatusIs(absl::StatusCode::kNotFound,
                       "Could not load proto_db: (na.proto)"));
}

TEST_F(DescriptorUtilTest, LoadDescriptors) {
  // Explicit
  EXPECT_THAT(
      TestLoadDescriptors(
          "proto_builder.ImportImportMessage.Sub"),
      IsOkAndHolds(Pair(
          ElementsAre("proto_builder.ImportImportMessage.Sub"),
          MessageSearchMode::kExplicit)));
  EXPECT_THAT(
      TestLoadDescriptors(
          "proto_builder.ImportImportMessage.Rep,"
          "proto_builder.ImportImportMessage.Sub"),
      IsOkAndHolds(Pair(
          ElementsAre("proto_builder.ImportImportMessage.Rep",
                      "proto_builder.ImportImportMessage.Sub"),
          MessageSearchMode::kExplicit)));
  EXPECT_THAT(
      TestLoadDescriptors("*"),
      IsOkAndHolds(
          Pair(ElementsAre("proto_builder.ImportImportMessage"),
               MessageSearchMode::kAllTopLevel)));
  EXPECT_THAT(
      TestLoadDescriptors("*+"),
      IsOkAndHolds(Pair(
          ElementsAre("proto_builder.ImportImportMessage",
                      "proto_builder.ImportImportMessage.Rep",
                      "proto_builder.ImportImportMessage.Value"),
          MessageSearchMode::kTransitiveRepeated)));
  EXPECT_THAT(
      TestLoadDescriptors("**"),
      IsOkAndHolds(Pair(
          ElementsAre("proto_builder.ImportImportMessage",
                      "proto_builder.ImportImportMessage.Rep",
                      "proto_builder.ImportImportMessage.Sub",
                      "proto_builder.ImportImportMessage.Value"),
          MessageSearchMode::kTransitiveAll)));
}

}  // namespace
}  // namespace proto_builder
