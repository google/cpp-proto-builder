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

#ifndef PROTO_BUILDER_FIELD_BUILDER_H_
#define PROTO_BUILDER_FIELD_BUILDER_H_

#include <memory>
#include <string>

#include "proto_builder/builder_writer.h"
#include "proto_builder/proto_builder.pb.h"
#include "proto_builder/proto_builder_config.h"
#include "proto_builder/util.h"
#include "google/protobuf/descriptor.h"
#include "absl/strings/str_join.h"

namespace proto_builder {

using ::google::protobuf::FieldDescriptor;

// Contains all const data necessary to write a single field.
struct FieldData {
  const ProtoBuilderConfigManager& config;
  BuilderWriter* const writer;
  // copy (likely created from temp object)
  const FieldBuilderOptions raw_field_options;
  const FieldDescriptor& field;
  const std::string class_name;
  const std::string data_parent;
  const std::string name_parent;
  const bool use_get_raw_data = false;
  const bool make_interface = false;
  const bool first_method = false;
  const bool use_status = false;

  std::string DebugString() const {
    return absl::StrJoin(
        {absl::StrCat("Field: ", field.full_name()),
         absl::StrCat("Options: ", raw_field_options.ShortDebugString()),
         absl::StrCat("class_name: ", class_name),
         absl::StrCat("data_parent: ", data_parent),
         absl::StrCat("name_parent: ", name_parent)},
        "\n");
  }
};

// Class that writes the code for a single field.
// The only public method is WriteField which gets invoked directly after
// constructing an instance, which can only be performed by a MessageBuilder.
class FieldBuilder {
 public:
  // Writes the code for this field using Write().
  void WriteField() const;

 private:
  explicit FieldBuilder(const FieldData& data);

  bool UseTemplate() const;
  bool UseForeach() const;
  bool UseForeachAdd() const;
  bool UseInitializerList() const;
  bool UseHeader() const;
  bool UseSource() const;
  bool UseMapInsert() const;

  // Returns name_parent appended with name if not empty or field_.name()
  // converted into a CamelCase name.
  std::string CamelCaseFieldName(const std::string& name = "") const;

  // Get field type relative to package_path (via data_.writer->CodeInfo).
  std::string GetRelativeFieldType() const;

  // Return the conversion expression 'type(value)' if type has a conversion,
  // or 'value' if no conversion exists. The conversion may use placeholders:
  // @type@: will be replaced with the field type (useful as template parameter)
  // @value@: will be replaced with the value or variable to be converted.
  std::string ApplyData(std::string input, const std::string& value) const;

  // Returns the translated options type: type_info_->type if that exists
  // or GetOptionsType(). This is still absolute or whatever was configured
  // and should be turned into a relative type.
  std::string GetRawCppType() const;

  // Returns the const& decorated type if decorate is true, else the plain type.
  std::string Decorate(bool decorate, const std::string& type) const;

  // Returns the type to use. If 'decorate' is true, then this is for the
  // function definition and the type will be 'decorated_type' vs. 'type'. If
  // 'decorated_type' does not exist, then 'const ...&' will be used as needed.
  std::string ParameterType(bool decorate) const;

  std::string MethodName() const;
  std::string MethodParam(Where to) const;

  // The expression used in set and assignments.
  std::string SetValue() const;
  std::string Predicate() const;

  void AddIncludes(const FieldBuilderOptions& options) const;
  void AddIncludes() const;

  bool UseSetFromBuilder() const;

  void WriteTemplateLine(Where to) const;
  void WriteDeclaration(Where to) const;
  void WriteSetFromBuilder() const;
  void WriteBody(Where to) const;
  void WriteImplementation(Where to) const;
  void WritePredicate(Where to) const;

  // Writes an '#error...<error>' line. The error message should be the plain
  // error message without any additional field info, which will be appended
  // automatically.
  void WriteError(const std::string& error) const;

  // Returns true if state is valid or writes an error.
  bool IsValidOrWriteError() const;

  template <class... Args>
  void Write(Where to, const Args&... code) const {
    data_.writer->Write(to, absl::StrCat(code...));
  }

  const FieldData data_;
  const FieldBuilderOptions* type_info_;
  const FieldBuilderOptions options_;

  friend class FieldBuilderTest;
  friend class MessageBuilder;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_FIELD_BUILDER_H_
