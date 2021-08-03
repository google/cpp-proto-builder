#ifndef PROTO_BUILDER_TESTS_CONVERSION_DATA_UTIL_H_
#define PROTO_BUILDER_TESTS_CONVERSION_DATA_UTIL_H_

namespace proto_builder::tests {

template <class T>
inline T GetValueIfBetween(T value, T min_value, T max_value, T default_value) {
  return value >= min_value && value < max_value ? value : default_value;
}

template <class T>
inline T GetValueIfBetween(T value, T min_value, T max_value) {
  return GetValueIfBetween(value, min_value, max_value, T());
}

}  // namespace proto_builder::tests

#endif  // PROTO_BUILDER_TESTS_CONVERSION_DATA_UTIL_H_
