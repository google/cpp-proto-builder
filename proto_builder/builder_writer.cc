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

#include "proto_builder/builder_writer.h"

#include <string>
#include <utility>

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging.h"
#include "google/protobuf/descriptor.pb.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"

namespace proto_builder {

std::string Where_Name(Where where) {
  switch (where) {
    case HEADER:
      return "HEADER";
    case SOURCE:
      return "SOURCE";
    case INTERFACE:
      return "INTERFACE";
  }
}

std::string FormatInclude(absl::string_view include) {
  if (include[0] == '<' || include[0] == '"') {
    return std::string(include);
  } else {
    return absl::StrCat("\"", include, "\"");
  }
}

static inline bool TryStripPrefixString(absl::string_view str,
                                        absl::string_view prefix,
                                        std::string* result) {
  bool res = absl::ConsumePrefix(&str, prefix);
  result->assign(str.begin(), str.end());
  return res;
}

CodeInfoCollector::CodeInfoCollector()
    : CodeInfoCollector(std::vector<std::string>()) {}

CodeInfoCollector::CodeInfoCollector(
    const std::vector<std::string>& package_path)
    : package_path_(package_path),
      namespace_path_(
          package_path.empty()
              ? ""
              : absl::StrCat("::", absl::StrJoin(package_path, "::"))),
      include_map_({{HEADER, {}}, {SOURCE, {}}, {INTERFACE, {}}}) {}

void CodeInfoCollector::AddInclude(Where where, absl::string_view include) {
  include_map_[where].insert(FormatInclude(include));
}

void CodeInfoCollector::AddInclude(Where where, const ::google::protobuf::Descriptor& descriptor) {
  AddInclude(where, descriptor.full_name(), descriptor.file());
}

void CodeInfoCollector::AddInclude(Where where,
                                   const EnumDescriptor& descriptor) {
  AddInclude(where, descriptor.full_name(), descriptor.file());
}

void CodeInfoCollector::AddInclude(Where where, const std::string& full_name,
                                   const ::google::protobuf::FileDescriptor* file) {
  if (!file) {
    AddInclude(where, absl::StrCat("ERROR_HEADER_UNKNOWN_FOR_", full_name));
    return;
  }

  const std::string& filename = file->name();
  // Proto libraries defining cc_api_version == 1 don't have .proto files
  // available, so we have to chop that off and use .pb instead.
  std::string include_string_base =
      true // api version 1
          ? absl::Substitute("$0.pb", filename.substr(0, filename.rfind('.')))
          : filename;

  AddInclude(where, absl::Substitute(
                        "\"$0.h\"  // IWYU pragma: export",  // Export the type
                        include_string_base));
}

const std::set<std::string>& CodeInfoCollector::GetIncludes(Where where) const {
  static const auto& no_includes = *new std::set<std::string>();
  auto iter = include_map_.find(where);
  if (iter == include_map_.end()) {
    return no_includes;
  }
  return iter->second;
}

std::string CodeInfoCollector::RelativeType(const std::string& cpp_type) const {
  if (namespace_path_.empty()) {
    return cpp_type;
  }
  std::string result;
  if (!TryStripPrefixString(cpp_type, namespace_path_, &result) ||
      !TryStripPrefixString(result, "::", &result) || result.empty()) {
    return cpp_type;
  }
  return result;
}

CodeInfoCollector* WrappingBuilderWriter::CodeInfo() {
  return wrapped_writer_->CodeInfo();
}

const CodeInfoCollector* WrappingBuilderWriter::CodeInfo() const {
  return wrapped_writer_->CodeInfo();
}

void WrappingBuilderWriter::WrappedWrite(Where where, const std::string& line) {
  wrapped_writer_->Write(where, line);
}

BufferWriter::BufferWriter(const std::vector<std::string>& package_path)
    : buffer_{{HEADER, {}}, {SOURCE, {}}, {INTERFACE, {}}},
      code_info_(package_path) {}

void BufferWriter::Write(Where to, const std::string& line) {
  buffer_[to].push_back(line);
}

absl::Status BufferWriter::WriteFile(Where from,
                                     const std::string& filename) const {
  return file::oss::SetContents(filename, absl::StrJoin(From(from), "\n"));
}

void BufferWriter::MoveContents(Where from, BufferWriter* to_writer) {
  for (auto& line : buffer_.at(from)) {
    to_writer->buffer_.at(from).emplace_back(std::move(line));
  }
  buffer_.at(from).clear();
}

CodeInfoCollector* BufferWriter::CodeInfo() { return &code_info_; }
const CodeInfoCollector* BufferWriter::CodeInfo() const { return &code_info_; }

void NoDoubleEmptyLineWriter::Write(Where to, const std::string& line) {
  // Do not start with an empty line or write two consecutive empty lines.
  // We store the negation into the map. That means upon the first line, the
  // map assumes the last line was (not-etmpty==false) == empty.
  bool is_empty = line.empty();
  auto last_non_empty_it = last_non_empty_.insert({to, !is_empty}).first;
  if (!is_empty || last_non_empty_it->second) {
    WrappedWrite(to, line);
  }
  last_non_empty_it->second = !is_empty;
}

IndentWriter::IndentWriter(BuilderWriter* wrapped_writer,
                           const std::string& head_indent,
                           const std::string& body_indent)
    : WrappingBuilderWriter(PBCC_DIE_IF_NULL(wrapped_writer)) {
  SetIndent(HEADER, head_indent);
  SetIndent(INTERFACE, head_indent);
  SetIndent(SOURCE, body_indent);
}

void IndentWriter::Write(Where to, const std::string& line) {
  auto indent_it = indent_.find(to);
  if (line.empty() || indent_it == indent_.end()) {
    WrappedWrite(to, line);
  } else {
    WrappedWrite(to, absl::StrCat(indent_it->second, line));
  }
}

void IndentWriter::SetIndent(Where to, absl::string_view indent) {
  indent_[to] = std::string(indent);
}

}  // namespace proto_builder
