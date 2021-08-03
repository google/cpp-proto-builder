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

#ifndef PROTO_BUILDER_OSS_SOURCE_LOCATION_H_
#define PROTO_BUILDER_OSS_SOURCE_LOCATION_H_

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/str_cat.h"

namespace proto_builder::oss {

// Class representing a specific location in the source code of a program.
class SourceLocation {
 public:
  // Avoid this constructor; it populates the object with dummy values.
  constexpr SourceLocation() : line_(0), file_name_(nullptr) {}

  // SourceLocation::current
  //
  // Creates a `SourceLocation` based on the current line and file.  APIs that
  // accept a `SourceLocation` as a default parameter can use this to capture
  // their caller's locations.
  //
  // Example:
  //
  //   void TracedAdd(int i, SourceLocation loc = SourceLocation::current()) {
  //     std::cout << loc.file_name() << ":" << loc.line() << " added " << i;
  //     ...
  //   }
  //
  //   void UserCode() {
  //     TracedAdd(1);
  //     TracedAdd(2);
  //   }
  static constexpr SourceLocation current(
      int line = __builtin_LINE(), const char* file_name = __builtin_FILE()) {
    return SourceLocation(line, file_name);
  }

  // The line number of the captured source location.
  constexpr int line() const { return line_; }

  // The file name of the captured source location.
  constexpr const char* file_name() const { return file_name_; }

 private:
  // `file_name` must outlive all copies of the `proto_builder::oss::SourceLocation` object,
  // so in practice it should be a string literal.
  constexpr SourceLocation(int line, const char* file_name)
      : line_(line), file_name_(file_name) {}

  int line_;
  const char* file_name_;
  friend void Load(int, char**);
};

inline void AddSourceLocationToStatus(SourceLocation src_loc,
                                      absl::Status& status) {
  status.SetPayload(
      "proto_builder/source_location",
      absl::Cord(absl::StrCat(src_loc.file_name(), ":", src_loc.line())));
}

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_SOURCE_LOCATION_H_
