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

#include "proto_builder/tests/advanced_cc_proto_builder.h"

#include <ostream>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"
#include "proto_builder/oss/source_location.h"

namespace absl {

std::ostream& operator<<(std::ostream& o, proto_builder::oss::SourceLocation s) {
  o << s.file_name() << ":" << s.line();
  return o;
}

std::ostream& operator<<(std::ostream& o,
                         absl::Span<const proto_builder::oss::SourceLocation> v) {
  for (const auto& s : v) {
    o << s << "\n";
  }
  return o;
}

}  // namespace absl

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;
using ::testing::status::oss::IsOk;
using ::testing::status::oss::IsOkAndHolds;
using ::testing::status::oss::StatusIs;

class AdvancedBuilderTest : public ::testing::Test {};

TEST_F(AdvancedBuilderTest, Test) {
  EXPECT_THAT(AdvancedBuilder().SetText("bla"),
              EqualsProto(R"pb(text: "bla")pb"));
  EXPECT_THAT(AdvancedBuilder().SetText("bla").Build(),
              IsOkAndHolds(EqualsProto(R"pb(text: "bla")pb")));
  EXPECT_THAT(AdvancedBuilder().SetText("bla").Consume(),
              IsOkAndHolds(EqualsProto(R"pb(text: "bla")pb")));
}

TEST_F(AdvancedBuilderTest, ConsumeIsConsuming) {
  auto builder = AdvancedBuilder().SetText("X");
  EXPECT_THAT(builder.Consume(), IsOkAndHolds(EqualsProto(R"pb(text: "X")pb")));
  EXPECT_THAT(builder.Consume(), IsOkAndHolds(EqualsProto("")));
}

TEST_F(AdvancedBuilderTest, Error) {
  auto builder = AdvancedBuilder().SetText("X");
  EXPECT_THAT(builder.ok(), true);
  EXPECT_THAT(builder.status(), IsOk());
  EXPECT_THAT(builder.Build(), IsOkAndHolds(EqualsProto(R"pb(text: "X")pb")));
  EXPECT_THAT(builder, EqualsProto(R"pb(text: "X")pb"));
  builder.UpdateStatus(absl::UnknownError("error"));
  EXPECT_THAT(builder.ok(), false);
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kUnknown));
  EXPECT_THAT(builder, EqualsProto(""));  // Not ok -> default proto
  // Build does not affect the status.
  EXPECT_THAT(builder.Build(), StatusIs(absl::StatusCode::kUnknown));
  EXPECT_THAT(builder.ok(), false);
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kUnknown));
  EXPECT_THAT(builder, EqualsProto(""));
  // Update to ok, resets status and makes the data available again!
  builder.UpdateStatus(absl::OkStatus());
  EXPECT_THAT(builder.ok(), true);
  EXPECT_THAT(builder.status(), IsOk());
  EXPECT_THAT(builder.Build(), IsOkAndHolds(EqualsProto(R"pb(text: "X")pb")));
  EXPECT_THAT(builder, EqualsProto(R"pb(text: "X")pb"));
  // Consume *does* reset status to Ok..
  builder.UpdateStatus(absl::UnknownError("error"));
  EXPECT_THAT(builder.Consume(), StatusIs(absl::StatusCode::kUnknown));
  EXPECT_THAT(builder.ok(), true);
  EXPECT_THAT(builder.status(), IsOk());
  EXPECT_THAT(builder, EqualsProto(""));
  // Consume consumes, whether or not status was ok.
  EXPECT_THAT(builder.Build(), IsOkAndHolds(EqualsProto("")));
  // Another
  builder.SetText("Y");
  EXPECT_THAT(builder, EqualsProto(R"pb(text: "Y")pb"));
  builder.UpdateStatus(absl::UnknownError("error"));
  builder.SetText("Z");  // Does not reset the error, but sets the data.
  EXPECT_THAT(builder, EqualsProto(""));
  EXPECT_THAT(builder.ok(), false);
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kUnknown));
  EXPECT_THAT(builder.Build(), StatusIs(absl::StatusCode::kUnknown));
  builder.UpdateStatus(absl::OkStatus());
  EXPECT_THAT(builder.ok(), true);
  EXPECT_THAT(builder.status(), IsOk());
  EXPECT_THAT(builder.Build(), IsOkAndHolds(EqualsProto(R"pb(text: "Z")pb")));
  EXPECT_THAT(builder, EqualsProto(R"pb(text: "Z")pb"));
}

}  // namespace
}  // namespace proto_builder::tests
