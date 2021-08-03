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

"""Build configuration for oss version."""

# READ: https://google.github.io/cpp-proto-builder

_ARGS = []

_CC_TOOLCHAIN = "@rules_cc//cc:current_cc_toolchain"

# Compatibility for all main public rules.
_COMPATIBLE_WITH = []

# Additional conversion libraries that require 'proto_builder_conversions'.
_CONVERSION_DEPS = [
    "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
    # 'anchor' required to pass bzl_library test
    "@com_google_protobuf//:protobuf",
]

# Automatically added conversion libraries.
__CONVERSION_AUTO_DEPS = [
    "@com_google_absl//absl/status",
    "@com_google_absl//absl/status:statusor",
    "@com_google_absl//absl/strings",
    "@com_google_absl//absl/time",
    "@com_google_cpp_proto_builder//proto_builder/oss:source_location_cc",
    "@com_google_cpp_proto_builder//proto_builder/oss:parse_text_proto_cc",
    "@com_google_cpp_proto_builder//proto_builder/oss:proto_conversion_helpers_cc",
]

# Headers used in conversions, must match 'proto_builder_config*.textproto'.
_CONVERSION_HEADERS = [
    "proto_builder/oss/parse_text_proto.h",
    "proto_builder/oss/source_location.h",
    "absl/status/status.h",
    "absl/status/statusor.h",
    "absl/strings/string_view.h",
    "absl/time/time.h",
    "google/protobuf/util/time_util.h",
]

# Labels for conversion libraries. DO NOT MODIFY.
_CONVERSION_LIBRARIES = [
    Label("@com_google_cpp_proto_builder//proto_builder:proto_builder_conversions"),
] + [lib for lib in _CONVERSION_DEPS + __CONVERSION_AUTO_DEPS]

# Default dependencies for 'cc_proto_builder_library' rules.
_DEFAULT_DEPS = __CONVERSION_AUTO_DEPS + []

# Additional fragments for 'proto_builder' rule.
_FRAGMENTS = ["cpp"]

# Placeholder: not used in OSS.
_GREP_INCLUDES = "@com_google_cpp_proto_builder//proto_builder/oss:parse_textproto_file"

_USE_CC_LIBRARY_MACRO = True

_CC_PROTO_EXTENSION = ".pb.h"

def _proto_builder_processing(tpl, default_tpl, src):
    """Processing command in 'proto_builder' rule.

    There is no processing in the 'proto_builder' rule. An empty command string
    is returned.

    Args:
      tpl: The template file used to generate the src file.
      default_tpl: The default template file for the src file type (e.g. header,
                   source, interface header).
      src: The source file.

    Returns:
      The command string.
    """
    return ""

def _proto_builder_test_processing(ctx, tpl, default_tpl, src, output_filename):
    """Processing in 'proto_builder_test' rule.

    Creates an action in the ctx that processes the src file. The action creates
    a new file, the processed file.

    The processed file has the same content as the src file.

    Args:
      ctx: The current rule's context object.
      tpl: The template file used to generate the src file.
      default_tpl: The default template file for the src file type (e.g. header,
                   source, interface header).
      src: The source file.
      output_filename: The filename of the processed

    Returns:
      The processed file.
    """
    processed_file = ctx.actions.declare_file(output_filename)
    ctx.actions.run_shell(
        outputs = [processed_file],
        inputs = [src],
        tools = [ctx.executable._stable_clang_format_tool],
        command = "cat {} > {}".format(
            src.path,
            processed_file.path,
        ),
        mnemonic = "ProtoBuilderTestProcessing",
        progress_message = "Proto Builder Test Processing on file: %s." % (src.path),
    )
    return processed_file

