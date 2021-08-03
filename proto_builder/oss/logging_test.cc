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

#include "proto_builder/oss/logging.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::oss {
namespace {

using ::testing::HasSubstr;

TEST(LoggingTest, PBCC_DIE_IF_NULL) {
  int64_t temp = 42;
  EXPECT_EQ(*PBCC_DIE_IF_NULL(&temp), 42);
  EXPECT_DEATH(
      {
        bool result = nullptr == PBCC_DIE_IF_NULL(nullptr);
        // spoof a use of result
        result = !result;
      },
      HasSubstr("must be non-null."));
}

}  // namespace
}  // namespace proto_builder::oss
