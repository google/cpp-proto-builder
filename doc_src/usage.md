# ProtoBuilder usage

https://google.github.io/cpp-proto-builder/usage

[TOC]

## `BUILD` integration {#BUILD}

The best way to use `proto_builder` is via its `BUILD` integration. There are
three rules that allow `proto_builder` integration into `BUILD` files.

*   `cc_proto_builder_library`: Generate a cc_library rule using proto_builder.
*   `proto_builder_test`: Compares the generated files of a proto_builder target
    with expected results.
*   `proto_builder`: Generate source files only.

### `cc_proto_builder_library`

The most integrated rule is `cc_proto_builder_library`. This rule creates source
files (.cc, .h) and provides them as a stand alone `cc_library` rule. This means
the build system will re-generate the sources whenever the inputs to the rule
changes. The name of the rule can be used just like any other cc_library rule's
name and thus the name is what users have to depend on.

See below for an illustration:

```build
load("//proto_builder:proto_builder.bzl", "cc_proto_builder_library")

proto_library(
    name = "my_type_proto",
    srcs = ["my_type.proto"],
)

cc_proto_library(
    name = "my_type_cc_proto",
    deps = [":my_type_proto"],
)

cc_proto_builder_library(
    name = "my_type_cc_proto_builder",
    cc_proto_library_deps = [":my_type_cc_proto"],
    proto = ["my.package.path.MyType"],
    deps = [
        # Add any non transitive `cc_proto_library` rule dependencies here.
        #
        # In particular validators belong here.
        #
        # Add any non default configured rule here.
        #
        # NOTE: Not all conversion libraries will be appended automatically in
        # order to prevent dependency creep. However, when conversions are used
        # it is otherwise safe to depend on `:proto_builder_conversions`.
        "//proto_builder:proto_builder_conversions",
    ],
)

cc_library(
    ...
    deps = [
      ":my_type_cc_proto_builder",
    ],
    ...
)
```

In the example, the proto file `my_type.proto` has `package my.package.path` and
defines a message type named `MyType`.

The `cc_proto_builder_library` rule will generate a class that has the same name
as the wrapped class with the `Builder` suffix appended. So the proto type
`MyType` becomes the `MyTypeBuilder`. The rule generates a header file whose
file name is derived from the rule name by appending '.h'. In the above example
`my_type_proto_builder.h`. Thus use the builder as:

```c++
#include "<PATH/TO/BUILD>/my_type_proto_builder.h"

namespace my::package::path {

void Use() {
  MyType data = MyTypeBuilder().SetIntField(42);
}

}  // namespace my::package::path
```

The `cc_proto_builder_library` rule will reference the `cc_proto_library` rule
and generate a builder with the name `MyTypeBuilder` in the same namespace as
the type's package path. The `deps` list in the example lists all libraries
needed if all supported type conversions are used.

