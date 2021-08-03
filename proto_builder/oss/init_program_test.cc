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

#include "proto_builder/oss/init_program.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace internal {
namespace {

using ::testing::ElementsAreArray;

TEST(InitProgramTest, ReorderArguments) {
  std::vector<const char *> args = {"arg0", "arg1"};
  int argc = args.size();
  char **argv = const_cast<char **>(args.data());
  std::vector<const char *> remaining = {"arg1"};
  ReorderArguments(argc, &argv, remaining);
  EXPECT_THAT(remaining, ElementsAreArray(argv, remaining.size()));
}

}  // namespace
}  // namespace internal
