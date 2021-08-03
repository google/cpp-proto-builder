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

#include "proto_builder/proto_builder_config.h"

#include <set>
#include <string>

#include "proto_builder/field_builder.h"
#include "proto_builder/oss/parse_text_proto.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/algorithm/container.h"
#include "absl/strings/match.h"

namespace proto_builder {
namespace {

using ::proto_builder::oss::ParseTextProtoOrDie;
using ::testing::oss::EqualsProto;
using ::testing::Ge;
using ::testing::IsNull;
using ::testing::IsSupersetOf;
using ::testing::Key;
using ::testing::Pointee;
using ::testing::SizeIs;
using ::testing::oss::Partially;

class ProtoBuilderConfigTest : public ::testing::Test {
 public:
  ProtoBuilderConfigTest() : config_manager_() {}

  template <typename T>
  static std::set<typename T::key_type> KeySet(const T& container) {
    std::set<typename T::key_type> result;
    absl::c_for_each(container, [&](const typename T::value_type& t) {
      result.emplace(t.first);
    });
    return result;
  }

  static std::set<std::string> GetBuiltInTypeNames() {
    return {
        "string",
        "bytes",
        "@absl::string_view",
        "@Map:absl::string_view",
        "@TextProto",
        "@TextProto:absl::string_view",
        "@TextProto:Map:Value:absl::string_view",
        "@ToInt64Seconds",
        "@ToInt64Milliseconds",
        "@ToDoubleSeconds",
        "@ToDoubleMilliseconds",
        "@ToProtoDuration",
        "@ToProtoTimestamp",
        "%SourceLocation",
        "%Status",
        "%StatusOr",
        "%Validate",
        "%LogSourceLocation",
    };
  }

  const ProtoBuilderConfigManager config_manager_;
};

TEST_F(ProtoBuilderConfigTest, VerifyTypeEntry) {
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("type: '@'")),
               "May not use '@' \\(beyond '@type@'\\) in type:");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("name: ''")),
               "May not provide 'name':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("name: 'a'")),
               "May not provide 'name':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("decorated_type: 'a'")),
               "May not use 'decorated_type' without 'type':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("value: '@type@'")),
               "May not use '@type@' in 'value':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("value: '@value@'")),
               "May not use '@value@' in 'value':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("include: ''")),
               "May not use empty 'include':");
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("include: '\\n'")),
               "May not use new-line in 'include', use multiple includes:");
  EXPECT_DEATH(VerifyTypeEntry("@", {}),
               "Type names \\(key\\) starting with '@' or '%' are reserved for "
               "internal use:");
  EXPECT_DEATH(VerifyTypeEntry("%", {}),
               "Type names \\(key\\) starting with '@' or '%' are reserved for "
               "internal use:");
  EXPECT_DEATH(VerifyTypeEntry("", {}), "Must specify a non-empty 'key':");
  EXPECT_DEATH(VerifyTypeEntry("$", {}), "Custom keys must start with '\\$'");
  EXPECT_DEATH(VerifyTypeEntry("$_", {}), "Custom keys must start with '\\$'");
  EXPECT_DEATH(VerifyTypeEntry("$0", {}), "Custom keys must start with '\\$'");
  EXPECT_DEATH(VerifyTypeEntry("$_a", {}), "Custom keys must start with '\\$'");
  EXPECT_DEATH(VerifyTypeEntry("$ a", {}), "Custom keys must start with '\\$'");
  EXPECT_DEATH(VerifyTypeEntry("$a ", {}), "Custom keys must start with '\\$'");
  EXPECT_TRUE(VerifyTypeEntry("$a", {}));
  EXPECT_TRUE(VerifyTypeEntry("$a0", {}));
  EXPECT_TRUE(VerifyTypeEntry("$a_", {}));
  EXPECT_DEATH(VerifyTypeEntry("", ParseTextProtoOrDie("macro: ''")),
               "The `macro` field can only be used for field annotations:");
}

