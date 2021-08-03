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

#ifndef PROTO_BUILDER_OSS_UNIFIED_DIFF_H_
#define PROTO_BUILDER_OSS_UNIFIED_DIFF_H_

#include "absl/strings/string_view.h"

namespace proto_builder::oss {

// Returns the unified line-by-line diff between the contents of left
// and right. left and right will not be copied.
//
// left_name and right_name are used as the file names in the diff headers,
// context_size is ignored.
// TODO context_size has the same meaning as the -u X argument to diff.
//
// If left and right are identical, return the empty string.
std::string UnifiedDiff(absl::string_view left, absl::string_view right,
                        absl::string_view left_name,
                        absl::string_view right_name, int context_size);

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_UNIFIED_DIFF_H_
