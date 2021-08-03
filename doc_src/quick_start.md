# ProtoBuilder quick start

https://google.github.io/cpp-proto-builder/quick_start

[TOC]

## Prerequisites

ProtoBuilder requires a protocol buffer file and rule in the bazel build system.

```proto
syntax "proto2";

package config_package;

message Config {
  optional string address = 1;
  message Fallback {
    repeated string address = 1;
  }
  repeated Fallback fallback = 2;

  message Alternative {
    optional string address = 1;
  }
  optional Alternative alternative = 3;
}
```

```BUILD
proto_library(
    name = "config_proto",             # name + "_proto"
    srcs = ["config.proto"],           # name + ".proto"
)

cc_proto_library(
    name = "config_cc_proto",          # name + "_cc_proto"
    deps = [":config_proto"],          # ":" + name + "_proto"
)

cc_proto_builder_library(
    name = "config_cc_proto_builder",              # name + "_cc_proto_builder"
    cc_proto_library_deps = [":config_cc_proto"],  # ":" + name + "_cc_proto"
```

This will generate the new `config_cc_proto_builder` library which provides
builders for `Config` and `Config_Fallback`, since the latter is used as a
repeated sub message field. However, by default there won't be a builder for
`Config::Alternative` and the alternative field will be available in the
flattened `Config` builder.

The resulting header will look something like:

```C++
namespace config_package {

class ConfigBuilder {
 public:
  ConfigBuilder();
  ConfigBuilder(const Config& data);
  ConfigBuilder(Config&& data);

  operator const Config&() const;

  ConfigBuilder& SetAddress(const std::string& value);
  ConfigBuilder& AddFallback(const Fallback& fallback);
  ConfigBuilder& SetAlternativeAddress(const std::string& value);
};

class Config_FallbackBuilder {
 public:
  Config_FallbackBuilder();
  Config_FallbackBuilder(const Config_Fallback& data);
  Config_FallbackBuilder(Config_Fallback&& data);

  operator const Config_Fallback&() const;

  Config_FallbackBuilder& AddAddress(const std::string& value);
};

}  // namespace config_package
```

The reason that builders for classes `Config` and `Config_Fallback` but not
class `Config_Alternative` are generated is in the default message finding mode
(read more in [finding messages](usage.md#finding_messages)).

NOTE: The generated builders do not have a method named `Build` since there is
nothing to build and the proto data is always available (very different from
Java). However, if you use [validation](validation.md), or annotate the proto
message with [`use_build`](#MessageBuilderOptions.use_build), then your builders
will have a `absl::StatusOr<ProtoType> Build() const` method which will validate
the proto data before returning it.

Check out the following for more:

*   [Home](index.md) for more on the generated setters.
*   [Config](config.md) for fine grained control of the code generation.
*   [Templates](templates.md) for customizing the code completely.
*   [Validation](validation.md) for data validation.
*   [Usage](usage.md) for more on the various rules and their interaction.
