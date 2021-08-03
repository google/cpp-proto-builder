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

load(":build_oss.bzl", "proto_builder_config")
load("//proto_builder/oss:build_oss.bzl", "ALL_IMPLEMENTATIONS")
load("//proto_builder:build_oss.bzl", "text_proto_test")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(default_visibility = ["//visibility:private"])

exports_files([
    "proto_builder_verify_test.sh",
    "proto_builder_config_oss.textproto",
])

proto_library(
    name = "proto_builder_proto",
    srcs = ["proto_builder.proto"],
    compatible_with = proto_builder_config.COMPATIBLE_WITH,
    features = ["-proto_dynamic_mode_static_link"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_proto_library(
    name = "proto_builder_cc_proto",
    compatible_with = proto_builder_config.COMPATIBLE_WITH,
    visibility = ["//visibility:public"],
    deps = [":proto_builder_proto"],
)

cc_library(
    name = "builder_writer_cc",
    srcs = ["builder_writer.cc"],
    hdrs = ["builder_writer.h"],
    deps = [
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "builder_writer_test",
    srcs = ["builder_writer_test.cc"],
    deps = [
        ":builder_writer_cc",
        ":proto_builder_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

genrule(
    name = "proto_builder_config_data_gen",
    srcs = [
        ":proto_builder_config_oss.textproto",
        ":proto_builder_config_data.gen.cc",
    ],
    outs = ["proto_builder_config_data.cc"],
    cmd = """
    export DATA="$(location proto_builder_config_oss.textproto)" &&
    cat  $(location proto_builder_config_data.gen.cc) > $@ &&
    # Replace the subject string with preprocessed text from the file.
    sed -i -e "/__PROTO_TEXT_CONFIG__/{r $${DATA}" -e 'd}' $@
""",
)

cc_library(
    name = "proto_builder_config_cc",
    srcs = [
        ":proto_builder_config.cc",
        ":proto_builder_config_data.cc",
    ],
    hdrs = ["proto_builder_config.h"],
    deps = [
        ":proto_builder_cc_proto",
        ":util_cc",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:util_cc",
        "@com_google_protobuf//:protobuf",
        "@com_google_re2//:re2",
    ],
)

cc_test(
    name = "proto_builder_config_test",
    srcs = ["proto_builder_config_test.cc"],
    deps = [
        ":field_builder_cc",
        ":proto_builder_config_cc",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

# This is a 'magic' library that interacts with the builder. DO NOT MODIFY HERE.
cc_library(
    name = "proto_builder_conversions",
    compatible_with = proto_builder_config.COMPATIBLE_WITH,
    visibility = ["//visibility:public"],
    deps = proto_builder_config.CONVERSION_DEPS,
)

cc_library(
    name = "field_builder_cc",
    srcs = ["field_builder.cc"],
    hdrs = ["field_builder.h"],
    deps = [
        ":builder_writer_cc",
        ":proto_builder_cc_proto",
        ":proto_builder_config_cc",
        ":util_cc",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)

cc_test(
    name = "field_builder_test",
    srcs = ["field_builder_test.cc"],
    deps = [
        ":field_builder_cc",
        ":proto_builder_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/tests:source_location_cc_proto",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_message_cc_proto",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_types_cc_proto",
    ],
)

cc_library(
    name = "message_builder_cc",
    srcs = ["message_builder.cc"],
    hdrs = ["message_builder.h"],
    deps = [
        ":field_builder_cc",
        ":proto_builder_cc_proto",
        ":proto_builder_config_cc",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)

cc_test(
    name = "message_builder_test",
    srcs = ["message_builder_test.cc"],
    data = ["@com_google_cpp_proto_builder//proto_builder/tests:golden_files"],
    deps = [
        ":message_builder_cc",
        "//proto_builder/oss:unified_diff_cc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_message_cc_proto",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_output_cc_proto",
        "@com_google_cpp_proto_builder//proto_builder/tests:validator_cc_proto",
    ],
)

cc_library(
    name = "template_builder_cc",
    srcs = ["template_builder.cc"],
    hdrs = ["template_builder.h"],
    deps = [
        ":builder_writer_cc",
        ":message_builder_cc",
        ":proto_builder_config_cc",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:template_dictionary_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:util_cc",
        "@com_google_protobuf//:protobuf",
        "@com_google_re2//:re2",
    ],
)

cc_test(
    name = "template_builder_test",
    srcs = ["template_builder_test.cc"],
    deps = [
        ":template_builder_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
    ],
)

cc_library(
    name = "descriptor_util_cc",
    srcs = ["descriptor_util.cc"],
    hdrs = ["descriptor_util.h"],
    deps = [
        ":template_builder_cc",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:sourcefile_database_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:util_cc",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "descriptor_util_test",
    srcs = ["descriptor_util_test.cc"],
    data = [
        "@com_google_cpp_proto_builder//proto_builder/tests:test_import_message_proto",
    ],
    deps = [
        ":descriptor_util_cc",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:util_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_import_message_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "util_cc",
    srcs = ["util.cc"],
    hdrs = ["util.h"],
    deps = [
        ":proto_builder_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)

cc_test(
    name = "util_test",
    srcs = ["util_test.cc"],
    deps = [
        ":proto_builder_cc_proto",
        ":util_cc",
        "@com_google_absl//absl/strings",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        "@com_google_cpp_proto_builder//proto_builder/tests:test_message_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

genrule(
    name = "proto_builder_data_cc_gen",
    srcs = [
        ":default.cc.tpl",
        ":default.h.tpl",
        ":default_interface.h.tpl",
        ":proto_builder_data.gen.cc",
    ],
    outs = ["proto_builder_data.cc"],
    cmd = """
    cat  $(location proto_builder_data.gen.cc) > $@ &&

    # Replace the subject string with preprocessed text from the file.
    sed -i -e '/__DEFAULT_CC_TPL__/{r $(location default.cc.tpl)' -e 'd}' $@ &&

    # Replace the subject string with preprocessed text from the file.
    sed -i -e '/__INTERFACE_H_TPL__/{r $(location default_interface.h.tpl)' -e 'd}' $@ &&

    # Replace the subject string with preprocessed text from the file.
    sed -i -e '/__DEFAULT_H_TPL__/{r $(location default.h.tpl)' -e 'd}' $@
""",
)

cc_binary(
    name = "proto_builder",
    srcs = [
        "proto_builder.cc",
        "proto_builder_data.h",
        ":proto_builder_data.cc",
    ],
    args = proto_builder_config.ARGS,
    visibility = ["//visibility:public"],
    deps = [
        ":descriptor_util_cc",
        ":field_builder_cc",
        ":message_builder_cc",
        ":proto_builder_config_cc",
        ":template_builder_cc",
        "//proto_builder/oss:init_program_cc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_cpp_proto_builder//proto_builder/oss:file_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:logging_macros_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:sourcefile_database_cc",
        "@com_google_cpp_proto_builder//proto_builder/oss:util_cc",
        "@com_google_protobuf//:protobuf",
    ],
)

bzl_library(
    name = "build_oss_bzl",
    srcs = ["build_oss.bzl"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
)

bzl_library(
    name = "proto_builder_bzl",
    srcs = ["proto_builder.bzl"],
    visibility = ["@com_google_cpp_proto_builder//proto_builder:__subpackages__"],
    deps = [
        ":build_oss_bzl",
        "@bazel_skylib//lib:dicts",
        "@rules_cc//cc:find_cc_toolchain.bzl",
    ],
)

[
    text_proto_test(
        name = "proto_builder_config_" + target + "_textproto_test",
        src = "proto_builder_config_" + target + ".textproto",
        message = "proto_builder.ProtoBuilderConfig",
        deps = [":proto_builder_proto"],
    )
    for target in ALL_IMPLEMENTATIONS
]

# export files for bazel rules
exports_files(
    [
        "default.cc.tpl",
        "default.h.tpl",
        "default_interface.h.tpl",
    ],
)
