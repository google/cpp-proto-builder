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

#include "proto_builder/tests/test_message_recursive_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder {
namespace {

using ::testing::oss::EqualsProto;

// Test presence of top level message.
// Tests related to the properties of the top level message can be found in:
// test_message_builder_tests.cc
TEST(TestMessageBuilderRecursiveTest, TestMessageBuilder) {
  EXPECT_THAT(TestMessageBuilder(), EqualsProto(""));
  const TestMessageBuilder message = TestMessageBuilder().SetOne(1);
  EXPECT_THAT(message, EqualsProto(R"pb(
                one: 1
              )pb"));
}

TEST(TestMessageBuilderRecursiveTest, TestMessage_SubBuilder) {
  EXPECT_THAT(TestMessage_SubBuilder(), EqualsProto(""));
  const TestMessage::Sub message =  // Init all fields
      TestMessage_SubBuilder().SetSubOne("3-1").AddSubTwo("3-2").SetSub3("3-3");
  EXPECT_THAT(message,
              EqualsProto(R"pb(
                sub_one: "3-1" sub_two: "3-2" __sub__3__: "3-3"
              )pb"));
}

}  // namespace
}  // namespace proto_builder
