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

#include "proto_builder/oss/unified_diff.h"

#include <string>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::oss {
namespace {

using ::testing::HasSubstr;
using ::testing::IsEmpty;

TEST(UnifiedDiffTest, Empty) {
  EXPECT_THAT(UnifiedDiff(/*left=*/"", /*right=*/"", "left", "right", 0),
              IsEmpty());
}

TEST(UnifiedDiffTest, DiffLineNum) {
  EXPECT_THAT(UnifiedDiff(/*left=*/"extra_left_content\n", /*right=*/"", "left",
                          "right", 0),
              HasSubstr("Line Number: 2-2"));
}

TEST(UnifiedDiffTest, SingleDiff) {
  EXPECT_THAT(UnifiedDiff("left_content", "right_content", "left", "right", 0),
              HasSubstr("Line Number: 1\n"
                        "--- left_content\n"
                        "+++ right_content"));
}

TEST(UnifiedDiffTest, DiffOnDiffLines) {
  std::string diff = UnifiedDiff(
      "same_content\n"
      "left_content",
      "same_content\n"
      "right_content\n"
      "extra_right_content",
      "left", "right", 0);
  EXPECT_THAT(diff, HasSubstr("Line Number: 2\n"
                              "--- left_content\n"
                              "+++ right_content\n"));
  EXPECT_THAT(diff, HasSubstr("Line Number: 3-3\n"
                              "+++ extra_right_content"));
}

}  // namespace
}  // namespace proto_builder::oss
