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

#ifndef {{INTERFACE_GUARD}}  // NOLINT
#define {{INTERFACE_GUARD}}  // NOLINT

{{#INTERFACE_INCLUDES}}
{{INCLUDE}}
{{/INTERFACE_INCLUDES}}

{{#ALL_NAMESPACES}}
namespace {{NAMESPACE}} {
{{/ALL_NAMESPACES}}

{{#BUILDER}}
template <class {{CLASS_NAME}}>
class {{INTERFACE_NAME}} {
 public:
  {{INTERFACE_NAME}}() = default;
  virtual ~{{INTERFACE_NAME}}() = default;

  // https://google.github.io/cpp-proto-builder/templates#BEGIN

  // {{GENERATED_INTERFACE_CODE}}
  // https://google.github.io/cpp-proto-builder/templates#END
};

{{/BUILDER}}
{{#ALL_NAMESPACES}}
}  // namespace {{NAMESPACE}}

{{/ALL_NAMESPACES}}
#endif  // {{INTERFACE_GUARD}} // NOLINT
