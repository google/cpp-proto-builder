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

#include "proto_builder/field_builder.h"

#include <map>
#include <string>

#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "proto_builder/builder_writer.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/proto_builder_config.h"
#include "proto_builder/util.h"
#include "google/protobuf/descriptor.pb.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/substitute.h"

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
    return absl::StrCat("::google::protobuf::Map<", GetFieldType(*key_type), ", ",
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

std::string DefaultFieldValueAsString(const FieldDescriptor& descriptor) {
  if (!descriptor.has_default_value()) {
    return "{}";
  }
  switch (descriptor.cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return absl::StrCat(descriptor.default_value_int32());
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      return absl::StrCat(descriptor.default_value_int64());
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      return absl::StrCat(descriptor.default_value_uint32());
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      return absl::StrCat(descriptor.default_value_uint64());
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      return absl::StrCat(descriptor.default_value_float());
      break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return absl::StrCat(descriptor.default_value_double());
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      return descriptor.default_value_bool() ? "true" : "false";
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      return absl::StrCat(
          "\"", absl::CEscape(descriptor.default_value_string()), "\"");
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      return descriptor.default_value_enum()->name();
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      break;
  }
  return "{}";
}

FieldBuilder::FieldBuilder(const FieldData& data)
    : data_(data),
      type_info_(data.config.GetTypeInfo(
          GetOptionsType(data_.raw_field_options, data_.field))),
      options_(UpdateFieldBuilderOptions(
          MergeFieldBuilderOptions(
              data.raw_field_options,
              type_info_ ? *type_info_
                         : FieldBuilderOptions::default_instance()),
          data_.field)) {}

bool FieldBuilder::UseTemplate() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;
    case FieldBuilderOptions::HEADER:           return false;
    case FieldBuilderOptions::SOURCE:           return false;
    case FieldBuilderOptions::BOTH:             return false;
    case FieldBuilderOptions::TEMPLATE:         return true;
    case FieldBuilderOptions::FOREACH:          return true;
    case FieldBuilderOptions::FOREACH_ADD:      return true;
    case FieldBuilderOptions::INITIALIZER_LIST: return true;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseForeach() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;;
    case FieldBuilderOptions::HEADER:           return false;
    case FieldBuilderOptions::SOURCE:           return false;
    case FieldBuilderOptions::BOTH:             return false;
    case FieldBuilderOptions::TEMPLATE:         return false;
    case FieldBuilderOptions::FOREACH:          return true;
    case FieldBuilderOptions::FOREACH_ADD:      return true;
    case FieldBuilderOptions::INITIALIZER_LIST: return true;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseForeachAdd() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;
    case FieldBuilderOptions::HEADER:           return false;
    case FieldBuilderOptions::SOURCE:           return false;
    case FieldBuilderOptions::BOTH:             return false;
    case FieldBuilderOptions::TEMPLATE:         return false;
    case FieldBuilderOptions::FOREACH:          return false;
    case FieldBuilderOptions::FOREACH_ADD:      return true;
    case FieldBuilderOptions::INITIALIZER_LIST: return true;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseInitializerList() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;
    case FieldBuilderOptions::HEADER:           return false;
    case FieldBuilderOptions::SOURCE:           return false;
    case FieldBuilderOptions::BOTH:             return false;
    case FieldBuilderOptions::TEMPLATE:         return false;
    case FieldBuilderOptions::FOREACH:          return false;
    case FieldBuilderOptions::FOREACH_ADD:      return false;
    case FieldBuilderOptions::INITIALIZER_LIST: return true;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseHeader() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;
    case FieldBuilderOptions::HEADER:           return true;
    case FieldBuilderOptions::SOURCE:           return false;
    case FieldBuilderOptions::BOTH:             return true;
    case FieldBuilderOptions::TEMPLATE:         return true;
    case FieldBuilderOptions::FOREACH:          return true;
    case FieldBuilderOptions::FOREACH_ADD:      return true;
    case FieldBuilderOptions::INITIALIZER_LIST: return true;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseSource() const {
  switch (options_.output()) {  // clang-format off
    case FieldBuilderOptions::SKIP:             break;
    case FieldBuilderOptions::HEADER:           return false;
    case FieldBuilderOptions::SOURCE:           return true;
    case FieldBuilderOptions::BOTH:             return true;
    case FieldBuilderOptions::TEMPLATE:         return false;
    case FieldBuilderOptions::FOREACH:          return false;
    case FieldBuilderOptions::FOREACH_ADD:      return false;
    case FieldBuilderOptions::INITIALIZER_LIST: return false;
  }  // clang-format on
  return false;
}

bool FieldBuilder::UseSetFromBuilder() const {
  if (!data_.first_method || !data_.use_get_raw_data) {
    return false;
  }
  if (!data_.field.is_map()) {
    return data_.field.message_type();
  }
  const ::google::protobuf::Descriptor& message_type = *data_.field.message_type();
  const FieldDescriptor& value_type = *PBCC_DIE_IF_NULL(message_type.field(1));
  return value_type.message_type();
}

bool FieldBuilder::UseMapInsert() const {
  return data_.field.is_map() && (UseInitializerList() || !UseForeachAdd());
}

std::string FieldBuilder::CamelCaseFieldName(const std::string& name) const {
  return absl::StrCat(data_.name_parent,
                      !name.empty() ? name : CamelCaseName(data_.field));
}

std::string FieldBuilder::GetRelativeFieldType() const {
  const auto& code_info = *data_.writer->CodeInfo();
  return code_info.RelativeType(GetFieldType(data_.field));
}

std::string FieldBuilder::ApplyData(std::string input,
                                    const std::string& value) const {
  if (input.empty()) {
    return value;
  }
  std::vector<std::pair<std::string, std::string>> replacements;
  replacements.reserve(options_.data().size() + 4);
  replacements.emplace_back("@type@", GetRelativeFieldType());
  replacements.emplace_back("@value@", value);
  replacements.emplace_back("@default@",
                            DefaultFieldValueAsString(data_.field));
  replacements.emplace_back(
      "@source_location@",
      data_.config.GetExpandedType(data_.raw_field_options.add_source_location()
                                       ? "%SourceLocation%param"
                                       : "%SourceLocation%value"));
  for (const auto& [key, value] : options_.data()) {
    replacements.emplace_back(absl::StrCat("%", key, "%"), value);
  }
  absl::StrReplaceAll(replacements, &input);
  return input;
}

std::string FieldBuilder::GetRawCppType() const {
  return !options_.type().empty() ? options_.type() : GetFieldType(data_.field);
}

std::string FieldBuilder::Decorate(bool decorate,
                                   const std::string& type) const {
  decorate &= type != "absl::string_view";
  decorate &= *type.crbegin() != '*';
  decorate &= *type.crbegin() != '&';
  return decorate ? absl::StrCat("const ", type, "&") : type;
}

std::string FieldBuilder::ParameterType(bool decorate) const {
  if (UseInitializerList()) {
    const std::string item_type =
        data_.field.is_map() ? GetRawCppType() : "Item";
    return absl::StrCat("std::initializer_list<", item_type, ">");
  } else if (UseTemplate()) {
    return Decorate(decorate, UseForeach() ? "Container" : "Value");
  } else if (decorate && !options_.decorated_type().empty()) {
    return options_.decorated_type();
  } else {
    const std::string type = GetRawCppType();
    decorate &= (absl::StrContains(type, "::") || type == "string");
    decorate &= data_.field.type() != FieldDescriptor::TYPE_ENUM;
    const auto& code_info = *data_.writer->CodeInfo();
    return Decorate(decorate, code_info.RelativeType(type));
  }
}

std::string FieldBuilder::MethodName() const {
  if (data_.field.is_map()) {
    return absl::StrCat("Insert", CamelCaseFieldName(options_.name()));
  } else if (data_.field.is_repeated()) {
    return absl::StrCat("Add", CamelCaseFieldName(options_.name()));
  } else {
    return absl::StrCat("Set", CamelCaseFieldName(options_.name()));
  }
}

std::string FieldBuilder::MethodParam(Where to) const {
  std::string param;
  if (options_.value().empty()) {
    param = absl::StrCat(ParameterType(true), " ",
                         data_.field.is_map() ? "key_value_pair" : "value",
                         UseForeach() ? "s" : "");
  }
  if (options_.add_source_location()) {
    const FieldBuilderOptions* src_loc_options = data_.config.GetTypeInfo(
        "%SourceLocation", ProtoBuilderTypeInfo::kSpecial);
    if (src_loc_options) {
      if (!param.empty()) {
        absl::StrAppend(&param, ", ");
      }
      absl::StrAppend(&param, src_loc_options->type(), " ",
                      data_.config.GetExpandedType("%SourceLocation%param"));
      if (to == HEADER && !src_loc_options->value().empty()) {
        absl::StrAppend(&param, " = ", src_loc_options->value());
      }
    }
  }
  return param;
}

std::string FieldBuilder::SetValue() const {
  return ApplyData(
      options_.conversion(),
      options_.value().empty()
          ? (UseMapInsert() && (!UseForeach() || options_.conversion().empty())
                 ? absl::StrCat("key_value_pair", UseForeach() ? "s" : "")
                 : (UseForeach() ? "v" : "value"))
          : options_.value());
}

std::string FieldBuilder::Predicate() const {
  return ApplyData(options_.predicate(),
                   options_.value().empty()
                       ? (UseMapInsert() ? absl::StrCat("key_value_pair",
                                                        UseForeach() ? "s" : "")
                                         : "value")
                       : options_.value());
}

void FieldBuilder::WriteTemplateLine(Where to) const {
  if (UseInitializerList()) {
    // map types cannot use a template with initializer lists due to the values
    // being compound types.
    if (!data_.field.is_map()) {
      Write(to, "template <class Item>");
    }
  } else if (UseTemplate()) {
    if (UseForeach()) {
      Write(to,
            absl::Substitute(
                "template <class $0, class = typename "
                "std::enable_if<!std::is_convertible<$0, $1>::value>::type>",
                ParameterType(false), GetRelativeFieldType()));
    } else {
      Write(to, "template <class ", ParameterType(false), ">");
    }
  }
}

void FieldBuilder::WriteDeclaration(Where to) const {
  WriteTemplateLine(to);
  const bool is_virtual = to == INTERFACE && data_.make_interface;
  const bool is_override =  // Needed only in the actual builder
      (to != INTERFACE && data_.make_interface) ||
      data_.raw_field_options.override();
  const bool is_abstract = to == INTERFACE && data_.make_interface;
  const std::string prefix = is_virtual ? "virtual " : "";
  const std::string suffix = is_override   ? " override"
                             : is_abstract ? " =0"
                                           : "";
  Write(to, prefix, data_.class_name, "& ", MethodName(), "(",
        MethodParam(HEADER), ")", suffix, ";");
  if (!data_.make_interface) {
    WriteSetFromBuilder();
  }
}

void FieldBuilder::WriteSetFromBuilder() const {
  if (!UseSetFromBuilder()) {
    return;
  }
  // TODO Add map support
  std::string type = GetRawCppType();
  std::string params = "Builder builder";
  std::string args = "*std::move(value)";
  if (data_.field.is_map()) {
    const auto [key_type, value_type] = GetKeyValueTypes(data_.field);
    type = GetFieldType(*value_type);
    const bool decorate = key_type->type() == FieldDescriptor::TYPE_STRING;
    params = absl::StrCat(Decorate(decorate, GetFieldType(*key_type)), " key, ",
                          params);
    args = absl::StrCat("{key, ", args, "}");
  }
  Write(HEADER, "");
  Write(HEADER, "template <");
  Write(HEADER, "    class Builder,");
  Write(HEADER, "    class = std::enable_if_t<std::is_same_v<");
  Write(HEADER, "        std::invoke_result_t<");
  Write(HEADER, "            decltype(&Builder::MaybeGetRawData), Builder>,");
  Write(HEADER, "        absl::StatusOr<", type, ">>>>");
  Write(HEADER, data_.class_name, "& ", MethodName(), "(", params, ") {");
  Write(HEADER, "  auto value = std::move(builder).MaybeGetRawData();");
  Write(HEADER, "  if (value.ok()) {");
  // We do not write conversions or predicates here since they will be handled
  // by the original setter.
  Write(HEADER, "    ", MethodName(), "(", args, ");");
  Write(HEADER, "  } else {");
  Write(HEADER, "    UpdateStatus(value.status());");
  Write(HEADER, "  }");
  Write(HEADER, "  return *this;");
  Write(HEADER, "}");
  Write(HEADER, "");
}

void FieldBuilder::WriteBody(Where to) const {
  const std::string field_name = google::protobuf::compiler::cpp::FieldName(&data_.field);
  if (UseMapInsert()) {
    if (UseForeach()) {
      if (options_.conversion().empty()) {
        Write(to, "  ", data_.data_parent, "mutable_", field_name,
              "()->insert(", SetValue(), ".begin(), ", SetValue(), ".end());");
        return;
      }
    } else {
      Write(to, "  ", data_.data_parent, "mutable_", field_name, "()->insert(",
            SetValue(), ");");
      return;
    }
  }
  if (data_.field.is_repeated()) {
    const std::string add_method =
        absl::StrCat(data_.data_parent, "add_", field_name);
    const std::string add_value =
        IsMessage(data_.field)
            ? absl::StrCat("*", add_method, "() = ", SetValue(), ";")
            : absl::StrCat(add_method, "(", SetValue(), ");");
    if (UseForeach() || data_.field.is_map()) {
      Write(to, "  for (const auto& v : ",
            (data_.field.is_map() ? "key_value_pairs" : "values"), ") {");
      if (UseForeachAdd()) {
        std::string source_location =
            options_.add_source_location()
                ? absl::StrCat(", ", data_.config.GetExpandedType(
                                         "%SourceLocation%param"))
                : "";
        if (data_.field.is_map() && options_.conversion().empty()) {
          Write(to, "    ", "Insert", CamelCaseFieldName(), "(",
                GetFieldType(data_.field), "(", SetValue(), ".first, ",
                SetValue(), ".second)", source_location, ");");
        } else {
          Write(to, "    ", (data_.field.is_map() ? "Insert" : "Add"),
                CamelCaseFieldName(), "(", SetValue(), source_location, ");");
        }
      } else {
        Write(to, "    ", add_value);
      }
      Write(to, "  }");
    } else {
      Write(to, "  ", add_value);
    }
  } else {
    const std::string set_value =
        IsMessage(data_.field)
            ? absl::StrCat("*", data_.data_parent, "mutable_", field_name,
                           "() = ", SetValue(), ";")
            : absl::StrCat(data_.data_parent, "set_", field_name, "(",
                           SetValue(), ");");
    Write(to, "  ", set_value);
  }
}

void FieldBuilder::WriteImplementation(Where to) const {
  const std::string function_name =
      to == HEADER ? MethodName()
                   : absl::StrCat(data_.class_name, "::", MethodName());
  Write(to, "");
  WriteTemplateLine(to);
  const bool is_override = to == HEADER && data_.raw_field_options.override();
  const std::string suffix = is_override ? " override" : "";
  Write(to, data_.class_name, "& ", function_name, "(", MethodParam(to), ")",
        suffix, " {");
  WritePredicate(to);
  WriteBody(to);
  Write(to, "  return *this;");
  Write(to, "}");
  Write(to, "");
}

void FieldBuilder::WritePredicate(Where to) const {
  if (data_.raw_field_options.predicate().empty()) {
    return;
  }
  if (data_.use_status) {
    // Whether the field will be set only depends on the predicate result. If it
    // is `OkStatus`, then the field will be set, otherwise it will not be set.
    // This is irrespective of the builder's overall status if `uses_status` is
    // active.
    // But if `use_status` is active, then the predicate will only set a non
    // `OkStatus` if the builder is still in `OkStatus`. In other words the
    // predicate can set an error status but it cannot reset the builder to an
    // `OkStatus`. The builder's status gets updated using `UpdateStatus()` as
    // that allows integration with full validation and conversions.
    // This is done so that `UpdateStatus()` can actually put the overall data
    // back into `OkStatus`.
    // This restriction is in place so that if two fields are being set and the
    // first set operation puts the builder into a non `OkStatus`, then the
    // second field's setting should not override that first error state.
    Write(to, "const auto status = ", Predicate(), ";");
    Write(to, "if (!status.ok()) {");
    Write(to, "  if (status_.ok()) {");
    Write(to, "    UpdateStatus(status);");
    Write(to, "  }");
  } else {
    // Do not save the result, so there is no reason to include "absl/status.h".
    Write(to, "if (!", Predicate(), ".ok()) {");
  }
  Write(to, "  return *this;");
  Write(to, "}");
}

void FieldBuilder::WriteError(const std::string& error) const {
  const std::string lines[] = {
      // clang-format off
    "",
    error,
    absl::StrCat("Field: ", data_.field.full_name()),
    absl::StrCat("FieldBuilderOptions: <", options_.ShortDebugString(), ">"),
    ""};  // clang-format on
  for (const auto& line : lines) {
    if (!line.empty()) {
      LOG(ERROR) << line;
    }
    const std::string l = line.empty() ? line : absl::StrCat("#error ", line);
    if (UseHeader()) {
      Write(HEADER, l);
    }
    if (UseSource()) {
      Write(SOURCE, l);
    }
    if (data_.make_interface) {
      Write(INTERFACE, l);
    }
  }
}

bool FieldBuilder::IsValidOrWriteError() const {
  const std::string output = absl::StrCat(
      "'output: ", FieldBuilderOptions::OutputMode_Name(options_.output()),
      "'");
  if (UseForeach() && !data_.field.is_repeated()) {
    WriteError(
        absl::StrCat("Cannot use ", output, " with a non repeated field."));
    return false;
  }
  if (UseTemplate() && options_.has_value()) {
    WriteError(absl::StrCat("Cannot use ", output, " and specify a value."));
    // We could technically allow this, so the user might create a custom
    // assignment, say using a callback expression.
    return false;
  }
  return true;
}

void FieldBuilder::AddIncludes(const FieldBuilderOptions& options) const {
  auto* code_info = data_.writer->CodeInfo();
  for (auto& include : options.include()) {
    code_info->AddInclude(HEADER, include);
  }
  for (auto& include : options.source_include()) {
    code_info->AddInclude(SOURCE, include);
  }
}

void FieldBuilder::AddIncludes() const {
  auto* code_info = data_.writer->CodeInfo();
  switch (data_.field.cpp_type()) {
    case FieldDescriptor::CPPTYPE_MESSAGE:
      code_info->AddInclude(HEADER, *data_.field.message_type());
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      code_info->AddInclude(HEADER, *data_.field.enum_type());
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      // The default type for string fields is "std::string" which requires the
      // <string> include. However we don't need it if the type is overloaded.
      if (GetOptionsType(data_.raw_field_options, data_.field) ==
          "std::string") {
        code_info->AddInclude(HEADER, "<string>");
      }
      break;
    default:
      break;
  }
  AddIncludes(options_);
  if (options_.add_source_location()) {
    const FieldBuilderOptions* src_loc_options = data_.config.GetTypeInfo(
        "%SourceLocation", ProtoBuilderTypeInfo::kSpecial);
    if (src_loc_options) {
      AddIncludes(*src_loc_options);
    }
  }
}

void FieldBuilder::WriteField() const {
  if (options_.output() == FieldBuilderOptions::SKIP) {
    return;
  }
  if (!IsValidOrWriteError()) {
    return;
  }
  AddIncludes();
  if (UseHeader()) {
    if (UseTemplate() || UseForeach()) {
      WriteImplementation(HEADER);
    } else {
      WriteDeclaration(HEADER);
    }
  }
  if (UseSource()) {
    WriteImplementation(SOURCE);
  }
  if (data_.make_interface) {
    WriteDeclaration(INTERFACE);
  }
}

}  // namespace proto_builder
