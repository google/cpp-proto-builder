# Copyright 2021 The CPP Proto Builder Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# READ: https://google.github.io/cpp-proto-builder

"""Private helper functions for proto_builder testing."""

load("//proto_builder:build_oss.bzl", "proto_builder_config")
load(
    "//proto_builder:proto_builder.bzl",
    "cc_proto_builder_library",
    "proto_builder_test",
)
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

def get_proto_builder_dep(name):
    if proto_builder_config.USE_CC_LIBRARY_MACRO:
        return "__" + name + "_proto_builder"
    else:
        return name

def proto_builder_test_case(
        name = None,
        proto_deps = [],
        hdrs = [],
        srcs = [],
        cc_deps = [],
        cc_test_deps = [],
        extra_hdrs = [],
        extra_srcs = [],
        make_interface = False,
        visibility = None):
    """Simplifies testing proto_builder.

    Args:
        name:           Prefix to all rules.
        proto_deps:     Extra proto_library dependencies.
        hdrs:           Headers for cc_proto_builder_library rule.
        srcs:           Sources for cc_proto_builder_library rule.
        cc_deps:        Extra cc_proto_bilder_library dependencies.
        cc_test_deps:   Extra cc_test dependencies.
        extra_hdrs:     Extra headers for the cc_proto_library_builder rule.
        extra_srcs:     Extra sources for the cc_proto_library_builder rule.
        make_interface: Whether to make an interface.
    """
    native.proto_library(
        name = name + "_proto",
        testonly = 1,
        srcs = [name + ".proto"],
        # cc_api_version is 2
        deps = [
            "//proto_builder:proto_builder_proto",
        ] + proto_deps,
        visibility = visibility,
    )
    cc_proto_library(
        name = name + "_cc_proto",
        testonly = 1,
        deps = [":" + name + "_proto"],
        visibility = visibility,
    )
    cc_proto_builder_library(
        name = name + "_cc_proto_builder",
        testonly = 1,
        srcs = srcs,
        hdrs = hdrs,
        cc_proto_library_deps = [name + "_cc_proto"],
        extra_hdrs = extra_hdrs,
        extra_srcs = extra_srcs,
        make_interface = make_interface,
        visibility = visibility,
        deps = cc_deps,
    )
    proto_builder_test(
        name = name + "_proto_builder_builder_test",
        expected_hdr = name + "_cc_proto_builder.h.exp",
        expected_ifc = (name + "_cc_proto_builder.interface.h.exp") if make_interface else None,
        expected_src = name + "_cc_proto_builder.cc.exp",
        proto_builder_dep = get_proto_builder_dep(name + "_cc_proto_builder"),
    )
    native.cc_test(
        name = name + "_cc_proto_builder_test",
        srcs = [name + "_cc_proto_builder_test.cc"],
        deps = [
            ":" + name + "_cc_proto_builder",
            "@com_google_cpp_proto_builder//proto_builder/oss/testing:cpp_pb_gunit_cc",
        ] + cc_deps + cc_test_deps,
    )
    native.test_suite(
        name = name + "_tests",
        tests = [
            name + "_proto_builder_builder_test",
            name + "_cc_proto_builder_test",
        ],
    )
