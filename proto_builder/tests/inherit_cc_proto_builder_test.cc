#include "proto_builder/tests/inherit_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;

class InheritTest : public ::testing::Test {};

TEST_F(InheritTest, Direct) {
  EXPECT_THAT(InheritBuilder().SetFirstName("First").SetLastName("Last"),
              EqualsProto(R"pb(
                first_name: "First" last_name: "Last"
              )pb"));
}

TEST_F(InheritTest, CrtpTest) {
  EXPECT_THAT(InheritBuilder().SetFirstLast("First", "Last"), EqualsProto(R"pb(
                first_name: "First"
                last_name: "Last"
              )pb"));
}

}  // namespace
}  // namespace proto_builder::tests
