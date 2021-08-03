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

#include "proto_builder/util.h"

#include <string>
#include <utility>

#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/proto_builder.pb.h"
#include "google/protobuf/descriptor.pb.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"

namespace proto_builder {

static void TitlecaseString(std::string* s, absl::string_view delimiters) {
  bool upper = true;
  for (auto& ch : *s) {
    if (upper) {
      ch = absl::ascii_toupper(ch);
    }
    upper = absl::StrContains(delimiters, ch);
  }
}

std::string AbsoluteCppTypeName(const std::string& type) {
  if (absl::StartsWith(type, "std::")) {
    return type;
  }
  // translations from proto2 and proto3 types to Cpp native types
  static const auto& translation =
      *new absl::flat_hash_map<const std::string, const std::string>{
          {"int32", "int32_t"},    {"int64", "int64_t"},
          {"uint32", "uint32_t"},  {"uint64", "uint64_t"},
          {"sint32", "int32_t"},   {"sint64", "int64_t"},
          {"fixed32", "uint32_t"}, {"fixed64", "uint64_t"},
          {"sfixed32", "int32_t"}, {"sfixed64", "int64_t"},
      };
  auto iter = translation.find(type);
  if (iter != translation.end()) {
    return iter->second;
  }
  const std::string c = absl::StrReplaceAll(type, {{".", "::"}});
  return c[0] != ':' && c.find(':', 2) != std::string::npos
             ? absl::StrCat("::", c)
             : c;
}

std::string CamelCaseName(const FieldDescriptor& field_descriptor) {
  std::string field_name = field_descriptor.name();
  TitlecaseString(&field_name, "_");
  return absl::StrReplaceAll(field_name, {{"_", ""}});
}

bool IsMessage(const FieldDescriptor& field_descriptor) {
  return field_descriptor.cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE;
}

bool IsNonRepeatedMessage(const FieldDescriptor& field_descriptor) {
  return IsMessage(field_descriptor) && !field_descriptor.is_repeated();
}

absl::optional<const ::google::protobuf::Descriptor*> MaybeGetMapValueDescriptor(
    const FieldDescriptor& field_descriptor) {
  if (!field_descriptor.is_map()) {
    return absl::nullopt;
  }
  // "value" is the name of value field in the generated map-entry type.
  // See: https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/descriptor.proto
  const FieldDescriptor* map_value_field_descriptor =
      field_descriptor.message_type()->FindFieldByName("value");
  LOG_IF(FATAL, map_value_field_descriptor == nullptr)
      << "Oops, we couldn't find a field name 'value' in the ::google::protobuf::Descriptor for "
      << "the map entry type for field '" << field_descriptor.full_name()
      << "'. That shouldn't happen. There is either a programming error or "
      << "there may have been a change in how the proto map is represented in "
      << "the descriptor.";
  if (map_value_field_descriptor->cpp_type() !=
      FieldDescriptor::CPPTYPE_MESSAGE) {
    return absl::nullopt;
  }
  return map_value_field_descriptor->message_type();
}

FieldBuilderOptions MergeFieldBuilderOptions(
    const FieldBuilderOptions& from_field,
    const FieldBuilderOptions& defaults) {
  FieldBuilderOptions result(defaults);
  result.MergeFrom(from_field);
  if (!defaults.type().empty()) {
    result.set_type(defaults.type());
  }
  return result;
}

FieldBuilderOptions UpdateFieldBuilderOptions(
    const FieldBuilderOptions& options, const FieldDescriptor& field) {
  FieldBuilderOptions result(options);
  if (!result.type().empty()) {
    std::string type = result.type();
    absl::StrReplaceAll({{"@type@", GetFieldType(field)}}, &type);
    result.set_type(type);
  }
  return result;
}

std::pair<const FieldDescriptor*, const FieldDescriptor*> GetKeyValueTypes(
    const FieldDescriptor& field) {
  // Map types hide their actual type and do not provide a workable alias.
  // Instead the accessors refer to proto's Map type with the two field types.
  const ::google::protobuf::Descriptor& message_type = *PBCC_DIE_IF_NULL(field.message_type());
  const FieldDescriptor* key_type = PBCC_DIE_IF_NULL(message_type.field(0));
  const FieldDescriptor* value_type = PBCC_DIE_IF_NULL(message_type.field(1));
  return {key_type, value_type};
}

std::string GetFieldType(const FieldDescriptor& field) {
  if (field.is_map()) {
    const auto [key_type, value_type] = GetKeyValueTypes(field);
    return absl::StrCat("::proto2::Map<", GetFieldType(*key_type), ", ",
                        GetFieldType(*value_type), ">::value_type");
  }
  switch (field.cpp_type()) {
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return AbsoluteCppTypeName(field.message_type()->full_name());
    case FieldDescriptor::CPPTYPE_ENUM:
      return AbsoluteCppTypeName(field.enum_type()->full_name());
    case FieldDescriptor::CPPTYPE_STRING:
      if (absl::string_view("string") == field.cpp_type_name()) {
        return "std::string";
      }
      ABSL_FALLTHROUGH_INTENDED;
    default:
      return AbsoluteCppTypeName(field.cpp_type_name());
  }
}

std::string GetOptionsType(const FieldBuilderOptions& options,
                           const FieldDescriptor& field) {
  if (options.has_type()) {
    return options.type();
  }
  return GetFieldType(field);
}

const FieldBuilderOptions& GetFieldBuilderOptionsOrDefault(
    const FieldDescriptor& field_descriptor, int index) {
  return field_descriptor.options().ExtensionSize(field) > index
             ? field_descriptor.options().GetExtension(field, index)
             : FieldBuilderOptions::default_instance();
}

}  // namespace proto_builder
