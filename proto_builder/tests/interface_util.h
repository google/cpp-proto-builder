#ifndef PROTO_BUILDER_TESTS_INTERFACE_UTIL_H_
#define PROTO_BUILDER_TESTS_INTERFACE_UTIL_H_

#include <string>

#include "proto_builder/tests/interface.pb.h"
#include "proto_builder/tests/interface_cc_proto_builder.interface.h"

namespace proto_builder::tests {

template <class InterfaceBuilder, class InterfaceBuilderInterface>
class InterfaceBase : public InterfaceBuilderInterface {
 public:
  InterfaceBase() = default;
  ~InterfaceBase() override = default;

  InterfaceBuilder& SetFirstLast(const std::string& first,
                                 const std::string& last) {
    return this->SetFirstName(first).SetLastName(last);
  }
};

}  // namespace proto_builder::tests

#endif  // PROTO_BUILDER_TESTS_INTERFACE_UTIL_H_
