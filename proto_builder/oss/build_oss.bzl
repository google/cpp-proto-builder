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

OSS_IMPLEMENTATIONS = {
    "file": [":file_impl_oss_cc"],
    "sfdb": [":sourcefile_database_impl_oss_cc"],
}

ALL_IMPLEMENTATIONS = {
    "oss": OSS_IMPLEMENTATIONS,
}

DEFINES = {"logging_cc": ["PBCC_LOGGING_MACROS"]}

IMPLEMENTATION = "oss"
