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

#include "proto_builder/field_builder.h"

#include <array>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/parse_text_proto.h"
#include "proto_builder/proto_builder.pb.h"
#include "proto_builder/tests/source_location.pb.h"
#include "proto_builder/tests/test_message.pb.h"
#include "proto_builder/tests/test_types.pb.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace proto_builder {
namespace {
constexpr char kIncludeString[] = "<string>";
constexpr char kIncludeStringView[] = "absl/strings/string_view.h";
constexpr char kIncludeDuration[] = "absl/time/time.h";
constexpr char kIncludeTestMessage[] =
    "\"proto_builder/tests/test_message.pb.h\""
    "  // IWYU pragma: export";
constexpr char kIncludeTestTypes[] =
    "\"proto_builder/tests/test_types.pb.h\""
    "  // IWYU pragma: export";
constexpr char kIncludeSourceLocation[] =
    "proto_builder/oss/source_location.h";
}  // namespace

using ::proto_builder::oss::ParseTextProtoOrDie;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::IsEmpty;
using ::testing::Mock;
using ::testing::Not;
using ::testing::StrictMock;

class WriterMock : public BuilderWriter {
 public:
  WriterMock() : BuilderWriter() { ReplaceCodeInfoCollector(""); }
  MOCK_METHOD(void, Write, (Where, const std::string&), (override));
  MOCK_METHOD(void, AddInclude, (Where, absl::string_view));
  CodeInfoCollector* CodeInfo() final { return code_info_.get(); }
  const CodeInfoCollector* CodeInfo() const final { return code_info_.get(); }
  void ReplaceCodeInfoCollector(absl::string_view cpp_type) {
    code_info_ = absl::make_unique<TestCodeInfo>(
        this, absl::StrSplit(cpp_type, "::", absl::SkipEmpty()));
  }

 private:
  // In order to verify AddInclude() calls, we should make this a Mock class.
  // However we also need to be able to replace the collector for some tests.
  // Thus we do not make the collector a public base but instead provide this
  // derived class that forwards the AddInclude calls to the MockWriter. This
  // way the test call sites stay simple.
  class TestCodeInfo : public CodeInfoCollector {
   public:
    explicit TestCodeInfo(WriterMock* owner,
                          const std::vector<std::string>& package)
        : CodeInfoCollector(package), owner_(owner) {}
    void AddInclude(Where where, absl::string_view include) override {
      owner_->AddInclude(where, include);
    }

   private:
    WriterMock* const owner_;
  };

  std::unique_ptr<TestCodeInfo> code_info_;
};

class FieldBuilderTest : public ::testing::Test {
 protected:
  template <class Msg>
  const FieldDescriptor& FindFieldByName(absl::string_view name) const {
    return *PBCC_DIE_IF_NULL(Msg::descriptor()->FindFieldByName(std::string(name)));
  }

  const FieldBuilder Builder(const google::protobuf::FieldDescriptor& field_descriptor,
                             const FieldBuilderOptions& options) {
    auto field_options = GetFieldBuilderOptionsOrDefault(field_descriptor);
    field_options.MergeFrom(options);
    return FieldBuilder({global_config_, &writer_, field_options,
                         field_descriptor, "my_type", "data_.", "my_parent"});
  }

  template <class Msg>
  const FieldBuilder Builder(absl::string_view field_name,
                             const FieldBuilderOptions& options) {
    return Builder(FindFieldByName<Msg>(field_name), options);
  }

  template <class Msg>
  void WriteField(absl::string_view field_name, absl::string_view options) {
    Builder<Msg>(field_name, ParseTextProtoOrDie(options)).WriteField();
  }

  template <class Msg>
  void WriteField(absl::string_view field_name,
                  FieldBuilderOptions::OutputMode out) {
    FieldBuilderOptions options;
    options.set_output(out);
    Builder<Msg>(field_name, options).WriteField();
  }