TEST_F(ProtoBuilderConfigTest, DuplicateTypeName) {
  EXPECT_DEATH(  // Verify we spot the duplicate key name.
      VerifyProtoBuilderConfig(
          R"pb(
            type_map {
              key: "bad"
              value { type: "1" }
            }
            type_map {
              key: "duplicate"
              value { type: "2" }
            }
            type_map {
              key: "duplicate"
              value { type: "3" }
            }
            type_map {
              key: "bad"
              value { type: "4" }
            }
          )pb"),
      R"(Configuration contains duplicate key\(s\): "bad", "duplicate")");
}

TEST_F(ProtoBuilderConfigTest, RequiredTypesPresent) {
  // Check that the default configurations have all known built-in types.
  // This restriction does not apply to user provided configurations.
  EXPECT_THAT(config_manager_.GetProtoBuilderConfig().type_map(),
              SizeIs(Ge(GetBuiltInTypeNames().size())));
  EXPECT_THAT(KeySet(config_manager_.GetProtoBuilderConfig().type_map()),
              IsSupersetOf(GetBuiltInTypeNames()));
}

TEST_F(ProtoBuilderConfigTest, GetTypeInfo) {
  ASSERT_THAT(config_manager_.GetTypeInfo("Does not exist"), IsNull());
  ASSERT_DEATH(
      config_manager_.GetTypeInfo("%Bla"),
      R"(Check failed: \(type == ProtoBuilderTypeInfo::kSpecial\) == absl::StartsWith\(raw_type, "%"\).*Raw type: '%Bla')");
  ASSERT_THAT(
      config_manager_.GetTypeInfo("%Bla", ProtoBuilderTypeInfo::kSpecial),
      IsNull());
  for (const auto& entry : config_manager_.GetProtoBuilderConfig().type_map()) {
    ProtoBuilderTypeInfo type = absl::StartsWith(entry.first, "%")
                                    ? ProtoBuilderTypeInfo::kSpecial
                                    : ProtoBuilderTypeInfo::kParameter;
    EXPECT_THAT(config_manager_.GetTypeInfo(entry.first, type),
                Pointee(EqualsProto(entry.second)));
  }
}

TEST_F(ProtoBuilderConfigTest, GetBuiltinSpecialTypes) {
  EXPECT_THAT(
      config_manager_.GetSpecialTypes(),
      ElementsAre(
          Key("%LogSourceLocation"),
          Pair("%SourceLocation", Pointee(Partially(EqualsProto(
                                      R"pb(type: "proto_builder::oss::SourceLocation")pb")))),
          Pair("%Status",
               Pointee(Partially(EqualsProto(R"pb(type: "absl::Status")pb")))),
          Pair("%StatusOr", Pointee(Partially(
                                EqualsProto(R"pb(type: "absl::StatusOr")pb")))),
          Key("%Validate")));
}

