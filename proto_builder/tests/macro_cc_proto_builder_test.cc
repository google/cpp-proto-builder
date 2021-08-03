#include "proto_builder/tests/macro_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;

class MacroTest : public ::testing::Test {};

TEST_F(MacroTest, Test) {
  MacroBuilder builder;
  EXPECT_THAT(builder.SetNumber(30), EqualsProto<Macro>("number: 30"));
  EXPECT_THAT(builder.SetNumber(10), EqualsProto<Macro>("number: 30"));
  EXPECT_THAT(builder.SetNumber(40), EqualsProto<Macro>("number: 40"));
}

}  // namespace
}  // namespace proto_builder::tests
