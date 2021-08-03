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

#include "proto_builder/builder_writer.h"

#include "proto_builder/proto_builder.pb.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace proto_builder {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

class WriterMock : public BuilderWriter, public CodeInfoCollector {
 public:
  MOCK_METHOD(void, Write, (Where, const std::string&), (override));
  MOCK_METHOD(void, AddInclude, (Where, absl::string_view), (override));
  CodeInfoCollector* CodeInfo() final { return this; }
  const CodeInfoCollector* CodeInfo() const final { return this; }
};

class CodeInfoCollectorTest : public ::testing::Test {
 protected:
  CodeInfoCollector code_info_;
};

TEST_F(CodeInfoCollectorTest, AddInclude) {
  // HEADER and SOURCE are completely independent (though we might want to
  // change that later). They suppress duplicates.
  // However, we do not understand comments.
  EXPECT_THAT(code_info_.GetIncludes(HEADER), IsEmpty());
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), IsEmpty());
  code_info_.AddInclude(HEADER, "a");
  code_info_.AddInclude(HEADER, "a");
  EXPECT_THAT(code_info_.GetIncludes(HEADER), ElementsAre("\"a\""));
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), IsEmpty());
  code_info_.AddInclude(HEADER, "\"a\"");
  code_info_.AddInclude(HEADER, "\"b\"");
  EXPECT_THAT(code_info_.GetIncludes(HEADER), ElementsAre("\"a\"", "\"b\""));
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), IsEmpty());
  code_info_.AddInclude(SOURCE, "a");
  code_info_.AddInclude(SOURCE, "c");
  EXPECT_THAT(code_info_.GetIncludes(HEADER), ElementsAre("\"a\"", "\"b\""));
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), ElementsAre("\"a\"", "\"c\""));
  code_info_.AddInclude(HEADER, "<a>");
  EXPECT_THAT(code_info_.GetIncludes(HEADER),
              ElementsAre("\"a\"", "\"b\"", "<a>"));
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), ElementsAre("\"a\"", "\"c\""));
  code_info_.AddInclude(HEADER, "<a>  // A");
  code_info_.AddInclude(HEADER, "<a>  // A");
  code_info_.AddInclude(HEADER, "\"a\"  // A");
  code_info_.AddInclude(HEADER, "\"a\"  // A");
  EXPECT_THAT(code_info_.GetIncludes(HEADER),
              ElementsAre("\"a\"", "\"a\"  // A", "\"b\"", "<a>", "<a>  // A"));
  EXPECT_THAT(code_info_.GetIncludes(SOURCE), ElementsAre("\"a\"", "\"c\""));
}

TEST_F(CodeInfoCollectorTest, AddInclude_Descriptor) {
  EXPECT_THAT(code_info_.GetIncludes(HEADER), IsEmpty());
  code_info_.AddInclude(HEADER, *MessageBuilderOptions::descriptor());
  EXPECT_THAT(code_info_.GetIncludes(HEADER),
              ElementsAre("\"proto_builder/"
                          "proto_builder.pb.h\"  // IWYU pragma: export"));
}

TEST_F(CodeInfoCollectorTest, AddInclude_EnumDescriptor) {
  EXPECT_THAT(code_info_.GetIncludes(HEADER), IsEmpty());
  code_info_.AddInclude(HEADER, *FieldBuilderOptions::OutputMode_descriptor());
  EXPECT_THAT(code_info_.GetIncludes(HEADER),
              ElementsAre("\"proto_builder/"
                          "proto_builder.pb.h\"  // IWYU pragma: export"));
}

TEST_F(CodeInfoCollectorTest, RelativeType) {
  const CodeInfoCollector foo_bar({"foo", "bar"});
  struct tests {
    const char* type;
    const char* relative;
  } static const kTests[] = {
      {"int", "int"},
      {"::foo::bar::Baz", "Baz"},
      {"::foo::bar::bla::Baz", "bla::Baz"},
      {"::foo::barsome::Baz", "::foo::barsome::Baz"},
      {"::foo::bar", "::foo::bar"},
      {"::foo::bar::", "::foo::bar::"},
      {"::foo::Baz", "::foo::Baz"},
      {"foo::bar::Baz", "foo::bar::Baz"},
      {"foo::Baz", "foo::Baz"},
  };
  for (const auto& test : kTests) {
    SCOPED_TRACE(
        absl::StrCat("In: '", test.type, "' out: '", test.relative, "'"));
    EXPECT_THAT(code_info_.RelativeType(test.type), test.type);
    EXPECT_THAT(foo_bar.RelativeType(test.type), test.relative);
  }
}

class BuilderWriterTest : public ::testing::Test {
 protected:
  bool HasConsecutiveEmptyLines(Where where, const BufferWriter& writer) const {
    bool last_empty = false;  // Allow single empty line.
    for (const auto& l : writer.From(where)) {
      if (l.empty()) {
        if (last_empty) {
          return true;
        }
        last_empty = true;
      } else {
        last_empty = false;
      }
    }
    return false;
  }

