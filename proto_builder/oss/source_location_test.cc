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

#include "proto_builder/oss/source_location.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/types/optional.h"

namespace proto_builder::oss {
namespace {

using ::testing::EndsWith;
using ::testing::Not;

class SourceLocationTest : public ::testing::Test {};

TEST_F(SourceLocationTest, Current) {
  SourceLocation location = SourceLocation::current();

  EXPECT_EQ(location.line(), __LINE__ - 2);
  EXPECT_THAT(location.file_name(), EndsWith("source_location_test.cc"));
}

TEST_F(SourceLocationTest, AddSourceLocationToStatus) {
  absl::Status status = absl::InternalError("");
  const size_t line = __LINE__ + 1;
  AddSourceLocationToStatus(SourceLocation::current(), status);
  absl::optional<absl::Cord> payload =
      status.GetPayload("proto_builder/source_location");
  ASSERT_THAT(payload, Not(absl::nullopt));
  std::string s;
  absl::CopyCordToString(payload.value(), &s);
  EXPECT_THAT(s, absl::StrCat(__FILE__, ":", line));
}

}  // namespace
}  // namespace proto_builder::oss