  template <class... Line>
  void Expect(Where to, int times, const Line&... line) {
    if (times) {
      // First ignore empty lines.
      EXPECT_CALL(writer_, Write(_, "")).Times(AnyNumber());
      const std::string lines[sizeof...(line)] = {std::string(line)...};
      for (const auto& l : lines) {
        EXPECT_CALL(writer_, Write(to, l)).Times(times);
      }
    } else {
      EXPECT_CALL(writer_, Write(to, Not(IsEmpty()))).Times(0);
    }
  }

  template <Where to, class... Line>
  void Expect(int times, const Line&... line) {
    Expect(to, times, line...);
  }

  template <Where to, class... Line>
  void Expect(const Line&... line) {
    Expect(to, 1, line...);
  }

  template <Where to>
  void ExpectNo() {
    EXPECT_CALL(writer_, Write(to, _)).Times(0);
  }

  void GroupOptionsTest();
  void GetTypeTest();
  void ValidateOrWriteErrorTest();
  void GetRelativeFieldTypeTest();
  void LookupAndConversionTest();

  const ProtoBuilderConfigManager global_config_;
  StrictMock<WriterMock> writer_;
};

// Helpers for GetRelativeFieldTypeTest. This function is required for OSS
// support.
// Prepends the C++ namespace delimeter '::', if the str contains a C++
// namespace delimeter '::', but does not start with the delimeter.
std::string GetRelativeFieldTypeTestHelper(absl::string_view str) {
  if (str.size() > 2 && absl::StrContains(str, "::") &&
      str.substr(0, 2) != "::") {
    return "::" + std::string(str);
  }
  return std::string(str);
}

TEST_F(FieldBuilderTest, GroupOptions) { GroupOptionsTest(); }

void FieldBuilderTest::GroupOptionsTest() {
  const FieldBuilder builder = Builder<TestTypes::Optional>("fieldgroup", {});
  EXPECT_THAT(builder.IsValidOrWriteError(), true);
}

TEST_F(FieldBuilderTest, GetType) { GetTypeTest(); }

void FieldBuilderTest::GetTypeTest() {
  struct {
    absl::string_view field;
    const char* type;
    bool decorate;
    absl::string_view include;
  } constexpr kTests[] = {
      // clang-format off
    {"field_double", "double", false},
    {"field_float", "float", false},
    {"field_int64", "int64_t", false},
    {"field_uint64", "uint64_t", false},
    {"field_int32", "int32_t", false},
    {"field_fixed64", "uint64_t", false},
    {"field_fixed32", "uint32_t", false},
    {"field_bool", "bool", false},
    {"field_string", "std::string", true, kIncludeString},
    {"fieldgroup",
     "::proto_builder::TestTypes::Optional::FieldGroup",
     true, kIncludeTestTypes},
    {"field_message", "::proto_builder::TestTypes::SubMsg",
     true, kIncludeTestTypes},
    {"field_bytes", "std::string", true, kIncludeString},
    {"field_uint32", "uint32_t", false},
    {"field_enum", "::proto_builder::TestTypes::Enum", false,
     kIncludeTestTypes},
    {"field_sfixed32", "int32_t", false},
    {"field_sfixed64", "int64_t", false},
    {"field_sint32", "int32_t", false},
    {"field_sint64", "int64_t", false},
  };  // clang-format on
  for (auto t : kTests) {
    SCOPED_TRACE(absl::StrCat("Test: ", t.field, " -> ", t.type));
    const FieldBuilder builder = Builder<TestTypes::Optional>(t.field, {});
    EXPECT_THAT(GetFieldType(builder.data_.field), t.type);
    EXPECT_THAT(builder.GetRawCppType(), t.type);
    EXPECT_THAT(builder.ParameterType(true),
                t.decorate ? absl::StrCat("const ", t.type, "&") : t.type)
        << "Of all possible field types only messages get decorated.";
    // Check incldues.
    if (t.include.empty()) {
      EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
    } else {
      EXPECT_CALL(writer_, AddInclude(HEADER, t.include)).Times(1);
      EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
    }
    builder.AddIncludes();
    Mock::VerifyAndClearExpectations(&writer_);
  }
}