NOTE: The `my_type_builder` BUILD target from the example above only generates a
builder for `my.package.path.MyType` but not for any sub messages. Read about
[Finding Messages](#finding_messages) to automatically select message types.

NOTE: The `cc_proto_builder_library` rule must reference the `proto_library`
rule through a `cc_proto_library` rule. This means rules depending on the
builder rule can simply depend on it rather than having to depend on both the
`cc_proto_library` and the `cc_proto_builder_library` rule.

NOTE: No `cc_proto_library` rule for any `proto_library` rule that is
transitively referenced by the specified `cc_proto_library_deps` rule has to be
added as a dependency in `deps`. However, that does not mean that the builder
rule actually depends on those `cc_proto_library` rules. The generated builder
code merely has access to the generated headers file through the specified
`cc_proto_library_deps` rule and if the build system supports it, then those
headers will be marked as exported, so that a user of a builder rule does not
need to depend on those rules or include their headers manually.

> TIP: The involved rule names should follow a pattern where the base name is
> the name of the `.proto` file with the '.' replaced by a '_'. Example:
>
> *   `proto_library`: name := srcs[0]/./_/; e.g.: *my.proto -> my_proto*
> *   `cc_proto_library`: name + '_cc_proto'; e.g.: *my_cc_proto*
> *   `cc_proto_builder_library`: name + '_cc_proto_builder'; e.g.:
>     *my_cc_proto_builder*
>
> This makes it easy to connect the `.proto` source file with all related rules.

Consider the following message which has a sub-message:

```proto
message MyType {
  message EmbeddedType {
    ...
  }
}
```

NOTE: No builder will be created for `MyType.EmbeddedType` by the build rule
above. It is however possible to create a builder for this by updating the
`cc_proto_builder_library` to specify the exact messages for which a builder
should be created or by using `proto = ["**"]`, see
[Finding Messages](#finding_messages).

```build
cc_proto_builder_library(
    name = "my_type_cc_proto_builder",
    cc_proto_library_deps = [":my_type_cc_proto"],
    proto = [
        "my.package.path.MyType",
        "my.package.path.MyType.EmbeddedType",
    ],
    ...
)
```

NOTE: In order to prevent naming collisions, the '.' in the message type path
will be converted to a '_' in the generated builder. In the example that results
in `MyType_EmbeddedTypeBuilder`.

#### Finding messages {#finding_messages}

If the `proto` field in the `cc_proto_builder_library` rule is not specified, it
defaults to `*` which creates builders for all top-level messages while
recursing all message fields (aka flat builders). But in this mode builders will
not be created for imported or embedded message types.

If the `proto` field is specified as `**` then builders will be created for all
top-level messages as well as all recursive messages declared from the top-level
messages (aka simple builders). But in this mode there still will not be any
builder created for imported message types.

Consider the following example:

File: my_other_type.proto

```proto
package my.package;

message MyOtherType { ... }
```

File: my_type.proto

```proto
package my.package;

import "my/package/my_other_type.proto";

message MyType {
  optional MyOtherType other = 1;

  message SubType { ... }

  optional SubType sub = 2;

  message RepType { ... }

  repeated RepType rep = 3;
}
```

```build
cc_proto_builder_library(
    name = "my_type_cc_proto_builder",
    cc_proto_library_deps = ["my_type_cc_proto"],
    proto = ["*+"],  # OR ["*"] OR ["**"]
    ...
)
```

The setting `proto = ["*"]` generates builders for all top level messages
defined in the proto file. In the example that means only generate a builder for
`MyType`.

The setting `proto = ["*+"]` (the default) generates builders for all top level
messages and for all messages used as repeated fields or values of maps. For the
example builders will be created for `MyType` and `MyType.Rep` while all fields
will be recused to repeated fields. This allows to use single builders in the
absence of repeated fields of message types while being able to use separate
builders for those fields and their sub fields.

The setting `proto = ["**"]` generates builders for all messages defined in the
proto file. In the example that means messages: `MyType`, `MyType.SubType` and
`MyType.RepType` but not `MyOtherType`. However, in this mode the tool does not
flatten out the generated builders, and per message only the top level fields
will have setters.

> TIP: Chose the mode as follows:
>
> *   `proto = ["**"]`: For complex types.
> *   `proto = ["*+"]`: If any repeated field has a message type.
> *   `proto = ["*"]`: For simple types.
>
> However, within a project the mode should always be the same.

#### Templates

When a plain out-of-the-box builder is fine, then `template_src` and
`template_hdr` can be omitted and the default template will be used. Read more
about how to write these in config section [templating](templates.md).

### `proto_builder_test`

IMPORTANT: Use this rule **only** if the direct use of the
`cc_proto_builder_library` rule would cause a cyclic dependency. And in most
cases [`cc_proto_builder_manual_library`](#cc_proto_builder_manual_library) will
be a much better solution.

The rule assumes that the input files (`expected_src` and `expected_hdr`) were
manually created with proto_builder as _golden_ files. The rule compares the
result of a `proto_builder` target with those _golden_ input files. If the rule
fails, the developer needs to regenerate the _golden_ files (in the same CL).
That way, it is ensured that the files are always up-to-date without a cyclic
dependency.

```build
load("//proto_builder:proto_builder.bzl", "proto_builder", "proto_builder_test")

proto_library(
    name = "my_type_proto",
    srcs = ["my_type.proto"],
)

cc_proto_library(
    name = "my_type_cc_proto",
    deps = [":my_type_proto"],
)

# The library uses checked-in sources and does not depend on the builder rule.
cc_library(
    name = "my_type_cc_proto_builder",
    hdrs = [":my_type_cc_proto_builder.cc"],  # Checked-in.
    srcs = [":my_type_cc_proto_builder.h"],   # Checked-in.
    deps = [":my_type_cc_proto"],
)

# The builder rule is only used for the proto_builder_test rule.
proto_builder(
    name = "my_type_cc_proto_builder_gen",       # Note additional '_gen'.
    testonly = 1,                                # Only for proto_builder_test.
    cc_proto_library_deps = [":my_proto_cc_proto"],
    header_out = "my_type_cc_proto_builder.h",   # Generated (name as checked-in).
    proto = ["my.proto.MyMessage"],
    source_out = "my_type_cc_proto_builder.cc",  # Generated (name as checked-in).
    visibility = ["//visibility:private"],       # Prevent accidental use.
)

proto_builder_test(
    name = "my_type_cc_proto_builder_gen_test",
    expected_hdr = ":my_type_cc_proto_builder.h",   # Checked-in.
    expected_src = ":my_type_cc_proto_builder.cc",  # Checked-in.
    proto_builder_dep = "my_type_cc_proto_builder_gen",
)
```

NOTE: The above specifies the name of the generated source and header file, so
that their header guards and includes are correct and match with the checked-in
version. The test will distinguish the checked-in files from the generated files
through rule attributes. And while the filename is the same, the directories are
actually different.

> TIP: If such a test fails, then its output contains the diffs between the
> generated (result) file vs the checked-in (golden) file. To make the test
> pass, the golden file can be modified by adding or removing the appropriate
> content from the diffed output.
>
> ```sh
> bazel test --test_strategy=exclusive --test_output=streamed my/proto:my_type_cc_proto_builder_gen_test
> ```

NOTE: It is of course possible that custom header/source templates may also need
to be adjusted. This happens for instance if new includes become necessary.

### `cc_proto_builder_manual_library`

Creates a cc_library 'name' and verifies src and hdr as proto_builder.

This macro simplifies cases where cyclic dependencies have to be broken. Instead
of the macro depending directly on a builder the macro assumes that there is
exactly one source and one header file which are manually created builder source
and header files. The macro generates a `cc_library` from these sources.

The macro then creates a builder for `cc_proto_library_deps` and the specified
proto types. It then compares the manual sources against the generated ones.

This allows to break dependency cycles and can ensure that the sources for the
`cc_library` are up to date.

The deps must be managed manually, however, the macro produces an output file
(name + 'cc_proto_builder.cc_deps.txt') that contains the necessary
dependencies.

If the generated test (name + '_golden_test') fails, then there are
automatically created versions of source and header in the bazel output that can
be copied as new input source and header. If the test fails to run, then the
generator (name + '_golden') must be run explicitly.

```BUILD
proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
)

cc_proto_library(
    name = "example_cc_proto",
    deps = [":example_proto"],
)

cc_proto_builder_manual_library(
    name = "example_cc_proto_builder",
    srcs = ["example_cc_proto_builder.cc"],
    hdrs = ["example_cc_proto_builder.h"],
    cc_proto_library_deps = [":example_cc_proto"],
)
```

### `proto_builder`

This is the rule that generates the source files. The rule also reformats the
generated files using `clang-format`.

IMPORTANT: Use this only in conjunction with `proto_builder_test` as described
above. In all normal cases rely on `cc_proto_builder_library` to depend on this.

## Commandline tool

While the intended use is through the BUILD rules, it is possible to manually
run the tool in a shell. The command line tool allows to create header (.h) and
source (.cc) code embedded into templates or existing code.

```sh
proto_builder --proto="<my.Type:/path/file.proto>"
              --header="<filename_of_generated_header>"
              --source="<filename_of_generated_source>"
              [--header_in="<filename_of_header_template>"]
              [--source_in="<filename_of_source_template>"]
              [--tpl_value_header="<target_header_filename_for_template>"]
              [--template_builder_strip_prefix_dir="<base_dir>"]
              [--workdir="<cwd>"]
              [--max_field_depth=<max_field_depth>]
```

TIP: You can run: `bazel run net/proto2/contrib/proto_builder --
--workdir="${PWD}" ...`

TIP: The best way to use the tool is to leave the default template as is or to
copy and modify them as needed and then to re-generate the builder through the
[`BUILD`](#BUILD) integration.

TIP: The tool also supports flags `--protofiles`, `--proto_paths` and
`--use_global_db` from `SourceFileDatabase`. Use these only, if you cannot
otherwise load the required dependencies using the `--proto` flag.
