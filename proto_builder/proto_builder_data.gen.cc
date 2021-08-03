#line 2 "proto_builder/proto_builder_data.gen.cc"
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

const std::string& DefaultHeaderTemplate() {
  static const std::string& default_h_tpl = *new std::string(R"default_h(
  __DEFAULT_H_TPL__
)default_h");
  return default_h_tpl;
}

const std::string& DefaultInterfaceTemplate() {
  static const std::string& interface_h_tpl = *new std::string(R"interface_h(
  __INTERFACE_H_TPL__
)interface_h");
  return interface_h_tpl;
}

const std::string& DefaultSourceTemplate() {
  static const std::string& default_cc_tpl = *new std::string(R"default_cc(
  __DEFAULT_CC_TPL__
)default_cc");
  return default_cc_tpl;
}

}  // namespace proto_builder
