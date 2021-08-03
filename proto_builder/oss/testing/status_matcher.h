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

// TODO 06-27-2021 Add test cases.

#ifndef PROTO_BUILDER_OSS_TESTING_STATUS_MATCHER_H_
#define PROTO_BUILDER_OSS_TESTING_STATUS_MATCHER_H_

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace testing {
namespace status {
namespace oss {
inline const ::absl::Status& GetStatus(const ::absl::Status& status) {
  return status;
}

template <typename T>
inline const ::absl::Status& GetStatus(const ::absl::StatusOr<T>& status) {
  return status.status();
}

// Monomorphic implementation of matcher IsOkAndHolds(m).  StatusOrType is a
// reference to StatusOr<T>.
template <typename StatusOrType>
class IsOkAndHoldsMatcherImpl
    : public ::testing::MatcherInterface<StatusOrType> {
 public:
  typedef
      typename std::remove_reference<StatusOrType>::type::value_type value_type;

  template <typename InnerMatcher>
  explicit IsOkAndHoldsMatcherImpl(InnerMatcher&& inner_matcher)
      : inner_matcher_(::testing::SafeMatcherCast<const value_type&>(
            std::forward<InnerMatcher>(inner_matcher))) {}

  void DescribeTo(std::ostream* os) const override {
    *os << "is OK and has a value that ";
    inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "isn't OK or has a value that ";
    inner_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(
      StatusOrType actual_value,
      ::testing::MatchResultListener* result_listener) const override {
    if (!actual_value.ok()) {
      *result_listener << "which has status " << actual_value.status();
      return false;
    }

    ::testing::StringMatchResultListener inner_listener;
    const bool matches =
        inner_matcher_.MatchAndExplain(*actual_value, &inner_listener);
    const std::string inner_explanation = inner_listener.str();
    if (!inner_explanation.empty()) {
      *result_listener << "which contains value "
                       << ::testing::PrintToString(*actual_value) << ", "
                       << inner_explanation;
    }
    return matches;
  }

 private:
  const ::testing::Matcher<const value_type&> inner_matcher_;
};

// Implements IsOkAndHolds(m) as a polymorphic matcher.
template <typename InnerMatcher>
class IsOkAndHoldsMatcher {
 public:
  explicit IsOkAndHoldsMatcher(InnerMatcher inner_matcher)
      : inner_matcher_(std::move(inner_matcher)) {}

  // Converts this polymorphic matcher to a monomorphic matcher of the
  // given type.  StatusOrType can be either StatusOr<T> or a
  // reference to StatusOr<T>.
  template <typename StatusOrType>
  operator ::testing::Matcher<StatusOrType>() const {  // NOLINT
    return ::testing::Matcher<StatusOrType>(
        new IsOkAndHoldsMatcherImpl<const StatusOrType&>(inner_matcher_));
  }

 private:
  const InnerMatcher inner_matcher_;
};

// Monomorphic implementation of matcher IsOk() for a given type T.
// T can be Status, StatusOr<>, or a reference to either of them.
template <typename T>
class MonoIsOkMatcherImpl : public ::testing::MatcherInterface<T> {
 public:
  void DescribeTo(std::ostream* os) const override { *os << "is OK"; }
  void DescribeNegationTo(std::ostream* os) const override {
    *os << "is not OK";
  }
  bool MatchAndExplain(T actual_value,
                       ::testing::MatchResultListener*) const override {
    return GetStatus(actual_value).ok();
  }
};

// Implements IsOk() as a polymorphic matcher.
class IsOkMatcher {
 public:
  template <typename T>
  operator ::testing::Matcher<T>() const {  // NOLINT
    return ::testing::Matcher<T>(new MonoIsOkMatcherImpl<T>());
  }
};

// Macros for testing the results of functions that return absl::Status or
// absl::StatusOr<T> (for any type T).
#undef STATUS_MACROS_IMPL_CONCAT_INNER
#undef STATUS_MACROS_IMPL_CONCAT
#define STATUS_MACROS_IMPL_CONCAT_INNER(x, y) x##y
#define STATUS_MACROS_IMPL_CONCAT(x, y) STATUS_MACROS_IMPL_CONCAT_INNER(x, y)

#undef EXPECT_OK
#define EXPECT_OK(expression) EXPECT_THAT(expression, testing::status::oss::IsOk())
#undef ASSERT_OK
#define ASSERT_OK(expression) ASSERT_THAT(expression, testing::status::oss::IsOk())
#undef ASSERT_OK_AND_ASSIGN
#define ASSERT_OK_AND_ASSIGN(lhs, rexpr) \
  ASSERT_OK_AND_ASSIGN_IMPL_(            \
      STATUS_MACROS_IMPL_CONCAT(_status_or_value, __LINE__), lhs, rexpr)

#define ASSERT_OK_AND_ASSIGN_IMPL_(statusor, lhs, rexpr) \
  auto statusor = (rexpr);                               \
  ASSERT_TRUE(statusor.ok());                            \
  lhs = std::move(statusor.value())

// Returns a gMock matcher that matches a StatusOr<> whose status is
// OK and whose value matches the inner matcher.
template <typename InnerMatcher>
IsOkAndHoldsMatcher<typename std::decay<InnerMatcher>::type> IsOkAndHolds(
    InnerMatcher&& inner_matcher) {
  return IsOkAndHoldsMatcher<typename std::decay<InnerMatcher>::type>(
      std::forward<InnerMatcher>(inner_matcher));
}

// Returns a gMock matcher that matches a Status or StatusOr<> which is OK.
inline IsOkMatcher IsOk() { return IsOkMatcher(); }

// Implements a status matcher interface to verify a status error code, and
// error message if set, matches an expected value.
//
// Sample usage:
//   EXPECT_THAT(MyCall(), StatusIs(absl::StatusCode::kNotFound,
//                                  HasSubstr("message")));
//
// Sample output on failure:
//   Value of: MyCall()
//   Expected: NOT_FOUND and has substring "message"
//     Actual: UNKNOWN: descriptive error (of type absl::lts_2020_02_25::Status)
class StatusIsMatcher {
 public:
  StatusIsMatcher(const absl::StatusCode& status_code,
                  const testing::Matcher<const std::string&>& message_matcher)
      : status_code_(status_code), message_matcher_(message_matcher) {}

  void DescribeTo(std::ostream* os) const {
    *os << status_code_ << " status code where the message ";
    message_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "not (";
    DescribeTo(os);
    *os << ")";
  }

  template <typename StatusType>
  bool MatchAndExplain(const StatusType& actual,
                       testing::MatchResultListener* listener) const {
    const absl::Status& actual_status = GetStatus(actual);
    return actual_status.code() == status_code_ &&
           message_matcher_.MatchAndExplain(
               std::string{actual_status.message()}, listener);
  }

 private:
  const absl::StatusCode status_code_;
  const testing::Matcher<const std::string&> message_matcher_;
};

// Status matcher that checks the StatusCode for an expected value.
inline testing::PolymorphicMatcher<StatusIsMatcher> StatusIs(
    const absl::StatusCode& code) {
  return testing::MakePolymorphicMatcher(StatusIsMatcher(code, testing::_));
}

// Status matcher that checks the StatusCode and message for expected values.
template <typename MessageMatcher>
testing::PolymorphicMatcher<StatusIsMatcher> StatusIs(
    const absl::StatusCode& code, const MessageMatcher& message) {
  return testing::MakePolymorphicMatcher(
      StatusIsMatcher(code, testing::MatcherCast<const std::string&>(message)));
}

}  // namespace oss
}  // namespace status
}  // namespace testing

#endif  // PROTO_BUILDER_OSS_TESTING_STATUS_MATCHER_H_
