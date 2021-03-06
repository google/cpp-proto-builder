#line 2 "proto_builder/proto_builder_config_data.gen.cc"
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

#include <string>

namespace proto_builder {

const std::string& GetProtoTextConfig() {
  static const std::string& config = *new std::string(R"config(
  __PROTO_TEXT_CONFIG__
)config");
  return config;
}

}  // namespace proto_builder
