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

// IWYU pragma: private, include "proto_builder/oss/logging.h"
// IWYU pragma: friend net/proto2/contrib/proto_builder/oss/*

#ifndef PROTO_BUILDER_OSS_LOGGING_MACROS_H_
#define PROTO_BUILDER_OSS_LOGGING_MACROS_H_

#include <iostream>
#include <ostream>
#include <string>

#include "proto_builder/oss/qcheck.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"

namespace proto_builder::oss {

template <typename T>
inline std::string StatusStr(const absl::StatusOr<T>& s) {
  return s.status().ToString();
}

inline std::string StatusStr(const absl::Status& s) { return s.ToString(); }

class LogMessage {
 public:
  explicit LogMessage(std::ostream& o, bool abort = false)
      : o_(o), abort_(abort) {}
  ~LogMessage() {
    o_ << "\n";
    std::flush(o_);
    if (abort_) {
      std::abort();
    }
  }

  std::ostream& stream() { return o_; }

 private:
  LogMessage() = delete;
  LogMessage(LogMessage&&) = delete;
  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;
  LogMessage& operator=(LogMessage&&) = delete;

  std::ostream& o_;
  const bool abort_;
};

}  // namespace proto_builder::oss

#undef _CONCAT
#undef CONCAT
#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)

#undef SPACE_TOKEN
#define SPACE_TOKEN " "

#undef QCHECK_CLASS
#define QCHECK_CLASS ::proto_builder::oss::QCheck

// `CHECK` and friends are syntactic sugar for the expression being tested and
// evaluates the values on either side. Example:
//   int x = 3, y = 5;
//   CHECK_EQ(2 * x, y) << "oops!";
//
// May produce a message on standard error like:
//  Check failed: 2 * x == 5
//  oops!
#undef CHECK_OK
#define CHECK_OK(expr)                                                        \
  auto CONCAT(var, __LINE__) = expr;                                          \
  QCHECK_CLASS(                                                               \
      CONCAT(var, __LINE__).ok(),                                             \
      ::proto_builder::oss::StatusStr(CONCAT(var, __LINE__)) \
          .c_str())

#undef CHECK
#define CHECK(expr) QCHECK_CLASS(expr, #expr)

#undef CHECK_CMP
#define CHECK_CMP(x, y, OP) \
  QCHECK_CLASS(x OP y, #x SPACE_TOKEN #OP SPACE_TOKEN #y)

#undef CHECK_EQ
#define CHECK_EQ(x, y) CHECK_CMP(x, y, ==)

#undef CHECK_GE
#define CHECK_GE(x, y) CHECK_CMP(x, y, >=)

#undef CHECK_GT
#define CHECK_GT(x, y) CHECK_CMP(x, y, >)

// `QCHECK` and friends are syntactic sugar for the expression being tested and
// evaluates the values on either side. Example:
//   int x = 3, y = 5;
//   QCHECK_EQ(2 * x, y) << "oops!";
//
// May produce a message on standard error like:
//  File: 'foo.c' Line: 42
//  Condition: '2 * x == 5'
//  oops!

#undef QCHECK
#define QCHECK(x) QCHECK_CLASS(x, #x)

#undef QCHECK_OK
#define QCHECK_OK(expr)                                                       \
  auto CONCAT(var, __LINE__) = expr;                                          \
  QCHECK_CLASS(                                                               \
      CONCAT(var, __LINE__).ok(),                                             \
      ::proto_builder::oss::StatusStr(CONCAT(var, __LINE__)) \
          .c_str())

#undef QCHECK_CMP
#define QCHECK_CMP(x, y, OP) \
  QCHECK_CLASS(x OP y, #x SPACE_TOKEN #OP SPACE_TOKEN #y)

#undef QCHECK_EQ
#define QCHECK_EQ(x, y) QCHECK_CMP(x, y, ==)

#undef QCHECK_GE
#define QCHECK_GE(x, y) QCHECK_CMP(x, y, >=)

#undef QLOG_ERROR
#define QLOG_ERROR std::cerr

#undef QLOG
#define QLOG(severity) QLOG_##severity

// `LOG_IF` and friends are syntactic sugar for the condition being tested.
// The severity can be:
//    INFO: describes expected events that are important for understanding the
//    state of the binary but which are not indicative of a problem.
//    WARNING: describes unexpected events which may indicate a problem.
//    ERROR: describes unexpected problematic events that the binary is able to
//    recover from.
//    FATAL: describes unrecoverable problems.
//
//    INFO: Prints log to standard out.
//    WARNING: Prints log to standard out.
//    ERROR: Prints log to standard error.
//    FATAL: Prints log to standard error, aborts the program.
//
// If the condition is false, nothing is logged.
// Example:
//
//   LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies!";
//
// May produce a message on standard error like:
//  File: 'foo.c' Line: 42
//  Condition: 'num_cookies > 10'
//  Got lots of cookies!
#undef LOG_IF_INFO
#define LOG_IF_INFO(condition) \
  QCHECK_CLASS(!(condition), #condition, QCheckOptions::kNoOptionsEnabled)

#undef LOG_IF_WARNING
#define LOG_IF_WARNING(condition) \
  QCHECK_CLASS(!(condition), #condition, QCheckOptions::kNoOptionsEnabled)

#undef LOG_IF_ERROR
#define LOG_IF_ERROR(condition) \
  QCHECK_CLASS(!(condition), #condition, QCheckOptions::kPrintToStandardError)

#undef LOG_IF_FATAL
#define LOG_IF_FATAL(condition) QCHECK_CLASS(!(condition), #condition)

#undef LOG_IF
#define LOG_IF(severity, condition) LOG_IF_##severity(condition)

#undef LOG_INFO
#define LOG_INFO \
  proto_builder::oss::LogMessage(std::cout).stream()

#undef LOG_ERROR
#define LOG_ERROR \
  proto_builder::oss::LogMessage(std::cerr).stream()

#undef LOG_FATAL
#define LOG_FATAL \
  proto_builder::oss::LogMessage(std::cerr, true).stream()

#undef LOG
#define LOG(severity) LOG_##severity

#endif  // PROTO_BUILDER_OSS_LOGGING_MACROS_H_
