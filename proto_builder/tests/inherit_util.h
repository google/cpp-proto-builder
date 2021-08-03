#ifndef PROTO_BUILDER_TESTS_INHERIT_UTIL_H_
#define PROTO_BUILDER_TESTS_INHERIT_UTIL_H_

#include <string>

namespace proto_builder::tests {

template <class ProtoBuilder>
class InheritBase {
 public:
  InheritBase() = default;
  virtual ~InheritBase() = default;

  ProtoBuilder& SetFirstLast(const std::string& first,
                             const std::string& last) {
    SetFirstName(first);
    SetLastName(last);
    return *static_cast<ProtoBuilder*>(this);
  }

 protected:
  virtual ProtoBuilder& SetFirstName(const std::string& value) = 0;
  virtual ProtoBuilder& SetLastName(const std::string& value) = 0;
};

}  // namespace proto_builder::tests

#endif  // PROTO_BUILDER_TESTS_INHERIT_UTIL_H_
