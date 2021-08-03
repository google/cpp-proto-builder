All you have to do is provide a validator rule to the builder rule.

The following example is the one from [quick_start](../quick_start.md) with the
validator added (see **!!!**):

```BUILD
proto_library(
    name = "config_proto",                 # name + "_proto"
    srcs = ["config.proto"],               # name + ".proto"
)

cc_proto_library(
    name = "config_cc_proto",              # name + "_cc_proto"
    deps = [":config_proto"],              # ":" + name + "_proto"
)

cc_library(
    name = "config_cc_proto_validator",    # name + "_cc_proto_validator" !!!
    hdrs = ["config.proto.validator.h"],   # name + ".proto.validator.h"  !!!
    deps = [":config_cc_proto"],           # ":" + name + "_cc_proto"
)

cc_proto_builder_library(
    name = "config_cc_proto_builder",              # name + "_cc_proto_builder"
    cc_proto_library_deps = [":config_cc_proto"],  # ":" + name + "_cc_proto"
    deps = ["config_cc_proto_validator"],  # name + "_cc_proto_validator" !!!
```

Rule `config_cc_proto_validator` must have a matching `config.proto.validator.h`
header. That header must provide `Validate` methods for all message types:
