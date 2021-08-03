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

#ifndef PROTO_BUILDER_OSS_UTIL_INTERNAL_H_
#define PROTO_BUILDER_OSS_UTIL_INTERNAL_H_

#include <memory>
#include <type_traits>

namespace proto_builder {
namespace internal {

template <class T>
struct is_smart_pointer_impl : std::false_type {};

template <class T>
struct is_smart_pointer_impl<std::shared_ptr<T>> : std::true_type {};

template <class T>
struct is_smart_pointer_impl<std::unique_ptr<T>> : std::true_type {};

template <class T>
struct is_smart_pointer_impl<std::weak_ptr<T>> : std::true_type {};

template <class T>
struct is_smart_pointer : is_smart_pointer_impl<typename std::remove_cv_t<T>> {
};

template <typename T>
inline constexpr bool is_smart_pointer_v = is_smart_pointer<T>::value;

}  // namespace internal
}  // namespace proto_builder

#endif  // PROTO_BUILDER_OSS_UTIL_INTERNAL_H_
