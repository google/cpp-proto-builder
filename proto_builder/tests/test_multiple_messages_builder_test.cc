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

#include "proto_builder/tests/test_multiple_messages_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder {
namespace {

using ::testing::oss::EqualsProto;

TEST(TestMultipleMessagesBuilderTest, Empty) {
  EXPECT_THAT(TestMessageCBuilder(), EqualsProto(""));
}

TEST(TestMultipleMessagesBuilderTest, Everything) {
  const TestMessageC msg = TestMessageCBuilder()
                               .SetA(TestMessageABuilder().SetId(1))
                               .AddB(TestMessageBBuilder().SetId("two"));
  EXPECT_THAT(msg, EqualsProto(R"pb(
                a { id: 1 }
                b { id: 'two' })pb"));
}

}  // namespace
}  // namespace proto_builder
