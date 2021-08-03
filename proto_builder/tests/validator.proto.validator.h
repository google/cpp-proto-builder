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

// Automatically generated using https://google.github.io/cpp-proto-builder

#ifndef PROTO_BUILDER_TESTS_VALIDATOR_PROTO_VALIDATOR_H_
#define PROTO_BUILDER_TESTS_VALIDATOR_PROTO_VALIDATOR_H_

#include <string>

#include "proto_builder/tests/validator.pb.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

namespace protobuf::contrib::validator {

using proto_builder::tests::Validator;

inline bool IsValidAddress(const std::string& address) {
  return address.empty() || address == "8.8.8.8" || address == "8.8.4.4";
}

inline absl::Status Validate(const Validator::Fallback& v) {
  if (!IsValidAddress(v.address())) {
    return absl::InvalidArgumentError("Bad fallback address");
  }
  return absl::OkStatus();
}

inline absl::Status Validate(const Validator& v) {
  if (!IsValidAddress(v.address())) {
    return absl::InvalidArgumentError("Bad address");
  }
  for (const auto& f : v.fallback()) {
    absl::Status s = Validate(f);
    if (!s.ok()) {
      return s;
    }
  }
  for (const auto& n : v.not_validated()) {
    if (!IsValidAddress(n.address())) {
      return absl::InvalidArgumentError("Bad non-self-validated address");
    }
  }
  for (const auto& [k, f] : v.named_fallback()) {
    if (!IsValidAddress(f.address())) {
      return absl::InvalidArgumentError(absl::StrCat("Bad address named: ", k));
    }
  }
  if (!IsValidAddress(v.options().address())) {
    return absl::InvalidArgumentError("Bad options address");
  }
  return absl::OkStatus();
}

}  // namespace protobuf::contrib::validator

#endif  // PROTO_BUILDER_TESTS_VALIDATOR_PROTO_VALIDATOR_H_
