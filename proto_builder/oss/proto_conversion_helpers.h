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

#ifndef PROTO_BUILDER_OSS_PROTO_CONVERSION_HELPERS_H_
#define PROTO_BUILDER_OSS_PROTO_CONVERSION_HELPERS_H_

#include "google/protobuf/duration.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "absl/status/statusor.h"
#include "absl/time/time.h"

namespace proto_builder::oss {

// Encodes an absl::Duration as a google::protobuf::Duration.
absl::StatusOr<google::protobuf::Duration> ConvertToProto(
    const absl::Duration value);

// Encodes an absl::Time as a google::protobuf::Timestamp.
absl::StatusOr<google::protobuf::Timestamp> ConvertToProto(
    const absl::Time value);

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_PROTO_CONVERSION_HELPERS_H_
