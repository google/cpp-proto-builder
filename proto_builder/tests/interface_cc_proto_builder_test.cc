#include "proto_builder/tests/interface_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;

class InterfaceTest : public ::testing::Test {};

TEST_F(InterfaceTest, Direct) {
  EXPECT_THAT(InterfaceBuilder().SetFirstName("First").SetLastName("Last"),
              EqualsProto(R"pb(
                first_name: "First" last_name: "Last"
              )pb"));
}

TEST_F(InterfaceTest, CrtpTest) {
  EXPECT_THAT(InterfaceBuilder().SetFirstLast("First", "Last"),
              EqualsProto(R"pb(
                first_name: "First" last_name: "Last"
              )pb"));
}

}  // namespace
}  // namespace proto_builder::tests