  bool HasConsecutiveEmptyLines(const BufferWriter& writer) const {
    return HasConsecutiveEmptyLines(HEADER, writer) ||
           HasConsecutiveEmptyLines(SOURCE, writer);
  }

  WriterMock writer_;
};

TEST_F(BuilderWriterTest, BufferWriter_DoubleEmptyLines) {
  {
    BufferWriter writer;
    EXPECT_THAT(writer.From(HEADER), IsEmpty());
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), false);
    writer.Write(HEADER, "");
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), false);
    writer.Write(HEADER, "");
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), true);
    EXPECT_THAT(writer.From(HEADER), ElementsAre("", ""));
  }
  {
    BufferWriter writer;
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), false);
    writer.Write(HEADER, "");
    writer.Write(HEADER, "X");
    writer.Write(HEADER, "");
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), false);
    writer.Write(HEADER, "");
    EXPECT_THAT(HasConsecutiveEmptyLines(writer), true);
    EXPECT_THAT(writer.From(HEADER), ElementsAre("", "X", "", ""));
  }
}

TEST_F(BuilderWriterTest, BufferWriter_MoveContents) {
  BufferWriter writer1;
  writer1.Write(SOURCE, "B");
  writer1.Write(HEADER, "11");
  writer1.Write(HEADER, "12");
  BufferWriter writer2;
  writer2.Write(HEADER, "21");
  EXPECT_THAT(writer1.From(SOURCE), ElementsAre("B"));
  EXPECT_THAT(writer1.From(HEADER), ElementsAre("11", "12"));
  EXPECT_THAT(writer2.From(SOURCE), IsEmpty());
  EXPECT_THAT(writer2.From(HEADER), ElementsAre("21"));
  writer1.MoveContents(HEADER, &writer2);
  EXPECT_THAT(writer1.From(SOURCE), ElementsAre("B"));
  EXPECT_THAT(writer1.From(HEADER), IsEmpty());
  EXPECT_THAT(writer2.From(SOURCE), IsEmpty());
  EXPECT_THAT(writer2.From(HEADER), ElementsAre("21", "11", "12"));
  writer1.Write(HEADER, "13");
  writer2.Write(HEADER, "22");
  EXPECT_THAT(writer1.From(SOURCE), ElementsAre("B"));
  EXPECT_THAT(writer1.From(HEADER), ElementsAre("13"));
  EXPECT_THAT(writer2.From(SOURCE), IsEmpty());
  EXPECT_THAT(writer2.From(HEADER), ElementsAre("21", "11", "12", "22"));
}

TEST_F(BuilderWriterTest, NoDoubleEmptyLines) {
  BufferWriter buffer_writer;
  NoDoubleEmptyLineWriter test_writer(&buffer_writer);
  EXPECT_THAT(buffer_writer.From(HEADER), IsEmpty());
  test_writer.Write(HEADER, "");
  EXPECT_THAT(buffer_writer.From(HEADER), IsEmpty())
      << "We do not allow to start with an empty lnie.";
  test_writer.Write(HEADER, "");
  test_writer.Write(HEADER, "");
  EXPECT_THAT(HasConsecutiveEmptyLines(buffer_writer), false);
  EXPECT_THAT(buffer_writer.From(HEADER), IsEmpty());
  test_writer.Write(HEADER, "X");
  test_writer.Write(HEADER, "");
  test_writer.Write(HEADER, "");
  EXPECT_THAT(HasConsecutiveEmptyLines(buffer_writer), false);
  EXPECT_THAT(buffer_writer.From(HEADER), ElementsAre("X", ""));
}

TEST_F(BuilderWriterTest, IndentWriter) {
  BufferWriter buffer_writer;
  IndentWriter test_writer(&buffer_writer);
  EXPECT_THAT(buffer_writer.From(HEADER), IsEmpty());
  EXPECT_THAT(buffer_writer.From(SOURCE), IsEmpty());
  test_writer.Write(HEADER, "1");
  test_writer.Write(SOURCE, "1");
  EXPECT_THAT(buffer_writer.From(HEADER), ElementsAre("1"));
  EXPECT_THAT(buffer_writer.From(SOURCE), ElementsAre("1"));
  test_writer.SetIndent(HEADER, ".");
  test_writer.SetIndent(SOURCE, " ");
  test_writer.Write(HEADER, "2");
  test_writer.Write(SOURCE, "2");
  test_writer.Write(HEADER, ".3");
  test_writer.Write(SOURCE, ".3");
  test_writer.Write(HEADER, " 4");
  test_writer.Write(SOURCE, " 4");
  test_writer.Write(HEADER, "");
  test_writer.Write(SOURCE, "");
  test_writer.SetIndent(HEADER, "");
  test_writer.SetIndent(SOURCE, "");
  test_writer.Write(HEADER, "6");
  test_writer.Write(SOURCE, "6");
  EXPECT_THAT(buffer_writer.From(HEADER),
              ElementsAre("1", ".2", "..3", ". 4", "", "6"));
  EXPECT_THAT(buffer_writer.From(SOURCE),
              ElementsAre("1", " 2", " .3", "  4", "", "6"));
}

}  // namespace
}  // namespace proto_builder
