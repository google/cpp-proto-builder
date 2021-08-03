# ProtoBuilder validation

https://google.github.io/cpp-proto-builder/validation

## Validation

[TOC]

ProtoBuilder allows to integrate with proto data validation that allows for full
proto data verification.

<!--#include file="inc/validation_rules_oss.md"-->

Adding a validator dependency to the `cc_proto_builder_library` rule will by
default enable the following message options for all messages in the
`config.proto` file:

*   [`use_build`](templates.md#use_build)
*   [`use_status`](templates.md#use_status)
*   [`use_validator`](templates.md#use_validator)

The resulting public interface will now be:

```C++
namespace config_package {

class ConfigBuilder {
 public:
  ConfigBuilder();
  ConfigBuilder(const Config& data);
  ConfigBuilder(Config&& data);

  absl::StatusOr<Config> Build() const;
  absl::StatusOr<Config> Consume();
  absl::StatusOr<Config> MaybeGetRawData() const;

  operator const Config&() const;

  bool ok() const;
  absl::Status status() const;

  ConfigBuilder& UpdateStatus(absl::Status status);

  ConfigBuilder& SetAddress(const std::string& value);
  ConfigBuilder& AddFallback(const Fallback& fallback);
  template <Builder, class = std::enable_if_t<...>>
  ConfigBuilder& AddFallback(Builder builder);
  ConfigBuilder& SetAlternativeAddress(const std::string& value);
};

class Config_FallbackBuilder {
 public:
  Config_FallbackBuilder();
  Config_FallbackBuilder(const Config_Fallback& data);
  Config_FallbackBuilder(Config_Fallback&& data);

  absl::StatusOr<Config_Fallback> Build() const;
  absl::StatusOr<Config_Fallback> Consume();
  absl::StatusOr<Config_Fallback> MaybeGetRawData() const;

  operator const Config_Fallback&() const;

  bool ok() const;
  absl::Status status() const;

  Config_FallbackBuilder& UpdateStatus(absl::Status status);

  Config_FallbackBuilder& AddAddress(const std::string& value);
};

}  // namespace config_package
```

### Validation and MaybeGetRawData {#MaybeGetRawData}

The builders call `Validate` for the current data for the read access methods
(`Build`, `Consume`, `operator const Type&`). If a sub-builder gets passed to a
builder, then the actual builder should be passed and not the result of a
conversion, `Build` or `Consume` call. This results in conditionally enabled
setter to be called (e.g. `AddFallback(Builder builder)` as opposed to
`AddFallback(const Fallback& fallback)`. That setter will internally invoke the
magic `MaybeGetRawData` method on the builder. That allows to check the state of
the sub builder as follows:

*   If that sub builder has ok Status, then the data is being passed to the
    outer builder (`MaybeGetRawData` returns the data).
*   If that sub builder has a non-ok Status resulting from validation, then the
    actual data is passed. The outer builder therefore has all data and can
    validate the transitive data on next access (`MaybeGetRawData` returns the
    data).
*   If that builder has a non-ok Status resulting from a call to `UpdateStatus`,
    then that status is passed to the outer builder which then will be in that
    error state (`MaybeGetRawData` returns `status()`).

This behavior is different from `Build` and `Consume` which would return the
non-ok status from validation as well as from the conversion operator that would
return the default instance for any error state.

The consequence is that the builder has to call Validation for the sub message
itself, which happens because the `Validate` functions are recursive but that
allows the validation to provide the full proto path in validation error
messages.

Also note that the conversion operator will return an empty proto in case of a
non-ok status.

### Custom validator

<!--#include file="inc/validation_validator_oss.md"-->

```C++
#ifndef __CONFIG_PROTO_VALIDATOR_H_
#define __CONFIG_PROTO_VALIDATOR_H_

#include ".../config.pb.h"
#include "absl/status/status.h"

namespace protobuf::contrib::validator {

using config_package::Config;
using config_package::Config_Fallback;

inline bool IsValidAddress(const std::string& address) {
  return address.empty() || address == "8.8.8.8" || address == "8.8.4.4";
}

inline absl::Status Validate(const Config_Fallback& v) {
  for (const auto& address : v.address)) {
    if (!IsValidAddress(address)) {
      return absl::InvalidArgumentError("Bad fallback address");
    }
  }
  return absl::OkStatus();
}

inline absl::Status Validate(const Config& v) {
  if (!IsValidAddress(v.address())) {
    return absl::InvalidArgumentError("Bad address");
  }
  for (const auto& f : v.fallback()) {
    absl::Status s = Validate(f);
    if (!s.ok()) {
      return s;
    }
  }
  return absl::OkStatus();
}

}  // namespace protobuf::contrib::validator

#endif // __CONFIG_PROTO_VALIDATOR_H_
```

WARNING: If either the header file name or the rule name are wrong, then the
validator will not be found.

NOTE: Validate must repeat validation for all sub messages, even if they were
already handled by an independent sub-builder.

NOTE: There must be a `Validate` specialization for each message that has a
builder. All of them must be in the `protobuf::contrib::validator` namespace.

### Per field validation

Instead of the full proto validation described above, Proto Builder also
supports [predicates](config.md#predicates). Further it is possible to use
conversion annotations to verify and conditionally set fields as described in
[conditions](config.md#conversion_conditions) and
[conversion data](config.md#conversion_data).

IMPORTANT: When choosing between validation and predicates (or conversions) it
is important to recognize the difference in status handling. Validation occurs
prior to access and applies to the whole message and all its transitive sub
messages. Predicates on the other hand apply to single fields before setting
them, see details in the [predicates](config.md#predicates) documentation.
