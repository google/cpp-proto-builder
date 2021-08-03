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

#ifndef PROTO_BUILDER_OSS_INIT_PROGRAM_H_
#define PROTO_BUILDER_OSS_INIT_PROGRAM_H_

#include "absl/types/span.h"

// Typically called early on in main() and must be called before other
// threads start using functions from this file.
//
// 'usage' provides a short usage message passed to
//         absl::SetProgramUsageMessage().
//         Most callers provide the name of the app as 'usage' ?!
// 'argc' and 'argv' are the command line flags to parse. There is no
//         requirement for an element (*argv)[*argc] to exist or to have
//         any particular value, unlike the similar array that is passed
//         to the `main` function.
// If 'remove_flags' then parsed flags are removed from *argc/*argv.
void InitProgram(const char* usage, int* argc, char*** argv, bool remove_flags);

namespace internal {
/*
 * Reorder the arguments in 'argv' to match the order in 'args'.
 *
 * The
 *
 */
void ReorderArguments(int argc, char*** argv,
                      absl::Span<const char* const> args);
}  // namespace internal

#endif  // PROTO_BUILDER_OSS_INIT_PROGRAM_H_
