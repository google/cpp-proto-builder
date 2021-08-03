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

#include "proto_builder/proto_builder_config.h"

#include <map>
#include <set>
#include <string>

// Add logging for OSS.
#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging.h"
#include "proto_builder/oss/util.h"
#include "proto_builder/proto_builder.pb.h"
#include "proto_builder/util.h"
#include "google/protobuf/text_format.h"
#include "absl/algorithm/container.h"
#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, proto_builder_config, "",
          "ProtoBuilderConfig textproto file.");

namespace proto_builder {
namespace {

using RegExpStringPiece = ::re2::StringPiece;

static std::set<std::string> GetBuiltInTypeNames() {
  return {
      "string",
      "bytes",
      "@absl::string_view",
      "@Map:absl::string_view",
      "@TextProto",
      "@TextProto:absl::string_view",
      "@TextProto:Map:Value:absl::string_view",
      "@ToInt64Seconds",
      "@ToInt64Milliseconds",
      "@ToDoubleSeconds",
      "@ToDoubleMilliseconds",
      "@ToProtoDuration",
      "@ToProtoTimestamp",
      "%SourceLocation",
      "%Status",
      "%StatusOr",
      "%Validate",
      "%LogSourceLocation",
  };
}

}  // namespace

const std::string& GetProtoTextConfig();  // external, no header

std::string ReplaceType(std::string type) {
  absl::StrReplaceAll({{"@type@", "Type"}}, &type);
  return type;
}

bool VerifyTypeEntry(const std::string& key,
                     const FieldBuilderOptions& options) {
  const std::string entry =
      absl::StrCat("key: '", key, "' -> { ", options.DebugString(), " }");
  QCHECK_EQ(ReplaceType(options.type()).find('@'), std::string::npos)
      << "May not use '@' (beyond '@type@') in type: " << entry;
  QCHECK(!options.has_name()) << "May not provide 'name': " << entry;
  QCHECK_GE(!options.type().empty(), !options.decorated_type().empty())
      << "May not use 'decorated_type' without 'type': " << entry;
  QCHECK_EQ(options.value().find("@type@"), std::string::npos)
      << "May not use '@type@' in 'value': " << entry;
  QCHECK_EQ(options.value().find("@value@"), std::string::npos)
      << "May not use '@value@' in 'value': " << entry;
  QCHECK(absl::c_none_of(
      options.include(),
      [](const std::string& include) { return include.empty(); }))
      << "May not use empty 'include': " << entry;
  QCHECK(absl::c_none_of(options.include(),
                         [](const std::string& include) {
                           return absl::StrContains(include, '\n');
                         }))
      << "May not use new-line in 'include', use multiple includes: " << entry;
  QCHECK(!options.automatic() || absl::StartsWith(key, "="))
      << "Automatic types must not start with '=': " << entry;
  if (!key.empty()) {
    QCHECK((key[0] != '@' && key[0] != '%') || GetBuiltInTypeNames().count(key))
        << "Type names (key) starting with '@' or '%' are reserved for "
        << "internal use: " << entry;
    static LazyRE2 kCustomKey = {R"re(\$[[:alpha:]][[:word:]]*)re"};
    QCHECK(key[0] != '$' || RE2::FullMatch(key, *kCustomKey))
        << "Custom keys must start with '$', followed by an alphabetical "
        << "character, followed by any number of alphanumeric characters: "
        << entry;
  }
  QCHECK(!options.has_macro())
      << "The `macro` field can only be used for field annotations:" << entry;

  // MUST BE LAST:
  QCHECK(!key.empty()) << "Must specify a non-empty 'key': " << entry;
  return true;
}

ProtoBuilderConfig VerifyProtoBuilderConfig(absl::string_view textproto) {
  ProtoBuilderConfig config;
  QCHECK(google::protobuf::TextFormat::ParseFromString(std::string(textproto), &config));
  std::string custom_config_file = absl::GetFlag(FLAGS_proto_builder_config);
  if (!custom_config_file.empty()) {
    auto [status, custom_config_textproto] =
        UnpackStatusOrDefault(file::oss::GetContents(custom_config_file));
    QCHECK(status.ok()) << "Custom config file error: " << status;
    ProtoBuilderConfig custom_config;
    QCHECK(google::protobuf::TextFormat::ParseFromString(std::string(custom_config_textproto),
                                               &custom_config))
        << "Custom config file error: " << custom_config_file;
    config.MergeFrom(custom_config);  // Custom can override default config.
  }
  for (const auto& type : config.type_map()) {
    // NOTE: The API does not allow us to check for duplicate keys...
    VerifyTypeEntry(type.first, type.second);
  }
  ProtoBuilderConfig tmp_config;
  tmp_config.mutable_type_map()->swap(*config.mutable_type_map());
  for (auto [t, options] : *tmp_config.mutable_type_map()) {
    std::string type = t;
    if (options.automatic()) {
      type = absl::StrCat("=", AbsoluteCppTypeName(type.substr(1)));
      if (!options.has_recurse()) {
        options.set_recurse(false);
      }
    }
    config.mutable_type_map()->insert({type, options});
  }
  // .. so we protect against duplicates by looking at the textproto.
  static LazyRE2 kReKey = {R"re(key:\s*"((?:[^"]|\\\")*)")re"};
  std::map<std::string, size_t> keys;
  std::set<std::string> duplicate_keys;
  std::string key;
  size_t textproto_keys = 0;
  RegExpStringPiece remainder = textproto;
  while (RE2::FindAndConsume(&remainder, *kReKey, &key)) {
    if (++keys[key] > 1) {
      duplicate_keys.emplace(key);
    }
    ++textproto_keys;
  }
  CHECK_EQ(textproto_keys, config.type_map().size())
      << "Configuration contains duplicate key(s): \""
      << absl::StrJoin(duplicate_keys, R"(", ")") << R"(")";
  return config;
}

const ProtoBuilderConfig& GetGlobalProtoBuilderConfig() {
  static const auto& kConfig =
      *new ProtoBuilderConfig(VerifyProtoBuilderConfig(GetProtoTextConfig()));
  return kConfig;
}

std::string NormalizeLabel(absl::string_view label) {
  if (absl::StrContains(label, ":")) {
    return std::string(label);
  }
  const std::vector<std::string> parts = absl::StrSplit(label, '/');
  return absl::StrCat(label, ":", parts.back());
}

ProtoBuilderConfigManager::ProtoBuilderConfigManager()
    : ProtoBuilderConfigManager(GetGlobalProtoBuilderConfig()) {}

ProtoBuilderConfigManager::ProtoBuilderConfigManager(ProtoBuilderConfig config)
    : config_(config),
      special_types_(MakeSpecialTypes(config_)),
      automatic_types_(MakeAutomaticTypes(config_)),
      expanded_types_(MakeExpandedTypes(config_)) {}

ProtoBuilderConfigManager ProtoBuilderConfigManager::Update(
    const MessageBuilderOptions& message_options) const {
  ProtoBuilderConfig result(config_);
  const auto& type_map = message_options.type_map();
  for (const auto& [key, options] : type_map) {
    if (absl::StartsWith(key, "@") || absl::StartsWith(key, "%")) {
      const std::string entry =
          absl::StrCat("key: '", key, "' -> { ", options.DebugString(), " }");
      CHECK_EQ(GetBuiltInTypeNames().count(key), 0)
          << "Cannot update configuration of builtin types: " << entry;
    }
    VerifyTypeEntry(key, options);
    result.mutable_type_map()->erase(key);  // There is no insert_or_assign
    result.mutable_type_map()->insert({key, options});
  }
  return ProtoBuilderConfigManager(result);
}

const ProtoBuilderConfig& ProtoBuilderConfigManager::GetProtoBuilderConfig()
    const {
  return config_;
}

FieldBuilderOptions ProtoBuilderConfigManager::MergeFieldBuilderOptions(
    FieldBuilderOptions fbo) const {
  if (fbo.macro().empty()) {
    return fbo;
  }
  auto it = config_.type_map().find(fbo.macro());
  if (it == config_.type_map().end()) {
    return fbo;
  }
  FieldBuilderOptions result(it->second);
  result.MergeFrom(fbo);
  return result;
}

absl::Status CheckConversionDependencies(const std::string& conv_deps_file) {
  const auto [result, conv_deps] =
      UnpackStatusOrDefault(file::oss::GetContents(conv_deps_file));
  if (!result.ok()) {
    return result;
  }

  std::set<std::string> conv_deps_set;
  for (auto dep : absl::StrSplit(conv_deps, '\n', absl::SkipEmpty())) {
    conv_deps_set.emplace(NormalizeLabel(dep));
  }

  const ProtoBuilderConfig& config = GetGlobalProtoBuilderConfig();
  for (const auto& [name, field_options] : config.type_map()) {
    for (const auto& dependency : field_options.dependency()) {
      if (!conv_deps_set.count(NormalizeLabel(dependency))) {
        return absl::NotFoundError(absl::StrCat(
            "Type: '", name, "' has dependency '", dependency,
            "' which is not configured in proto_builder/build_*.bzl."));
      }
    }
  }
  return absl::OkStatus();
}

const FieldBuilderOptions* ProtoBuilderConfigManager::GetTypeInfo(
    const std::string& raw_type, ProtoBuilderTypeInfo type) const {
  CHECK_EQ((type == ProtoBuilderTypeInfo::kSpecial),
           absl::StartsWith(raw_type, "%"))
      << "Raw type: '" << raw_type << "'";
  auto it = config_.type_map().find(raw_type);
  if (it == config_.type_map().end()) return nullptr;
  return &it->second;
}

const std::map<std::string, const FieldBuilderOptions*>&
ProtoBuilderConfigManager::GetSpecialTypes() const {
  return special_types_;
}

std::map<std::string, const FieldBuilderOptions*>
ProtoBuilderConfigManager::MakeSpecialTypes(const ProtoBuilderConfig& config) {
  std::map<std::string, const FieldBuilderOptions*> result;
  for (const auto& [type, options] : config.type_map()) {
    if (absl::StartsWith(type, "%") || absl::StartsWith(type, "$")) {
      result.emplace(type, &options);
    }
  }
  return result;
}

const std::map<std::string, const FieldBuilderOptions*>&
ProtoBuilderConfigManager::GetAutomaticTypes() const {
  return automatic_types_;
}

std::map<std::string, const FieldBuilderOptions*>
ProtoBuilderConfigManager::MakeAutomaticTypes(
    const ProtoBuilderConfig& config) {
  std::map<std::string, const FieldBuilderOptions*> result;
  for (const auto& [type, options] : config.type_map()) {
    if (options.automatic()) {
      result.emplace(type.substr(1), &options);
    }
  }
  return result;
}

const FieldBuilderOptions* ProtoBuilderConfigManager::GetAutomaticType(
    const std::string& type) const {
  auto it = automatic_types_.find(type);
  if (it == automatic_types_.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

std::string CamelCaseToSnakeCase(absl::string_view input) {
  std::string result;
  if (input.empty()) {  // Allows to always access at least one char in input.
    return result;
  }
  // We want to retain a single initial underscore, to do so we change how
  // the loop handles the next underscore.
  char last = ' ';
  bool underscore = input[0] == '_';
  for (char c : input) {
    if (absl::ascii_isupper(c)) {
      if (underscore) {
        result.push_back('_');
      }
      underscore = false;
      result.push_back(absl::ascii_tolower(c));
    } else if (absl::ascii_islower(c) || absl::ascii_isdigit(c)) {
      underscore = true;
      result.push_back(c);
    } else {
      if ((!result.empty() && last != '_') || (result.empty() && c == '_')) {
        result.push_back('_');
      }
      c = '_';
      underscore = false;
    }
    last = c;
  }
  if (*input.rbegin() == '_' && last != '_') {
    result.push_back('_');
  }
  return result;
}

const std::map<std::string, std::string>&
ProtoBuilderConfigManager::GetExpandedTypes() const {
  return expanded_types_;
}

std::map<std::string, std::string> ProtoBuilderConfigManager::MakeExpandedTypes(
    const ProtoBuilderConfig& config) {
  std::map<std::string, std::string> result;
  for (const auto& [type, options] : config.type_map()) {
    if (!absl::StartsWith(type, "%") && !absl::StartsWith(type, "$")) {
      continue;  // Only special types.
    }
    result.emplace(type, options.type());
    const std::string param = CamelCaseToSnakeCase(
        options.param().empty() ? type.substr(1) : options.param());
    if (!param.empty()) {
      result.emplace(absl::StrCat(type, "%param"), param);
      result.emplace(absl::StrCat(type, "+param"),
                     absl::StrCat(options.type(), " ", param));
    }
    if (!options.value().empty()) {
      result.emplace(absl::StrCat(type, "%value"), options.value());
    }
    if (!param.empty() && !options.value().empty()) {
      result.emplace(
          absl::StrCat(type, "+param=value"),
          absl::StrCat(options.type(), " ", param, " = ", options.value()));
    }
  }
  return result;
}

std::string ProtoBuilderConfigManager::GetExpandedType(
    const std::string& type) const {
  auto it = expanded_types_.find(type);
  if (it == expanded_types_.end()) {
    return "";
  } else {
    return it->second;
  }
}

}  // namespace proto_builder
