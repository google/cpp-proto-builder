#include "proto_builder/tests/validator_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;
using ::testing::HasSubstr;
using ::testing::status::oss::IsOkAndHolds;
using ::testing::status::oss::StatusIs;

class ValidatorBuilderTest : public ::testing::Test {};

TEST_F(ValidatorBuilderTest, TestField) {
  EXPECT_THAT(ValidatorBuilder(), EqualsProto(""));
  EXPECT_THAT(ValidatorBuilder().SetAddress("8.8.8.8").Build(),
              IsOkAndHolds(EqualsProto("address: '8.8.8.8'")));
  EXPECT_THAT(
      ValidatorBuilder().SetAddress("foo").Build(),
      StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("Bad address")));
}

TEST_F(ValidatorBuilderTest, TestSubBuilder) {
  ASSERT_THAT(Validator_FallbackBuilder().SetAddress("BAD").MaybeGetRawData(),
              IsOkAndHolds(EqualsProto(R"pb(
                address: "BAD"
              )pb")));
  ASSERT_THAT(Validator_FallbackBuilder().SetAddress("BAD").Build(),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Bad fallback address")));
  EXPECT_THAT(
      ValidatorBuilder()
          .AddFallback(Validator_FallbackBuilder().SetAddress("8.8.4.4"))
          .Build(),
      IsOkAndHolds(EqualsProto(R"pb(
        fallback { address: "8.8.4.4" }
      )pb")));
  EXPECT_THAT(ValidatorBuilder()
                  .AddFallback(Validator_FallbackBuilder().SetAddress("BAD"))
                  .Build(),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Bad fallback address")));
}

TEST_F(ValidatorBuilderTest, TestSubBuilderWithNonValidationError) {
  const auto fallback_builder =
      Validator_FallbackBuilder().SetAddress("8.8.8.8").UpdateStatus(
          absl::InternalError("other"));
  ASSERT_THAT(fallback_builder.Build(),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("other")));
  const ValidatorBuilder validator =
      ValidatorBuilder().AddFallback(fallback_builder);
  EXPECT_THAT(validator.Build(),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("other")));
}

TEST_F(ValidatorBuilderTest, TestSubBuilderNotValidated) {
  EXPECT_THAT(ValidatorBuilder()
                  .AddNotValidated(
                      Validator_NotValidatedBuilder().SetAddress("8.8.4.4"))
                  .Build(),
              IsOkAndHolds(EqualsProto(R"pb(
                not_validated { address: "8.8.4.4" }
              )pb")));
  EXPECT_THAT(
      ValidatorBuilder()
          .AddNotValidated(Validator_NotValidatedBuilder().SetAddress("BAD"))
          .Build(),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("Bad non-self-validated address")));
}

TEST_F(ValidatorBuilderTest, TestMapField) {
  EXPECT_THAT(
      ValidatorBuilder()
          .InsertNamedFallback(
              "first", Validator_FallbackBuilder().SetAddress("8.8.8.8"))
          .Build(),
      IsOkAndHolds(EqualsProto(R"pb(
        named_fallback {
          key: "first"
          value { address: "8.8.8.8" }
        }
      )pb")));
  EXPECT_THAT(  // Inject error into sub-builder, check it's detected.
      ValidatorBuilder()
          .InsertNamedFallback("first",
                               Validator_FallbackBuilder().SetAddress("BAD"))
          .Build(),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("Bad address named: first")));
}

TEST_F(ValidatorBuilderTest, TestConversion) {
  EXPECT_THAT(
      ValidatorBuilder().SetOptions("8.8.8.8").Build(),
      IsOkAndHolds(EqualsProto(R"pb(options { address: "8.8.8.8" })pb")));
  EXPECT_THAT(ValidatorBuilder().SetOptions("0.0.0.0").Build(),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Bad options address")));
}

}  // namespace
}  // namespace proto_builder::tests
