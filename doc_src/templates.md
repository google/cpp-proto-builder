# ProtoBuilder Templates and Inheritance

https://google.github.io/cpp-proto-builder/templates

[TOC]

## Templates and Inheritance

It is possible to fully control code generation by changing the code templates
used by the tool. And while that provides ample opportunity, there is a risk of
creating builders that do not interact well with other builders. Further in many
cases a better approach is to [inherit the builder](#inheritance).

## Templates

The tool uses the [Google Template System](http://goog-ctemplate.sourceforge.net/) so any `{{.*}}`
sequence is expected to be a template parameter to be expanded.

When the tool expands the template, then it is important to ensure correct code
will be generated. For cases where a single proto message builder is being
generated, any part outside the accessors can be edited. However, when using
multiple message types, only the code outside the generated class code can be
modified, or it has to be written in a way that the modification applies to all
generated classes using the template vars. Please refer to the default template
as a starting point when developing custom templates.

TIP: The generated header 'exports' all message types it works with. Thus when
including the generated builder header, it is not necessary to also import all
`*.pb.h` files as well.

> NOTE: If a template section marker is preceded only by spaces and followed by
> a new-line, then its whole line will be ignored. That in turn means that new
> lines from sections must be explicit, and also that an empty section will be
> replaced with nothing, even if the section markers are preceded by whitespace.
>
> ```txt
>   {{#FOO}}
>   foo();
>   {{/FOO}}
>
>   {{BAR}}{{BAZ}}
> ```
>
> In the above example the `{{#FOO}}` and `{{/FOO}}` tags are on a line by
> themselves only preceded by whitespace, thus they will be removed completely
> and `foo();` will have no additional whitespace. The tags `{{BAR}}` and
> `{{BAZ}}` do not adhere to the restriction and thus the whitespace will not be
> removed.

Supported top level template variables and sections:

*   `{{HEADER_GUARD}}` The header guard to use (only the macro name). This will
    be generated from flag `--header` unless flag `--tpl_value_header` is used
    to control this value independent of the target header file.
*   `{{HEADER_FILE}}` The generated header filename to be used in an include in
    the `.cc` file. This will also be generated from `--header=` or
    `--tpl_value_header`.
*   `{{#INCLUDES}}`...`{{/INCLUDES}}` A section for include directives in the
    generated header. This contains headers for both headers and sources. It is
    useful when only the header file should have includes listed and the source
    should only reference it; or when the source should repeat the header
    includes.
    *   `{{INCLUDE}}` Each required header file name.
*   `{{#SOURCE_INCLUDES}}`...`{{/SOURCE_INCLUDES}}` A section for include
    directives in the generated source, containing only source specific
    includes.
    *   `{{INCLUDE}}` Each required header file name.
*   `{{#HEADER_INCLUDES}}`...`{{/HEADER_INCLUDES}}` A section for include
    directives in the generated header which excludes source specific includes.
    This could be used in a header file instead of `{{#INCLUDES}}`...
    `{{/INCLUDES}}`. The result is that some types will no longer be available
    to users of the generated header and they thus need to include those headers
    themselves.
    *   `{{INCLUDE}}` Each required header file name.
*   `{{#NAMESPACES}}`...`{{/NAMESPACES}}` A section for the namespaces.
    *   `{NAMESPACE}}` Each subsequent namespace to make up the nested namespace
        for the generated builder.
*   `{{#ALL_NAMESPACES}}`...`{{/ALL_NAMESPACES}}` A section for the namespaces.
    *   `{NAMESPACE}}` All namespaces appear as single entry, joined by `::` if
        at least one namespace is in use.
*   `{{#BUILDER}}`...`{{/BUILDER}}` The section for the generated class code.
    *   See [basics](#basics)
    *   See [builtins](#builtins).
    *   `{{GENERATED_HEADER_CODE}}` Place for all generated field accessor
        declarations.
    *   `{{GENERATED_SOURCE_CODE}}` Place for all generated field accessor
        implementations.
*   `{{#USE_BUILD}}` Available if `use_builder` or `use_validator` message
    options are enabled (default: automatic). If validators are in use anywhere
    in the project codebase, or `Build` and `Consume` are preferred over the
    conversion operator, then this should be enabled for the whole project.
    *   See [basics](#basics).
    *   See [builtins](#builtins).
    *   See [custom vars](#custom_vars).
    *   `{{#NOT_BUILD}}` Becomes available if `{{#USE_BUILD}}` is not.
*   `{{#USE_CONVERSION}}` Available if `use_conversion` message option is
    enabled (default: true).
    *   See [basics](#basics).
    *   See [builtins](#builtins).
    *   See [custom vars](#custom_vars).
    *   `{{#NOT_CONVERSION}}` Becomes available if `{{#USE_CONVERSION}}` is not.
*   `{{#USE_STATUS}}` Available is `use_build`, `use_status` or `use_validator`
    message options are enabled (default: automatic).
    *   See [basics](#basics).
    *   See [builtins](#builtins).
    *   See [custom vars](#custom_vars).
    *   `{{#NOT_STATUS}}` Becomes available if `{{#USE_STATUS}}` is not.
*   `{{USE_VALIDATOR}}` Available if `use_validator` message option is enabled
    or if a validator library was added as a dependency and the message options
    do not actively disable it (default: automatic).
    *   See [basics](#basics).
    *   See [builtins](#builtins).
    *   See [custom vars](#custom_vars).
    *   `{{#NOT_VALIDATOR}}` Becomes available if `{{#USE_VALIATOR}}` is not.

NOTE: Sections cannot be nested.

### Basics

*   `{{CLASS_NAME}}` The name of the generated class for each proto message type
    (by appending `Builder` to the message type name).
*   `{{NAMESPACE}}` The namespace string (e.g. `foo:bar::baz`).
*   `{{PROTO_TYPE}}` The type name of the wrapped proto message type.
*   `{{PROTO_TYPE_SHORT}}` The proto type name without the namespace.
*   `{{ROOT_DATA}}` refers to the internal proto data. This reflects the message
    option `root_data` but with `.` or `->` removed.
*   `{{VALIDATE_DATA}}` If message option `use_validator` is set, then this will
    be set to `ValidateData();`, otherwise it will be empty.

### Interfaces

When interfaces are being generated, then additional variables are available:

*   `{{INTERFACE_GUARD}}` The header guard for the interface header file.
*   `{{INTERFACE_INCLUDES}}` All header includes for the interface. This is the
    same as `{{INCLUDES}}` but without the include for the generated builder and
    no includes configured as `builder_include` in the message options.
*   `{{INTERFACE_NAME}}` The name of the generated interface
*   `{{GENERATED_INTERFACE_CODE}}` All generated pure virtual setters.

### Builtins

All '%' types from the configuration are present as `{{%type}}`. That means for
example `{{%Status}}` can be used to represent the configured Status type, which
by default is `absl::Status`. These types must be present in a custom
`config.textproto` file, but can be modified to suite user needs.

Builtins can be configured for parameters, arguments and with default values,
this is in particular used with `%SourceLocation`:

```textproto
type_map {
  key: "%SourceLocation"
  value {
    type: "absl::SourceLocation"
    value: "absl::SourceLocation::current()"
    include: "absl/types/source_location.h"
    dependency: "//absl/types:source_location"
    param: "source_location"
  }
}
```

That configuration leads to the following template variables:

template var                      | result                                                                   | required `FieldBuilderOptions`
--------------------------------- | ------------------------------------------------------------------------ | ------------------------------
`{{%SourceLocation}}`             | `absl::SourceLocation`                                                   | type
`{{%SourceLocation%param}}`       | `source_location`                                                        | type
`{{%SourceLocation+param}}`       | `absl::SourceLocation source_location`                                   | type
`{{%SourceLocation+param=value}}` | `absl::SourceLocation source_location = absl::SourceLocation::current()` | type + value
`{{%SourceLocation%value}}`       | `absl::SourceLocation::current()`                                        | type + value

### Custom vars {#custom_vars}

Custom template variables function exactly like builtins, with the only
difference that custom variables are not checked and can be added arbitrarily.
Custom keys must start with the `$` character, followed by one alphabetical
character, followed by any number of alphanumeric characters.

```textproto
type_map {
  key: "$MyType"
  value {
    type: "my::Type"
    value: "my::Value()"
    param: "my_param"
  }
}
```

That configuration leads to the following template variables:

<!-- mdformat off(breaks code) -->
| template var              | result          | required `FieldBuilderOptions` |
| ------------------------- | --------------------------------- | ------------ |
| `{{$MyType}}`             | `my::Type`                        | type         |
| `{{$MyType%param}}`       | `my_param`                        | type         |
| `{{$MyType+param}}`       | `my::Type my_param`               | type         |
| `{{$MyType+param=value}}` | `my::Type my_param = my::Value()` | type + value |
| `{{$MyType%value}}`       | `my::Value()`                     | type + value |
<!-- mdformat on -->

NOTE: If `value` is not specified, then it will not be available.

NOTE: If `param` is not specified then it defaults to `type` without the initial
`%` or `$` removed and converted to snake-case (retaining initial and suffix '_'
while converting all uppercase letters into lowercase ones and inserting '_'
before any non-consecutive uppercase letter). All non alphanumeric characters
will be replaced with underscores but no consecutive underscores will be
generated. In the example above the type `my::Type` converts to `my_type` and
`my::smallZero0type` converts to `my_small_zero0type`.

### code-insertion-markers {#BEGIN}

The default templates use two important code-insertion-markers:

*   `// https://google.github.io/cpp-proto-builder/templates#BEGIN`
*   `// https://google.github.io/cpp-proto-builder/templates#END` {#END}

These markers are obvious links, that direct the reader directly to their
documentation. The best way to use these markers in a source `.cc` template is:

```c++
// https://google.github.io/cpp-proto-builder/templates#BEGIN
// {{#BUILDER}}
// {{GENERATED_SOURCE_CODE}}
// {{/BUILDER}}
// https://google.github.io/cpp-proto-builder/templates#END
```

For the header `.h` file the inner template will be `{{GENERATED_HEADER_CODE}}`.
It is possible to handle generation of the header guards by the template as well
while still allowing a template that can be compiled and has no lint issues.

```c++
#ifndef PATH_TO_TEMPLATE_HEADER_TPL_H_  // {{HEADER_GUARD}}  // NOLINT
#define PATH_TO_TEMPLATE_HEADER_TPL_H_  // {{HEADER_GUARD}}  // NOLINT

// ...

// https://google.github.io/cpp-proto-builder/templates#BEGIN
// {{#BUILDER}}
// {{GENERATED_SOURCE_CODE}}
// {{/BUILDER}}
// https://google.github.io/cpp-proto-builder/templates#END

// ...

// clang-format off
#endif // PATH_TO_TEMPLATE_HEADER_TPL_H_ {{HEADER_GUARD}} NOLINT
```

> NOTE: See how the three special section markers above are preceded by
> comments:
>
> *   `// {{#BUILDER}}`
> *   `// {{GENERATED_SOURCE_CODE}}`
> *   `// {{/BUILDER}}`
>
> They will be preprocessed in a way that removes the comment, leaving them by
> themselves on an empty line, thus they will be removed completely.

While this works well for files with a single message, this fails for header
files with multiple messages where the template creates much of the content like
the default template does
([default.h.tpl](http://google.github.io/cpp-proto-builder/proto_builder/default.h.tpl)).
The issue with repeated messages is that each message needs its own class
structure which can only be done by replacing everything (all class code).
However the replacement cannot determine the original template or manual code
edits. Thus the markers are placed only around the generated methods. But those
occur in each class and that cannot be handled with the templating system.
Similarly inner repeated messages, will also appear as separate top level
builders.

```proto
message Foo {
  repeated Bar {
  }
  optional Baz {
  }
}
```

Consider the following 'silly' template:

```txt
{{#BUILDER}}
class {{CLASS_NAME}} {};
{{/BUILDER}}
```

The above by default results in two builders:

```c++
class FooBuilder {};
class Foo_BarBuilder {};
```

### MessageBuilderOptions

Each message can have its own `MessageBuilderOptions` that override any defaults
otherwise applied. Sub messages do not inherit overrides, so those must be
repeated.

#### class_name

The name of the generated class. By default the name of the generated builder is
the message type name followed by "Builder". So the builder for the message
"Foo" will be named "FooBuilder".

Available as `{{CLASS_NAME}}`.

`optional string class_name = 1;`

#### root_data

The name under which root data is accessible. For structs and classes add the
'.'. For pointers add the '->'.

Available as `{{ROOT_DATA}}` with '.' or '->' removed.

`optional string root_data = 2 [default = "data_."];`

#### root_name

The root name prefix. For further explanation see
[FieldBuilderOptions.name](config.md#FieldBuilderOptions.name).

`optional string root_name = 3;`

#### use_build

Automatically adds two methods for the default template:

*   `absl::StatusOr<ProtoType> Build() const`
*   `absl::StatusOr<ProtoType> Consume()`.

This also enables `use_status`.

Available via section `{{#USE_BUILD}}`.

`optional bool use_build = 4;`

#### use_conversion

Adds a conversion operator to the default template.

Available via section `{{#USE_CONVERSION}}`.

`optional bool use_conversion = 5 [default = true];`

The default template uses a conversion to `const&` that either returns the proto
or the default instance if `use_status` is active and the internal status is
non-ok.

*   `operator const ProtoType&() const;`

#### use_status

Adds an 'absl::Status status_' member and related handling for the default
template. Otherwise it is assumed that a member `status_` is present.

Available via section `{{#USE_STATUS}}`.

This setting also adds the following special methods:

*   `absl::StatusOr<ProtoType> MaybeGetRawData() const;
*   `bool ok() const;`
*   `absl::Status status() const`
*   `Builder& UpdateStatus(absl::Status status);`

Further all setters for message types will have an additional setter that takes
a Builder type instead of the proto type. That setter will only be available for
actual builder types that support the correct magic `MaybeGetRawData` method.
That method will control whether the raw data or the status has to be returned
which in turn is controlled by [validation](validation.md#MaybeGetRawData) if
that is active.

`optional bool use_status = 6;`

#### use_validator

Adds 'void ValidateData();' method, that validates the current data and sets
status_ accordingly. This setting also automatically activates `use_build` and
`use_status`.

Available via section `{{#USE_VALIDATOR}}`.

Also sets `{{VALIDATE_DATA}}` to `ValidateData();`.

`optional bool use_validator = 7;`

Read more about [Validation](validation.md).

### Adding custom functions

It is possible to add custom functions and data members to all generated
builders using a custom template. It is further possible to change the way how
the proto data is stored.

The following custom template moves the proto data into struct `Data`. In order
to use that in setter generation, the messages need a message level attribute
[MessageBuilderOptions.root_data](config.md#MessageBuilderOptions.root_data).

```proto
syntax = "proto2";

package my_proto;

import "proto_builder/proto_builder.proto";

message Foo {
  option (proto_builder.message) = {
    root_data: "data_.proto."
  };

  optional string test = 1;
}
```

The whole class is actually residing in the `{{#BUILDER}}...{{/BUILDER}}`
section. The class name FooBuilder is provided through `{{CLASS_NAME}}` and the
proto buffer type (the message type name) is available through `{{PROTO_TYPE}}`.
The setters will be inserted in the class declaration through
`{{GENERATED_HEADER_CODE}}` and the implementation will be added through
`{{GENERATED_SOURCE_CODE}}`.

```txt
{{#BUILDER}}
class {{CLASS_NAME}} {
 public:
  {{CLASS_NAME}}() = default;

  {{PROTO_TYPE}} data() const { return data_.proto; }

  void Clear() { data_.proto.Clear(); }

  // https://google.github.io/cpp-proto-builder#BEGIN

  // {{GENERATED_HEADER_CODE}}
  // https://google.github.io/cpp-proto-builder#END

 private:
  struct Data {
    {{PROTO_TYPE}} proto;
  };

  Data data_;
};

void {{CLASS_NAME}}::Clear() { data_.proto.Clear(); }

{{GENERATED_SOURCE_CODE}}

{{/BUILDER}}
```

This results in the following header:

```c++
class FooBuilder {
 public:
  FooBuilder() = default;

  Foo data() const { return data_.proto; }

  void Clear();

  // https://google.github.io/cpp-proto-builder#BEGIN

  FooBuilder& SetTest(const std::string& value);
  // https://google.github.io/cpp-proto-builder#END

 private:
  struct Data {
    Foo proto;
  };

  Data data_;
};

void FooBuilder::Clear() { data_.proto.Clear(); }

FooBuilder& FooBuilder::SetTest(const std::string& value) {
  data_.proto.set_test(value);
  return *this;
}

```

The generated code has the custom function `Clear` and its `Foo proto` stored in
a private struct. Th template also puts both declaration and implementation of
the setters into a single output header file.

This template example is extremely limited and it is likely necessary that more
aspects of the default templates are copied before it becomes useful.

TIP: When customizing templates, copy the default templates. Then start adding
and modifying as needed.

## Builder Inheritance {#inheritance}

Builders are normal classes, so they can be extended, unless a custom template
marks them as final.

```proto
message Inherit {
  option (proto_builder.message) = {
    class_name: "InheritProtoBuilder"
  };

  optional string first_name = 1;
  optional string last_name = 2;
}
```

```c++
class InheritProtoBuilder {
  InheritProtoBuilder() = default;
  InheritProtoBuilder(const InheritProtoBuilder&);
  InheritProtoBuilder(InheritProtoBuilder&&);

  InheritProtoBuilder& SetFirstName(const std::string& value);
  InheritProtoBuilder& SetLastName(const std::string& value);
};
```

Now the class `InheritProtoBuilder` can be extended. But the problem is that the
setters will return `InheritProtoBuilder&`, so normal inheritance is fairly
restricted. In order to return the derived class, each setter will have to be
overridden:

```c++
class InheritBuilder : protected InheritProtoBuilder {
 public:
  using InheritProtoBuilder;

  // Repeating `SetFirstName` to change its return type.
  InheritBuilder& SetFirstName(const std::string& value) {
    InheritProtoBuilder::SetFirstName(value);
    return *this;
  }

  // Repeating `SetLastName` to change its return type.
  InheritBuilder& SetLastName(const std::string& value) {
    InheritProtoBuilder::SetLastName(value);
    return *this;
  }

  InheritBuilder& SetFirstLast(const std::string& value) {
    return SetFirstName(value).SetLastName(name);
  }
};
```

### CRTP

ProtoBuilder supports
[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) which
allows for much more elegant ways of inheritance and extendability of builder.
Instead of adding functionality later, as is done when deriving a builder, CRTP
allows to mix-in the additional functionality.

First there needs to be a CRTP base template that can reference the Builder via
a template parameter. This template knows its actual type via the template
parameter and thus it can return itself converted to the builder type. In order
for this base template to access the builder there are two ways.

1.  Provide virtual base methods and mark the corresponding setters as
    `override` so that the generated setters will override the pure virtual
    methods. This is pretty manual but does not require additional source files.
    If the corresponding fields are not marked `override` then the compiler
    might issue warnings, depending on the compiler flags.
1.  If the template header does not directly implement anything that requires
    access to members of the base template parameter, then no overriding is
    necessary and the implementations can be provided in another source file
    that includes both the base template header and the builder header.

#### CRTP using overrides

Here is the single source file solution to add `SetFirstLast` to the
`InheritBuilder` from the earlier example.

```c++
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
```

Next the proto configuration needs to trigger the CRTP support. It needs to
configure a `base_class`, which can refer to the builder itself via
`@class_name@`. The proto message also needs to include the base template.

```proto
message Inherit {
  option (proto_builder.message) = {
    base_class: "public InheritBase<@class_name@>"
    include: "path/to/inherit_base.h"
  };

  optional string first_name = 1 [(proto_builder.field) = { override: true }];
  optional string last_name = 2 [(proto_builder.field) = { override: true }];
}
```

NOTE: The key improvement in this approach is that the generated builder is the
user API and no wrapper is needed. Thus all setters already return the correct
type. However, the user still has to predeclare all methods that should be used
in the CRTP and each of the corresponding fields must be maked as `override`.

#### CRTP using auto generated CRTP interface

ProtoBuilder can generated pure virtual interfaces that simplify CRTP
implementations and remove the need for extra field annotations.

BEST PRACTICE: For this to work the `cc_proto_builder_library` rule must set
`make_interface = 1` and the CRTP code must become extra headers/sources to the
rule. Further, the include for the CRTP user class must be declared as
`builder_include` as opposed to `include` so that it does not end up as an
include to the generated interface.

```proto
message Inherit {
  option (proto_builder.message) = {
    base_class: "public InheritBase<{{CLASS_NAME}}, {{INTERFACE_NAME}}<{{CLASS_NAME}}>>"
    builder_include: "path/to/inherit_base.h"
  };

  optional string first_name = 1;
  optional string last_name = 2;
}
```

Because of `make_interface` ProtoBuilder will generate the following pure
virtual class (aka. interface) which in itself is a CRT template, as it needs to
know the final class (the builder), so that the setters can return the builder
reference. The name of this interface is the name of the builder with
'Interface' appended, so message `Inherit` has an `InheritBuilderInterface` and
an `InheritBuilder`.

```c++
#include <string>

#include "path/to/generated/inherit.pb.h"

template <class InheritBuilder>
class InheritBuilderInterface {
 public:
  InheritBuilderInterface() = default;
  virtual ~InheritBuilderInterface() = default;

  virtual InheritBuilder& SetFirstName(const std::string& value) = 0;
  virtual InheritBuilder& SetLastName(const std::string& value) = 0;
};
```

NOTE: The default interface template uses `{{CLASS_NAME}}` as the template
parameter name to make it appear as if it was actually using that class. In the
compiled result it does anyway, since the template parameter resolves to that.
So this is merely syntactic sugar.

The user CRTP now has access to both the actual builder as well as to the
interface:

```c++
#include <string>

#include "path/to/generated/inherit.pb.h"
#include "path/to/generated/inherit_cc_proto_builder.interface.h"

template <class InheritBuilder, class InheritBuilderInterface>
class InheritBase : public Interface {
 public:
  InheritBase() = default;

  InheritBuilder& SetFirstLast(const std::string& first,
                               const std::string& last) {
    return this->SetFirstName(first).SetLastName(last);
  }
};
```

NOTE: Calls to interface methods must be expressed as `this` calls, due to
[nondependent-name-lookup-members](http://www.parashift.com/c++-faq-lite/nondependent-name-lookup-members.html).

Since the user CRTP's header now needs access to the generated interface, it
must be declared as an extra header to the builder rule:

```BUILD
cc_proto_builder_library(
    name = "inherit_cc_proto_builder",
    cc_proto_library_deps = ["inherit_cc_proto"],
    extra_hdrs = ["inherit_base.h"],
    make_interface = 1,
)
```

In conclusion, this approach allows to inject arbitrary code into a builder
without the need to modify any code templates or the need to save and manually
maintain generated code.
