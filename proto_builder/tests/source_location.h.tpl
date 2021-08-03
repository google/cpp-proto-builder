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

// Automatically generated using https://google.github.io/cpp-proto-builder

#ifndef PROTO_BUILDER_TESTS_SOURCE_LOCATION_CC_PROTO_BUILDER_H_
#define PROTO_BUILDER_TESTS_SOURCE_LOCATION_CC_PROTO_BUILDER_H_

#include <string>

#include "proto_builder/tests/source_location.pb.h"  // IWYU pragma: export
#include "proto_builder/oss/source_location.h"

namespace proto_builder::tests {

// {{#BUILDER}}
class SourceLocationBuilder {
 public:
  SourceLocationBuilder() = default;

  proto_builder::oss::SourceLocation source_location() const { return source_location_; }

  SourceLocation data() const { return data_; }

  // https://google.github.io/cpp-proto-builder/templates#BEGIN

  // {{GENERATED_HEADER_CODE}}
  // https://google.github.io/cpp-proto-builder/templates#END

 private:
  SourceLocation data_;
  {{%SourceLocation+param}}_;
};

SourceLocationBuilder& SourceLocationBuilder::AddTarget(
    const std::string& value, {{%SourceLocation+param}}) {
  {{%SourceLocation%param}}_ = {{%SourceLocation%param}};
  data_.add_target(value);
  return *this;
}
// {{/BUILDER}}

}  // namespace proto_builder::tests

#endif  // PROTO_BUILDER_TESTS_SOURCE_LOCATION_CC_PROTO_BUILDER_H_
