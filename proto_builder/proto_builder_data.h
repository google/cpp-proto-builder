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

#ifndef PROTO_BUILDER_PROTO_BUILDER_DATA_H_
#define PROTO_BUILDER_PROTO_BUILDER_DATA_H_

#include <string>

namespace proto_builder {

const std::string& DefaultHeaderTemplate();
const std::string& DefaultInterfaceTemplate();
const std::string& DefaultSourceTemplate();

}  // namespace proto_builder

#endif  // PROTO_BUILDER_PROTO_BUILDER_DATA_H_
