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

#include "proto_builder/oss/logging_macros.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"

namespace internal {
void ReorderArguments(int argc, char*** argv,
                      absl::Span<const char* const> args) {
  for (size_t new_loc = 0; new_loc < args.size(); ++new_loc) {
    bool found = false;
    const char* new_args = args[new_loc];
    // backwards search in argv to perform swap
    for (size_t old_loc = argc - 1; old_loc >= new_loc; --old_loc) {
      if (strcmp(new_args, (*argv)[old_loc]) == 0) {
        std::swap((*argv)[old_loc], (*argv)[new_loc]);
        found = true;
        break;
      }
    }
    CHECK(found) << "Internal error in ReorderArguments.";
  }
}
}  // namespace internal

// Not tested because the testing framework calls SetProgramUsageMessage(),
// which envokes a 'SetProgramUsageMessage() called twice' error when testing
// this function.
void InitProgram(const char* usage, int* argc, char*** argv,
                 bool remove_flags) {
  absl::SetProgramUsageMessage(usage);
  std::vector<char*> remaining = absl::ParseCommandLine(*argc, *argv);
  if (remove_flags) {
    ::internal::ReorderArguments(*argc, argv, remaining);
    *argc = remaining.size();
  }
}
