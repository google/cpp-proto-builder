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

#ifndef PROTO_BUILDER_PROTO_BUILDER_CONFIG_H_
#define PROTO_BUILDER_PROTO_BUILDER_CONFIG_H_

#include <string>

#include "proto_builder/proto_builder.pb.h"
#include "absl/status/status.h"

namespace proto_builder {

// Verify an entry in the global configuration.
// NOTE: Will crash on any violation.
bool VerifyTypeEntry(const std::string& key,
                     const FieldBuilderOptions& options);

// Verify a proto_builder configuration.
// NOTE: Will crash on any violation.
ProtoBuilderConfig VerifyProtoBuilderConfig(absl::string_view textproto);

// Verify that all `dependency` specifications in the config are met.
absl::Status CheckConversionDependencies(const std::string& conv_deps_file);

// Convert input from CamelCase to snake_case, keeping '_'s at the beginning
// and end. The function treats all non alphanumeric characters as potential
// underscores and will never return two or more consecutive underscores. The
// function does not validate the input and may return an empty string.
std::string CamelCaseToSnakeCase(absl::string_view input);

enum class ProtoBuilderTypeInfo {
  kParameter = 0,  // Normal parameter handling.
  kSpecial = 1,    // Internal special type.
};

class ProtoBuilderConfigManager {
 public:
  ProtoBuilderConfigManager();

  ProtoBuilderConfigManager Update(
      const MessageBuilderOptions& message_options) const;

  // Get access to proto_builder configuration (e.g. type_map).
  const ProtoBuilderConfig& GetProtoBuilderConfig() const;

  FieldBuilderOptions MergeFieldBuilderOptions(FieldBuilderOptions fbo) const;

  // Get the options for the specified type from the global config.
  // If `is_special_type` is true, then the name must start with a '%'.
  const FieldBuilderOptions* GetTypeInfo(
      const std::string& raw_type,
      ProtoBuilderTypeInfo type = ProtoBuilderTypeInfo::kParameter) const;

  // Returns a map of type name to FieldBuilderOptions for all '%'/'$' types.
  const std::map<std::string, const FieldBuilderOptions*>& GetSpecialTypes()
      const;

  // Returns all types marked `automatic: true` (name prefixed with '=').
  const std::map<std::string, const FieldBuilderOptions*>& GetAutomaticTypes()
      const;
  const FieldBuilderOptions* GetAutomaticType(const std::string& type) const;

  // Expands all builtin and custom variables from the configuration.
  const std::map<std::string, std::string>& GetExpandedTypes() const;
  std::string GetExpandedType(const std::string& type) const;

 private:
  static std::map<std::string, const FieldBuilderOptions*> MakeSpecialTypes(
      const ProtoBuilderConfig& config);
  static std::map<std::string, const FieldBuilderOptions*> MakeAutomaticTypes(
      const ProtoBuilderConfig& config);
  static std::map<std::string, std::string> MakeExpandedTypes(
      const ProtoBuilderConfig& config);

  explicit ProtoBuilderConfigManager(ProtoBuilderConfig config);

  const ProtoBuilderConfig config_;
  const std::map<std::string, const FieldBuilderOptions*> special_types_;
  const std::map<std::string, const FieldBuilderOptions*> automatic_types_;
  const std::map<std::string, std::string> expanded_types_;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_PROTO_BUILDER_CONFIG_H_
