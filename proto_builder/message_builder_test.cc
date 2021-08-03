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

#include "proto_builder/message_builder.h"

#include <map>
#include <string>
#include <vector>

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/tests/test_message.pb.h"
#include "proto_builder/tests/test_output.pb.h"
#include "proto_builder/tests/validator.pb.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "proto_builder/oss/unified_diff.h"

namespace proto_builder {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;
using tests::Validator;

namespace {

constexpr char kDir[] = "com_google_cpp_proto_builder/proto_builder";
constexpr char kMessagePrefix[] = "proto_builder";

struct TestCase {
  const ::google::protobuf::Descriptor& descriptor;
  const Where where;
  const absl::string_view filename;

  template <class M>
  static TestCase For(Where where, absl::string_view filename) {
    return TestCase{*PBCC_DIE_IF_NULL(M::descriptor()), where, filename};
  }

  static std::string Name(const testing::TestParamInfo<TestCase>& data) {
    return absl::StrCat(data.index, "_", data.param.descriptor.name(), "_",
                        Where_Name(data.param.where), "_",
                        absl::StrReplaceAll(data.param.filename, {{".", "_"}}));
  }
};

void PrintTo(const TestCase& test_case, std::ostream* out) {
  *out << "Message: " << test_case.descriptor.full_name()
       << " Where: " << Where_Name(test_case.where)
       << " Filename: " << test_case.filename;
}

}  // namespace
class MessageBuilderTest : public ::testing::Test {
 public:
  const ProtoBuilderConfigManager global_config_;
};

TEST_F(MessageBuilderTest, GetPackageAndClassName) {
  EXPECT_THAT(GetPackageAndClassName(TestMessage::descriptor()),
              Pair(kMessagePrefix, "TestMessage"));
  EXPECT_THAT(GetPackageAndClassName(TestMessage::Sub::descriptor()),
              Pair(kMessagePrefix, "TestMessage_Sub"));
  EXPECT_THAT(GetPackageAndClassName(TestMessage_Sub::descriptor()),
              Pair(kMessagePrefix, "TestMessage_Sub"));
}

TEST_F(MessageBuilderTest, Includes) {
  BufferWriter writer;
  const ::google::protobuf::Descriptor& descriptor = *PBCC_DIE_IF_NULL(TestMessage::descriptor());
  MessageBuilder({
                     .config = global_config_,
                     .writer = &writer,
                     .descriptor = descriptor,
                     .max_field_depth = 99,
                 })
      .WriteBuilder();
  EXPECT_THAT(
      writer.CodeInfo()->GetIncludes(HEADER),
      UnorderedElementsAre(
          "\"proto_builder/tests/"
          "extra_test_message.pb.h\""
          "  // IWYU pragma: export",
          "\"proto_builder/tests/"
          "map_value_test_message.pb.h\""
          "  // IWYU pragma: export",
          "\"proto_builder/tests/test_message.pb.h\""
          "  // IWYU pragma: export",
          "\"absl/strings/string_view.h\"",  // "s included
          "<string>"));
  EXPECT_THAT(writer.CodeInfo()->GetIncludes(SOURCE), IsEmpty());
}

class MessageBuilderFileTest : public ::testing::TestWithParam<TestCase> {
 protected:
  std::string ReadFile(absl::string_view filename) const {
    std::string out;
    CHECK_OK(file::oss::GetContents(filename, &out));
    return out;
  }

  std::string WriteBuilder(const google::protobuf::Descriptor& descriptor) const {
    BufferWriter writer;
    MessageBuilder({
                       .config = global_config_,
                       .writer = &writer,
                       .descriptor = descriptor,
                       .max_field_depth = 99,
                       .use_validator = &descriptor == Validator::descriptor(),
                   })
        .WriteBuilder();
    return absl::StrJoin(writer.From(GetParam().where), "\n");
  }

  void TestGoldenFile() {
    const google::protobuf::Descriptor& descriptor = GetParam().descriptor;
    const absl::string_view filename = GetParam().filename;
    SCOPED_TRACE(absl::StrCat("Message: ", descriptor.full_name()));
    SCOPED_TRACE(absl::StrCat("Testing: ", Where_Name(GetParam().where)));
    const std::string new_filename =
        file::oss::JoinPath(getenv("TEST_TMPDIR"), filename);
    const std::string old_filename = file::oss::JoinPath(
        getenv("TEST_SRCDIR"), kDir, "tests", filename);
    const std::string new_contents = WriteBuilder(descriptor);
    const std::string old_contents = ReadFile(old_filename);
    CHECK_OK(file::oss::SetContents(new_filename, new_contents));
    const std::string diff = ::proto_builder::oss::UnifiedDiff(old_contents, new_contents,
                                                 old_filename, new_filename, 3);
    EXPECT_TRUE(diff.empty()) << "Diff result:\n" << diff;
  }

  const ProtoBuilderConfigManager global_config_;
};

TEST_P(MessageBuilderFileTest, TestGoldenFile) { TestGoldenFile(); }

INSTANTIATE_TEST_SUITE_P(
    Golden, MessageBuilderFileTest,
    ::testing::Values(TestCase::For<TestMessage>(HEADER, "test_message.h.exp"),
                      TestCase::For<TestMessage>(SOURCE, "test_message.cc.exp"),
                      TestCase::For<TestOutput>(HEADER, "test_output.h.exp"),
                      TestCase::For<TestOutput>(SOURCE, "test_output.cc.exp"),
                      TestCase::For<Validator>(HEADER, "validator.h.exp"),
                      TestCase::For<Validator>(SOURCE, "validator.cc.exp")),
    TestCase::Name);

}  // namespace proto_builder
