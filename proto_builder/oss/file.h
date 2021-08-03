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

#ifndef PROTO_BUILDER_OSS_FILE_H_
#define PROTO_BUILDER_OSS_FILE_H_

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace file {
namespace oss {  // namespace to avoid conflict with libraries using the file
                 // namespace

namespace internal {
std::string JoinPathSimplify(absl::string_view path);  // See JoinPath.
}  // namespace internal

struct Options {
  // empty options to conforms with internal API
};

Options Defaults();

// Writes the data provided in `content` to the file `file_name`, overwriting
// any existing content. Fails if directory does not exist.
//
// NOTE: Will return OK iff all of the data in `content` was written.
// May write some of the data and return an error.
//
// Typical return codes (not guaranteed exhaustive):
//  * OK
//  * UNKNOWN (a Write, a Close or Open error occurred)
absl::Status SetContents(absl::string_view file_name, absl::string_view content,
                         const Options& options = Defaults());

// Answers the question, "Does the named file exist, and is it readable?"
//
// Typical return codes (not guaranteed exhaustive):
//  * OK - The file definitely exists and is readable.
//  * NOT_FOUND - The file definitely does not exist.
//  * FAILED_PRECONDITION - The file is not readable.
//  * UNKNOWN (a Close error occurred)
//
//  Note that if called on a directory, the behaviour is implementation-defined
//  and may be filesystem-specific.
//
// Example:
//    #include "file.h"
//
//    // Ensures 'file_name' exists and can be read by the current user.
//    assert(file::oss:Readable(filename, file::oss:Defaults()));
absl::Status Readable(absl::string_view file_name,
                      const Options& options = Defaults());

// Reads the contents of the file `file_name` into `output`.
//
// REQUIRES: `output` is non-null.
//
// NOTE: `output` is out-only (any existing value is ignored).
// *output will be cleared before any data is read into it.
//
// NOTE: may write into *output AND return non-OK status if (for example)
// there was a read error encountered partway through the file.
//
// Typical return codes (not guaranteed exhaustive):
//  * OK
//  * UNKNOWN (a Read or Open error occurred)
absl::Status GetContents(absl::string_view file_name, std::string* output,
                         const Options& options = Defaults());

// Reads the contents of `file_name` and returns them unless an error occurred.
//
// Return codes:
//  * OK
//  * UNKNOWN (a Read or Open error occurred)
absl::StatusOr<std::string> GetContents(absl::string_view file_name,
                                        const Options& options = Defaults());

// Return true if path is an absolute path.
bool IsAbsolutePath(absl::string_view path);

// Combines all parameters as if they were path elements.
// Cleans the result: '//' -> '/', no trailing '/' but preserves a single
// starting '/' to handle absolute files. This means the function removes
// empty parameters.
template <typename... T>
inline std::string JoinPath(absl::string_view path1, const T&... args) {
  return internal::JoinPathSimplify([&] {
    if constexpr (sizeof...(args) == 0) {
      return std::string(path1);
    } else if (path1.empty()) {
      return JoinPath(args...);
    } else {
      return absl::StrCat(path1, "/", JoinPath(args...));
    }
  }());
}

}  // namespace oss
}  // namespace file

#endif  // PROTO_BUILDER_OSS_FILE_H_
