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

"""Functionality for bazel workspace."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def init_cpp_pb_external_repositories():
    """Initializes external repositories with third-party code."""

    # rules_cc toolchain
    http_archive(
        name = "rules_cc",
        strip_prefix = "rules_cc-main",
        urls = ["https://github.com/bazelbuild/rules_cc/archive/main.zip"],
    )

    # GoogleTest
    http_archive(
        name = "com_google_googletest",
        strip_prefix = "googletest-aa533abfd4232b01f9e57041d70114d5a77e6de0",
        urls = ["https://github.com/google/googletest/archive/aa533abfd4232b01f9e57041d70114d5a77e6de0.zip"],
    )

    # Abseil, March 2021 LTS commit.
    http_archive(
        name = "com_google_absl",
        sha256 = "441db7c09a0565376ecacf0085b2d4c2bbedde6115d7773551bc116212c2a8d6",
        strip_prefix = "abseil-cpp-20210324.1",
        urls = [
            "https://github.com/abseil/abseil-cpp/archive/refs/tags/20210324.1.tar.gz",
        ],
    )

    # Protocol buffers. Official release 3.13.0.1
    http_archive(
        name = "com_google_protobuf",
        sha256 = "7d663c8dc81d282dc92e884b38e9c179671e31ccacce311154420e65f7d142c6",
        strip_prefix = "protobuf-3.13.0.1",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.13.0.1.tar.gz"],
    )

    # Protobuf dependencies -----------------------------------------------------
    http_archive(
        name = "zlib",
        build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
    )

    http_archive(
        name = "rules_python",
        sha256 = "b6d46438523a3ec0f3cead544190ee13223a52f6a6765a29eae7b7cc24cc83a0",
        url = "https://github.com/bazelbuild/rules_python/releases/download/0.1.0/rules_python-0.1.0.tar.gz",
    )

    http_archive(
        name = "com_github_gflags_gflags",
        sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
        strip_prefix = "gflags-2.2.2",
        urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
    )

    http_archive(
        name = "com_github_google_glog",
        sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
        strip_prefix = "glog-0.5.0",
        urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
    )

    http_archive(
        name = "com_google_re2",
        strip_prefix = "re2-2021-04-01",
        urls = ["https://github.com/google/re2/archive/refs/tags/2021-04-01.zip"],
    )

    # Load bazel skylib as per https://github.com/bazelbuild/bazel-skylib/releases
    http_archive(
        name = "bazel_skylib",
        sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
        urls = [
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        ],
    )
