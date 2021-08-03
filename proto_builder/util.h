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

#ifndef PROTO_BUILDER_UTIL_H_
#define PROTO_BUILDER_UTIL_H_

#include <string>

#include "proto_builder/proto_builder.pb.h"
#include "google/protobuf/descriptor.h"
#include "absl/types/optional.h"

namespace proto_builder {

using ::google::protobuf::FieldDescriptor;

// Convert the type into a c-type ('.' - > '::') and make it an absolute name
// if it has any "::".
std::string AbsoluteCppTypeName(const std::string& type);

// Convert field.name() into a CamelCaseName.
std::string CamelCaseName(const FieldDescriptor& field_descriptor);

// Returns FieldBuilderOptions at 'index' or the default.
const FieldBuilderOptions& GetFieldBuilderOptionsOrDefault(
    const FieldDescriptor& field_descriptor, int index = 0);

// Returns true if the field is a message field.
bool IsMessage(const FieldDescriptor& field_descriptor);

// Returns true if the field is a non repeated message field.
bool IsNonRepeatedMessage(const FieldDescriptor& field_descriptor);

// If the field is of type map with a value whose type is Message,
// then the ::google::protobuf::Descriptor for that message's type will be returned.
absl::optional<const ::google::protobuf::Descriptor*> MaybeGetMapValueDescriptor(
    const FieldDescriptor& field_descriptor);

// Merges 'from_field' options with 'default' options.
FieldBuilderOptions MergeFieldBuilderOptions(
    const FieldBuilderOptions& from_field, const FieldBuilderOptions& defaults);

// Updates the 'type' in case it contains '@type@'.
FieldBuilderOptions UpdateFieldBuilderOptions(
    const FieldBuilderOptions& options, const FieldDescriptor& field);

// Returns the actual underlying type of the field (message, enum, c++ type).
std::string GetFieldType(const FieldDescriptor& field);

// Returns options.type if specified, otherwise GetFieldType().
std::string GetOptionsType(const FieldBuilderOptions& options,
                           const FieldDescriptor& field);

}  // namespace proto_builder

#endif  // PROTO_BUILDER_UTIL_H_
