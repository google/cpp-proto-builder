# Copyright 2021 The CPP Proto Builder Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# READ: https://google.github.io/cpp-proto-builder

load("@rules_cc//cc:defs.bzl", "cc_proto_library")

package(
    # Only make libraries visible to the tool when necessary.
    # Otherwise we risk wide spread use of simplified re-implementations here.
    default_visibility = ["//visibility:private"],
)

exports_files([
    "simple_message_error.proto.bad",
    "simple_message.proto",
])

proto_library(
    name = "simple_message_proto",
    srcs = ["simple_message.proto"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
)

cc_proto_library(
    name = "simple_message_cc_proto",
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
    deps = [":simple_message_proto"],
)
