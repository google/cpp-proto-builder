#ifndef PROTO_BUILDER_TESTS_PREDICATE_UTIL_H_
#define PROTO_BUILDER_TESTS_PREDICATE_UTIL_H_

#include "absl/status/status.h"

namespace proto_builder::tests {

template <class T>
inline absl::Status IsBetween(T value, T min, T max) {
  if (min <= value && value < max) {
    return absl::OkStatus();
  } else {
    return absl::InvalidArgumentError("Value out of range");
  }
}

inline absl::Status IsPair(const std::pair<int64_t, std::string>& data,
                           int64_t key, const std::string& val) {
  if (data.first == key && data.second == val) {
    return absl::OkStatus();
  } else {
    return absl::InvalidArgumentError("Bad key or value");
  }
}

}  // namespace proto_builder::tests

#endif  // PROTO_BUILDER_TESTS_PREDICATE_UTIL_H_
