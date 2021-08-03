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

#ifndef PROTO_BUILDER_OSS_PARSE_TEXT_PROTO_H_
#define PROTO_BUILDER_OSS_PARSE_TEXT_PROTO_H_

#include <string>

#include "proto_builder/oss/qcheck.h"
#include "proto_builder/oss/source_location.h"
#include "google/protobuf/message.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace proto_builder::oss {

using ::google::protobuf::Message;

namespace internal {

absl::Status ParseTextInternal(absl::string_view text_proto, Message* message,
                               SourceLocation loc);

}  // namespace internal

// Parses the text in 'text_proto' into a prot message of type 'T'.
// The function dies if parsing fails.
template <typename T>
T ParseTextOrDie(absl::string_view text_proto,
                 SourceLocation loc = SourceLocation::current()) {
  T message;
  absl::Status result = internal::ParseTextInternal(text_proto, &message, loc);
  QCheck(
      result.ok(),
      absl::StrFormat("ParseTextOrDie<%s>", T::GetDescriptor()->name()).c_str(),
      QCheckOptions::kDefault, loc)
      << result;
  return message;
}

class ParseTextProtoHelper final {
 public:
  ParseTextProtoHelper(absl::string_view text_proto, SourceLocation loc)
      : text_proto_(text_proto), loc_(loc), parsed_(false) {}

  ~ParseTextProtoHelper() {
    QCheck(parsed_,
           absl::StrFormat("ParseTextProtoOrDie<T> result unused").c_str());
  }

  template <typename T>
  operator T() {  // NOLINT clangtidy(google-explicit-constructor)
    parsed_ = true;
    return ParseTextOrDie<T>(text_proto_, loc_);
  }

 private:
  const std::string text_proto_;
  const SourceLocation loc_;
  bool parsed_;
};

inline ParseTextProtoHelper ParseTextProtoOrDie(
    absl::string_view text_proto,
    SourceLocation loc = SourceLocation::current()) {
  return ParseTextProtoHelper(text_proto, loc);
}

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_PARSE_TEXT_PROTO_H_
