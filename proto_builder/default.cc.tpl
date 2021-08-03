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

#include "{{HEADER_FILE}}"

{{#SOURCE_INCLUDES}}
{{INCLUDE}}
{{/SOURCE_INCLUDES}}

{{#ALL_NAMESPACES}}
namespace {{NAMESPACE}} {

{{/ALL_NAMESPACES}}
// https://google.github.io/cpp-proto-builder/templates#BEGIN
// {{#BUILDER}}
{{#USE_BUILD}}

{{%StatusOr}}<{{PROTO_TYPE}}> {{CLASS_NAME}}::Build() const {
  if (ok()) {
    return {{ROOT_DATA}};
  } else {
    return status_;
  }
}

{{%StatusOr}}<{{PROTO_TYPE}}> {{CLASS_NAME}}::Consume() {
  if (ok()) {
    return std::move({{ROOT_DATA}});
  } else {
    data_.Clear();
    {{%Status}} result(std::move(status_));
    status_ = {{%Status}}();
    get_raw_data_ = true;
    return result;
  }
}
{{/USE_BUILD}}
{{#USE_STATUS}}

{{%StatusOr}}<{{PROTO_TYPE}}> {{CLASS_NAME}}::MaybeGetRawData() const {
  if (get_raw_data_) {
    return data_;
  } else {
    return status_;
  }
}

{{CLASS_NAME}}& {{CLASS_NAME}}::UpdateStatus({{%Status+param}}) {
  status_ = std::move({{%Status%param}});
  if (status_.ok()) {
    get_raw_data_ = true;
    {{VALIDATE_DATA}}
  } else {
    get_raw_data_ = false;
    {{%LogSourceLocation}}
  }
  return *this;
}
{{/USE_STATUS}}
{{#USE_VALIDATOR}}

void {{CLASS_NAME}}::ValidateData() const {
  if (status_.ok()) {
    status_ = {{%Validate}}(data_);
    if (!status_.ok()) {
      {{%LogSourceLocation}}
    }
  }
}
{{/USE_VALIDATOR}}

// {{GENERATED_SOURCE_CODE}}
// {{/BUILDER}}
// https://google.github.io/cpp-proto-builder/templates#END

{{#ALL_NAMESPACES}}
}  // namespace {{NAMESPACE}}
{{/ALL_NAMESPACES}}
