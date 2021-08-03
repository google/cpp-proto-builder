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

#include "proto_builder/oss/qcheck.h"

#include <iostream>

namespace proto_builder::oss {

QCheck::QCheck(bool result, const char* condition, QCheckOptions options,
               SourceLocation source_location)
    : result_(result), options_(options) {
  message_ << "File: '" << source_location.file_name()
           << "', Line: " << source_location.line() << "\n";
  message_ << "Check failed: " << condition << "\n";
}

QCheck::~QCheck() {
  if (!result_) {
    if ((options_ & QCheckOptions::kPrintToStandardError) ==
        QCheckOptions::kPrintToStandardError) {
      std::cerr << message_.str() << "\n";
    } else {
      std::cout << message_.str() << "\n";
    }
    if ((options_ & QCheckOptions::kAbortOnError) ==
        QCheckOptions::kAbortOnError) {
      std::abort();
    }
  }
}

}  // namespace proto_builder::oss
