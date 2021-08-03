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

#include "proto_builder/tests/test_message_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder {
namespace {

using ::testing::oss::EqualsProto;

TEST(TestMessageBuilderTest, Empty) {
  EXPECT_THAT(TestMessageBuilder(), EqualsProto(""));
}

TEST(TestMessageBuilderTest, InitializedWithConstTemplate) {
  const TestMessage template_msg = TestMessageBuilder().SetOne(1);
  EXPECT_THAT(template_msg, EqualsProto("one: 1"));

  EXPECT_THAT(TestMessageBuilder(template_msg).AddTwo(2), EqualsProto(R"pb(
                one: 1
                two: 2)pb"));
}

TEST(TestMessageBuilderTest, InitializedWithMovableTemplate) {
  TestMessage template_msg = TestMessageBuilder().SetOne(1);
  EXPECT_THAT(template_msg, EqualsProto("one: 1"));

  EXPECT_THAT(TestMessageBuilder(std::move(template_msg)).AddTwo(2),
              EqualsProto("one: 1 two: 2"));
}

TEST(TestMessageBuilderTest, Everything) {
  const TestMessage message =  // Init all fields
      TestMessageBuilder()
          .SetOne(1)
          .AddTwo(2)
          .SetThreeSubOne("3-1")
          .AddThreeSubTwo("3-2")
          .SetThreeSub3("3-3")
          .AddFour(TestMessage::Sub())
          .SetFive(ExtraTestMessage())
          .SetSix(6)
          .SetSeven("seven")
          .InsertEight({42, "blabla"})
          .InsertEight({43, "more"})
          .InsertNine({"nine", {}})
          .SetSeventeenEighteen(18)
          .SetNamespace(11)
          .AddAnd(12)
          .AddAnd(122)
          .SetOr({})
          .AddNot({})
          .SetOperator(15);
  EXPECT_THAT(message, EqualsProto(R"pb(
                one: 1
                two: 2
                three { sub_one: "3-1" sub_two: "3-2" __sub__3__: "3-3" }
                four {}
                five {}
                seven: "seven"
                eight { key: 42 value: "blabla" }
                eight { key: 43 value: "more" }
                nine: {
                  key: "nine"
                  value {}
                }
                Seventeen: { eighteen: 18 }
                namespace: 11
                and: [ 12, 122 ]
                or {}
                not {}
                operator: 15
              )pb"));
}

}  // namespace
}  // namespace proto_builder
