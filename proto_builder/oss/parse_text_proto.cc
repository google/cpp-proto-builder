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

#include "proto_builder/oss/parse_text_proto.h"

#include <string>
#include <vector>

#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/text_format.h"
#include "absl/base/log_severity.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace proto_builder::oss {
namespace internal {
namespace {

// Collects errors from proto parsing.
class SilentErrorCollector : public google::protobuf::io::ErrorCollector {
 public:
  struct ErrorInfo {
    int line;
    int column;
    std::string message;
    absl::LogSeverity severity = absl::LogSeverity::kError;
  };

  void AddError(int line, int column, const std::string& message) override;
  void AddWarning(int line, int column, const std::string& message) override;

  std::string GetErrors() const;
  const std::vector<ErrorInfo>& errors() const { return errors_; }

 private:
  std::vector<ErrorInfo> errors_;
};

void SilentErrorCollector::AddError(int line, int column,
                                    const std::string& message) {
  ErrorInfo error;
  error.line = line;
  error.column = column;
  error.message = message;
  error.severity = absl::LogSeverity::kError;
  errors_.push_back(error);
}

void SilentErrorCollector::AddWarning(int line, int column,
                                      const std::string& message) {
  ErrorInfo error;
  error.line = line;
  error.column = column;
  error.message = message;
  error.severity = absl::LogSeverity::kWarning;
  errors_.push_back(error);
}

std::string SilentErrorCollector::GetErrors() const {
  std::string result;
  for (const auto& error : errors()) {
    absl::StrAppendFormat(&result, "Line %d, Col %d: %s\n", error.line,
                          error.column, error.message);
  }
  return result;
}

}  // namespace

absl::Status ParseTextInternal(absl::string_view text_proto, Message* message,
                               SourceLocation loc) {
  google::protobuf::TextFormat::Parser parser;
  SilentErrorCollector error_collector;
  parser.RecordErrorsTo(&error_collector);
  if (parser.ParseFromString(std::string(text_proto), message)) {
    return absl::OkStatus();
  }
  // TODO When SourceLocation becomes open source, pass loc to error
  // status. Until then, append loc to error message.
  return absl::InvalidArgumentError(
      absl::StrFormat("%s\nFile: '%s', Line: %d", error_collector.GetErrors(),
                      loc.file_name(), loc.line()));
}

}  // namespace internal
}  // namespace proto_builder::oss
