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

#ifndef PROTO_BUILDER_TEMPLATE_BUILDER_H_
#define PROTO_BUILDER_TEMPLATE_BUILDER_H_

#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "proto_builder/builder_writer.h"
#include "proto_builder/message_builder.h"
#include "proto_builder/oss/template_dictionary.h"
#include "proto_builder/proto_builder_config.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace proto_builder {

using ::google::protobuf::Descriptor;

namespace ctemplate = proto_builder::oss;

std::string StripPrefixDir(absl::string_view in,
                           const std::string& prefix_dir_list);
std::string HeaderGuard(std::string in);

class TemplateBuilder {
 public:
  struct Options {
    const ProtoBuilderConfigManager& config;
    BufferWriter* writer;
    const std::vector<const ::google::protobuf::Descriptor*>& descriptors;
    const std::string& header;
    const std::string& tpl_head;
    const std::string& tpl_body;
    const size_t max_field_depth;
    const bool use_validator = false;
    const std::string validator_header = "";
    const bool make_interface = false;
    const std::string tpl_iface;
    const std::string interface_header;
  };

  explicit TemplateBuilder(Options options);

  absl::Status WriteBuilder();

 private:
  // A structure containing builders and writer for generating source for a
  // single message builder.
  struct MessageOutput {
    MessageOutput(const std::vector<std::string>& package_path,
                  const ::google::protobuf::Descriptor& descriptor, const Options& options);

    const ProtoBuilderConfigManager config;
    BufferWriter writer;
    MessageBuilder builder;
  };

  absl::StatusOr<std::unique_ptr<ctemplate::TemplateDictionary>>
  FillDictionary() const;
  void MaybeAddSection(const MessageOutput& message, absl::string_view section,
                       std::function<bool(const MessageBuilderOptions&)> select,
                       ctemplate::TemplateDictionary* dict) const;
  void FillDictionaryBasics(const MessageOutput& message,
                            ctemplate::TemplateDictionary* dict) const;
  void FillIncludes(absl::string_view section_name, std::vector<Where> wheres,
                    bool strip_export,
                    const std::set<std::string>& drop_headers,
                    ctemplate::TemplateDictionary* dict) const;
  absl::StatusOr<std::string> ExpandTemplate(
      Where where, const ctemplate::TemplateDictionary& dict) const;

  absl::Status LoadTemplate(Where where);
  void Write(Where to, absl::string_view line);

  static std::vector<std::unique_ptr<MessageOutput>> CreateMessageOutputs(
      const std::vector<std::string>& package_path, const Options& options);

  const Options options_;
  const std::vector<std::string> package_path_;
  const std::string header_;
  const std::map<Where, std::string> tpl_;
  NoDoubleEmptyLineWriter target_writer_;
  std::vector<std::unique_ptr<MessageOutput>> message_outputs_;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_TEMPLATE_BUILDER_H_