TEST_F(ProtoBuilderConfigTest, CamelCaseToSnakeCase) {
  EXPECT_THAT(CamelCaseToSnakeCase(""), "");
  EXPECT_THAT(CamelCaseToSnakeCase("_"), "_");
  EXPECT_THAT(CamelCaseToSnakeCase("a"), "a");
  EXPECT_THAT(CamelCaseToSnakeCase("ABC"), "abc");
  EXPECT_THAT(CamelCaseToSnakeCase("AbCd"), "ab_cd");
  EXPECT_THAT(CamelCaseToSnakeCase("_ABcDE_"), "_abc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_A_Bc_DE_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_A_bc_De_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_a_Bc_de_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_A__Bc__DE_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_A__bc__De_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("_a__Bc__de_"), "_a_bc_de_");
  EXPECT_THAT(CamelCaseToSnakeCase("__"), "_");
  EXPECT_THAT(CamelCaseToSnakeCase("___"), "_");
  EXPECT_THAT(CamelCaseToSnakeCase("__A__"), "_a_");
  EXPECT_THAT(CamelCaseToSnakeCase("my::Type"), "my_type");
  EXPECT_THAT(CamelCaseToSnakeCase("my::smallType"), "my_small_type");
  EXPECT_THAT(CamelCaseToSnakeCase("::my::Type"), "my_type");
  EXPECT_THAT(CamelCaseToSnakeCase("::my::type"), "my_type");
  EXPECT_THAT(CamelCaseToSnakeCase("::_my::_Type"), "_my_type");
  EXPECT_THAT(CamelCaseToSnakeCase("::_my::_type"), "_my_type");
  EXPECT_THAT(CamelCaseToSnakeCase("::_"), "_");
  EXPECT_THAT(CamelCaseToSnakeCase("::"), "");
  EXPECT_THAT(CamelCaseToSnakeCase("zero0sum"), "zero0sum");
  EXPECT_THAT(CamelCaseToSnakeCase("zero0Sum"), "zero0_sum");
}

TEST_F(ProtoBuilderConfigTest, ExpandSpecialTypes) {
  EXPECT_THAT(config_manager_.GetExpandedType("%DoesNotExist"), "");
  EXPECT_THAT(config_manager_.GetExpandedType("%LogSourceLocation%param"),
              "log_source_location");
  EXPECT_THAT(config_manager_.GetExpandedType("%Status%value"),
              "absl::OkStatus()");
  EXPECT_THAT(config_manager_.GetExpandedType("%Status+param"),
              "absl::Status status");
  EXPECT_THAT(config_manager_.GetExpandedType("%Status+param=value"),
              "absl::Status status = absl::OkStatus()");
}

TEST_F(ProtoBuilderConfigTest, MergeFieldBuilderOptions) {
  EXPECT_THAT(config_manager_.MergeFieldBuilderOptions({}), EqualsProto(""));
  EXPECT_THAT(config_manager_.MergeFieldBuilderOptions(
                  ParseTextProtoOrDie("macro: 'foo'")),
              EqualsProto("macro: 'foo'"));
  const ProtoBuilderConfigManager config =
      config_manager_.Update(ParseTextProtoOrDie(R"pb(
        type_map {
          key: "foo"
          value {
            predicate: "Test"
            data { key: "1" value: "11" }
            data { key: "2" value: "22" }
          }
        })pb"));
  EXPECT_THAT(  // Plain 'lookup'
      config.MergeFieldBuilderOptions(ParseTextProtoOrDie("macro: 'foo'")),
      EqualsProto(R"pb(
        macro: "foo"
        predicate: "Test"
        data { key: "1" value: "11" }
        data { key: "2" value: "22" }
      )pb"));
  EXPECT_THAT(  // Reference and update/override
      config.MergeFieldBuilderOptions(ParseTextProtoOrDie(
          R"pb(
            macro: "foo"
            data { key: "2" value: "updated" }
            data { key: "3" value: "33" }
          )pb")),
      EqualsProto(R"pb(
        macro: "foo"
        predicate: "Test"
        data { key: "1" value: "11" }
        data { key: "2" value: "updated" }
        data { key: "3" value: "33" }
      )pb"));
  EXPECT_DEATH(  // The keys are checked, for starters they cannot be empty.
      config.Update(ParseTextProtoOrDie(
          R"pb(
            type_map {
              key: ""
              value {}
            }
          )pb")),
      "Check failed: !key.empty\\(\\)[[:space:]]+Must specify a non-empty "
      "'key'");
  EXPECT_DEATH(  // The values are checked, in particular the `macro` field.
      config.Update(ParseTextProtoOrDie(
          R"pb(
            type_map {
              key: "foo"
              value { macro: "foo" }
            }
          )pb")),
      "Check failed: !options.has_macro\\(\\)[[:space:]]+The `macro` field can "
      "only be used for field annotations:");
  EXPECT_DEATH(  // The values are checked, in particular the `macro` field.
      config.Update(ParseTextProtoOrDie(
          R"pb(
            type_map {
              key: "%Status"
              value {}
            }
          )pb")),
      "Cannot update configuration of builtin types:");
  EXPECT_THAT(  // Types `bytes` and `string` can be updated.
      config
          .Update(ParseTextProtoOrDie(
              R"pb(
                type_map {
                  key: "bytes"
                  value { type: "XXX" }
                }
              )pb"))
          .GetProtoBuilderConfig(),
      Partially(EqualsProto(R"pb(
        type_map {
          key: "bytes"
          value { type: "XXX" }
        }
      )pb")));
}

}  // namespace
}  // namespace proto_builder