proto_builder_config = struct(
    ARGS = _ARGS,
    CC_PROTO_EXTENSION = _CC_PROTO_EXTENSION,
    CC_TOOLCHAIN = _CC_TOOLCHAIN,
    COMPATIBLE_WITH = _COMPATIBLE_WITH,
    CONVERSION_DEPS = _CONVERSION_DEPS,
    CONVERSION_HEADERS = _CONVERSION_HEADERS,
    CONVERSION_LIBRARIES = _CONVERSION_LIBRARIES,
    DEFAULT_DEPS = _DEFAULT_DEPS,
    FRAGMENTS = _FRAGMENTS,
    GREP_INCLUDES = _GREP_INCLUDES,
    proto_builder_processing = _proto_builder_processing,
    proto_builder_test_processing = _proto_builder_test_processing,
    USE_CC_LIBRARY_MACRO = _USE_CC_LIBRARY_MACRO,
)

_VIRTUAL_IMPORTS = "/_virtual_imports/"
_EXTERNAL_IMPORTS = "/external/"

def _strip_from_external_for_proto(file):
    """Strip path of file from 'external'.

    Args:
      file: The file.

    Returns:
      The path of the file.
    """
    directory = file.path

    if _EXTERNAL_IMPORTS in directory:
        root, relative = file.path.split(_EXTERNAL_IMPORTS, 2)
        directory = _EXTERNAL_IMPORTS[1:] + relative
    if _VIRTUAL_IMPORTS in directory:
        root, relative = directory.split(_VIRTUAL_IMPORTS, 2)
        return root + _VIRTUAL_IMPORTS + relative.split("/", 1)[0]
    return "."

def _text_proto_test_impl(ctx):
    """The implementation of the 'text_proto_test' rule.

    Args:
      ctx: The current rule's context object.

    Returns:
      DefaultInfo provider
    """

    # Get paths from transitive sources
    proto_files = []
    for dep in ctx.attr.deps:
        proto_files += dep[ProtoInfo].transitive_sources.to_list()
    runfiles = proto_files
    proto_paths = ",".join(
        [_strip_from_external_for_proto(proto) for proto in proto_files],
    )

    # Get direct sources
    proto_files = []
    for dep in ctx.attr.deps:
        proto_files += dep[ProtoInfo].direct_sources
    proto_files = [file.path for file in proto_files]
    executable_file = ctx.actions.declare_file(ctx.label.name + ".sh")
    ctx.actions.write(
        output = executable_file,
        content = "\n".join([
            "#!/bin/bash",
            "set -e",
            "{} --proto_type='{}' --textproto='{}' --proto_paths=$PWD,{} {}".format(
                ctx.executable._parse_textproto_file_tool.short_path,
                ctx.attr.message,
                ctx.attr.src.files.to_list()[0].short_path,
                proto_paths,
                " ".join(proto_files),
            ),
            "exit 0",
        ]),
        is_executable = True,
    )
    runfiles.append(ctx.executable._parse_textproto_file_tool)
    runfiles += ctx.attr.src.files.to_list()
    return [DefaultInfo(
        executable = executable_file,
        files = depset([executable_file]),
        runfiles = ctx.runfiles(files = runfiles),
    )]

text_proto_test = rule(
    attrs = {
        "src": attr.label(
            doc = "The file (label expanding to a single file) whose content" +
                  "is tested to be a valid text proto instance. path " +
                  "relative to current package is a valid label.",
            allow_single_file = [".textproto"],
            mandatory = True,
        ),
        "message": attr.string(
            doc = "The name of the message type that \"src\" file represents.",
            default = "",
        ),
        "deps": attr.label_list(
            doc = "A list of proto_library targets where \"message\" is " +
                  "defined. Transitive dependencies are pulled in " +
                  "automatically.",
            allow_rules = ["proto_library"],
            providers = [ProtoInfo],
        ),
        "_parse_textproto_file_tool": attr.label(
            doc = "The target of the parse textproto file executable.",
            default = Label("@com_google_cpp_proto_builder//proto_builder/oss:parse_textproto_file"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
    doc = "Generate a test for a text format protocol buffer.",
    implementation = _text_proto_test_impl,
    test = True,
)
