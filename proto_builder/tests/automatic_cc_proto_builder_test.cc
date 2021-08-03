#include "proto_builder/tests/automatic_cc_proto_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/time/time.h"

namespace proto_builder::tests {
namespace {

using ::testing::oss::EqualsProto;

class AutomaticTest : public ::testing::Test {};

TEST_F(AutomaticTest, Duration) {
  EXPECT_THAT(AutomaticBuilder()
                  .SetDuration(absl::Minutes(2))
                  .AddDurationList(absl::Microseconds(42))
                  .AddDurationList(absl::Seconds(25)),
              EqualsProto(R"pb(
                duration { seconds: 120 }
                duration_list { nanos: 42000 }
                duration_list { seconds: 25 }
              )pb"));
}

TEST_F(AutomaticTest, Timestamp) {
  EXPECT_THAT(AutomaticBuilder()
                  .SetTimestamp(absl::FromUnixMillis(1626669889001))
                  .AddTimestampList(absl::FromUnixSeconds(0))
                  .AddTimestampList(absl::FromUnixSeconds(1626669889)),
              EqualsProto(R"pb(
                timestamp { seconds: 1626669889 nanos: 1000000 }
                timestamp_list { seconds: 0 }
                timestamp_list { seconds: 1626669889 }
              )pb"));
}

}  // namespace
}  // namespace proto_builder::tests
