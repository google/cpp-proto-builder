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

#include "proto_builder/oss/testing/proto_test_util.h"

#include "proto_builder/oss/parse_text_proto.h"
#include "proto_builder/oss/tests/simple_message.pb.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace testing {
namespace oss {
namespace {

using ::proto_builder::oss::ParseTextProtoOrDie;
using ::proto_builder::oss::SimpleMessage;
using ::testing::Not;

// This test is testing EqualsProto and examining nesting
// Approximately(), TreatingNaNsAsEqual(), Partially(), IgnoringFields(), and
// IgnoringRepeatedFieldOrdering(); however, note that the order is irrelevant
// so we do not need to test in different orders.
TEST(ProtoTestUtilTest, ProtosAreEqual) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    one: 1 two: 2
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    one: 1 two: 2
  )pb");
  EXPECT_THAT(pb1, EqualsProto(pb2));
}

TEST(ProtoTestUtilTest, ProtosAreEquiv) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    one: 1
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    one: 1
  )pb");
  EXPECT_THAT(pb1, EquivToProto(pb2));
}

TEST(ProtoTestUtilTest, DifferentOrderingUnequal) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    two: 1 two: 2
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    two: 2 two: 1
  )pb");
  EXPECT_THAT(pb1, Not(EqualsProto(pb2)));
}

TEST(ProtoTestUtilTest, ProtoEqualsStringProto) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(one: 1 two: 1)pb");
  EXPECT_THAT(pb1, EqualsProto(R"pb(one: 1 two: 1)pb"));
}

TEST(ProtoTestUtilTest, IgnoringFields) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(one: 1 two: 1)pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(two: 1)pb");
  EXPECT_THAT(pb1, IgnoringFields(
                       {"proto_builder.oss.SimpleMessage.one"},
                       EqualsProto(pb2)));
}

TEST(ProtoTestUtilTest, IgnoringRepeatedFieldOrdering) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    two: 1 two: 2 two: 3 two: 4
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    two: 4 two: 3 two: 2 two: 1
  )pb");
  EXPECT_THAT(pb1, IgnoringRepeatedFieldOrdering(EqualsProto(pb2)));
}

TEST(ProtoTestUtilTest, IgnoringRepeatedFieldOrderingRepeatedElementNotEqual) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    two: 1 two: 2 two: 3 two: 4
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    two: 4 two: 3 two: 2
  )pb");
  EXPECT_THAT(pb1, Not(IgnoringRepeatedFieldOrdering(EqualsProto(pb2))));
}

TEST(ProtoTestUtilTest, Partially) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(
    one: 1 two: 1
  )pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(
    one: 1
  )pb");
  EXPECT_THAT(pb1, Partially(EqualsProto(pb2)));
}

TEST(ProtoTestUtilTest, WhenDeserialized) {
  const SimpleMessage pb = ParseTextProtoOrDie(R"pb(one: 1)pb");
  EXPECT_THAT(pb.SerializeAsString(), WhenDeserialized(EqualsProto(pb)));
}

TEST(ProtoTestUtilTest, WhenDeserializedUnequal) {
  const SimpleMessage pb1 = ParseTextProtoOrDie(R"pb(one: 1)pb");
  const SimpleMessage pb2 = ParseTextProtoOrDie(R"pb(one: 2)pb");
  EXPECT_THAT(pb1.SerializeAsString(), Not(WhenDeserialized(EqualsProto(pb2))));
}

}  // namespace
}  // namespace oss
}  // namespace testing
