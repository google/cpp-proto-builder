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

#include "proto_builder/oss/proto_conversion_helpers.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/time/time.h"

namespace proto_builder::oss {
namespace {

using ::testing::oss::EqualsProto;

TEST(ProtoConversionsHelpersTest, ConvertToDuration) {
  EXPECT_THAT(*ConvertToProto(absl::Seconds(123)), EqualsProto("seconds: 123"));
}

TEST(ProtoConversionsHelpersTest, ToProtoTimestamp) {
  absl::Time absl_time = absl::FromCivil(
      absl::CivilSecond(2021, 2, 13, 10, 30, 0), absl::UTCTimeZone());
  ASSERT_THAT(absl::ToUnixSeconds(absl_time), 1613212200);
  EXPECT_THAT(*ConvertToProto(absl_time), EqualsProto("seconds: 1613212200"));
}

}  // namespace
}  // namespace proto_builder::oss
