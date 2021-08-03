#include "proto_builder/tests/source_location_cc_proto_builder.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;
using ::testing::Not;

class SourceLocationTest : public ::testing::Test {};

TEST_F(SourceLocationTest, Basic) {
  SourceLocationBuilder builder;
  ASSERT_THAT(builder.source_location().file_name(),
              Not(proto_builder::oss::SourceLocation::current().file_name()));
  builder.AddTarget("bla");
  EXPECT_THAT(builder.source_location().file_name(),
              proto_builder::oss::SourceLocation::current().file_name());
  EXPECT_THAT(builder.data(), EqualsProto(R"pb(target: "bla")pb"));
}

TEST_F(SourceLocationTest, PassAlongFromContainer) {
  SourceLocationBuilder builder;
  ASSERT_THAT(builder.source_location().file_name(),
              Not(proto_builder::oss::SourceLocation::current().file_name()));
  builder.AddTargets(std::vector<std::string>{"1", "2"});
  EXPECT_THAT(builder.source_location().file_name(),
              proto_builder::oss::SourceLocation::current().file_name());
  EXPECT_THAT(builder.data(), EqualsProto(R"pb(target: "1" target: "2")pb"));
}

TEST_F(SourceLocationTest, PassAlongFromInitializerList) {
  SourceLocationBuilder builder;
  ASSERT_THAT(builder.source_location().file_name(),
              Not(proto_builder::oss::SourceLocation::current().file_name()));
  builder.AddTargets({"1", "2"});
  EXPECT_THAT(builder.source_location().file_name(),
              proto_builder::oss::SourceLocation::current().file_name());
  EXPECT_THAT(builder.data(), EqualsProto(R"pb(target: "1" target: "2")pb"));
}

}  // namespace
}  // namespace proto_builder::tests
