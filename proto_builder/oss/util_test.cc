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

#include "proto_builder/oss/util.h"

#include <list>
#include <memory>
#include <type_traits>
#include <vector>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"

namespace proto_builder {
namespace {

using ::testing::ElementsAre;
using ::testing::Pointee;
using ::testing::Truly;
using ::testing::Types;
using ::testing::status::oss::IsOk;
using ::testing::status::oss::StatusIs;

// We count the number MoveIt constructor calls to prove whether such variables
// were initialized or not.
class TestBase : public ::testing::Test {
 public:
  TestBase()
      : def_ctor_before_(g_def_ctor_count_),
        exp_ctor_before_(g_exp_ctor_count_),
        mov_ctor_before_(g_mov_ctor_count_) {}

  bool CtorCalled() const {
    return g_def_ctor_count_ > def_ctor_before_ ||
           g_exp_ctor_count_ > exp_ctor_before_;
  }

  bool MovCtorCalled() const { return g_mov_ctor_count_ > mov_ctor_before_; }

  size_t def_ctor_before_;
  size_t exp_ctor_before_;
  size_t mov_ctor_before_;

  static size_t g_def_ctor_count_;
  static size_t g_exp_ctor_count_;
  static size_t g_mov_ctor_count_;
};

size_t TestBase::g_def_ctor_count_ = 0;
size_t TestBase::g_exp_ctor_count_ = 0;
size_t TestBase::g_mov_ctor_count_ = 0;

// A move only type that counts its constructors.
class MoveIt {
 public:
  MoveIt() { ++TestBase::g_def_ctor_count_; }
  explicit MoveIt(int data) : data_(data) { ++TestBase::g_exp_ctor_count_; }
  MoveIt(const MoveIt&) = delete;
  MoveIt& operator=(const MoveIt& o) = delete;
  MoveIt(MoveIt&& o) : data_(o.data_) {
    ++TestBase::g_mov_ctor_count_;
    o.data_ = ~data_;  // Just change the value so it no longer is equal.
  }
  MoveIt& operator=(MoveIt&& o) = delete;  // not needed here

  bool operator==(const MoveIt& o) const { return data_ == o.data_; }

  int data() const { return data_; }

 private:
  int data_ = 0;
};

static_assert(std::is_move_constructible_v<MoveIt> &&
                  !std::is_move_assignable_v<MoveIt> &&
                  !std::is_copy_constructible_v<MoveIt> &&
                  !std::is_copy_assignable_v<MoveIt>,
              "");

class TrivDtor {
 public:
  explicit TrivDtor(int v = 0) : data_(v) {}
  ~TrivDtor() = default;

  bool operator==(const TrivDtor& o) const { return data_ == o.data_; }

  int data() const { return data_; }

