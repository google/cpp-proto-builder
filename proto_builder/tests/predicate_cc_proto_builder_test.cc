#include "proto_builder/tests/predicate_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;
using ::testing::status::oss::IsOk;
using ::testing::status::oss::StatusIs;

class PredicateTest : public ::testing::Test {};

TEST_F(PredicateTest, Predicate) {
  PredicateBuilder builder;
  EXPECT_THAT(builder.SetNumber(30), EqualsProto<Predicate>("number: 30"));
  EXPECT_THAT(builder.SetNumber(10), EqualsProto<Predicate>("number: 30"));
  EXPECT_THAT(builder.SetNumber(40), EqualsProto<Predicate>("number: 40"));
}

TEST_F(PredicateTest, PredicateStatus) {
  PredicateStatusBuilder builder;
  EXPECT_THAT(builder.SetNumber(30), EqualsProto<PredicateStatus>("number:30"));
  EXPECT_THAT(builder.status(), IsOk());
  EXPECT_THAT(builder.SetNumber(10), EqualsProto<PredicateStatus>(""));
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(builder.SetNumber(40), EqualsProto<PredicateStatus>(""));
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(builder.SetNumber(50), EqualsProto<PredicateStatus>(""));
  EXPECT_THAT(builder.status(), StatusIs(absl::StatusCode::kInvalidArgument));
  // Resetting to OkStatus means access to the data is restored.
  // Only settings 30 and later 40 were accepted while 10 and 50 were rejected.
  // This means we allow settings in non OkStatus. That is to allow cases where
  // a setting might put the overall data back into OkStatus.
  EXPECT_THAT(builder.UpdateStatus(absl::OkStatus()).status(), IsOk());
  EXPECT_THAT(builder, EqualsProto<PredicateStatus>("number: 40"));
}

TEST_F(PredicateTest, PredicateStatusMap) {
  PredicateStatusBuilder builder;
  EXPECT_THAT(builder.InsertIndex({25, "yes"}),
              EqualsProto<PredicateStatus>(R"pb(
                index { key: 25 value: "yes" }
              )pb"));
  EXPECT_THAT(builder.InsertIndex({1, "no"}), EqualsProto<PredicateStatus>(""));
  EXPECT_THAT(builder.UpdateStatus(absl::OkStatus()).status(), IsOk());
  EXPECT_THAT(builder, EqualsProto<PredicateStatus>(R"pb(
                index { key: 25 value: "yes" }
              )pb"));
}

}  // namespace
}  // namespace proto_builder::tests
