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

#include "proto_builder/oss/template_dictionary.h"

#include <string>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/strings/string_view.h"

namespace proto_builder::oss {
namespace {

using ::testing::Key;
using ::testing::Pair;

class TemplateDictionaryTest : public ::testing::Test {
 public:
  static std::pair<bool, std::string> Expand(const TemplateDictionary& dict,
                                             absl::string_view input) {
    std::string output(input);
    bool result = dict.Expand(&output);
    return {result, output};
  }
};

TEST_F(TemplateDictionaryTest, Replace) {
  const absl::string_view tmpl = R"(
    {{foo}}
    {{foo}}
    {{bar}}{{bar}}
    {{baz}}
    {{foo}})";
  TemplateDictionary dict("blabla");
  dict.SetValue("foo", "foo");
  dict.SetValue("bar", "bar");
  dict.SetValue("grr", "grr");  // Does not need to be present.
  // No 'baz'
  const absl::string_view expected = R"(
    foo
    foo
    barbar
    {{baz}}
    foo)";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, Single) {
  const absl::string_view tmpl = "{{single}}";
  TemplateDictionary dict("blabla");
  EXPECT_THAT(dict.name(), "blabla");
  dict.SetValue("single", "single");
  const absl::string_view expected = "single";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, Shorter) {
  const absl::string_view tmpl = "<{{shorter}}>";
  TemplateDictionary dict("blabla");
  dict.SetValue("shorter", "s");
  const absl::string_view expected = "<s>";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, Longer) {
  const absl::string_view tmpl = "<{{l}}>";
  TemplateDictionary dict("blabla");
  dict.SetValue("l", "longer");
  const absl::string_view expected = "<longer>";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, Braces) {
  const absl::string_view tmpl = "{{{l}}}{{}}";
  TemplateDictionary dict("blabla");
  dict.SetValue("l", "}|{");
  const absl::string_view expected = "{}|{}{{}}";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, Self) {
  const absl::string_view tmpl = "{{self}}";
  TemplateDictionary dict("blabla");
  dict.SetValue("self", "{{self}}");
  const absl::string_view expected = "{{self}}";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, FlipFlop) {
  const absl::string_view tmpl = "{{flip}}{{flop}}";
  {
    TemplateDictionary dict("blabla");
    dict.SetValue("flip", "{{flop}}");
    dict.SetValue("flop", "{{flip}}");
    const absl::string_view expected = "{{flip}}{{flip}}";
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
  }
  {
    TemplateDictionary dict("blabla");
    dict.SetValue("flop", "{{flip}}");
    dict.SetValue("flip", "{{flop}}");
    // Same result because we use a map and do not take order into account.
    const absl::string_view expected = "{{flip}}{{flip}}";
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
  }
}

TEST_F(TemplateDictionaryTest, DictSimple) {
  const absl::string_view tmpl = "{{#dict}}<{{foo}}>{{/dict}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  dict.AddSectionDictionary("dict")->SetValue("foo", "bar");
  dict.AddSectionDictionary("dict")->SetValue("foo", "baz");
  const absl::string_view expected = "<foo><bar><baz>";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, DictRepeated) {
  const absl::string_view tmpl =
      "{{#dict}}<{{foo}}>{{/dict}}{{#dict}}({{foo}}){{/dict}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  dict.AddSectionDictionary("dict")->SetValue("foo", "bar");
  dict.AddSectionDictionary("dict")->SetValue("foo", "baz");
  const absl::string_view expected = "<foo><bar><baz>(foo)(bar)(baz)";
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, DictEmpty) {
  const absl::string_view tmpl = "{{#dict}}<{{foo}}>{{/dict}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict");
  const absl::string_view expected = "<{{foo}}>";  // Same as unknown case.
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, expected));
}

TEST_F(TemplateDictionaryTest, DictNoStart) {
  const absl::string_view tmpl = "{{/dict}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  dict.AddSectionDictionary("dict")->SetValue("foo", "bar");
  dict.AddSectionDictionary("dict")->SetValue("foo", "baz");
  EXPECT_THAT(Expand(dict, tmpl), Key(false));
}

TEST_F(TemplateDictionaryTest, DictNoEnd) {
  const absl::string_view tmpl = "{{#dict}}<{{foo}}>{{/dictT}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  dict.AddSectionDictionary("dict")->SetValue("foo", "bar");
  dict.AddSectionDictionary("dict")->SetValue("foo", "baz");
  EXPECT_THAT(Expand(dict, tmpl), Key(false));
}

TEST_F(TemplateDictionaryTest, DictNotPresent) {
  const absl::string_view tmpl = "{{#dict}}X{{/dict}}{{#bar}}{{/bar}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  EXPECT_THAT(Expand(dict, tmpl), Pair(true, "X"));
}

TEST_F(TemplateDictionaryTest, Mismatch) {
  const absl::string_view tmpl = "{{#foo}}{{/bar}}";
  TemplateDictionary dict("blabla");
  EXPECT_THAT(Expand(dict, tmpl), Key(false));
}

TEST_F(TemplateDictionaryTest, DictAsValue) {
  const absl::string_view tmpl = "{{dict}}{{foo}}";
  TemplateDictionary dict("blabla");
  dict.AddSectionDictionary("dict")->SetValue("foo", "foo");
  EXPECT_THAT(Expand(dict, tmpl), Key(false));
}

TEST_F(TemplateDictionaryTest, DictWhitespace) {
  const absl::string_view tmpl = R"(1
    {{#dict}}
    {{foo}}
    {{/dict}}
    2)";
  {
    TemplateDictionary dict("blabla");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    2"));
    auto section = dict.AddSectionDictionary("dict");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    {{foo}}\n    2"));
    section->SetValue("foo", "foo");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    foo\n    2"));
  }
  {
    TemplateDictionary dict("blabla");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    2"));
    auto section = dict.AddSectionDictionary("dict");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    {{foo}}\n    2"));
    section->SetValue("foo", "");
    EXPECT_THAT(Expand(dict, tmpl), Pair(true, "1\n    2"));
  }
  const absl::string_view tmpl2 = R"(1
    {{foo}}
    {{bar}}
    {{#dict}}
    {{more}}
    {{baz}}
    {{/dict}}
    2)";
  {
    TemplateDictionary dict("blabla");
    dict.SetValue("foo", "");
    dict.SetValue("bar", "");
    EXPECT_THAT(Expand(dict, tmpl2), Pair(true, "1\n    2"));
    auto section = dict.AddSectionDictionary("dict");
    section->SetValue("more", "");
    section->SetValue("baz", "");
    EXPECT_THAT(Expand(dict, tmpl2), Pair(true, "1\n    2"));
  }
}

}  // namespace
}  // namespace proto_builder::oss