 private:
  const int data_;  // Must init | But UnpackStatusOr() will not.
};

static_assert(std::is_trivially_destructible_v<TrivDtor> &&
                  !std::is_trivially_constructible_v<TrivDtor>,
              "");

template <typename T>
inline constexpr bool IsMoveIt = std::is_same_v<T, MoveIt>;

template <typename V>
struct Handle {
  static V Make(int v) { return V{v}; }
  static int CtorArg(int v) { return v; }
  static void Compare(int l, int r) { EXPECT_THAT(l, r); }
};

template <>
struct Handle<int*> {
  static int* Make(int v) {
    static auto& vs = *new std::list<int>();  // Need pointer stability
    vs.emplace_back(v);
    return &vs.back();
  }
  static int* CtorArg(int v) { return Make(v); }
  static void Compare(int* l, int r) { EXPECT_THAT(l, Pointee(r)); }
};

template <>
struct Handle<std::shared_ptr<int>> {
  static std::shared_ptr<int> Make(int v) { return std::make_shared<int>(v); }
  static std::shared_ptr<int> CtorArg(int v) { return Make(v); }
  static void Compare(const std::shared_ptr<int>& l, int r) {
    EXPECT_THAT(l, Pointee(r));
  }
};

template <>
struct Handle<std::unique_ptr<int>> {
  static std::unique_ptr<int> Make(int v) { return std::make_unique<int>(v); }
  static std::unique_ptr<int> CtorArg(int v) { return Make(v); }
  static void Compare(const std::unique_ptr<int>& l, int r) {
    EXPECT_THAT(l, Pointee(r));
  }
};

template <>
struct Handle<std::list<int>> {
  static std::list<int> Make(int v) { return std::list<int>{v}; }
  static std::list<int> CtorArg(int v) { return Make(v); }
  static void Compare(const std::list<int>& l, int r) {
    EXPECT_THAT(l, ElementsAre(r));
  }
};

template <>
struct Handle<std::vector<int>> {
  static std::vector<int> Make(int v) { return std::vector<int>{v}; }
  static std::vector<int> CtorArg(int v) { return Make(v); }
  static void Compare(const std::vector<int>& l, int r) {
    EXPECT_THAT(l, ElementsAre(r));
  }
};

template <>
struct Handle<MoveIt> {
  static MoveIt Make(int v) { return MoveIt(v); }
  static MoveIt CtorArg(int v) { return Make(v); }
  static void Compare(const MoveIt& l, int r) { EXPECT_THAT(l.data(), r); }
};

template <>
struct Handle<TrivDtor> {
  static TrivDtor Make(int v) { return TrivDtor(v); }
  static TrivDtor CtorArg(int v) { return Make(v); }
  static void Compare(const TrivDtor& l, int r) { EXPECT_THAT(l.data(), r); }
};

template <typename T>
class DefaultTest : public TestBase {};

TYPED_TEST_SUITE_P(DefaultTest);

TYPED_TEST_P(DefaultTest, FailMove) {
  absl::StatusOr<TypeParam> status_or = absl::InternalError("Error");
  auto [status, v] = UnpackStatusOrDefault(std::move(status_or));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_THAT(DefaultTest<TypeParam>::CtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(DefaultTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(v, Truly([=](const TypeParam& o) { return o == TypeParam(); }));
}

TYPED_TEST_P(DefaultTest, FailMoveArg) {
  absl::StatusOr<TypeParam> status_or = absl::InternalError("Error");
  auto [status, v] = UnpackStatusOrDefault(std::move(status_or),
                                           Handle<TypeParam>::CtorArg(42));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_THAT(DefaultTest<TypeParam>::CtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(DefaultTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(DefaultTest, FailTemp) {
  auto [status, v] = UnpackStatusOrDefault(
      absl::StatusOr<TypeParam>(absl::InternalError("Error")));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_THAT(DefaultTest<TypeParam>::CtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(DefaultTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(v, Truly([=](const TypeParam& o) { return o == TypeParam(); }));
}

TYPED_TEST_P(DefaultTest, FailTempArg) {
  auto [status, v] = UnpackStatusOrDefault(
      absl::StatusOr<TypeParam>(absl::InternalError("Error")),
      Handle<TypeParam>::CtorArg(42));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_THAT(DefaultTest<TypeParam>::CtorCalled(), IsMoveIt<TypeParam>);
  EXPECT_THAT(DefaultTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(DefaultTest, PassMove) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto [status, v] = UnpackStatusOrDefault(std::move(status_or));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(DefaultTest, PassMoveArg) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto&& [status, v] = UnpackStatusOrDefault(
      std::move(status_or), Handle<TypeParam>::CtorArg(25));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(DefaultTest, PassTemp) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto [status, v] = UnpackStatusOrDefault(std::move(status_or));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(DefaultTest, PassTempArg) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto [status, v] = UnpackStatusOrDefault(
      std::move(status_or), Handle<TypeParam>::CtorArg(25));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

REGISTER_TYPED_TEST_SUITE_P(DefaultTest,  // Suite and Tests
                            FailMove, FailMoveArg, FailTemp, FailTempArg,
                            PassMove, PassMoveArg, PassTemp, PassTempArg);

using TestTypes = Types<int, int*, std::shared_ptr<int>, std::unique_ptr<int>,
                        MoveIt, TrivDtor>;

INSTANTIATE_TYPED_TEST_SUITE_P(Normal, DefaultTest, TestTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(Vector, DefaultTest, Types<std::vector<int>>);

template <typename T>
class UninitTest : public TestBase {};

TYPED_TEST_SUITE_P(UninitTest);

TYPED_TEST_P(UninitTest, FailMove) {
  absl::StatusOr<TypeParam> status_or = absl::InternalError("Error");
  auto [status, v] = UnpackStatusOr(std::move(status_or));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_FALSE(UninitTest<TypeParam>::CtorCalled()) << "v is not initialized";
  EXPECT_THAT(UninitTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
}

TYPED_TEST_P(UninitTest, FailTemp) {
  auto [status, v] =
      UnpackStatusOr(absl::StatusOr<TypeParam>(absl::InternalError("Error")));
  EXPECT_THAT(status, StatusIs(absl::StatusCode::kInternal, "Error"));
  EXPECT_FALSE(UninitTest<TypeParam>::CtorCalled()) << "v is not initialized";
  EXPECT_THAT(UninitTest<TypeParam>::MovCtorCalled(), IsMoveIt<TypeParam>);
}

TYPED_TEST_P(UninitTest, PassMove) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto [status, v] = UnpackStatusOr(std::move(status_or));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

TYPED_TEST_P(UninitTest, PassTemp) {
  absl::StatusOr<TypeParam> status_or(Handle<TypeParam>::Make(42));
  const auto [status, v] = UnpackStatusOr(std::move(status_or));
  ASSERT_THAT(status, IsOk());
  Handle<TypeParam>::Compare(v, 42);
}

REGISTER_TYPED_TEST_SUITE_P(UninitTest,  // Suite and Tests
                            FailMove, FailTemp, PassMove, PassTemp);

INSTANTIATE_TYPED_TEST_SUITE_P(Normal, UninitTest, TestTypes);
// UninitTest does not work for std::vector. The returned vector would be
// destructed and it's memory management would of course fail, likely causing
// a SEGV. In general the behavior is undefined.

}  // namespace
}  // namespace proto_builder
