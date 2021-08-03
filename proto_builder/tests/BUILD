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

# Since this is a tool, there is no need to interact with the code directly.
# Thus all rules are 'private' with the exception of:
# . 'proto_builder_proto': The configuration framework.
# . 'proto_builder': The actual 'proto_builder' tool.

load(
    "//proto_builder:proto_builder.bzl",
    "cc_proto_builder_library",
    "proto_builder",
    "proto_builder_test",
)
load(
    "//proto_builder/tests:test_util.bzl",
    "get_proto_builder_dep",
    "proto_builder_test_case",
)
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(default_visibility = ["//visibility:private"])

filegroup(
    name = "golden_files",
    data = [
        "test_message.cc.exp",
        "test_message.h.exp",
        "test_output.cc.exp",
        "test_output.h.exp",
        "validator.cc.exp",
        "validator.h.exp",
    ],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
)

proto_library(
    name = "test_message_proto",
    testonly = 1,
    srcs = ["test_message.proto"],
    deps = [
        ":extra_test_message_proto",
        ":map_value_test_message_proto",
        "@com_google_cpp_proto_builder//proto_builder:proto_builder_proto",
    ],
)

cc_proto_library(
    name = "test_message_cc_proto",
    testonly = 1,
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [":test_message_proto"],
)

proto_library(
    name = "extra_test_message_proto",
    testonly = 1,
    srcs = ["extra_test_message.proto"],
)

cc_proto_library(
    name = "extra_test_message_cc_proto",
    testonly = 1,
    deps = [":extra_test_message_proto"],
)

proto_library(
    name = "test_import_message_proto",
    testonly = 1,
    srcs = ["test_import_message.proto"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = ["extra_test_message_proto"],
)

cc_proto_library(
    name = "test_import_message_cc_proto",
    testonly = 1,
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = ["test_import_message_proto"],
)

proto_builder(
    name = "test_import_message_builder_star",
    testonly = 1,
    cc_proto_library_deps = ["test_import_message_cc_proto"],
    proto = ["*"],
)

proto_builder_test(
    name = "test_import_message_builder_star_builder_test",
    expected_hdr = "test_import_message_builder_star.h.exp",
    expected_src = "test_import_message_builder_star.cc.exp",
    proto_builder_dep = "test_import_message_builder_star",
)

proto_builder(
    name = "test_import_message_builder_auto",
    testonly = 1,
    cc_proto_library_deps = ["test_import_message_cc_proto"],
    max_field_depth = 99,
    proto = ["*+"],
)

proto_builder_test(
    name = "test_import_message_builder_auto_builder_test",
    expected_hdr = "test_import_message_builder_auto.h.exp",
    expected_src = "test_import_message_builder_auto.cc.exp",
    proto_builder_dep = "test_import_message_builder_auto",
)

proto_builder(
    name = "test_import_message_builder_99",
    testonly = 1,
    cc_proto_library_deps = ["test_import_message_cc_proto"],
    max_field_depth = 99,
    proto = ["**"],
)

proto_builder_test(
    name = "test_import_message_builder_99_builder_test",
    expected_hdr = "test_import_message_builder_99.h.exp",
    expected_src = "test_import_message_builder_99.cc.exp",
    proto_builder_dep = "test_import_message_builder_99",
)

proto_library(
    name = "map_value_test_message_proto",
    testonly = 1,
    srcs = ["map_value_test_message.proto"],
)

cc_proto_library(
    name = "map_value_test_message_cc_proto",
    testonly = 1,
    deps = [":map_value_test_message_proto"],
)

proto_library(
    name = "test_types_proto",
    testonly = 1,
    srcs = ["test_types.proto"],
)

cc_proto_library(
    name = "test_types_cc_proto",
    testonly = 1,
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [":test_types_proto"],
)

proto_library(
    name = "test_conversions_proto",
    testonly = 1,
    srcs = ["test_conversions.proto"],
    deps = [
        "@com_google_cpp_proto_builder//proto_builder:proto_builder_proto",
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
)

cc_proto_library(
    name = "test_conversions_cc_proto",
    testonly = 1,
    deps = [":test_conversions_proto"],
)

proto_library(
    name = "test_output_proto",
    testonly = 1,
    srcs = ["test_output.proto"],
    deps = ["@com_google_cpp_proto_builder//proto_builder:proto_builder_proto"],
)

cc_proto_library(
    name = "test_output_cc_proto",
    testonly = 1,
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [":test_output_proto"],
)

proto_library(
    name = "test_multiple_messages_proto",
    testonly = 1,
    srcs = ["test_multiple_messages.proto"],
)

cc_proto_library(
    name = "test_multiple_messages_cc_proto",
    testonly = 1,
    deps = [":test_multiple_messages_proto"],
)

proto_library(
    name = "test_recursive_messages_proto",
    testonly = 1,
    srcs = ["test_recursive_messages.proto"],
)

cc_proto_library(
    name = "test_recursive_messages_cc_proto",
    testonly = 1,
    deps = ["test_recursive_messages_proto"],
)

cc_proto_builder_library(
    name = "test_recursive_messages_builder",
    testonly = 1,
    cc_proto_library_deps = ["test_recursive_messages_cc_proto"],
)

proto_builder_test(
    name = "test_recursive_messages_builder_builder_test",
    expected_hdr = "test_recursive_messages_builder.h.exp",
    expected_src = "test_recursive_messages_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep(
        "test_recursive_messages_builder",
    ),
)

proto_builder(
    name = "test_recursive_messages_builder_1",
    testonly = 1,
    cc_proto_library_deps = ["test_recursive_messages_cc_proto"],
    max_field_depth = 1,
    proto = ["*"],
)

proto_builder_test(
    name = "test_recursive_messages_builder_1_builder_test",
    expected_hdr = "test_recursive_messages_builder_1.h.exp",
    expected_src = "test_recursive_messages_builder_1.cc.exp",
    proto_builder_dep = "test_recursive_messages_builder_1",
)

cc_proto_builder_library(
    name = "test_message_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_message_cc_proto"],
    proto = ["proto_builder.TestMessage"],
)

