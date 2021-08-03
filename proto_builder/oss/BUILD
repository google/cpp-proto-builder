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

# copybara_config_test is used in this file.
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//proto_builder/oss:build_oss.bzl", "ALL_IMPLEMENTATIONS", "DEFINES", "IMPLEMENTATION")

package(
    # Only make libraries visible to the tool when necessary.
    # Otherwise we risk wide spread use of simplified re-implementations here.
    default_visibility = ["//visibility:private"],
)

cc_library(
    name = "init_program_cc",
    srcs = ["init_program.cc"],
    hdrs = ["init_program.h"],
    visibility = ["//proto_builder:__pkg__"],
    deps = [
        ":logging_macros_cc",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "init_program_test",
    srcs = ["init_program_test.cc"],
    deps = [
        ":init_program_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "qcheck_cc",
    srcs = ["qcheck.cc"],
    hdrs = ["qcheck.h"],
    deps = [":source_location_cc"],
)

cc_test(
    name = "qcheck_test",
    srcs = ["qcheck_test.cc"],
    deps = [
        ":qcheck_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "logging_cc",
    hdrs = ["logging.h"],
    defines = DEFINES["logging_cc"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
    deps = [
        ":logging_macros_cc",  # fixdeps: keep
        ":qcheck_cc",
        "@com_google_absl//absl/base:core_headers",
    ],
)

cc_test(
    name = "logging_test",
    srcs = ["logging_test.cc"],
    deps = [
        ":logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "logging_macros_cc",
    hdrs = ["logging_macros.h"],
    tags = ["avoid_dep"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
    deps = [
        ":qcheck_cc",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "logging_macros_test",
    srcs = ["logging_macros_test.cc"],
    deps = [
        ":logging_macros_cc",
        "@com_google_absl//absl/status",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "source_location_cc",
    hdrs = ["source_location.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
    ],
)

cc_test(
    name = "source_location_test",
    srcs = ["source_location_test.cc"],
    deps = [
        ":source_location_cc",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "parse_text_proto_cc",
    srcs = ["parse_text_proto.cc"],
    hdrs = ["parse_text_proto.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":qcheck_cc",
        ":source_location_cc",
        "@com_google_absl//absl/base:log_severity",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "parse_text_proto_test",
    srcs = ["parse_text_proto_test.cc"],
    deps = [
        ":parse_text_proto_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/tests:simple_message_cc_proto",
    ],
)

cc_library(
    name = "proto_conversion_helpers_cc",
    srcs = ["proto_conversion_helpers.cc"],
    hdrs = ["proto_conversion_helpers.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "proto_conversion_helpers_test",
    srcs = ["proto_conversion_helpers_test.cc"],
    deps = [
        ":proto_conversion_helpers_cc",
        "@com_google_absl//absl/time",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "file_base_cc",
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder/oss:__subpackages__"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "file_impl_oss_cc",
    srcs = ["file_impl_oss.cc"],
    deps = [
        ":file_base_cc",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

[
    [
        cc_library(
            name = "file_" + implementation + "_cc",
            hdrs = ["file.h"],
            deps = [
                ":file_base_cc",
                "@com_google_absl//absl/status:status",
                "@com_google_absl//absl/status:statusor",
                "@com_google_absl//absl/strings",
            ] + ALL_IMPLEMENTATIONS[implementation]["file"],
        ),
        cc_test(
            name = "file_" + implementation + "_test",
            srcs = ["file_test.cc"],
            deps = [
                ":file_" + implementation + "_cc",
                "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
            ] + ALL_IMPLEMENTATIONS[implementation]["file"],
        ),
    ]
    for implementation in ALL_IMPLEMENTATIONS
]

alias(
    name = "file_cc",
    actual = "file_" + IMPLEMENTATION + "_cc",
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
)

cc_library(
    name = "sourcefile_database_base_cc",
    hdrs = ["sourcefile_database.h"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder/oss:__subpackages__"],
    deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "sourcefile_database_impl_oss_cc",
    srcs = ["sourcefile_database_impl_oss.cc"],
    deps = [
        ":file_cc",
        ":sourcefile_database_base_cc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

[
    [
        cc_library(
            name = "sourcefile_database_" + implementation + "_cc",
            hdrs = [":sourcefile_database.h"],
            deps = [
                ":sourcefile_database_base_cc",
                "@com_google_protobuf//:protobuf",
            ] + ALL_IMPLEMENTATIONS[implementation]["sfdb"],
        ),
        cc_test(
            name = "sourcefile_database_" + implementation + "_test",
            srcs = ["sourcefile_database_test.cc"],
            data = [
                "@com_google_cpp_proto_builder//proto_builder/oss/tests:simple_message.proto",
                "@com_google_cpp_proto_builder//proto_builder/oss/tests:simple_message_error.proto.bad",
            ],
            deps = [
                ":file_cc",
                ":get_runfiles_dir_cc",
                "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
            ] + [":sourcefile_database_" + implementation + "_cc"],
        ),
    ]
    for implementation in ALL_IMPLEMENTATIONS
]

alias(
    name = "sourcefile_database_cc",
    actual = "sourcefile_database_" + IMPLEMENTATION + "_cc",
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
)

cc_library(
    name = "template_dictionary_cc",
    srcs = ["template_dictionary.cc"],
    hdrs = ["template_dictionary.h"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [
        ":logging_cc",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:variant",
        "@com_google_re2//:re2",
    ],
)

cc_test(
    name = "template_dictionary_test",
    srcs = ["template_dictionary_test.cc"],
    deps = [
        ":template_dictionary_cc",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "get_runfiles_dir_cc",
    srcs = ["get_runfiles_dir.cc"],
    hdrs = ["get_runfiles_dir.h"],
    deps = [
        ":logging_cc",
        "@bazel_tools//tools/cpp/runfiles",
    ],
)

cc_test(
    name = "get_runfiles_dir_test",
    srcs = ["get_runfiles_dir_test.cc"],
    data = ["get_runfiles_dir_test.cc"],
    deps = [
        ":file_cc",
        ":get_runfiles_dir_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "util_cc",
    srcs = ["util_internal.h"],
    hdrs = ["util.h"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__pkg__"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_binary(
    name = "parse_textproto_file",
    srcs = ["parse_textproto_file.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":file_cc",
        ":logging_cc",
        ":parse_text_proto_cc",
        ":source_location_cc",
        ":sourcefile_database_cc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "util_test",
    srcs = ["util_test.cc"],
    deps = [
        ":util_cc",
        "@com_google_absl//absl/status",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "unified_diff_cc",
    srcs = ["unified_diff.cc"],
    hdrs = ["unified_diff.h"],
    visibility = ["//proto_builder:__pkg__"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "unified_diff_test",
    srcs = ["unified_diff_test.cc"],
    deps = [
        ":unified_diff_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

bzl_library(
    name = "build_oss_bzl",
    srcs = ["build_oss.bzl"],
    visibility = ["//visibility:private"],
)
