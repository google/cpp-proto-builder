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

#include "proto_builder/oss/file.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <map>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/strip.h"

namespace file {
namespace oss {
namespace internal {

std::string JoinPathSimplify(absl::string_view path) {
  while (path.length() > 1 && absl::ConsumeSuffix(&path, "/")) {
  }
  std::string result{path};
  while (absl::StrReplaceAll({{"//", "/"}}, &result)) {
  }
  return result;
}

}  // namespace internal

Options Defaults() { return Options(); }

absl::StatusOr<std::string> GetContents(absl::string_view file_name,
                                        const Options& options) {
  std::string content;
  absl::Status result = GetContents(file_name, &content);
  if (result.ok()) {
    return content;
  } else {
    return result;
  }
}

}  // namespace oss
}  // namespace file