TEST_F(FieldBuilderTest, LookupAndConversion) { LookupAndConversionTest(); }

void FieldBuilderTest::LookupAndConversionTest() {
  struct {
    int lineno;
    const char* field;
    const char* options;
    const char* get_cpp_type;
    const char* parameter_type;
    const char* parameter_type_decorated;
    const char* get_method_param;
    const char* set_value;
    absl::string_view include;
  } constexpr kTests[]{
      // clang-format off
    {__LINE__, "one", "",
     "int32_t", "int32_t", "int32_t",
     "int32_t value", "value"},
    {__LINE__, "one", "type: '@ToInt64Seconds'",
     "absl::Duration", "absl::Duration", "absl::Duration",
     "absl::Duration value", "absl::ToInt64Seconds(value)",
     kIncludeDuration},
    {__LINE__, "one", "type: '@ToInt64Milliseconds'",
     "absl::Duration", "absl::Duration", "absl::Duration",
     "absl::Duration value", "absl::ToInt64Milliseconds(value)",
     kIncludeDuration},
    {__LINE__, "one", "type: '@ToDoubleSeconds'",
     "absl::Duration", "absl::Duration", "absl::Duration",
     "absl::Duration value", "absl::ToDoubleSeconds(value)",
     kIncludeDuration},
    {__LINE__, "one", "type: '@ToDoubleMilliseconds'",
     "absl::Duration", "absl::Duration", "absl::Duration",
     "absl::Duration value", "absl::ToDoubleMilliseconds(value)",
     kIncludeDuration},
    {__LINE__, "one", "output: TEMPLATE",
     "int32_t", "Value", "const Value&",
     "const Value& value", "value"},
    {__LINE__, "one", "output: FOREACH",
     "int32_t", "Container", "const Container&",
     "const Container& values", "v"},
    {__LINE__, "one", "output: FOREACH_ADD",
     "int32_t", "Container", "const Container&",
     "const Container& values", "v"},
    {__LINE__, "one", "output: INITIALIZER_LIST", "int32_t",
     "std::initializer_list<Item>", "std::initializer_list<Item>",
     "std::initializer_list<Item> values", "v"},
    {__LINE__, "one", "output: TEMPLATE type: '@ToInt64Seconds'",
     "absl::Duration", "Value", "const Value&",
     "const Value& value", "absl::ToInt64Seconds(value)",
     kIncludeDuration},
    {__LINE__, "one", "output: FOREACH type: '@ToInt64Seconds'",
     "absl::Duration", "Container", "const Container&",
     "const Container& values", "absl::ToInt64Seconds(v)",
     kIncludeDuration},
    {__LINE__, "one", "output: FOREACH_ADD type: '@ToInt64Seconds'",
     "absl::Duration", "Container", "const Container&",
     "const Container& values", "absl::ToInt64Seconds(v)",
     kIncludeDuration},
    {__LINE__, "one", "output: INITIALIZER_LIST type: '@ToInt64Seconds'",
     "absl::Duration", "std::initializer_list<Item>",
     "std::initializer_list<Item>",
     "std::initializer_list<Item> values", "absl::ToInt64Seconds(v)",
     kIncludeDuration},
    {__LINE__, "one", "value: '42'",
     "int32_t", "int32_t", "int32_t",
     "", "42"},
    {__LINE__, "one", "value: '42' type: '@ToInt64Seconds'",
     "absl::Duration", "absl::Duration", "absl::Duration",
     "", "absl::ToInt64Seconds(42)",
     kIncludeDuration},
    {__LINE__, "one", "value: '42' type: '@ToInt64Seconds' output: FOREACH",
     "absl::Duration", "Container", "const Container&",
     "", "absl::ToInt64Seconds(42)",
     kIncludeDuration},
    {__LINE__, "one", "value: '42' type: '@ToInt64Seconds' output: FOREACH_ADD",
     "absl::Duration", "Container", "const Container&",
     "", "absl::ToInt64Seconds(42)",
     kIncludeDuration},
    {__LINE__, "one",
     "value: '42' type: '@ToInt64Seconds' output: INITIALIZER_LIST",
     "absl::Duration", "std::initializer_list<Item>",
     "std::initializer_list<Item>",
     "", "absl::ToInt64Seconds(42)",
     kIncludeDuration},
  };  // clang-format on
  for (auto t : kTests) {
    SCOPED_TRACE(absl::StrCat("Test: ", t.field, ": ", t.options, " At:\n",
                              __FILE__, ":", t.lineno));
    auto builder =
        Builder<TestMessage>(t.field, ParseTextProtoOrDie(t.options));
    EXPECT_THAT(builder.GetRawCppType(), t.get_cpp_type);
    EXPECT_THAT(builder.ParameterType(false), t.parameter_type);
    EXPECT_THAT(builder.ParameterType(true), t.parameter_type_decorated);
    EXPECT_THAT(builder.MethodParam(HEADER), t.get_method_param);
    EXPECT_THAT(builder.SetValue(), t.set_value);
    // Check includes.
    if (t.include.empty()) {
      EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
    } else {
      EXPECT_CALL(writer_, AddInclude(HEADER, t.include)).Times(1);
      EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
    }
    builder.AddIncludes();
    Mock::VerifyAndClearExpectations(&writer_);
  }
}

