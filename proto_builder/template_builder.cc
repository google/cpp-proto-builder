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

#include "proto_builder/template_builder.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "proto_builder/builder_writer.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/util.h"
#include "proto_builder/proto_builder_config.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/descriptor.h"
#include "absl/algorithm/container.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"

namespace proto_builder {

using ::google::protobuf::Descriptor;
using RegExpStringPiece = ::re2::StringPiece;

namespace {

std::vector<std::string> GetPackageForDescriptors(
    const std::vector<const ::google::protobuf::Descriptor*>& descriptors) {
  QCHECK(!descriptors.empty()) << "At least one descriptor required.";
  const std::string first_package =
      GetPackageAndClassName(descriptors[0]).first;
  for (const ::google::protobuf::Descriptor* descriptor : descriptors) {
    QCHECK_EQ(first_package, GetPackageAndClassName(descriptor).first)
        << "All proto descriptors must be in the same package.";
  }
  return absl::StrSplit(first_package, '.');
}

bool UseBuild(const MessageBuilderOptions& options) {
  return options.use_build() || options.use_validator();
}

bool UseConversion(const MessageBuilderOptions& options) {
  return options.use_conversion();
}

bool UseStatus(const MessageBuilderOptions& options) {
  return options.use_status() || options.use_build() || options.use_validator();
}

bool UseValidator(const MessageBuilderOptions& options) {
  return options.use_validator();
}

void AddIncludesForGlobalType(const std::string& type,
                              const ProtoBuilderConfigManager& config,
                              CodeInfoCollector& code_info) {
  const FieldBuilderOptions* type_options =
      config.GetTypeInfo(type, ProtoBuilderTypeInfo::kSpecial);
  if (!type_options) {
    return;
  }
  for (const std::string& include : type_options->include()) {
    code_info.AddInclude(HEADER, include);
  }
  for (const std::string& include : type_options->source_include()) {
    code_info.AddInclude(SOURCE, include);
  }
}

}  // namespace

std::string StripPrefixDir(absl::string_view in,
                           const std::string& prefix_dir_list) {
  // See flag --template_builder_strip_prefix_dir documentation.
  RegExpStringPiece subject = in;  // Code needed for OSS
  // See flag --template_builder_strip_prefix_dir documentation.
  for (auto re : absl::StrSplit(prefix_dir_list, ',')) {
    if (absl::EndsWith(re, "/")) {
      re.remove_suffix(1);
    }
    if (RE2::Consume(&subject, absl::StrCat("^", re, "/*"))) {
      return std::string(subject);
    }
  }
  return std::string(subject);
}

std::string HeaderGuard(std::string in) {
  std::transform(in.begin(), in.end(), in.begin(),
                 [](unsigned char c) -> unsigned char {
                   return absl::ascii_isalnum(c) ? absl::ascii_toupper(c) : '_';
                 });
  return absl::StrCat(in, "_");
}

std::string InterfaceGuard(std::string in) {
  return absl::StrCat(absl::StripSuffix(HeaderGuard(in), "_H_"),
                      "_INTERFACE_H_");
}

TemplateBuilder::MessageOutput::MessageOutput(
    const std::vector<std::string>& package_path, const ::google::protobuf::Descriptor& descriptor,
    const Options& options)
    : config(options.config.Update(descriptor.options().GetExtension(message))),
      writer(package_path),
      builder({
          .config = config,
          .writer = &writer,
          .descriptor = descriptor,
          .max_field_depth = options.max_field_depth,
          .use_validator = options.use_validator,
          .make_interface = options.make_interface,
      }) {}

TemplateBuilder::TemplateBuilder(Options options)
    : options_(options),
      package_path_(GetPackageForDescriptors(options_.descriptors)),
      header_(options_.header),
      tpl_({
          {HEADER, options_.tpl_head},
          {INTERFACE, options_.tpl_iface},
          {SOURCE, options_.tpl_body},
      }),
      target_writer_(options_.writer),
      message_outputs_(CreateMessageOutputs(package_path_, options_)) {}

absl::Status TemplateBuilder::WriteBuilder() {
  for (auto& message : message_outputs_) {
    if (UseStatus(message->builder.root_options())) {
      for (const auto& type :
           {"%LogSourceLocation", "%SourceLocation", "%StatusOr", "%Status"}) {
        AddIncludesForGlobalType(type, message->config,
                                 *message->writer.CodeInfo());
      }
    }
    if (UseValidator(message->builder.root_options()) &&
        !options_.validator_header.empty()) {
      message->writer.CodeInfo()->AddInclude(HEADER, options_.validator_header);
    }
    message->builder.WriteBuilder();
  }
  if (auto status = LoadTemplate(HEADER); !status.ok()) {
    return status;
  }
  if (auto status = LoadTemplate(SOURCE); !status.ok()) {
    return status;
  }
  if (options_.make_interface) {
    if (auto status = LoadTemplate(INTERFACE); !status.ok()) {
      return status;
    }
  }
  const auto [dict_status, dict] = UnpackStatusOr(FillDictionary());
  if (!dict_status.ok()) {
    return dict_status;
  }
  std::vector<Where> targets{HEADER, SOURCE};
  if (options_.make_interface) {
    targets.push_back(INTERFACE);
  }
  for (auto where : targets) {
    if (const auto [status, expanded_template] =
            UnpackStatusOrDefault(ExpandTemplate(where, *dict));
        !status.ok()) {
      return status;
    } else {
      for (const auto line : absl::StrSplit(expanded_template, '\n')) {
        Write(where, line);
      }
      Write(where, "");  // Ensure terminating new-line.
    }
  }
  return absl::OkStatus();
}

void TemplateBuilder::FillIncludes(absl::string_view section_name,
                                   std::vector<Where> wheres, bool strip_export,
                                   const std::set<std::string>& drop_headers,
                                   ctemplate::TemplateDictionary* dict) const {
  std::set<std::string> all_includes;
  for (const auto& message : message_outputs_) {
    for (auto where : wheres) {
      const auto& includes = message->writer.CodeInfo()->GetIncludes(where);
      all_includes.insert(includes.begin(), includes.end());
    }
  }
  if (options_.make_interface) {
    all_includes.emplace(absl::StrCat("\"", options_.interface_header, "\""));
  }
  // Separate includes into system includes and everything else.
  std::vector<std::string> includes[2];
  for (std::string include : all_includes) {
    if (strip_export) {
      include = absl::StripSuffix(include, "  // IWYU pragma: export");
    }
    if (drop_headers.count(include) > 0) {
      continue;
    }
    if (include[0] == '<') {
      includes[0].push_back(absl::StrCat("#include ", include));
    } else {
      includes[1].push_back(absl::StrCat("#include ", include));
    }
  }
  for (const std::string& include : includes[0]) {
    dict->AddSectionDictionary(section_name)->SetValue("INCLUDE", include);
  }
  if (!includes[0].empty() && !includes[1].empty()) {
    // If we have both system and non system includes, then add an empty line.
    dict->AddSectionDictionary(section_name)->SetValue("INCLUDE", "");
  }
  for (const std::string& include : includes[1]) {
    dict->AddSectionDictionary(section_name)->SetValue("INCLUDE", include);
  }
}

void TemplateBuilder::FillDictionaryBasics(
    const MessageOutput& message, ctemplate::TemplateDictionary* dict) const {
  dict->SetValue("CLASS_NAME", message.builder.class_name());
  dict->SetValue("INTERFACE_NAME",
                 absl::StrCat(message.builder.class_name(), "Interface"));
  std::string base_classes;
  if (!message.builder.root_options().base_class().empty()) {
    base_classes = absl::StrCat(
        " : ",
        absl::StrJoin(message.builder.root_options().base_class(), ", "));
  }
  dict->SetValue("BASE_CLASSES", base_classes);
  std::string namespace_str = absl::StrJoin(package_path_, "::");
  dict->SetValue("NAMESPACE", namespace_str);
  const std::string proto_type_full = message.writer.CodeInfo()->RelativeType(
      AbsoluteCppTypeName(message.builder.root_descriptor().full_name()));
  absl::string_view proto_type = proto_type_full;
  absl::StrAppend(&namespace_str, "::");
  if (!absl::ConsumePrefix(&proto_type, absl::StrCat("::", namespace_str))) {
    absl::ConsumePrefix(&proto_type, namespace_str);
  }
  dict->SetValue("PROTO_TYPE", proto_type);
  dict->SetValue("PROTO_TYPE_SHORT",
                 message.writer.CodeInfo()->RelativeType(AbsoluteCppTypeName(
                     message.builder.root_descriptor().name())));
  absl::string_view root_data = message.builder.root_options().root_data();
  if (!absl::ConsumeSuffix(&root_data, ".")) {
    absl::ConsumeSuffix(&root_data, "->");
  }
  dict->SetValue("ROOT_DATA", root_data);
  dict->SetValue("VALIDATE_DATA", UseValidator(message.builder.root_options())
                                      ? absl::StrCat("ValidateData();")
                                      : "");
  for (const auto& [key, value] : message.config.GetExpandedTypes()) {
    dict->SetValue(key, value);
  }
}

void TemplateBuilder::MaybeAddSection(
    const MessageOutput& message, absl::string_view section,
    std::function<bool(const MessageBuilderOptions&)> select,
    ctemplate::TemplateDictionary* dict) const {
  if (select(message.builder.root_options())) {
    FillDictionaryBasics(message, dict->AddSectionDictionary(section));
  } else if (absl::ConsumePrefix(&section, "USE_")) {
    FillDictionaryBasics(
        message, dict->AddSectionDictionary(absl::StrCat("NOT_", section)));
  }
}

absl::StatusOr<std::unique_ptr<ctemplate::TemplateDictionary>>
TemplateBuilder::FillDictionary() const {
  std::unique_ptr<ctemplate::TemplateDictionary> dict(
      new ctemplate::TemplateDictionary("ProtoBuilder"));
  dict->SetValue("HEADER_GUARD", HeaderGuard(header_));
  dict->SetValue("INTERFACE_GUARD", InterfaceGuard(header_));
  dict->SetValue("HEADER_FILE", header_);
  const std::string header_include = absl::StrCat("\"", header_, "\"");
  const std::string interface_include =
      absl::StrCat("\"", options_.interface_header, "\"");
  std::set<std::string> drop_interface_includes{header_include,
                                                interface_include};
  for (const auto& message : message_outputs_) {
    const auto& includes = message->builder.root_options().builder_include();
    for (const auto& include : includes) {
      drop_interface_includes.emplace(FormatInclude(include));
    }
  }
  FillIncludes("INCLUDES", {HEADER, SOURCE}, false, {}, dict.get());
  FillIncludes("HEADER_INCLUDES", {HEADER}, false, {}, dict.get());
  FillIncludes("INTERFACE_INCLUDES", {HEADER}, false, drop_interface_includes,
               dict.get());
  FillIncludes("SOURCE_INCLUDES", {SOURCE}, true, {header_include}, dict.get());
  for (const auto& ns : package_path_) {
    dict->AddSectionDictionary("NAMESPACES")->SetValue("NAMESPACE", ns);
  }
  dict->AddSectionDictionary("ALL_NAMESPACES")
      ->SetValue("NAMESPACE", absl::StrJoin(package_path_, "::"));
  for (auto it = package_path_.crbegin(); it != package_path_.crend(); ++it) {
    dict->AddSectionDictionary("NAMESPACES_END")->SetValue("NAMESPACE", *it);
  }
  for (const auto& message : message_outputs_) {
    auto* builder_dict = dict->AddSectionDictionary("BUILDER");
    FillDictionaryBasics(*message, builder_dict);
    builder_dict->SetValue("GENERATED_HEADER_CODE",
                           absl::StrJoin(message->writer.From(HEADER), "\n"));
    builder_dict->SetValue(
        "GENERATED_INTERFACE_CODE",
        absl::StrJoin(message->writer.From(INTERFACE), "\n"));
    builder_dict->SetValue("GENERATED_SOURCE_CODE",
                           absl::StrJoin(message->writer.From(SOURCE), "\n"));
    MaybeAddSection(*message, "USE_BUILD", &UseBuild, builder_dict);
    MaybeAddSection(*message, "USE_CONVERSION", &UseConversion, builder_dict);
    MaybeAddSection(*message, "USE_STATUS", &UseStatus, builder_dict);
    MaybeAddSection(*message, "USE_VALIDATOR", &UseValidator, builder_dict);
  }
  return std::move(dict);
}

absl::StatusOr<std::string> TemplateBuilder::ExpandTemplate(
    Where where, const ctemplate::TemplateDictionary& dict) const {
  std::string output;
  if (!ctemplate::ExpandTemplate(tpl_.at(where), ctemplate::DO_NOT_STRIP, &dict,
                                 &output)) {
    return absl::UnknownError(::absl::StrFormat(
        "Error in ExpandTemplate. While expanding %s from %s.",
        Where_Name(where), tpl_.at(where)));
  }
  return output;
}

static const std::pair<const RE2*, absl::string_view> kReplacements[] = {
    {new RE2("^\\s*//\\s*({{#BUILDER}}).*$"), "\\1"},
    {new RE2("^\\s*//\\s*({{/BUILDER}}).*$"), "\\1"},
    {new RE2("^\\s*//\\s*({{GENERATED_HEADER_CODE}}).*$"), "\\1"},
    {new RE2("^\\s*//\\s*({{GENERATED_INTERFACE_CODE}}).*$"), "\\1"},
    {new RE2("^\\s*//\\s*({{GENERATED_SOURCE_CODE}}).*$"), "\\1"},
    {new RE2("^(\\s*#\\s*(?:ifndef\\s|define\\s|endif\\s+//\\s?))"
             ".*({{HEADER_GUARD}}).*$"),
     "\\1\\2"},
};

absl::Status TemplateBuilder::LoadTemplate(Where where) {
  const std::string more_info = absl::StrCat(
      "While expanding ", Where_Name(where), " from ", tpl_.at(where), ".");
  std::string contents = tpl_.at(where);
  std::string raw_template;
  for (const auto line_view : absl::StrSplit(contents, '\n')) {
    std::string line(line_view);
    for (const auto& replacement : kReplacements) {
      if (RE2::Replace(&line, *replacement.first, replacement.second)) {
        break;
      }
    }
    absl::StrAppend(&raw_template, line, "\n");
  }
  if (!ctemplate::StringToTemplateCache(tpl_.at(where), raw_template,
                                        ctemplate::DO_NOT_STRIP)) {
    return absl::InternalError(::absl::StrFormat(
        "Could not insert raw template into template cache. %s.", more_info));
  }
  return absl::OkStatus();
}

void TemplateBuilder::Write(Where to, absl::string_view line) {
  target_writer_.Write(to, std::string(line));
}

// static
std::vector<std::unique_ptr<TemplateBuilder::MessageOutput>>
TemplateBuilder::CreateMessageOutputs(
    const std::vector<std::string>& package_path, const Options& options) {
  std::vector<std::unique_ptr<MessageOutput>> outputs;
  outputs.reserve(options.descriptors.size());
  for (const ::google::protobuf::Descriptor* descriptor : options.descriptors) {
    outputs.emplace_back(absl::make_unique<MessageOutput>(
        package_path, *PBCC_DIE_IF_NULL(descriptor), options));
  }
  return outputs;
}

}  // namespace proto_builder
