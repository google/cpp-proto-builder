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

#include "proto_builder/oss/template_dictionary.h"

#include <string>
#include <utility>

#include "proto_builder/oss/logging.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "re2/re2.h"

namespace proto_builder::oss {

using RegExpStringPiece = ::re2::StringPiece;

static auto& g_template_cache = *new std::map<std::string, std::string>;

template <class DataType>
TemplateDictionary::TagInfo<DataType>::TagInfo(absl::string_view tag,
                                               DataType data)
    : tag(absl::StrCat("{{", tag, "}}")),
      tag_start(absl::StrCat("{{#", tag, "}}")),
      tag_end(absl::StrCat("{{/", tag, "}}")),
      data(std::move(data)) {}

TemplateDictionary* TemplateDictionary::AddSectionDictionary(
    absl::string_view name) {
  auto [it, inserted] =
      data_.emplace(name, TagInfo<SectionDictionary>(name, {}));
  auto& dict = absl::get<TagInfo<SectionDictionary>>(it->second).data;
  return &dict.emplace_back(TemplateDictionary(name));
}

void TemplateDictionary::SetValue(absl::string_view name,
                                  absl::string_view value) {
  auto [it, inserted] =
      data_.emplace(name, TagInfo<std::string>(name, std::string(value)));
  QCHECK(inserted);
}

bool TemplateDictionary::ExpandTemplate(absl::string_view name,
                                        std::string* output) const {
  auto it = g_template_cache.find(std::string(name));
  if (it == g_template_cache.end()) {
    output->clear();
    return false;
  }
  *output = it->second;
  return Expand(output);
}

std::pair<size_t, size_t> FindTag(const std::string& output,
                                  const std::string& tag) {
  const size_t tag_pos = output.find(tag);
  if (tag_pos == std::string::npos) {
    return {tag_pos, 0};
  }
  size_t end = tag_pos + tag.length();
  // Is the tag followed by a new-line?
  if (end < output.size() && (output[end] == '\n' || output[end] == '\r')) {
    ++end;
    size_t pos = tag_pos;
    // reverse past all space
    while (pos > 0 && (output[pos - 1] == ' ' || output[pos - 1] == '\t')) {
      --pos;
    }
    if (pos == 0 || output[pos - 1] == '\n' || output[pos - 1] == '\r') {
      // Only preceded by whitespace and followed directly by a new-line.
      return {pos, tag_pos - pos + tag.length() + 1};
    }
  }
  return {tag_pos, tag.length()};
}

// static
bool TemplateDictionary::RemoveTags(std::string* value) {
  static LazyRE2 kStartTags = {"{{#(\\w+)}}"};
  RegExpStringPiece pos = *value;
  RegExpStringPiece name;
  std::vector<std::string> names;
  while (RE2::FindAndConsume(&pos, *kStartTags, &name)) {
    names.emplace_back(name);
  }
  for (const std::string& name : names) {
    const std::string simple = absl::StrCat("{{", name, "}}");
    if (absl::StrContains(*value, simple)) {
      QLOG(ERROR) << "Section tag '" << name << "' also used as simple tag.";
      return false;
    }
    const std::string start = absl::StrCat("{{#", name, "}}");
    const std::string end = absl::StrCat("{{/", name, "}}");
    while (true) {
      const auto [pos_start, start_len] = FindTag(*value, start);
      const auto [pos_end, end_len] = FindTag(*value, end);
      if (pos_start == std::string::npos) {
        if (pos_end != std::string::npos) {
          QLOG(ERROR) << "Section '" << name << "' has no matching end tag.";
          return false;
        }
        break;
      }
      if (pos_end == std::string::npos) {
        QLOG(ERROR) << "Section '" << name << "' has start but no end marker.";
        return false;
      }
      if (pos_start > pos_end) {
        QLOG(ERROR) << "Section '" << name << "' has end before start marker.";
        return false;
      }
      size_t len = pos_end - pos_start + end_len;
      value->replace(pos_start, len, "");
    }
  }
  return true;
}

bool TemplateDictionary::Expand(std::string* output) const {
  for (const auto& [name, data] : data_) {
    if (!absl::visit([&](const auto& d) { return Expand(d, output); }, data)) {
      return false;
    }
  }
  return RemoveTags(output);
}

bool TemplateDictionary::Expand(const TagInfo<SectionDictionary>& tag,
                                std::string* output) const {
  if (absl::StrContains(*output, tag.tag)) {
    QLOG(ERROR) << "Simple tag '" << tag.tag << "' used in SectionDictionary.";
    return false;
  }
  while (true) {
    const auto [pos_start, start_len] = FindTag(*output, tag.tag_start);
    const auto [pos_end, end_len] = FindTag(*output, tag.tag_end);
    if (pos_start == std::string::npos) {
      // Presence is not required.
      // But end without start is bad.
      if (pos_end != std::string::npos) {
        return false;
      }
      break;
    }
    if (pos_end == std::string::npos || pos_end < pos_start) {
      return false;
    }
    const size_t tmpl_len = pos_end - pos_start - start_len;
    size_t pos_insert = pos_end + end_len;
    for (const auto& section : tag.data) {
      // Fetch the template (each time) into a new string.
      std::string value = output->substr(pos_start + start_len, tmpl_len);
      if (!section.Expand(&value)) {
        return false;
      }
      output->insert(pos_insert, value);
      pos_insert += value.length();
    }
    // Now remove the template as it is no longer needed.
    output->replace(pos_start, tmpl_len + start_len + end_len, "");
  }
  return true;
}

bool TemplateDictionary::Expand(const TagInfo<std::string>& tag,
                                std::string* output) const {
  if (tag.data.empty()) {
    while (true) {
      const auto [pos, len] = FindTag(*output, tag.tag);
      if (!len) {
        break;
      }
      output->erase(pos, len);
    }
  } else {
    absl::StrReplaceAll({{tag.tag, tag.data}}, output);
  }

  return true;
}

bool StringToTemplateCache(absl::string_view name, absl::string_view tpl,
                           DoNotStrip do_not_strip) {
  return g_template_cache.emplace(name, tpl).second;
}

bool ExpandTemplate(absl::string_view name, DoNotStrip do_not_strip,
                    const TemplateDictionary* dict, std::string* output) {
  return dict->ExpandTemplate(name, output);
}

}  // namespace proto_builder::oss