TEST_F(FieldBuilderTest, GetRelativeFieldType) { GetRelativeFieldTypeTest(); }

void FieldBuilderTest::GetRelativeFieldTypeTest() {
  struct {
    int lineno;
    const char* field;
    const char* options;
    const char* field_type;
    const char* parameter_type;
    const char* relative_field_type;
    const char* relative_parameter_type;
  } constexpr kTests[]{
      // clang-format off
    {__LINE__, "field_enum", "",
     "::proto_builder::TestTypes::Enum",
     "::proto_builder::TestTypes::Enum",
     "proto_builder::TestTypes::Enum",
     "proto_builder::TestTypes::Enum"},
    {__LINE__, "field_message", "",
     "::proto_builder::TestTypes::SubMsg",
     "::proto_builder::TestTypes::SubMsg",
     "proto_builder::TestTypes::SubMsg",
     "proto_builder::TestTypes::SubMsg"},
    {__LINE__, "field_int64", "type: '::proto_builder::Test'",
     "int64_t",
     "::proto_builder::Test",
     "int64_t",
     "proto_builder::Test"},
      // clang-format on
  };
  for (auto t : kTests) {
    SCOPED_TRACE(absl::StrCat("Test: ", t.field, ": ", t.options, " At:\n",
                              __FILE__, ":", t.lineno));
    auto builder =
        Builder<TestTypes::Optional>(t.field, ParseTextProtoOrDie(t.options));
    EXPECT_THAT(GetRelativeFieldTypeTestHelper(builder.GetRelativeFieldType()),
                GetRelativeFieldTypeTestHelper(t.field_type));
    EXPECT_THAT(GetRelativeFieldTypeTestHelper(builder.ParameterType(false)),
                GetRelativeFieldTypeTestHelper(t.parameter_type));
  }
  writer_.ReplaceCodeInfoCollector("::proto2::contrib");
  for (auto t : kTests) {
    SCOPED_TRACE(absl::StrCat("Test: ", t.field, ": ", t.options, " At:\n",
                              __FILE__, ":", t.lineno));
    auto builder =
        Builder<TestTypes::Optional>(t.field, ParseTextProtoOrDie(t.options));
    EXPECT_THAT(GetRelativeFieldTypeTestHelper(builder.GetRelativeFieldType()),
                GetRelativeFieldTypeTestHelper(t.relative_field_type));
    EXPECT_THAT(GetRelativeFieldTypeTestHelper(builder.ParameterType(false)),
                GetRelativeFieldTypeTestHelper(t.relative_parameter_type));
  }
}

TEST_F(FieldBuilderTest, ValidateOrWriteError) { ValidateOrWriteErrorTest(); }

