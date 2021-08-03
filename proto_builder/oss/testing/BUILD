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

package(
    # Only make libraries visible to the tool when necessary.
    # Otherwise we risk wide spread use of simplified re-implementations here.
    default_visibility = ["//visibility:private"],
)

cc_library(
    name = "status_matcher_cc",
    testonly = True,
    hdrs = ["status_matcher.h"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "proto_test_util_cc",
    testonly = True,
    srcs = ["proto_test_util.cc"],
    hdrs = ["proto_test_util.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_googletest//:gtest",
        "@com_google_protobuf//:protobuf",
        "@com_google_re2//:re2",
    ],
)

cc_test(
    name = "proto_test_util_test",
    srcs = ["proto_test_util_test.cc"],
    deps = [
        ":proto_test_util_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/tests:simple_message_cc_proto",
    ],
)

cc_library(
    name = "cpp_pb_gunit_cc",
    testonly = True,
    hdrs = ["cpp_pb_gunit.h"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
    deps = [
        ":proto_test_util_cc",
        ":status_matcher_cc",
    ],
)
