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

#include "proto_builder/oss/unified_diff.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/types/span.h"

namespace proto_builder::oss {

void AppendLines(absl::Span<absl::string_view> content, size_t& start_index,
                 absl::string_view marker, std::string* out) {
  absl::StrAppend(out, "Line Number: ", start_index + 1, "-", content.size(),
                  "\n");
  while (start_index < content.size()) {
    absl::StrAppend(out, marker, content[start_index], "\n");
    ++start_index;
  }
}

std::string UnifiedDiff(absl::string_view left, absl::string_view right,
                        absl::string_view left_name,
                        absl::string_view right_name, int context_size) {
  std::string out;
  absl::StrAppend(&out, "--- ", left_name, "\n");
  absl::StrAppend(&out, "+++ ", right_name, "\n");
  size_t baseline = out.size();
  std::vector<absl::string_view> left_content = absl::StrSplit(left, '\n');
  std::vector<absl::string_view> right_content = absl::StrSplit(right, '\n');
  size_t line_num = 0;
  while (line_num < left_content.size() && line_num < right_content.size()) {
    if (left_content[line_num] != right_content[line_num]) {
      absl::StrAppend(&out, "\n");
      absl::StrAppend(&out, "Line Number: ", line_num + 1, "\n");
      absl::StrAppend(&out, "--- ", left_content[line_num], "\n");
      absl::StrAppend(&out, "+++ ", right_content[line_num], "\n");
    }
    ++line_num;
  }

  // Check for unequal line numbers.
  if (line_num < left_content.size()) {
    AppendLines(absl::MakeSpan(left_content), line_num, "--- ", &out);
  }

  if (line_num < right_content.size()) {
    AppendLines(absl::MakeSpan(right_content), line_num, "+++ ", &out);
  }

  // If no difference, return empty string.
  return baseline != out.size() ? out : "";
}

}  // namespace proto_builder::oss
