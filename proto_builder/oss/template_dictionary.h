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

#ifndef PROTO_BUILDER_OSS_TEMPLATE_DICTIONARY_H_
#define PROTO_BUILDER_OSS_TEMPLATE_DICTIONARY_H_

#include <list>
#include <map>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/variant.h"

namespace proto_builder::oss {

enum DoNotStrip { DO_NOT_STRIP = 0 };

class TemplateDictionary {
 public:
  explicit TemplateDictionary(absl::string_view name) : name_(name) {}

  TemplateDictionary* AddSectionDictionary(absl::string_view name);
  void SetValue(absl::string_view name, absl::string_view value);

  bool Expand(std::string* output) const;
  bool ExpandTemplate(absl::string_view name, std::string* output) const;

  std::string name() const { return name_; }

 private:
  using SectionDictionary = std::list<TemplateDictionary>;

  template <class DataType>
  struct TagInfo {
    TagInfo(absl::string_view tag, DataType data);
    const std::string tag;
    const std::string tag_start;
    const std::string tag_end;
    DataType data;
  };

  // Type `Data` holds all possible information variants. Each of these needs
  // to have a matching `Expand(const TagInfo<Data-Type>&, std::string*)`.
  using Data = absl::variant<TagInfo<SectionDictionary>, TagInfo<std::string>>;

  static bool RemoveTags(std::string* value);

  bool Expand(const TagInfo<SectionDictionary>& tag, std::string* output) const;
  bool Expand(const TagInfo<std::string>& tag, std::string* output) const;

  const std::string name_;
  std::map<std::string, Data> data_;
};

bool StringToTemplateCache(absl::string_view name, absl::string_view tpl,
                           DoNotStrip do_not_strip);
bool ExpandTemplate(absl::string_view name, DoNotStrip do_not_strip,
                    const TemplateDictionary* dict, std::string* output);

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_TEMPLATE_DICTIONARY_H_