void FieldBuilderTest::ValidateOrWriteErrorTest() {
  struct {
    const char* field;
    const char* options;
    bool is_ok;
  } constexpr kTests[]{
      {"one", "value: ''", true},
      {"two", "value: ''", true},
      {"two", "output: TEMPLATE", true},
      {"two", "output: FOREACH", true},
      {"two", "output: FOREACH_ADD", true},
      {"two", "output: INITIALIZER_LIST", true},
      {"two", "value: '' output: TEMPLATE", false},
      {"two", "value: '' output: FOREACH", false},
      {"two", "value: '' output: FOREACH_ADD", false},
      {"one", "output: TEMPLATE", true},
      {"one", "value: '' output: TEMPLATE", false},
      {"one", "output: FOREACH", false},
      {"one", "output: FOREACH_ADD", false},
      {"one", "output: INITIALIZER_LIST", false},
  };
  int num_errors = 0;
  for (auto t : kTests) {
    if (!t.is_ok) {
      ++num_errors;
    }
  }
  EXPECT_CALL(writer_, Write(HEADER, IsEmpty())).Times(AnyNumber());
  EXPECT_CALL(writer_, Write(HEADER, Not(IsEmpty()))).Times(num_errors * 3);
  EXPECT_CALL(writer_, Write(SOURCE, _)).Times(0);
  for (auto t : kTests) {
    SCOPED_TRACE(absl::StrCat("Test: ", t.field, ": ", t.options));
    auto builder =
        Builder<TestMessage>(t.field, ParseTextProtoOrDie(t.options));
    EXPECT_THAT(builder.IsValidOrWriteError(), t.is_ok);
  }
}

TEST_F(FieldBuilderTest, WriteFieldBuilder) {
  struct {
    FieldBuilderOptions::OutputMode output;
    int header;
    int body;
  } constexpr kTests[]{{FieldBuilderOptions::SKIP, 0, 0},
                       {FieldBuilderOptions::HEADER, 1, 0},
                       {FieldBuilderOptions::SOURCE, 0, 1},
                       {FieldBuilderOptions::BOTH, 1, 1}};
  for (const auto& t : kTests) {
    SCOPED_TRACE(absl::StrCat("OutputMode: ",
                              FieldBuilderOptions::OutputMode_Name(t.output)));
    Expect<HEADER>(t.header, "my_type& Setmy_parentOne(int32_t value);");
    Expect<SOURCE>(  // clang-format off
        t.body,
        "my_type& my_type::Setmy_parentOne(int32_t value) {",
        "  data_.set_one(value);",
        "  return *this;",
        "}");  // clang-format on
    WriteField<TestMessage>("one", t.output);
  }
}

