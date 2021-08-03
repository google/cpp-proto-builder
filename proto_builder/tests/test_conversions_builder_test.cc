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

#include "proto_builder/tests/test_conversions_builder.h"

#include <string>
#include <vector>

#include "proto_builder/tests/test_conversions_sub_builder.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"

namespace proto_builder {
namespace {

using ::testing::oss::EqualsProto;

class TestConversionsBuilderTest : public ::testing::Test {
 protected:
};

TEST_F(TestConversionsBuilderTest, Empty) {
  EXPECT_THAT(ConversionsBuilder(), EqualsProto(""));
}

TEST_F(TestConversionsBuilderTest, Line) {
  EXPECT_THAT(ConversionsBuilder().AddLine("1").AddLine("2"),
              EqualsProto("line: '1' line: '2'"));
}

TEST_F(TestConversionsBuilderTest, Line_Container) {
  EXPECT_THAT(
      ConversionsBuilder().AddLines(std::vector<std::string>({"1", "2"})),
      EqualsProto("line: '1' line: '2'"));
}

TEST_F(TestConversionsBuilderTest, Line_InitializerList) {
  EXPECT_THAT(ConversionsBuilder().AddLines({"1", "2"}),
              EqualsProto("line: '1' line: '2'"));
}

TEST_F(TestConversionsBuilderTest, ToInt64Seconds) {
  EXPECT_THAT(ConversionsBuilder().SetTimeInt64Seconds(absl::Seconds(10)),
              EqualsProto("time { int64_seconds: 10 }"));
  EXPECT_THAT(
      ConversionsBuilder().SetTimeInt64Seconds(absl::Milliseconds(2000)),
      EqualsProto("time { int64_seconds: 2 }"));
}

TEST_F(TestConversionsBuilderTest, ToInt64Milliseconds) {
  EXPECT_THAT(ConversionsBuilder().SetTimeInt64Milliseconds(absl::Seconds(10)),
              EqualsProto("time { int64_milliseconds: 10000 }"));
  EXPECT_THAT(
      ConversionsBuilder().SetTimeInt64Milliseconds(absl::Milliseconds(2000)),
      EqualsProto("time { int64_milliseconds: 2000 }"));
}

TEST_F(TestConversionsBuilderTest, ToDoubleSeconds) {
  EXPECT_THAT(ConversionsBuilder().SetTimeDoubleSeconds(absl::Seconds(10)),
              EqualsProto("time { double_seconds: 10 }"));
  EXPECT_THAT(
      ConversionsBuilder().SetTimeDoubleSeconds(absl::Milliseconds(1200)),
      EqualsProto("time { double_seconds: 1.2 }"));
}

TEST_F(TestConversionsBuilderTest, ToDoubleMilliseconds) {
  EXPECT_THAT(ConversionsBuilder().SetTimeDoubleMilliseconds(absl::Seconds(1)),
              EqualsProto("time { double_milliseconds: 1000 }"));
  EXPECT_THAT(
      ConversionsBuilder().SetTimeDoubleMilliseconds(absl::Milliseconds(2)),
      EqualsProto("time { double_milliseconds: 2 }"));
}

TEST_F(TestConversionsBuilderTest, ToProtoDuration) {
  EXPECT_THAT(ConversionsBuilder().SetProtoTimeDuration(absl::Seconds(123)),
              EqualsProto("proto_time: { duration { seconds: 123 } }"));
}

TEST_F(TestConversionsBuilderTest, ToProtoTimestamp) {
  absl::Time absl_time = absl::FromCivil(
      absl::CivilSecond(2021, 2, 13, 10, 30, 0), absl::UTCTimeZone());
  ASSERT_THAT(absl::ToUnixSeconds(absl_time), 1613212200);
  EXPECT_THAT(ConversionsBuilder().SetProtoTimeTime(absl_time),
              EqualsProto("proto_time: { time { seconds: 1613212200 } }"));
}

#if 0  // TODO: Test once support is implemented.
TEST_F(TestConversionsBuilderTest, OptionalSub_TextProto) {
  EXPECT_THAT(
      ConversionsBuilder().SetOptionalSub("one: 1 two: 2"),
      EqualsProto("optional_sub { one: 1 two: 2 }"));
}
#endif

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1(s1).AddRepeatedSub1(s2),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_Builder) {
  EXPECT_THAT(
      ConversionsBuilder()
          .AddRepeatedSub1(Conversions_SubBuilder().SetOne(1).SetTwo(2))
          .AddRepeatedSub1(Conversions_SubBuilder().SetOne(3).SetTwo(4)),
      EqualsProto(R"pb(
        repeated_sub1 { one: 1 two: 2 }
        repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_TextProto) {
  EXPECT_THAT(ConversionsBuilder()
                  .AddRepeatedSub1("one: 1 two: 2")
                  .AddRepeatedSub1("one: 3 two: 4"),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_Container) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1(
                  std::vector<Conversions_Sub>({s1, s2})),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_Container_Builder) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1(std::vector<Conversions_Sub>(
                  {Conversions_SubBuilder().SetOne(1).SetTwo(2),
                   Conversions_SubBuilder().SetOne(3).SetTwo(4)})),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_Container_TextProto) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1Text(
                  std::vector<std::string>({"one: 1 two: 2", "one: 3 two: 4"})),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
  // The initializer_list here uses 'const char*'. Thus for item entry we call
  // AddRepeatedSub1(const char*);
  // If that is not present, we would call the Container version and have it
  // iterate over the single chars. That is the reason we write std::enable_if.
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1(
                  std::vector<const char*>({"one: 1 two: 2", "one: 3 two: 4"})),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
  // absl::string_view does not work here, since for 'repeated_sub1' we have no
  // type: "@TextProto:absl::string_view" support. We do in 'repeated_sub2'
  // below.
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_InitializerList) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1({s1, s2}), EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_InitializerList_Builder) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1(
                  {Conversions_SubBuilder().SetOne(1).SetTwo(2),
                   Conversions_SubBuilder().SetOne(3).SetTwo(4)}),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub1_InitializerList_TextProto) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub1Text(
                  {"one: 1 two: 2", "one: 3 two: 4"}),
              EqualsProto(R"pb(
                repeated_sub1 { one: 1 two: 2 }
                repeated_sub1 { one: 3 two: 4 })pb"));
  // Same as in AddRepeatedSub1_Container_TextProto
  EXPECT_THAT(
      ConversionsBuilder().AddRepeatedSub1({"one: 1 two: 2", "one: 3 two: 4"}),
      EqualsProto(R"pb(
        repeated_sub1 { one: 1 two: 2 }
        repeated_sub1 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(s1).AddRepeatedSub2(s2),
              EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_Builder) {
  EXPECT_THAT(
      ConversionsBuilder()
          .AddRepeatedSub2(Conversions_SubBuilder().SetOne(1).SetTwo(2))
          .AddRepeatedSub2(Conversions_SubBuilder().SetOne(3).SetTwo(4)),
      EqualsProto(R"pb(
        repeated_sub2 { one: 1 two: 2 }
        repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_Container) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(
                  std::vector<Conversions_Sub>({s1, s2})),
              EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_Container_Builder) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(std::vector<Conversions_Sub>(
                  {Conversions_SubBuilder().SetOne(1).SetTwo(2),
                   Conversions_SubBuilder().SetOne(3).SetTwo(4)})),
              EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_Container_TextProto) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(
                  std::vector<const char*>({"one: 1 two: 2", "one: 3 two: 4"})),
              EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
  // This works because we are supporting both conversions.
  EXPECT_THAT(
      ConversionsBuilder().AddRepeatedSub2(
          std::vector<absl::string_view>({"one: 1 two: 2", "one: 3 two: 4"})),
      EqualsProto(R"pb(
        repeated_sub2 { one: 1 two: 2 }
        repeated_sub2 { one: 3 two: 4 })pb"));
#if 0
  // This would require an overload for 'string' - or alternatively, turning
  // all 'AddRepeatedSub2' into templates with two parameters, where the second
  // determines the first-type-parameter to be std::is_convertible to one of
  // {to target-type, absl::string_view, neither} and additionally a version for
  // std::initializer_list.
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(
                  std::vector<string>({"one: 1 two: 2", "one: 3 two: 4"})),
              EqualsProto(R"pb(
                  repeated_sub2 { one: 1 two: 2 }
                  repeated_sub2 { one: 3 two: 4 })pb"));
#endif
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_Initializer) {
  const Conversions_Sub s1 = Conversions_SubBuilder().SetOne(1).SetTwo(2);
  const Conversions_Sub s2 = Conversions_SubBuilder().SetOne(3).SetTwo(4);
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2({s1, s2}), EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_InitializerList_Builder) {
  EXPECT_THAT(ConversionsBuilder().AddRepeatedSub2(
                  {Conversions_SubBuilder().SetOne(1).SetTwo(2),
                   Conversions_SubBuilder().SetOne(3).SetTwo(4)}),
              EqualsProto(R"pb(
                repeated_sub2 { one: 1 two: 2 }
                repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, AddRepeatedSub2_InitializerList_TextProto) {
  EXPECT_THAT(
      ConversionsBuilder().AddRepeatedSub2({"one: 1 two: 2", "one: 3 two: 4"}),
      EqualsProto(R"pb(
        repeated_sub2 { one: 1 two: 2 }
        repeated_sub2 { one: 3 two: 4 })pb"));
}

TEST_F(TestConversionsBuilderTest, Maymap_Container) {
  std::vector<std::pair<int, std::string>> container = {
      {1, "1"}, {2, "old"}, {2, "new"}, {3, "3"}};
  EXPECT_THAT(ConversionsBuilder().InsertMymap(container), EqualsProto(R"pb(
                mymap { key: 1 value: "1" }
                mymap { key: 2 value: "old" }
                mymap { key: 3 value: "3" }
              )pb"));
}

TEST_F(TestConversionsBuilderTest, Maymap_InitializerList) {
  EXPECT_THAT(ConversionsBuilder().InsertMymap(
                  {{1, "1"}, {2, "old"}, {2, "new"}, {3, "3"}}),
              EqualsProto(R"pb(
                mymap { key: 1 value: "1" }
                mymap { key: 2 value: "old" }
                mymap { key: 3 value: "3" }
              )pb"));
}

TEST_F(TestConversionsBuilderTest, OtherMap_ForeachAdd) {
  std::vector<std::pair<int, std::string>> container = {
      {1, "1"}, {2, "old"}, {2, "new"}, {3, "3"}};
  EXPECT_THAT(ConversionsBuilder().InsertOtherMap(container), EqualsProto(R"pb(
                other_map { key: 1 value: "1" }
                other_map { key: 2 value: "old" }
                other_map { key: 3 value: "3" }
              )pb"));
}

TEST_F(TestConversionsBuilderTest, SubMap_Conversion) {
  EXPECT_THAT(ConversionsBuilder().InsertSubMap({1, "one: 1 two: 2"}),
              EqualsProto(R"pb(
                sub_map {
                  key: 1
                  value: { one: 1 two: 2 }
                }
              )pb"));
}

TEST_F(TestConversionsBuilderTest, SubMap_ForaechAdd_Conversion) {
  std::vector<std::pair<int, absl::string_view>> container =  // Ignores 2nd 2.
      {{1, "one: 1 two: 1"},
       {2, "one: 2 two: 21"},
       {2, "one: 2 two: 22"},
       {3, "one: 3 two: 3"}};
  EXPECT_THAT(ConversionsBuilder().InsertSubMap(container), EqualsProto(R"pb(
                sub_map {
                  key: 1
                  value: { one: 1 two: 1 }
                }
                sub_map {
                  key: 2
                  value: { one: 2 two: 21 }
                }
                sub_map {
                  key: 3
                  value: { one: 3 two: 3 }
                }
              )pb"));
}

TEST_F(TestConversionsBuilderTest, SubMap_ForaechAdd_InitializerList) {
  EXPECT_THAT(ConversionsBuilder().InsertSubMap(  // Ignore 2nd 2
                  {{1, "one: 1 two: 1"},
                   {2, "one: 2 two: 21"},
                   {2, "one: 2 two: 22"},
                   {3, "one: 3 two: 3"}}),
              EqualsProto(R"pb(
                sub_map {
                  key: 1
                  value: { one: 1 two: 1 }
                }
                sub_map {
                  key: 2
                  value: { one: 2 two: 21 }
                }
                sub_map {
                  key: 3
                  value: { one: 3 two: 3 }
                }
              )pb"));
}

}  // namespace
}  // namespace proto_builder
