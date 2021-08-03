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

#include "proto_builder/util.h"

#include <string>

#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/parse_text_proto.h"
#include "proto_builder/proto_builder.pb.h"
#include "proto_builder/tests/test_message.pb.h"
#include "google/protobuf/descriptor.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace proto_builder {

using ::proto_builder::oss::ParseTextProtoOrDie;
using ::testing::oss::EqualsProto;
using ::testing::oss::IgnoringRepeatedFieldOrdering;

class UtilTest : public ::testing::Test {
 public:
  template <class Msg>
  static const FieldDescriptor& FindFieldByName(absl::string_view name) {
    return *PBCC_DIE_IF_NULL(Msg::descriptor()->FindFieldByName(std::string(name)));
  }
};

TEST_F(UtilTest, AbsoluteCppTypeName) {
  EXPECT_THAT(AbsoluteCppTypeName("int"), "int");
  EXPECT_THAT(AbsoluteCppTypeName("a_b"), "a_b");
  EXPECT_THAT(AbsoluteCppTypeName("a.b"), "::a::b");
  EXPECT_THAT(AbsoluteCppTypeName(".a"), "::a");
  EXPECT_THAT(AbsoluteCppTypeName(".a.b"), "::a::b");
  EXPECT_THAT(AbsoluteCppTypeName("a::b"), "::a::b");
  EXPECT_THAT(AbsoluteCppTypeName("::a"), "::a");
  EXPECT_THAT(AbsoluteCppTypeName("::a::b"), "::a::b");
  EXPECT_THAT(AbsoluteCppTypeName("std::string"), "std::string");
  EXPECT_THAT(AbsoluteCppTypeName("absl::Duration"), "::absl::Duration");
  EXPECT_THAT(AbsoluteCppTypeName("int32"), "int32_t");
  EXPECT_THAT(AbsoluteCppTypeName("int64"), "int64_t");
  EXPECT_THAT(AbsoluteCppTypeName("uint32"), "uint32_t");
  EXPECT_THAT(AbsoluteCppTypeName("uint64"), "uint64_t");
  EXPECT_THAT(AbsoluteCppTypeName("sint32"), "int32_t");
  EXPECT_THAT(AbsoluteCppTypeName("sint64"), "int64_t");
  EXPECT_THAT(AbsoluteCppTypeName("fixed32"), "uint32_t");
  EXPECT_THAT(AbsoluteCppTypeName("fixed64"), "uint64_t");
  EXPECT_THAT(AbsoluteCppTypeName("sfixed32"), "int32_t");
  EXPECT_THAT(AbsoluteCppTypeName("sfixed64"), "int64_t");
}

TEST_F(UtilTest, CamelCaseName) {
  using T = TestMessage;
  using S = TestMessage::Sub;
  EXPECT_THAT(CamelCaseName(FindFieldByName<T>("three")), "Three");
  EXPECT_THAT(CamelCaseName(FindFieldByName<S>("sub_one")), "SubOne");
  EXPECT_THAT(CamelCaseName(FindFieldByName<S>("__sub__3__")), "Sub3");
}

TEST_F(UtilTest, MergeFieldBuilderOptions) {
  EXPECT_THAT(MergeFieldBuilderOptions({}, {}), EqualsProto(""));
  EXPECT_THAT(MergeFieldBuilderOptions({}, {}).output(),
              FieldBuilderOptions::BOTH);
  EXPECT_THAT(MergeFieldBuilderOptions(ParseTextProtoOrDie("output: SKIP"), {}),
              EqualsProto("output: SKIP"));
  EXPECT_THAT(MergeFieldBuilderOptions({}, ParseTextProtoOrDie("output: BOTH")),
              EqualsProto("output: BOTH"));
  EXPECT_THAT(MergeFieldBuilderOptions(ParseTextProtoOrDie("output: SKIP"),
                                       ParseTextProtoOrDie("output: BOTH")),
              EqualsProto("output: SKIP"));
  for (auto name : {"type"}) {
    SCOPED_TRACE(absl::StrCat("Default field should win: ", name));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'a'")), {}),
                EqualsProto(absl::StrCat(name, ": 'a'")));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    {}, ParseTextProtoOrDie(absl::StrCat(name, ": 'b'"))),
                EqualsProto(absl::StrCat(name, ": 'b'")));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'explicit'")),
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'default'"))),
                EqualsProto(absl::StrCat(name, ": 'default'")));
  }
  for (auto name : {"name", "decorated_type", "conversion", "value"}) {
    SCOPED_TRACE(absl::StrCat("Explicit field should win: ", name));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'a'")), {}),
                EqualsProto(absl::StrCat(name, ": 'a'")));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    {}, ParseTextProtoOrDie(absl::StrCat(name, ": 'b'"))),
                EqualsProto(absl::StrCat(name, ": 'b'")));
    EXPECT_THAT(MergeFieldBuilderOptions(
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'explicit'")),
                    ParseTextProtoOrDie(absl::StrCat(name, ": 'default'"))),
                EqualsProto(absl::StrCat(name, ": 'explicit'")));
  }
  // Merge Includes. We do not care for order and AddIncldue() handles dupes.
  EXPECT_THAT(
      MergeFieldBuilderOptions(ParseTextProtoOrDie("include: ['a', 'b']"),
                               ParseTextProtoOrDie("include: ['b', 'c']")),
      IgnoringRepeatedFieldOrdering(
          EqualsProto("include: ['a', 'b', 'b', 'c']")));
}

}  // namespace proto_builder
