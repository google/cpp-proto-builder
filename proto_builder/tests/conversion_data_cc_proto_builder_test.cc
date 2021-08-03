#include "proto_builder/tests/conversion_data_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;
using ::testing::oss::EquivToProto;

class ConversionDataTest : public ::testing::Test {};

TEST_F(ConversionDataTest, Test) {
  EXPECT_THAT(ConversionDataBuilder().SetNumber(30), EqualsProto(R"pb(
                number: 30
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetNumber(50), EqualsProto(R"pb(
                number: 0
              )pb"));
}

TEST_F(ConversionDataTest, IgnoreDefault) {
  EXPECT_THAT(ConversionDataBuilder(), EquivToProto(R"pb(
                ignore_default: 10
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetIgnoreDefault(31), EqualsProto(R"pb(
                ignore_default: 31
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetIgnoreDefault(50), EqualsProto(R"pb(
                ignore_default: 0
              )pb"))
      << "We expect 0 which is the default for the field type.";
}

TEST_F(ConversionDataTest, RespectDefault) {
  EXPECT_THAT(ConversionDataBuilder(), EquivToProto(R"pb(
                respect_default: 10
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetRespectDefault(32), EqualsProto(R"pb(
                respect_default: 32
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetRespectDefault(50), EqualsProto(R"pb(
                respect_default: 10
              )pb"))
      << "We expect 10 which is the field's speficied default.";
}

TEST_F(ConversionDataTest, NoDefault) {
  EXPECT_THAT(ConversionDataBuilder(), EquivToProto(R"pb(
                no_default: 0
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetNoDefault(33), EqualsProto(R"pb(
                no_default: 33
              )pb"));
  EXPECT_THAT(ConversionDataBuilder().SetNoDefault(50), EqualsProto(R"pb(
                no_default: 0
              )pb"))
      << "We expect 0 which is the passed down default for the field type.";
}

}  // namespace
}  // namespace proto_builder::tests
