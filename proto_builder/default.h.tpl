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

#ifndef {{HEADER_GUARD}}  // NOLINT
#define {{HEADER_GUARD}}  // NOLINT

{{#INCLUDES}}
{{INCLUDE}}
{{/INCLUDES}}

{{#ALL_NAMESPACES}}
namespace {{NAMESPACE}} {
{{/ALL_NAMESPACES}}

{{#BUILDER}}
class {{CLASS_NAME}}{{BASE_CLASSES}} {
 public:
  {{#NOT_STATUS}}
  {{CLASS_NAME}}() = default;
  explicit {{CLASS_NAME}}(const {{PROTO_TYPE}}& data) : data_(data) {}
  explicit {{CLASS_NAME}}({{PROTO_TYPE}}&& data) : data_(data) {}
  {{/NOT_STATUS}}
  {{#USE_STATUS}}
  explicit {{CLASS_NAME}}({{%SourceLocation+param=value}})
      : source_location_({{%SourceLocation%param}}) {}
  explicit {{CLASS_NAME}}(const {{PROTO_TYPE}}& data,
                          {{%SourceLocation+param=value}})
      : source_location_({{%SourceLocation%param}}), data_(data) {}
  explicit {{CLASS_NAME}}({{PROTO_TYPE}}&& data,
                          {{%SourceLocation+param=value}})
      : source_location_({{%SourceLocation%param}}), data_(data) {}
  {{/USE_STATUS}}
  {{#USE_BUILD}}

  {{%StatusOr}}<{{PROTO_TYPE}}> Build() const;
  {{%StatusOr}}<{{PROTO_TYPE}}> Consume();
  {{/USE_BUILD}}
  {{#USE_STATUS}}
  {{%StatusOr}}<{{PROTO_TYPE}}> MaybeGetRawData() const;
  {{/USE_STATUS}}
  {{#USE_CONVERSION}}

  operator const {{PROTO_TYPE}}&() const {  // NOLINT
    {{VALIDATE_DATA}}
    {{/USE_CONVERSION}}
    {{#USE_STATUS}}
    if (!status_.ok()) {
      return {{PROTO_TYPE}}::default_instance();
    }
    {{/USE_STATUS}}
    {{#USE_CONVERSION}}
    return data_;
  }
  {{/USE_CONVERSION}}
  {{#USE_STATUS}}

  bool ok() const {
    {{VALIDATE_DATA}}
    return status_.ok();
  }

  {{%Status}} status() const {
    {{VALIDATE_DATA}}
    return status_;
  }

  {{CLASS_NAME}}& UpdateStatus({{%Status+param}});
  {{/USE_STATUS}}

  // https://google.github.io/cpp-proto-builder/templates#BEGIN

  // {{GENERATED_HEADER_CODE}}
  // https://google.github.io/cpp-proto-builder/templates#END
  {{#USE_VALIDATOR}}

 protected:
  void ValidateData() const;
  {{/USE_VALIDATOR}}

 private:
  {{#USE_STATUS}}
  const {{%SourceLocation}} source_location_;
  {{/USE_STATUS}}
  {{PROTO_TYPE}} data_;
  {{#USE_STATUS}}
  mutable {{%Status}} status_;
  bool get_raw_data_ = true;
  {{/USE_STATUS}}
};

{{/BUILDER}}
{{#ALL_NAMESPACES}}
}  // namespace {{NAMESPACE}}

{{/ALL_NAMESPACES}}
#endif  // {{HEADER_GUARD}} // NOLINT
