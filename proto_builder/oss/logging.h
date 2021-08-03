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

#ifndef PROTO_BUILDER_OSS_LOGGING_H_
#define PROTO_BUILDER_OSS_LOGGING_H_

#include <memory>

#ifdef PBCC_LOGGING_MACROS
#include "proto_builder/oss/logging_macros.h"  // IWYU pragma: export
#endif  // PBCC_LOGGING_MACROS
#include "proto_builder/oss/qcheck.h"
#include "absl/base/attributes.h"
#include "absl/base/optimization.h"

// `PBCC_DIE_IF_NULL` compares its argument vs `nullptr` but *also*
// "returns" its argument.  It is useful in initializers. `PBCC_DIE_IF_NULL`
// works for both raw pointers and (compatible) smart pointers including
// `std::unique_ptr` and `std::shared_ptr`; more generally, it works for any
// type that can be compared to nullptr_t.  For types that aren't raw pointers,
// `PBCC_DIE_IF_NULL` returns a reference to its argument, preserving the
// value category. Example:
//
//   Foo() : bar_(PBCC_DIE_IF_NULL(MethodReturningUniquePtr())) {}
#define PBCC_DIE_IF_NULL(val)                                  \
  proto_builder::logging_internal::DieIfNull( \
      __FILE__, __LINE__, #val, (val))

namespace proto_builder::logging_internal {
// Helper for `PBCC_DIE_IF_NULL`.
//
// In C++11, all cases can be handled by a single function. Since the value
// category of the argument is preserved (also for rvalue references), member
// initializer lists like the one below will compile correctly:
//
//   Foo() : bar_(PBCC_DIE_IF_NULL(MethodReturningUniquePtr())) {}
template <typename T>
ABSL_MUST_USE_RESULT ABSL_ATTRIBUTE_NOINLINE T
DieIfNull(const char* file, int line, const char* exprtext, T&& t) {
  oss::QCheck(t != nullptr, exprtext)
      << "'" << exprtext << "' must be non-null.";
  return std::forward<T>(t);
}

}  // namespace proto_builder::logging_internal

#endif  // PROTO_BUILDER_OSS_LOGGING_H_
