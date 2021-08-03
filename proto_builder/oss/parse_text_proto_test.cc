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

#include "proto_builder/oss/parse_text_proto.h"

#include "proto_builder/oss/tests/simple_message.pb.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::oss {
namespace {

using ::testing::oss::EqualsProto;

class ParseTextProtoTest : public ::testing::Test {};

TEST_F(ParseTextProtoTest, ParseOk) {
  EXPECT_THAT(ParseTextOrDie<SimpleMessage>(""), EqualsProto(""));
  EXPECT_THAT(ParseTextOrDie<SimpleMessage>("one: 42"), EqualsProto("one: 42"));
  EXPECT_THAT((SimpleMessage)ParseTextProtoOrDie("one: 25"),
              EqualsProto("one: 25"));
}

TEST_F(ParseTextProtoTest, ParseError) {
  EXPECT_DEATH(ParseTextOrDie<oss::SimpleMessage>("!!!"),
               // Using [0-9] in lieu of \\d to be compatible in open source.
               "File: '.*/parse_text_proto_test.cc', Line: [0-9]+\n"
               "Check failed: ParseTextOrDie<SimpleMessage>\n"
               "INVALID_ARGUMENT: Line 0, Col 0: Expected identifier, got: !");
}

}  // namespace
}  // namespace proto_builder::oss
