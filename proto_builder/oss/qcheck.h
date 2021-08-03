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

#ifndef PROTO_BUILDER_OSS_QCHECK_H_
#define PROTO_BUILDER_OSS_QCHECK_H_

#include <sstream>

#include "proto_builder/oss/source_location.h"

namespace proto_builder::oss {

// Options for the QCheck class.
enum class QCheckOptions : uint32_t {
  kNoOptionsEnabled = 0,
  kAbortOnError = 1 << 0,
  kPrintToStandardError = 1 << 1,
  kDefault = kAbortOnError | kPrintToStandardError,
};

// QCheckOptions is specified as a bitmask type, which means the
// following operations must be provided:
inline constexpr QCheckOptions operator&(QCheckOptions lhs, QCheckOptions rhs) {
  return static_cast<QCheckOptions>(static_cast<int>(lhs) &
                                    static_cast<int>(rhs));
}
inline constexpr QCheckOptions operator|(QCheckOptions lhs, QCheckOptions rhs) {
  return static_cast<QCheckOptions>(static_cast<int>(lhs) |
                                    static_cast<int>(rhs));
}
inline constexpr QCheckOptions operator^(QCheckOptions lhs, QCheckOptions rhs) {
  return static_cast<QCheckOptions>(static_cast<int>(lhs) ^
                                    static_cast<int>(rhs));
}
inline constexpr QCheckOptions operator~(QCheckOptions arg) {
  return static_cast<QCheckOptions>(~static_cast<int>(arg));
}
inline QCheckOptions& operator&=(QCheckOptions& lhs, QCheckOptions rhs) {
  lhs = lhs & rhs;
  return lhs;
}
inline QCheckOptions& operator|=(QCheckOptions& lhs, QCheckOptions rhs) {
  lhs = lhs | rhs;
  return lhs;
}
inline QCheckOptions& operator^=(QCheckOptions& lhs, QCheckOptions rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

// The QCheck class is a class that evaluates a predicate and prints a message
// to the standard out or standard error console when a predicate does not hold
// true. Moreover, the class has the capability of aborting the program when the
// predicate does not hold true.
//
// Example Usage:
//   QCheck(2 != 1, "2 != 1") << "Silly Rabbit!";
//
// May have the following print statement on standard error:
//  File: 'foo.c' Line: 42
//  Check failed: 2 != 1
//  Silly Rabbit!
//  <Program aborts>
class QCheck {
 public:
  QCheck(bool result, const char* condition,
         QCheckOptions options = QCheckOptions::kDefault,
         SourceLocation source_location = SourceLocation::current());

  template <typename T>
  QCheck& operator<<(T val) {
    message_ << val;
    return *this;
  }

  // When the instance is deconstructed, if the result is false, then the
  // instance prints its message to the appropriate console.
  //
  // The format of the message is:
  // File: '{filename}' Line: {line_number}
  // Check failed: {condition}
  // {message}
  ~QCheck();

 private:
  bool result_;
  QCheckOptions options_;
  std::ostringstream message_;
};

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_QCHECK_H_