TEST_F(FieldBuilderTest, WriteFieldBuilderReservedKeywordAddsUnderscoreSuffix) {
  struct {
    FieldBuilderOptions::OutputMode output;
    int header;
    int body;
  } constexpr kTests[]{{FieldBuilderOptions::SKIP, 0, 0},
                       {FieldBuilderOptions::HEADER, 1, 0},
                       {FieldBuilderOptions::SOURCE, 0, 1},
                       {FieldBuilderOptions::BOTH, 1, 1}};
  for (const auto& t : kTests) {
    SCOPED_TRACE(absl::StrCat("OutputMode: ",
                              FieldBuilderOptions::OutputMode_Name(t.output)));
    Expect<HEADER>(t.header, "my_type& Setmy_parentNamespace(int32_t value);");
    Expect<SOURCE>(  // clang-format off
        t.body,
        "my_type& my_type::Setmy_parentNamespace(int32_t value) {",
        "  data_.set_namespace_(value);",
        "  return *this;",
        "}");  // clang-format on
    WriteField<TestMessage>("namespace", t.output);
  }
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>("my_type& Setmy_parentOne(absl::Duration value);");
  Expect<SOURCE>(  // clang-format off
      "my_type& my_type::Setmy_parentOne(absl::Duration value) {",
      "  data_.set_one(absl::ToInt64Seconds(value));",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("one", "type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated) {
  struct {
    FieldBuilderOptions::OutputMode output;
    int header;
    int body;
  } constexpr kTests[]{{FieldBuilderOptions::SKIP, 0, 0},
                       {FieldBuilderOptions::HEADER, 1, 0},
                       {FieldBuilderOptions::SOURCE, 0, 1},
                       {FieldBuilderOptions::BOTH, 1, 1}};
  for (const auto& t : kTests) {
    SCOPED_TRACE(absl::StrCat("OutputMode: ",
                              FieldBuilderOptions::OutputMode_Name(t.output)));
    Expect<HEADER>(t.header, "my_type& Addmy_parentTwo(int32_t value);");
    Expect<SOURCE>(  // clang-format off
        t.body,
        "my_type& my_type::Addmy_parentTwo(int32_t value) {",
        "  data_.add_two(value);",
        "  return *this;",
        "}");  // clang-format on
    WriteField<TestMessage>("two", t.output);
  }
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_RepeatedReservedKeyword) {
  struct {
    FieldBuilderOptions::OutputMode output;
    int header;
    int body;
  } constexpr kTests[]{{FieldBuilderOptions::SKIP, 0, 0},
                       {FieldBuilderOptions::HEADER, 1, 0},
                       {FieldBuilderOptions::SOURCE, 0, 1},
                       {FieldBuilderOptions::BOTH, 1, 1}};
  for (const auto& t : kTests) {
    SCOPED_TRACE(absl::StrCat("OutputMode: ",
                              FieldBuilderOptions::OutputMode_Name(t.output)));
    Expect<HEADER>(t.header, "my_type& Addmy_parentAnd(int32_t value);");
    Expect<SOURCE>(  // clang-format off
        t.body,
        "my_type& my_type::Addmy_parentAnd(int32_t value) {",
        "  data_.add_and_(value);",
        "  return *this;",
        "}");  // clang-format on
    WriteField<TestMessage>("and", t.output);
  }
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>("my_type& Addmy_parentTwo(absl::Duration value);");
  Expect<SOURCE>(  // clang-format off
      "my_type& my_type::Addmy_parentTwo(absl::Duration value) {",
      "  data_.add_two(absl::ToInt64Seconds(value));",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("two", "type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_Template) {
  EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Value>",
      "my_type& Addmy_parentTwo(const Value& value) {",
      "  data_.add_two(value);",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", FieldBuilderOptions::TEMPLATE);
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_Template_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Value>",
      "my_type& Addmy_parentTwo(const Value& value) {",
      "  data_.add_two(absl::ToInt64Seconds(value));",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", "output: TEMPLATE type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_Foreach) {
  EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Container, class = typename "
      "std::enable_if<!std::is_convertible<Container, int32_t>::value>::type>",
      "my_type& Addmy_parentTwo(const Container& values) {",
      "  for (const auto& v : values) {",
      "    data_.add_two(v);",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", FieldBuilderOptions::FOREACH);
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_Foreach_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Container, class = typename "
      "std::enable_if<!std::is_convertible<Container, int32_t>::value>::type>",
      "my_type& Addmy_parentTwo(const Container& values) {",
      "  for (const auto& v : values) {",
      "    data_.add_two(absl::ToInt64Seconds(v));",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", "output: FOREACH type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_ForeachAdd) {
  EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Container, class = typename "
      "std::enable_if<!std::is_convertible<Container, int32_t>::value>::type>",
      "my_type& Addmy_parentTwo(const Container& values) {",
      "  for (const auto& v : values) {",
      "    Addmy_parentTwo(v);",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", FieldBuilderOptions::FOREACH_ADD);
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_ForeachAdd_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Container, class = typename "
      "std::enable_if<!std::is_convertible<Container, int32_t>::value>::type>",
      "my_type& Addmy_parentTwo(const Container& values) {",
      "  for (const auto& v : values) {",
      "    Addmy_parentTwo(absl::ToInt64Seconds(v));",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", "output: FOREACH_ADD type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_Repeated_InitializerList) {
  EXPECT_CALL(writer_, AddInclude(_, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Item>",
      "my_type& Addmy_parentTwo(std::initializer_list<Item> values) {",
      "  for (const auto& v : values) {",
      "    Addmy_parentTwo(v);",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two", FieldBuilderOptions::INITIALIZER_LIST);
}

TEST_F(FieldBuilderTest,
       WriteFieldBuilder_Repeated_InitializerList_Conversion) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeDuration)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Item>",
      "my_type& Addmy_parentTwo(std::initializer_list<Item> values) {",
      "  for (const auto& v : values) {",
      "    Addmy_parentTwo(absl::ToInt64Seconds(v));",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  ExpectNo<SOURCE>();
  WriteField<TestMessage>("two",
                          "output: INITIALIZER_LIST type: '@ToInt64Seconds'");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_NonRepeatedMessage) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeTestMessage)).Times(1);
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "my_type& Setmy_parentThree(const ::proto_builder::TestMessage::Sub& value);");  // NOLINT
  Expect<SOURCE>(
      "my_type& my_type::Setmy_parentThree(const ::proto_builder::TestMessage::Sub& value) {",  // NOLINT
      "  *data_.mutable_three() = value;",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("three", "");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_StringWithAnnotationString) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeString)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "my_type& Setmy_parentString22(const std::string& value);");  // NOLINT
  Expect<SOURCE>(
      "my_type& my_type::Setmy_parentString22(const std::string& value) {",  // NOLINT
      "  data_.set_string22(value);",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("string22", "");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_StringWithAnnotationStringView) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeString)).Times(AtMost(1));
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeStringView)).Times(1);
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "my_type& Setmy_parentString24(absl::string_view value);");  // NOLINT
  Expect<SOURCE>(
      "my_type& my_type::Setmy_parentString24(absl::string_view value) {",  // NOLINT
      "  data_.set_string24(std::string(value));",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("string24", "");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_BytesWithAnnotationString) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeString)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "my_type& Setmy_parentBytes26(const std::string& value);");  // NOLINT
  Expect<SOURCE>(
      "my_type& my_type::Setmy_parentBytes26(const std::string& value) {",  // NOLINT
      "  data_.set_bytes26(value);",
      "  return *this;",
      "}");  // clang-format on
  WriteField<TestMessage>("bytes26", "");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_SourceLocation_Single) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeString)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeSourceLocation))
      .Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "my_type& Addmy_parentTarget(const std::string& value, proto_builder::oss::SourceLocation source_location = proto_builder::oss::SourceLocation::current());");  // NOLINT
  Expect<SOURCE>(
      "my_type& my_type::Addmy_parentTarget(const std::string& value, proto_builder::oss::SourceLocation source_location) {",  // NOLINT
      "  data_.add_target(value);",
      "  return *this;",
      "}");  // clang-format on
  WriteField<tests::SourceLocation>(
      "target",
      R"pb(
        output: BOTH name: "Target" add_source_location: true
      )pb");
}

TEST_F(FieldBuilderTest, WriteFieldBuilder_SourceLocation_InitializerList) {
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeString)).Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(HEADER, kIncludeSourceLocation))
      .Times(AtLeast(1));
  EXPECT_CALL(writer_, AddInclude(SOURCE, _)).Times(0);
  Expect<HEADER>(  // clang-format off
      "template <class Item>",
      "my_type& Addmy_parentTargets(std::initializer_list<Item> values, proto_builder::oss::SourceLocation source_location = proto_builder::oss::SourceLocation::current()) {",  // NOLINT
      "  for (const auto& v : values) {",
      "    Addmy_parentTarget(v, source_location);",
      "  }",
      "  return *this;",
      "}");  // clang-format on
  Expect<SOURCE>();
  WriteField<tests::SourceLocation>(
      "target",
      R"pb(
        output: INITIALIZER_LIST name: "Targets" add_source_location: true
      )pb");
}

}  // namespace proto_builder
