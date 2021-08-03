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

#include "proto_builder/oss/proto_conversion_helpers.h"

namespace proto_builder::oss {

absl::StatusOr<google::protobuf::Duration> ConvertToProto(
    absl::Duration value) {
  absl::Duration my_duration = value;
  google::protobuf::Duration proto;
  const int64_t s =
      absl::IDivDuration(my_duration, absl::Seconds(1), &my_duration);
  const int64_t n =
      absl::IDivDuration(my_duration, absl::Nanoseconds(1), &my_duration);
  proto.set_seconds(s);
  proto.set_nanos(n);
  return proto;
}

absl::StatusOr<google::protobuf::Timestamp> ConvertToProto(absl::Time value) {
  google::protobuf::Timestamp proto;
  const int64_t s = absl::ToUnixSeconds(value);
  proto.set_seconds(s);
  proto.set_nanos((value - absl::FromUnixSeconds(s)) / absl::Nanoseconds(1));
  return proto;
}

}  // namespace proto_builder::oss