proto_builder_test(
    name = "test_message_builder_builder_test",
    expected_hdr = "test_message_builder.h.exp",
    expected_src = "test_message_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep("test_message_builder"),
)

cc_test(
    name = "test_message_builder_test",
    srcs = ["test_message_builder_test.cc"],
    deps = [
        ":test_message_builder",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_proto_builder_library(
    name = "test_message_recursive_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_message_cc_proto"],
    proto = ["**"],
)

proto_builder_test(
    name = "test_message_recursive_builder_builder_test",
    expected_hdr = "test_message_recursive_builder.h.exp",
    expected_src = "test_message_recursive_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep("test_message_recursive_builder"),
)

proto_builder(
    name = "test_message_recursive_builder_99",
    testonly = 1,
    cc_proto_library_deps = ["test_message_cc_proto"],
    max_field_depth = 99,
    proto = ["**"],
)

proto_builder_test(
    name = "test_message_recursive_builder_99_builder_test",
    expected_hdr = "test_message_recursive_builder_99.h.exp",
    expected_src = "test_message_recursive_builder_99.cc.exp",
    proto_builder_dep = "test_message_recursive_builder_99",
)

cc_test(
    name = "test_message_recursive_builder_test",
    srcs = ["test_message_recursive_builder_test.cc"],
    deps = [
        ":test_message_recursive_builder",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_proto_builder_library(
    name = "test_conversions_sub_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_conversions_cc_proto"],
    proto = ["proto_builder.Conversions.Sub"],
    deps = ["@com_google_cpp_proto_builder//proto_builder:proto_builder_conversions"],
)

proto_builder_test(
    name = "test_conversions_sub_builder_builder_test",
    expected_hdr = "test_conversions_sub_builder.h.exp",
    expected_src = "test_conversions_sub_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep("test_conversions_sub_builder"),
)

cc_proto_builder_library(
    name = "test_conversions_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_conversions_cc_proto"],
    proto = ["proto_builder.Conversions"],
    deps = [
        ":test_conversions_sub_builder",
        "@com_google_cpp_proto_builder//proto_builder:proto_builder_conversions",
    ],
)

proto_builder_test(
    name = "test_conversions_builder_builder_test",
    expected_hdr = "test_conversions_builder.h.exp",
    expected_src = "test_conversions_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep("test_conversions_builder"),
)

cc_test(
    name = "test_conversions_builder_test",
    srcs = ["test_conversions_builder_test.cc"],
    deps = [
        ":test_conversions_builder",
        ":test_conversions_sub_builder",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_proto_builder_library(
    name = "test_multiple_messages_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_multiple_messages_cc_proto"],
)

proto_builder_test(
    name = "test_multiple_messages_builder_builder_test",
    expected_hdr = "test_multiple_messages_builder.h.exp",
    expected_src = "test_multiple_messages_builder.cc.exp",
    proto_builder_dep = get_proto_builder_dep("test_multiple_messages_builder"),
)

cc_test(
    name = "test_multiple_messages_builder_test",
    srcs = ["test_multiple_messages_builder_test.cc"],
    deps = [
        ":test_multiple_messages_builder",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

# Test on a library that has more than one .proto file source.
proto_library(
    name = "multiple_proto_files_proto",
    testonly = 1,
    srcs = [
        "multiple_proto_files_1.proto",
        "multiple_proto_files_2.proto",
    ],
    deps = ["@com_google_cpp_proto_builder//proto_builder:proto_builder_proto"],
)

cc_proto_library(
    name = "multiple_proto_files_cc_proto",
    testonly = 1,
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [":multiple_proto_files_proto"],
)

cc_proto_builder_library(
    name = "multiple_proto_files_cc_proto_builder",
    testonly = 1,
    cc_proto_library_deps = [":multiple_proto_files_cc_proto"],
    proto = [
        "proto_builder.MessageFromFirstFile",
        "proto_builder.MessageFromSecondFile",
        "proto_builder.AnotherMessageFromSecondFile",
    ],
)

proto_builder_test(
    name = "multiple_proto_files_cc_proto_builder_builder_test",
    expected_hdr = "multiple_proto_files.h.exp",
    expected_src = "multiple_proto_files.cc.exp",
    proto_builder_dep = get_proto_builder_dep("multiple_proto_files_cc_proto_builder"),
)

proto_builder(
    name = "select_proto_builder",
    testonly = 1,
    cc_proto_library_deps = [":test_multiple_messages_cc_proto"],
    proto = [
        "proto_builder.TestMessageA",
        "proto_builder.TestMessageB",
        # Not emitting TestMessageC, only its sub-message TestMessageD.
        "proto_builder.TestMessageC.TestMessageD",
    ],
)

proto_builder_test(
    name = "select_proto_builder_test",
    expected_hdr = "test_selected_messages.h.exp",
    expected_src = "test_selected_messages.cc.exp",
    proto_builder_dep = "select_proto_builder",
)

proto_builder_test_case(
    name = "advanced",
    visibility = ["@com_google_cpp_proto_builder//proto_builder/tests/google:__pkg__"],
)

proto_builder_test_case(
    name = "automatic",
    proto_deps = [
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
)

cc_library(
    name = "conversion_data_util_cc",
    testonly = 1,
    hdrs = ["conversion_data_util.h"],
)

proto_builder_test_case(
    name = "conversion_data",
    cc_deps = [
        ":conversion_data_util_cc",
    ],
)

cc_library(
    name = "inherit_util_cc",
    testonly = 1,
    hdrs = ["inherit_util.h"],
)

proto_builder_test_case(
    name = "inherit",
    cc_deps = [
        ":inherit_util_cc",
    ],
)

cc_library(
    # Nothing depends on this, it is only here to make tooling happy.
    name = "interface_util",
    testonly = 1,
    hdrs = [
        "interface_cc_proto_builder.interface.h",
        "interface_util.h",
    ],
    deps = [":interface_cc_proto"],
)

proto_builder_test_case(
    name = "macro",
    extra_hdrs = ["predicate_util.h"],
)

proto_builder_test_case(
    name = "interface",
    extra_hdrs = ["interface_util.h"],
    make_interface = 1,
)

cc_library(
    # Nothing depends on this, it is only here to make tooling happy.
    name = "predicate_util",
    testonly = 1,
    hdrs = ["predicate_util.h"],
    deps = ["@com_google_absl//absl/status"],
)

proto_builder_test_case(
    name = "predicate",
    extra_hdrs = ["predicate_util.h"],
)

proto_builder_test_case(
    name = "proto3",
)

proto_builder_test_case(
    name = "source_location",
    hdrs = ["source_location.h.tpl"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
)

cc_library(
    name = "validator_cc_proto_validator",
    testonly = 1,
    hdrs = ["validator.proto.validator.h"],
    deps = [
        ":validator_cc_proto",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

proto_builder_test_case(
    name = "validator",
    cc_deps = [
        "validator_cc_proto_validator",
    ],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
)

bzl_library(
    name = "test_util_bzl",
    srcs = ["test_util.bzl"],
    deps = [
        "@com_google_cpp_proto_builder//proto_builder:build_oss_bzl",
        "@com_google_cpp_proto_builder//proto_builder:proto_builder_bzl",
    ],
)
