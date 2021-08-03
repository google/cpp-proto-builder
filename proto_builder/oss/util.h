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

// Implements StatusOr helpers, that unpack StatusOr into value and status.
//
// Usage:
//
//   auto [status, value] = UnpackStatusOr(SomeFunc());
//
//   auto [status, value] = UnpackStatusOrDefault(SomeFunc(), args...);
//
// The first version does **not** initialize `value` in case the received
// `StatusOr<T>` has a non `OKStatus`.
// This version is only available if `T` passes `is_trivially_destructible<T>`
// since otherwise the destructor would likely fail, crash or have undefined
// behavior. However, for convenience this version explicitly works for the
// standard smart pointer types which will be initialized to `nullptr`
// (std::shared_ptr, std::unique_ptr, std::weak_ptr).
//
// IMPORTANT: Any usage of `value` requires that `status.ok()` returns true.
//
// The second version will always initialize `value`, either from the returned
// value in case of an `OkStatus`, or by constructing a new `value` from the
// provided arguments (so it is slower). The chosen constructor is either the
// default constructor (no args) or determined by the provided arguments. This
// version can be used in all cases including those where first version works.
//
// NOTE: It is possible to achieve 'perfect forwarding' as explained in
// https://en.cppreference.com/w/cpp/language/structured_binding using:
//
//   auto && [status, value] = UnpackStatusOr(SomeFunc());
//
//   auto && [status, value] = UnpackStatusOrDefault(SomeFunc(), args...);
//
// NOTE: Structured bindings have a readability issue whereby it is impossible
// to note the type of the targets. In cases where this is a concern, the type
// of the value can be spelled out by providing it explicitly as the template
// parameter like so:
//
//   auto [status, value] = UnpackStatusOr<ValueType>(SomeFunc());
//
//   auto [status, value] = UnpackStatusOrDefault<ValueType>(SomeFunc(), ...);
//
// TIP: Some non trivially destructible classes do not and should not have an
// available constructor, leaving no simple way to get `UnpackStatusOrDefault`
// to work. In those cases it might be good to create a 'tagged' constructor:
//
// class Foo {
//  public:
//   enum class DefaultWillNotWork { kDefaultWillNotWork = 0; }
//
//   static absl::StatusOr<Foo> MakeFoo() { return Foo(); }
//
//   explicit Foo(DefaultWillNotWork) {}
//
//  private:
//   Foo() = default;
//
//   std::vector<Type> member_;  // Non trivially destructible *example*.
// };
//
// auto [status, foo] = UnpackStatusOrDefault(
//                          MakeFoo(),
//                          Foo::DefaultWillNotWork::kDefaultWillNotWork);
//
// In the example the factory method MakeFoo() is allowed to call any private
// constructor including the default constructor `Foo()`. But any outside code
// code calling `UnpackStatusOrDefault` would only be allowed to invoke public
// constructors. Therefore passing the `kDefaultWillNotWork` tag forces the
// public constructor. The demonstration uses an `enum class` so that the caller
// actually has to explicitly state the tag parameter `kDefaultWillNotWork`.

#ifndef PROTO_BUILDER_OSS_UTIL_H_
#define PROTO_BUILDER_OSS_UTIL_H_

#include <tuple>
#include <type_traits>

#include "proto_builder/oss/util_internal.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace proto_builder {

template <typename T,
          typename = std::enable_if_t<std::is_trivially_destructible_v<T> ||
                                      internal::is_smart_pointer_v<T>>>
std::tuple<absl::Status, T> UnpackStatusOr(absl::StatusOr<T>&& v) {
  if (v.ok()) {
    return {absl::OkStatus(), std::move(v).value()};
  } else {
    if constexpr (internal::is_smart_pointer_v<T>) {
      return {std::move(v).status(), T{}};
    } else {
      union Uninitialized {
        Uninitialized() {}
        ~Uninitialized() {}
        int dummy;
        T uninitialized;
      };
      return {std::move(v).status(), Uninitialized().uninitialized};
    }
  }
}

template <typename T, typename... Args>
std::tuple<absl::Status, T> UnpackStatusOrDefault(absl::StatusOr<T>&& v,
                                                  Args&&... a) {
  if (v.ok()) {
    return {absl::OkStatus(), std::forward<T>(std::move(v).value())};
  } else {
    return {std::move(v).status(), T(std::forward<Args>(a)...)};
  }
}

}  // namespace proto_builder

#endif  // PROTO_BUILDER_OSS_UTIL_H_
