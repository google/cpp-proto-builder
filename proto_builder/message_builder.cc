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

#include "proto_builder/message_builder.h"

#include <memory>
#include <string>

#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "proto_builder/field_builder.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/proto_builder_config.h"
#include "google/protobuf/descriptor.pb.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"

namespace proto_builder {

// Max recursion depth for sub-field setters.  We never expand more than five
// messages deep when generating setters.  Past this depth, we feel the setters
// start to become particularly unwieldy.
static const int kMaxSubFieldSetterDepth = 5;

std::pair<std::string, std::string> GetPackageAndClassName(
    const ::google::protobuf::Descriptor* descriptor) {
  std::string name = PBCC_DIE_IF_NULL(descriptor)->name();
  while (descriptor->containing_type()) {
    descriptor = descriptor->containing_type();
    name = absl::StrCat(descriptor->name(), "_", name);
  }
  const auto pos = descriptor->full_name().rfind('.');
  const std::string package =
      pos == std::string::npos ? "" : descriptor->full_name().substr(0, pos);
  return {package, name};
}

MessageBuilder::MessageBuilder(Options options)
    : options_(options),
      writer_(OwnWrappedWriter<NoDoubleEmptyLineWriter>::New(
          std::make_unique<IndentWriter>(options_.writer, "  "))),
      root_descriptor_(options_.descriptor),
      root_options_([&] {
        // May need to override options.
        MessageBuilderOptions root_options =
            options_.descriptor.options().GetExtension(message);
        if (!root_options.has_use_validator()) {
          root_options.set_use_validator(options.use_validator);
        }
        return root_options;
      }()),
      class_name_(!root_options_.class_name().empty()
                      ? root_options_.class_name()
                      : absl::StrCat(
                            GetPackageAndClassName(&options_.descriptor).second,
                            "Builder")) {
  CHECK_GT(class_name_.size(), 0);
}

void MessageBuilder::WriteBuilder() {
  messages_in_subfield_setter_stack_.clear();
  writer_->CodeInfo()->AddInclude(HEADER, root_descriptor_);
  WriteMessage(root_descriptor_, root_options().root_data(),
               root_options().root_name(), /* depth= */ 0);
  // Ensure generated code ends in empty lines.
  writer_->Write(HEADER, "");
  writer_->Write(SOURCE, "");
  writer_->Write(INTERFACE, "");
}

FieldData MessageBuilder::MakeFieldData(const FieldBuilderOptions& options,
                                        const FieldDescriptor& field_descriptor,
                                        const std::string& data_parent,
                                        const std::string& name_parent,
                                        bool first_method) const {
  bool use_get_raw_data =
      root_options_.has_use_build() || root_options_.has_use_status() ||
              root_options_.has_use_validator()
          ? root_options_.use_build() || root_options_.use_status() ||
                root_options_.use_validator()
          : options_.use_validator;
  return {
      .config = options_.config,
      .writer = writer_.get(),
      .raw_field_options = options,
      .field = field_descriptor,
      .class_name = class_name_,
      .data_parent = data_parent,
      .name_parent = name_parent,
      .use_get_raw_data = use_get_raw_data,
      .make_interface = options_.make_interface,
      .first_method = first_method,
      .use_status = root_options_.use_status(),
  };
}

void MessageBuilder::WriteMethod(const FieldData& field_data) {
  FieldBuilder(field_data).WriteField();
}

bool MessageBuilder::WriteField(const FieldDescriptor& field_descriptor,
                                const std::string& data_parent,
                                const std::string& name_parent) {
  const FieldBuilderOptions* automatic =
      options_.config.GetAutomaticType(GetFieldType(field_descriptor));
  const ::google::protobuf::FieldOptions& field_options = field_descriptor.options();
  const int size = field_options.ExtensionSize(field /* proto option */);
  bool recurse = options_.max_field_depth > 1;
  if (size == 0) {
    WriteMethod(
        MakeFieldData({}, field_descriptor, data_parent, name_parent, true));
    if (automatic) {
      WriteMethod(MakeFieldData(*automatic, field_descriptor, data_parent,
                                name_parent));
      // TODO Automatic types should disable recursion.
      // recurse &= automatic->recurse();
    }
  }
  for (int f = 0; f < size; ++f) {
    const FieldBuilderOptions options =
        options_.config.MergeFieldBuilderOptions(
            field_options.GetExtension(field, f));

    if (options.output() == FieldBuilderOptions::SKIP) {
      continue;
    }
    if (options.has_recurse()) {
      recurse &= options.recurse();
    } else {
      // If the field options do not actually control 'recurse', then we check
      // if the options reference a configured type, and if so, we check if the
      // type info has 'recurse' set.
      const FieldBuilderOptions* type_info = options_.config.GetTypeInfo(
          GetOptionsType(options, field_descriptor));
      if (type_info && type_info->has_recurse()) {
        recurse &= type_info->recurse();
      }
    }
    WriteMethod(
        MakeFieldData(options, field_descriptor, data_parent, name_parent, !f));
  }
  return recurse;
}

void MessageBuilder::WriteMessage(const ::google::protobuf::Descriptor& descriptor,
                                  const std::string& data_parent,
                                  const std::string& name_parent, int depth) {
  const std::string log_info =
      absl::StrCat("Message: ", descriptor.full_name(), "[", depth, "]");
  if (depth > kMaxSubFieldSetterDepth) {
    LOG(ERROR) << log_info << " Max sub-field setter depth reached.";
    return;
  }
  if (!messages_in_subfield_setter_stack_.insert(&descriptor).second) {
    LOG(INFO) << log_info << " Already used in sub-field setter stack.";
    return;
  }
  for (const auto& include : root_options_.include()) {
    writer_->CodeInfo()->AddInclude(HEADER, include);
  }
  for (const auto& include : root_options_.builder_include()) {
    writer_->CodeInfo()->AddInclude(HEADER, include);
  }
  for (const auto& include : root_options_.source_include()) {
    writer_->CodeInfo()->AddInclude(SOURCE, include);
  }
  for (int i = 0; i < descriptor.field_count(); ++i) {
    const auto& field_descriptor = *PBCC_DIE_IF_NULL(descriptor.field(i));
    const auto& builder = GetFieldBuilderOptionsOrDefault(field_descriptor);
    if (builder.output() == FieldBuilderOptions::SKIP) {
      continue;
    }
    const bool recurse = WriteField(field_descriptor, data_parent, name_parent);
    if (recurse && IsNonRepeatedMessage(field_descriptor)) {
      const ::google::protobuf::Descriptor& field_type =
          *PBCC_DIE_IF_NULL(field_descriptor.message_type());
      // We include proto types in the header as we can then rely on transitive
      // dependencies.
      writer_->CodeInfo()->AddInclude(HEADER, field_type);
      WriteMessage(
          field_type,
          absl::StrCat(data_parent, "mutable_",
                       google::protobuf::compiler::cpp::FieldName(&field_descriptor),
                       "()->"),
          absl::StrCat(name_parent, !builder.name().empty()
                                        ? builder.name()
                                        : CamelCaseName(field_descriptor)),
          depth + 1);
    }
    // Non-empty if the FieldDescriptor represents a map with a Message type as
    // its value.
    // TODO: Do something similar for Enum types.
    absl::optional<const ::google::protobuf::Descriptor*> map_value_descriptor =
        MaybeGetMapValueDescriptor(field_descriptor);
    if (map_value_descriptor != absl::nullopt) {
      writer_->CodeInfo()->AddInclude(HEADER, **map_value_descriptor);
    }
  }
  messages_in_subfield_setter_stack_.erase(&descriptor);
}

}  // namespace proto_builder
