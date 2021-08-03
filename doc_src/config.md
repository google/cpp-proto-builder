# ProtoBuilder config

https://google.github.io/cpp-proto-builder/config

## Configuration

[TOC]

The generated code can be configured in the following ways:

1.  Per message in the proto file via
    [MessageBuilderOptions](#MessageBuilderOptions) message option.
1.  Per field in the proto file via [FieldBuilderOptions](#FieldBuilderOptions)
    field options.
1.  Using a configuration file which can be set in the bazel rules via attribute
    `proto_builder_config` or on the command line via flag
    `--proto_builder_config`. In each case, the value must be a label/string
    identifying a textproto file of type `ProtoBuilderConfig` which is a map of
1.  Message annotations can have a [`type_map`](#MessageBuilderOptions.type_map)
    similar to a configuration file.
1.  The generated code is fully templatized, and the
    [templates can be customized](templates.md).

By default `ProtoBuilder` generates code for every field. But code generation
can be fully customized within the proto files. For that the following
prerequisites are necessary:

*   Add the `proto_builder.proto` dependency to your `proto_library` build rule:

    ```build
    deps = ["//proto_builder:proto_builder_proto"]
    ```

*   Import `proto_builder.proto` into your `.proto` file:

    ```proto
    import "proto_builder/proto_builder.proto";
    ```

*   Add message and field options similar to the following example:

    ```proto
    message MyMessage {
      option (proto_builder.message) = {
        use_status: true  // or any other message options
      };

      optional int64 my_field = 1 [(proto_builder.field) = {
        output: BOTH   // or any other field options
      }];
    };
    ```

Now each message type and field can be configured as described below. The main
goal of customizing a builder is to fine tune the generated API to better work
for actual data, as opposed to dealing directly with protocol buffers and their
serialization.

TIP: There are no configuration options to control code layout. Instead the
generated code should be run through a `tidy` or other means of reformatting. If
the tool is used through its [BUILD](usage.md#BUILD) integration, then automatic
formatting using `clang-format` will be performed.

### `MessageBuilderOptions` {#MessageBuilderOptions}

Allows customization of message type code generation.

The `MessageBuilderOptions` also play a role in [template](templates.md)
configuration.

#### `MessageBuilderOptions.class_name` {#MessageBuilderOptions.class_name}

By default the generated class name is composed of the message type name with
the suffix '`Builder`'. So the protocol message type '`FooBar`' will be named
'`FooBarBuilder`'. Using '`MessageBuilderOptions.class_name`' any custom name
can be provided.

NOTE: Custom names will not get the suffix '`Builder`'.

#### `MessageBuilderOptions.root_data` {#MessageBuilderOptions.root_data}

The generated code assumes that the proto instance to build is accessible by
(non pointer) data member '`data_`'. So any access will be prefixed with the
default value for '`root_data`' which is set to '`data_.`'.

This can be customized with any legal C++ code. This allows to use the result of
a pointer member as much as a function call. But the access operator '`.`' or
'`->`' must always be provided as part of '`root_data`'.

#### `MessageBuilderOptions.root_name` {#MessageBuilderOptions.root_name}

This config option allows to inject a root prefix to all generated access
methods. For more on this read about
[`FieldBuilderOptions.name`](#FieldBuilderOptions.name).

#### `MessageBuilderOptions.use_conversion` {#MessageBuilderOptions.use_conversion}

Controls whether or not the generated builder for that message has a conversion
operator. This is enabled by default. If you deactivate this, then your builder
will be incompatible with other builders, so this should be only used in very
special circumstances where the compatibility is not ever going to matter.

> NOTE: The incompatibility arises because it is assumed that a sub-builder can
> always produce the proto, e.g.:
>
> ```c++
> Foo foo = FooBuilder().SetSubMessage(SubMessageBuilder());
> ```
>
> By disabling `use_conversion` SubMessageBuilder() can no longer be used as an
> argument to `SetSubMessage`.

#### `MessageBuilderOptions.use_build` {#MessageBuilderOptions.use_build}

Changes `use_status` to true and adds two new methods to the generated builder:

```c++
StatusOr<ProtoType> Build() const;
StatusOr<ProtoType> Consume();
Builder& UpdateStatus(absl::Status status);
```

This is most useful for [validation](validation.md) where this is done
automatically.

#### `MessageBuilderOptions.use_status` {#MessageBuilderOptions.use_status}

Adds status management to the builder, which allows to track whether or not the
builder is in an ok or error state. This is most useful for
[validation](validation.md) where this is done automatically.

#### `MessageBuilderOptions.use_validator` {#MessageBuilderOptions.use_validator}

This adds [validation support](validation.md) and activates both `use_status`
and `use_build`.

```c++
bool ok() const;
absl::Status status() const;
```

#### `MessageBuilderOptions.include` {#MessageBuilderOptions.include}

Adds `include` to the generated header code. This is useful when multiple fields
have annotations that require the same include.

#### `MessageBuilderOptions.source_include` {#MessageBuilderOptions.source_include}

Adds `source_include` as an include line to the generated source. This is useful
when multiple fields have annotations that require the same source include.

#### `MessageBuilderOptions.base_class` {#MessageBuilderOptions.base_class}

Allows to specify one or multiple base classes, e.g. `base_class: "public
MyBase"`. This is explained under [inheritance](templates.md#inheritance).

#### `MessageBuilderOptions.builder_include` {#MessageBuilderOptions.builder_include}

Only relevant for [inheritance](templates.md#inheritance). These includes are
only available in the builder sources but not in the generated interface header.

#### `MessageBuilderOptions.type_map` {#MessageBuilderOptions.type_map}

This allows to provide types and [macros](#FieldBuilderOptions.macro) to the
whole message and all inner (sub) message types in the same proto file.

#### Complete `MessageBuilderOptions`

For reference, please refer to https://google.github.io/cpp-proto-builder/proto_builder/proto_builder.proto class:MessageBuilderOptions

### `FieldBuilderOptions` {#FieldBuilderOptions}

Allows customization of field code generation. Each nested leaf field will
generate setters with either `Set` or `Add` name prefix for optional and
repeated fields respectively. The most important options are `OutputMode output`
and `string name`.

For instance if you want to disable code generation for a particular field you
would set its `OutputMode` to `SKIP`:

```proto
optional Type field_name = 42 [
  (proto_builder.field) = {output: SKIP}
];
```

`FieldBuilderOptions` are also used to control [template](templates.md) behavior
of [builtin types](templates.md#builtins).

#### `FieldBuilderOptions.name` (Name composition) {#FieldBuilderOptions.name}

By default the generated method names are composed of the prefix `Set` or `Add`,
followed by the root message's `MessageBuilderOptions.root_name` (which defaults
to the empty string), the sequence of nested message field names leading to the
leaf field and finally the name of the leaf field. All name components are
turned into CamelCaps notation. This is done to follow the style guide and in
order to be able to easily create builders that are (somewhat) comparable to
[Java's proto builder](https://developers.google.com/protocol-buffers/docs/javatutorial#builders).

For example the following message type:

```proto
message One {
  optional int first = 1
  message Two {
    repeated int field_two = 1;
    message Three {
      optional int fourth = 1
    }
    optional Three third = 2;
  }
  optional Two second = 2;
}
```

Generates the access methods below (header first, source below):

```c++
OneBuilder& SetFirst(int value);
OneBuilder& AddSecondFieldTwo(int value);
OneBuilder& SetSecondThirdFourth(int value);
```

```c++
OneBuilder& OneBuilder::SetFirst(int value) {
  data_.set_first(value);
  return *this;
}

OneBuilder& OneBuilder::AddSecondFieldTwo(int value) {
  data_.mutable_second()->add_field_two(value);
  return *this;
}

OneBuilder& OneBuilder::SetSecondThirdFourth(int value) {
  data_.mutable_second()->mutable_third()->set_fourth(value);
  return *this;
}
```

Using `FieldBuilderOptions.name` allows to change a field name component. For
example, we want to use `InnerData` instead of `second`, `Number` instead of
`field_two` and skip `third` altogether, while making clear that all of these
set `MyConfig` related data:

```proto
message One {
  option (proto_builder.message) = {
    root_name: "MyConfig."
  };

  optional int first = 1

  message Two {
    repeated int field_two = 1 [
      (proto_builder.field) = {name: "Number"}
    ];

    message Three {
      optional int fourth = 1
    }

    optional Three third = 2 [
      (proto_builder.field) = {output: SKIP}
    ];
  }

  optional Two second = 2 [
    (proto_builder.field) = {name: "InnerData"}
  ];
}
```

The generated header code (with some additional explanatory comments):

```c++
OneBuilder& SetMyConfigFirst(int value);
//          \_/\______/\___/
//           |  |       |
//           |  |       automatically generated from field name 'first'
//           |  |
//           |  One's 'MessageBuilderOptions.root_name'
//           |
//           Prefix 'Set' for optional fields
```

```c++
OneBuilder& AddMyConfigInnerDataNumber(int value);
//          \_/\______/\_______/\____/
//           |  |       |        |
//           |  |       |        field name 'field_two' configured to 'Number'
//           |  |       |
//           |  |       field name 'second' configured to 'InnerData'
//           |  |
//           |  One's 'MessageBuilderOptions.root_name'
//           |
//           Prefix 'Add' for repeated fields
```

NOTE: Since field `third` is set to `SKIP` there is no generated code for its
leaf field `fourth`.

#### `FieldBuilderOptions.type` (Type handling) {#FieldBuilderOptions.type}

The next configuration `FieldBuilderOptions.type` allows to override how
ProtoBuilder deals with the field type. This is best explained with an example
where a field `delay` is meant to store a `::absl::Duration` in an `int64` proto
field type. In this example it is better to have the setter take the intended
type `absl::Duration` rather than the `int64` field type.

```proto
message Example {
  optional int64 delay = 1 [
    (proto_builder.field) = {type: "@ToInt64Seconds"}
  ];
}
```

This will generate the following header and source code:

```c++
ExampleBuilder& SetDelay(::absl::Duration value);
```

```c++
ExampleBuilder& ExampleBuilder::SetDelay(::absl::Duration value) {
  data_.set_delay(::absl::ToInt64Seconds(value));
  return *this;
}
```

The above example uses the builtin type `@ToInt64Seconds`. All builtin types
start with `@` (which would not allow for legal C++).

Beyond the builtin types any C++ type that allows for implicit conversion to the
field's type can be used. More complex operations are possible by using
`OutputMode TEMPLATE` (see below).

The full list of supported types is best explained with the actual type map:

<section class="zippy">

For reference, please refer to https://google.github.io/cpp-proto-builder/proto_builder/proto_builder_config_oss.textproto

</section>

The necessary conversions can be fully [customized](#Conversions).

#### `FieldBuilderOptions.decorated_type` (Parameter handling) {#FieldBuilderOptions.decorated_type}

It is possible to control how the type gets decorated as a parameter. Consider
using `type: "std::string"` where the methods should use `const std::string&` as
their parameter type. While at the same time `absl::string_view` should not be
decorated. To simplify matters, some defaults are applied: If a type is either
in a different namespace or is a subtype, then we assume `const ...&` is needed,
unless the type is a message's enum type.

#### `FieldBuilderOptions.value` (Value provisioning) {#FieldBuilderOptions.value}

Using `FieldBuilderOptions.value` it is possible to create a parameter less
method that sets a specific value. Since multiple `FieldBuilderOptions` specs
can be provided per field, this allows to generate a general setter together
with some defaults.

```proto
message Example {
  optional int64 delay = 1 [
    (proto_builder.field) = {type: "@ToInt64Seconds"}
    (proto_builder.field) = {value: "0" name: "ZeroDelay"}
    (proto_builder.field) = {
      value: "::absl::Seconds(10)" name: "DefaultDelay" type: "@ToInt64Seconds"
    }
  ];
}
```

This will generate three setters, one for each configuration. The first is the
default one with the provided type conversion. The second uses the hard-coded
value `0` but no type conversion. So the value will be the direct parameter for
the field setter. The third option combines both. It provides a default value
that is not taken from the field config but rather the options. This allows us
to provide the value with any valid C++ code. In the example we are using
`::absl::Seconds(10)` to make it absolutely clear that the value set by this
method will be 10 seconds. In order for the setter to handle this value we also
provide the type `@ToInt64Seconds`.

```c++
ExampleBuilder& SetDelay(::absl::Duration value);
ExampleBuilder& SetZeroDelay();
ExampleBuilder& SetDefaultDelay();
```

```c++
ExampleBuilder& ExampleBuilder::SetDelay(::absl::Duration value) {
  data_.set_delay(::absl::ToInt64Seconds(value));
  return *this;
}

ExampleBuilder& ExampleBuilder::SetZeroDelay() {
  data_.set_delay(0);
  return *this;
}

ExampleBuilder& ExampleBuilder::SetDefaultDelay() {
  data_.set_delay(::absl::ToInt64Seconds(::absl::Seconds(10)));
  return *this;
}
```

#### `FieldBuilderOptions.include` (`#include`s) {#FieldBuilderOptions.include}

When using `type` or `value`, then it is typically necessary to add one or more
include directives. These can be listed in this field. It is possible to specify
both system as well as user defined ones. For the latter the quotes can be
omitted. But they are needed when a comment is necessary, e.g. to control
[IWYU](http://include-what-you-use.org).

```proto
include: "<string>"
include: "absl/time/time.h"
include: "\"absl/strings/string_view.\"  // IWYU pragma: export"
```

The above will generate the following code:

```c++
#include <string>
#include "absl/time/time.h"
#include "absl/strings/string_view."  // IWYU pragma: export
```

In templates this becomes available through sections `{{#HEADER_INCLUDES}}` and
`{{#INCLUDES}}`.

#### `FieldBuilderOptions.source_include` (`#include`s) {#FieldBuilderOptions.source_include}

Similar to [`FieldBuilderOptions.include`](#FieldBuilderOptions.include) but
meant for includes in the generated source (.cc) file.

In templates this becomes available through sections `{{#SOURCE_INCLUDES}}` and
`{{#INCLUDES}}`.

#### `FieldBuilderOptions.output` (Output control) {#FieldBuilderOptions.output}

The introductory to `FieldBuilderOptions` already explained how `SKIP` can be
used to suppress code generation for a field. The following list explains all
options. The examples in this section are all taken from
[TestOutput](http://google.github.io/cpp-proto-builder/tests/test_output.proto) whose
generated header and source code are
[available](http://google.github.io/cpp-proto-builder/proto_builder/tests/test_message.*exp).

*   **`SKIP`**

    Do not generate any code.

*   **`HEADER`**

    Generate the header only. That means the actual method implementation must
    be provided outside the use of `ProtoBuilder`.

*   **`SOURCE`**

    Generate the source only. That means the actual method declaration must be
    provided outside the use of `ProtoBuilder`.

*   **`BOTH`**

    Generate header and source. This is the default and results in both
    declaration and implementation being created.

*   **`TEMPLATE`**

    Generate template code for the header only but provide the implementation in
    the header. The resulting setters will be template functions that take a
    single template type argument that must be convertible into the fields value
    type. This works well with type conversions specified with
    [`FieldBuilderOptions.type`](#FieldBuilderOptions.type).

    ```proto
    message TestOutput {
      optional string template51 = 51 [
        (proto_builder.field) = {output: TEMPLATE}
      ];
      repeated string template52 = 52 [
        (proto_builder.field) = {output: TEMPLATE}
      ];
    }
    ```

    This works for both repeated and non-repeated fields and generates the
    following code:

    ```c++
    template <class Value>
    TestOutputBuilder& SetTemplate51(const Value& value) {
      data_.set_template51(value);
      return *this;
    }

    template <class Value>
    TestOutputBuilder& AddTemplate52(const Value& value) {
      data_.add_template52(value);
      return *this;
    }
    ```

*   **`FOREACH`**

    Generate template code with foreach assignment. This only works for repeated
    fields and allows to add values from a template container.

    ```proto
    message TestOutput {
      optional string foreach61 = 61 [
        (proto_builder.field) = {output: FOREACH}
      ];
      repeated string foreach62 = 62 [
        (proto_builder.field) = {output: FOREACH}
      ];
    }
    ```

    The first field is non-repeatable, so the tool will generate a detailed set
    of error messages as the field's C++ code.

    ```c++
    #error Cannot use 'output: FOREACH' with a non repeated field.
    #error Field: proto_builder.TestOutput.foreach61
    #error FieldBuilderOptions: <output: FOREACH>

    template <class Container>
    TestOutputBuilder& AddForeach62(const Container& values) {
      for (const auto& v : values) {
        data_.add_foreach62(v);
      }
      return *this;
    }
    ```

*   **`FOREACH_ADD`**

    Generate template code with foreach Add*() semantics. This is closely
    related to `FOREACH` but requires that a standard Add*() method was
    generated (or is otherwise available) that will be called instead of
    executing a direct assignment.

    ```proto
    message TestOutput {
      optional string foreach_add71 = 71 [
        (proto_builder.field) = {output: FOREACH_ADD}
      ];
      repeated string foreach_add72 = 72 [
        (proto_builder.field) = {},
        (proto_builder.field) = {output: FOREACH_ADD}
      ];
    }
    ```

    Just like with `FOREACH` a non-repeated field results in an error message.

    Note that in this example we generate two setters for field `foreach_add72`.
    The first uses a default configuration. Since we do not need to configure
    anything for this setter, we simply need to enforce its presence using `{}`.
    The second is the `FOREACH_ADD` demonstration. Note that both have the same
    method name.

    The tool requires the explicit presence of the default configuration, as
    this provides the flexibility to either generate the necessary code or to
    manually write a non default implementation.

    ```c++
    #error Cannot use 'output: FOREACH_ADD' with a non repeated field.
    #error Field: proto_builder.TestOutput.foreach_add71
    #error FieldBuilderOptions: <output: FOREACH_ADD>

    TestOutputBuilder& AddForeachAdd72(const std::string& value) {
      data_.add_foreach_add72(value);
      return *this;
    }

    template <class Container>
    TestOutputBuilder& AddForeachAdd72(const Container& values) {
      for (const auto& v : values) {
        AddForeachAdd72(v);
      }
      return *this;
    }
    ```

*   **`INITIALIZER_LIST`**

    Generate template code with foreach Add*() semantics. This is very similar
    to `FOREACH_ADD` but will take a `std::iniailizer_list` as an argument.

    ```proto
    message TestOutput {
      optional string initializer_list81 = 81 [
        (proto_builder.field) = {output: INITIALIZER_LIST}
      ];
      repeated string initializer_list82 = 82 [
        (proto_builder.field) = {}
        (proto_builder.field) = {output: INITIALIZER_LIST}
      ];
    }
    ```

    Similar to `FOREACH` and `FOREACH_ADD`, using `INITIALIZER_LIST` for a
    non-repeated field results in an error message.

    Using `INITIALIZER_LIST` requires a standard `Add*()` setter to be present.
    So again we provide the standard configuration first followed by the
    `INITIALIZER_LIST` demonstration.

    ```c++
    #error Cannot use 'output: INITIALIZER_LIST' with a non repeated field.
    #error Field: proto_builder.TestOutput.initializer_list81
    #error FieldBuilderOptions: <output: INITIALIZER_LIST>

    TestOutputBuilder& AddInitializerList82(const std::string& value) {
      data_.add_initializer_list82(value);
      return *this;
    }

    template <class Item>
    TestOutputBuilder& AddInitializerList82(std::initializer_list<Item> values) {
      for (const auto& v : values) {
        AddInitializerList82(v);
      }
      return *this;
    }
    ```

TIP: Often repeated field names have a singular name, say `target`. So adding a
single target becomes `AddTarget`. A nice thing to do when using any of
`FOREACH`, `FOREACH_ADD` and `INITIALIZER_LIST` is to change the name to plural.
So here we would say `name: "Targets"` so that the method is named `AddTargets`.

#### `FieldBuilderOptions.recurse` {#FieldBuilderOptions.recurse}

The recurse option allows to control whether the builder will recurse into
message fields. If set false, then the builder will only generate code for the
message field itself. The default is true and results in generating code for all
recursive fields as explained in the [overview](#overview).

NOTE: If the field uses an automatic type, then `recurse` defaults to `false`.

#### `FieldBuilderOptions.conversion` {#FieldBuilderOptions.conversion}

Allows to convert the function argument into the required field type, see
[conversion](#conversions).

#### `FieldBuilderOptions.predicate` {#FieldBuilderOptions.predicate}

Allows to apply a predicate before setting the function argument to the field,
see [predicates](#predicates).

#### `FieldBuilderOptions.data` {#FieldBuilderOptions.data}

Used to provide additional data to [conversions](#conversions) and
[predicates](#redicates).

#### `FieldBuilderOptions.macro` {#FieldBuilderOptions.macro}

This allows to reference the [`type_map`](#MessageBuilderOptions.type_map) of
the message annotation, an annotation of an outer message or from a
configuration file. If the field options have other entries than the `macro`
name, then those will override the values from the referenced configuration.

#### `FieldBuilderOptions.dependency` {#FieldBuilderOptions.dependency}

Specifies a rule that is necessary as dependency. This does not automatically
add those rules as dependencies but allows Proto Builder to check dependencies.

#### `FieldBuilderOptions.param` {#FieldBuilderOptions.param}

Only used in [templating](templates.md) for [builtin types](templates#Builtins)
and [custom types](templates#custom_vars). This adds the `%type"+param"`
template variable.

#### `FieldBuilderOptions.add_source_location` {#FieldBuilderOptions.add_source_location}

Adds an `absl::SourceLocation` parameter to the end of the setter function
signature. The parameter will be available as `@source_location@` to
[conversions](#conversions) and [predicates](#predicates).

#### `FieldBuilderOptions.automatic` {#FieldBuilderOptions.automatic}

Only relevant for configuration files, see
[automatic conversions](#automatic_conversion).

#### `FieldBuilderOptions.override` {#FieldBuilderOptions.override}

Only relevant for [inheritance](templates.md#inheritance).

#### Complete `FieldBuilderOptions`

For reference, please refer to https://google.github.io/cpp-proto-builder/proto_builder/proto_builder.proto class:FieldBuilderOptions

### Notes on `map` types

Handling of map types differs slightly from other field types.

*   Map fields are a special form of repeated fields with an implicit sub
    message consisting of exactly a key and a value field. Internally all
    operations are performed calling the fields `insert` method. Consequently
    proto builder uses the `Insert` prefix rather than Add or Set.

    ```
    map<int32, string> mymap = 1 [
      (proto_builder.field) = { output: BOTH },
      (proto_builder.field) = { output: FOREACH },
      (proto_builder.field) = { output: INITIALIZER_LIST }
    ];
    ```

    Will generate:

    ```
    Builder& InsertMymap(const ::proto2::Map<int32, std::string>::value_type& key_value_pair);

    template<class Container, class = typename std::enable_if<!std::is_convertible<Container, ::proto2::Map<int32, std::string>::value_type>::value>::type>
    Builder& InsertMymap(const Container& key_value_pairs) {
      data_.mutable_mymap()->insert(key_value_pairs.begin(), key_value_pairs.end());
      return *this;
    }

    Builder& InsertMymap(std::initializer_list<::proto2::Map<int32, std::string>::value_type> key_value_pairs) {
      data_.mutable_mymap()->insert(key_value_pairs.begin(), key_value_pairs.end());
      return *this;
    }
    ```

*   Map fields will use their `insert` functions even for modes `FOREACH` and
    `INITIALIZER_LIST` resulting in the first key to take precedence. In mode
    `FOREACH_ADD` the generated code uses a loop to call the generated `Insert`
    method for each separate value to allow for conversions (see below). This
    also applies for `FOREACH` and `INITIALIZER_LIST` if a conversion is in use.

    ```
    template <class Container, class = typename std::enable_if<!std::is_convertible<Container, ::proto2::Map<int32, std::string>::value_type>::value>::type>
    Builder& InsertMymap(const Container& key_value_pairs) {
      for (const auto& v : key_value_pairs) {
        InsertMymap(v);
      }
      return *this;
    }
    ```

*   Map fields use their key value type for their accessor methods. Since the
    proto compiler explicitly hides the entry type, we refer to the map entries
    using the `::proto2::Map<K, V>` template type, just like the proto compiler
    does.

*   Map types do not use a template for `INITIALIZER_LIST` initialization. This
    is because the `Item` type cannot be inferred since it is a compound type.
    As a consequence we intentionally apply the same restrictions as the proto
    compiler for the construction elements. If this is not wanted, then a
    conversion can be used to use `std::pair` instead of `::proto::Map`, see
    below.

*   Map types can use a conversion and it is advisable to use placeholders
    `@type@` and `@value@`. They make it possible to reference the key and value
    types via `@type@::first_type` and `@type@::second_type`, as well as their
    values through `@value@.first` and `@value@.second` respectively. The
    following example configuration uses this to automatically convert values of
    `std::pair` from a `absl::string_view` to their mapped sub message using
    `ParseTextOrDie`:

    ```
    type_map {
      key: "@TextProto:Map:Value:absl::string_view"
      value {
        type: "std::pair<@type@::first_type, absl::string_view>"
        conversion: "@type@(@value@.first, ::proto_builder::oss::ParseTextOrDie<@type@::second_type>(@value@.second))"
        include: "net/proto2/contrib/parse_proto/parse_text_proto.h"
      }
    }
    ```

    This way it is possible to apply such a map conversion even in mode
    `INITIALIZER_LIST`. Note that even with conversions the setters will still
    only have one parameter.

### Notes on `oneof` types

*   The main field name of a `oneof` field does not generate code. Instead each
    sub field will generate its own setter.

    ```
    oneof select {
      int32 one = 1;
      string two = 2;
    }
    ```

    Will generate:

    ```
    Builder& SetOne(int32 value);
    Builder& SetTwo(const std::string & value);
    ```

*   When multiple sub fields of a `oneof` field are set, then only the last set
    call sticks.

    Given the above example, the call `SetOne(1).SetTwo("two")` will result in
    only the field `two` being set and `SetOne(1)` getting ignored.

### Templating

The next level of configuration can be achieved by controlling default template
behavior, through the use of custom [templates](templates.md) or by applying
[inheritance](templates.md#inheritance).

### Conversions

As explained in [FieldBuilderOptions.type](#FieldBuilderOptions.type) the
setters can apply automatic conversions. This can be done on a one to one
mapping where each field receives exactly one value that has to be converted as
well as a single setter that handles a message field. It is further possible to
configure some types to have their conversion automatically applied.

#### Field conversions

Generally any function `FieldType Function(InputType)` can be used as an
automatically applied conversion function. Below we are introducing the new type
`SomeTypeName` that uses the `Convert` function template which will be
instantiated for the field type (expressed as `@type@`) to convert the setter's
input value (expressed as `@value@`).

```proto
type_map {
  key: "SomeTypeName"                       # Name of the custom conversion
  value {
    type: "InputType"                       # Name of the setter input type
    conversion: "Convert<@type@>(@value@)"  # The conversion call
    include: "path/to/my/header.h"          # Name(s) of required includes
    dependency: "//path/to/my:library"      # Name(s) of the required libraries
  }
}
```

This configuration can be used through field annotation as follows:

```proto
optional int64 field_name = 1 [
  (proto_builder.field) = {type: "SomeTypeName"}
]
```

That results in the following setter:

```c++
Builder& SetFieldName(const InputType& value) {
  data_.set_field_name(Convert<int64>(value));
  return *this;
}
```

> TIP: More complex invocations can be achieved through lambdas or by adding
> conversion functions in the custom template or a separate library. In order to
> use a lambda it needs to be specified as the conversion, after all, a lambda
> is just a function, but remember to actually call the lambda:
>
> ```proto
> type_map {
>  key: "SomeTypeName"
>  value {
>    type: "InputType"
>    conversion: "[=] { return Convert<@type@>(@value@); }()"
>  }
> }
> ```

#### Message field conversions

It is possible to apply conversions to a whole sub proto or in other words to a
message field. This works exactly the same as for simple field conversions. The
difference is that the conversion functions now return a proto type and that
applying the conversion replaces all fields of that proto, unless the current
state is taken into account, as described in the next section below.

#### Selected sub fields and read/write

Customizations that should only affect some subfields of a message field, or are
supposed to read the state of the message field first, require manual writing of
the implementation. Though their declaration in the header can still be
automated using [`FieldBuilderOptions.output`](#FieldBuilderOptions.output)`=
HEADER`.

However, to some degree you can apply the described technique of conversions in
order to create context dependent operations. Generally every setter has access
to: 1) the underlying proto data (e.g. `data_` by default), 2) the status, if
[`use_status`](#MessageBuilderOptions.use_status) is active, and 3) all other
member variables of the Builder.

```textproto
type_map {
  key: "SomeTypeName"
  value {
    type: "InputType"
    conversion: "Convert<@type@>(@value@, &data_, &status_)"
    include: "my_include_file_for_convert.h"
  }
}
```

Assuming the field is of type `int64`:

```proto
message My {
  option (proto_builder.message) = {
    use_status: true
  };

  optional int32 field_name = 1 [(proto_builder.field) = {
    type: "SomeTypeName"
  }];
}
```

This results in:

```c++
MyBuilder& SetFieldName(int32_t value) {
  data_.set_field_name(Convert<int32_t>(value, &data_, &status_));
  return *this;
}
```

So the expected API is:

```c++
template <class T>
T Convert(InputType value, ProtoType* data, absl::Status* status);
```

Now the function `Convert` has full access to the current field, the data and
the status. Given the full access to the underlying proto data, the function can
manipulate all fields, that includes sub fields and repeated fields.

#### Conversion Conditions {#conversion_conditions}

While Proto Builder has [predicate support](#predicates), the conversion can
also be used to conditionally set the respective field, since the conversion can
just return the existing value or the default field type value. The following
example uses `CheckValue` to test the input `value`, returning an
`absl::Status`. If the result is `absl::OkStatus()`, then `Convert` returns the
`value`, otherwise it returns the existing data.

```c++
template <class FieldType>
FieldType Convert(InputType value, ProtoType* data, absl::Status* status) {
  if (status->ok()) {    // Do not overwrite prior errors
    *status = CheckValue(value);
    if (status->ok()) {
      return FieldType(value);
    }
  }
  return data->value();  // Returning the current value means no change.
  // return FieldType();    Or just return the default value.
}
```

This also works with `use_status` disabled. In that case there just is no
status.

```c++
template <class FieldType>
FieldType Convert(InputType value, ProtoType* data) {
  if (CheckValue(value).ok()) {
    return FieldType(value);
  }
  return data->value();  // Returning the current value means no change.
  // return FieldType();    Or just return the default value.
}
```

The `conversion` field can be used to apply per field limits and other
restrictions. The following example applies min and max limits using a generic
conversion/check method demonstrated with an int32 field:

```proto
message My {
  optional int32 field_name = 1 [(proto_builder.field) = {
    conversion: "GetValueIfBetween<@type@>(@value@, 25, 42)"
  }];
}
```

This results in:

```c++
MyBuilder& SetFieldName(int32 value) {
  data_.set_field_name(GetValueIfBetween<int32_t>(value, 25, 42));
  return *this;
}
```

So `GetValueIfBetween` could be implemented as:

```c++
template <class T>
T GetValueIfBetween(T value, T min_value, T max_value) {
  return value >= min_value && value < max_value ? value : T();
}
```

NOTE: ProtoBuilder does not verify the generated code works correctly with
customizations and it is the users responsibility to ensure API correctness.

> TIP: The field annotations can have an `include` attribute. But it is probably
> not a good idea to repeat the same include for every field. The include can
> instead be specified as a message annotation:
>
> ```proto
> message My {
>   option (proto_builder.message) = {
>     source_include: "path/to/include.h"
>   };
>
>   optional int32 field_name = 1 [(proto_builder.field) = {
>     conversion: "GetValueIfBetween(@value@, 25, 42)"
>   }];
> }
> ```

#### Conversion data {#conversion_data} {#data}

Each field annotation can provide a map of replacements. This allows to pass per
field data to a conversion (or check) function. Each key in that map becomes
available as a `%` enclosed custom replacement (as opposed to builtin
replacements that use `@` enclosure).

Example:

```proto
message My {
  option (proto_builder.message) = {
    source_include: "path/to/include.h"
  };

  optional int64 number = 1 [(proto_builder.field) = {
    conversion: "GetValueIfBetween<@type@>(@value@, %min%, %max%)"
    data { key: "min" value: "25" }
    data { key: "max" value: "42" }
  }];
}
```

Alternatively if the conversion is needed for multiple fields, then the message
could contain the call semantics in its type_map and the fields can reference
that as a `macro`:

```proto
message My {
  option (proto_builder.message) = {
    source_include: "path/to/include.h"
    type_map {
      key: "GetValueIfBetween"
      value {
        conversion: "GetValueIfBetween<@type@>(@value@, %min%, %max%)"
      }
    }
  };

  optional int64 number = 1 [(proto_builder.field) = {
    macro: "GetValueIfBetween"
    data { key: "min" value: "25" }
    data { key: "max" value: "42" }
  }];
}
```

This translates into:

```c++
MyBuilder& SetNumberme(int32 value) {
  data_.set_number(GetValueIfBetween<int32_t>(value, 25, 42));
  return *this;
}
```

Builtin conversion variables:

| Variable            | Value                                         |
| ------------------- | --------------------------------------------- |
| `@default@`         | Default value of the field                    |
| `@type@`            | Type of the field                             |
| `@value@`           | Value passed to the setter                    |
| `@source_location@` | If                                            |
:                     : [`add_source_location`](#add_source_location) :
:                     : is active for the field, then the result of   :
:                     : `%SourceLocation%param` else                  :
:                     : `%SourceLocation%value`, see [template        :
:                     : builtins](templates#Builtins).                :

TIP: Conditions, Conversion data and `use_status` can be used together to
implement per field input validation, but Proto Builder also supports
[predicates](#predicates) and full proto [validation](validation.md).

#### Automatic conversion {#automatic_conversion}

Types marked as `automatic` will automatically be triggered and result in two
setters being generated. The first setter will take the field type as is and the
second setter will have the original type parameter replaced with their
configured `type`. This is useful if outside of proto data a sub-message has a
different type. In other words, if the sub-message proto is the serialization
format for another type and that other type should be used everywhere outside
serialization.

NOTE: Automatic types must start with '='.

WARNING: In the future automatic types will change `recurse` to default to
`false`. If recursion is necessary, then the `recurse` should be set `true`
explicitly.

NOTE: Automatic types are only automatically applied to fields that do not have
any custom `FieldBuilderOptions` set. However, automatic types can be added just
like any other via `FieldBuilderOptions` annotations.

The following example shows the configuration that automatically applies to all
`google::protobuf::Duration` fields. This configuration changes all fields of
that proto type from accepting that proto type to taking an `absl::Duration`
parameter that will be internally converted to `google::protobuf::Duration`.

```textproto
type_map {
  key: "=google::protobuf::Duration"
  value {
    type: "absl::Duration"
    automatic: true
    conversion: "::proto_builder::oss::ConvertToDuration(@value@)"
    include: "proto_builder/oss/proto_conversion_helpers.h"
    dependency: "//proto_builder/oss:proto_conversion_helpers_cc"
  }
}
```

Given the following `proto` file:

```proto
message My {
  optional google::protobuf::Duration duration = 1;
}
```

This creates the setters below:

```c++
MyBuilder& MyBuilder::SetDuration(const ::google::protobuf::Duration& value);
MyBuilder& MyBuilder::SetDuration(const absl::Duration& value);
```

#### Type replacement

In addition to automatic conversion it is possible to completely replace types
by providing a configuration that matches the type you want to replace. The
difference to type conversion is that type replacement does not generate a
setter for the original field type, so there will be only one setter. Also, type
replacement applies unconditionally.

Most notably this is done for the string field types:

```textproto
type_map {
  key: "bytes"
  value {
    type: "std::string"
    decorated_type: "const std::string&"
    include: "<string>"
  }
}
type_map {
  key: "string"
  value {
    type: "std::string"
    decorated_type: "const std::string&"
    include: "<string>"
  }
}
```

### Predicates

Predicate support allows for per-field validation as opposed to
[full message validation](validation.md). This works in many ways similar to
[Conversion Conditions](#conversion_conditions). However, as the name suggests
these are applied before any other operation in the setters on the raw setter
arguments. The predicates have full [data annotation support](#data) and can
interact with [status support](#MessageBuilderOptions.use_status). And while
they cannot convert the input, predicates can be used together with conditions.

The following example uses a function `IsBetween` as a predicate that is
provided by the specified `source_include`. This header could just be an
`extra_hdr` to the `cc_proto_builder_library` rule. For convenience the function
call semantics are defined in the `type_map` of message, so they do not have to
be repeated for every field. Consider the proto:

```proto
message Predicate {
  option (proto_builder.message) = {
    source_include: "path/to/helper/predicate_util.h"
    type_map {
      key: "IsBetween"
      value {
        predicate: "IsBetween<@type@>(@value@, %min%, %max%)"
      }
    }
  };

  optional int64 number = 1 [(proto_builder.field) = {
    macro: "IsBetween"
    data { key: "min" value: "25" }
    data { key: "max" value: "42" }
  }];
}
```

The `predicate` requires a function call that must return an `absl::Status`:

```c++
template <class T>
inline absl::Status IsBetween(T value, T min, T max) {
  if (min <= value && value < max) {
    return absl::OkStatus();
  } else {
    return absl::InvalidArgumentError("Value out of range");
  }
}
```

The generated setter will be:

```c++
PredicateBuilder& PredicateBuilder::SetNumber(int64_t value) {
  if (!IsBetween<int64_t>(value, 25, 42).ok()) {
    return *this;
  }
  data_.set_number(value);
  return *this;
}
```

If the message has `use_status` enabled, then the generated setter will be
slightly different:

```c++
PredicateBuilder& PredicateBuilder::SetNumber(int64_t value) {
  const auto status = IsBetween<int64_t>(value, 25, 42);
  if (!status.ok()) {
    if (status_.ok()) {
      UpdateStatus(status);
    }
    return *this;
  }
  data_.set_number(value);
  return *this;
}
```

In the non `use_status` case, the setter will not use the returned status other
than as the predicate decision, meaning any book keeping of errors has to be
done in the predicate itself.

With `use_status` the generated code will again first check the input value, but
now it will also save the status result. In case of a non `OkStatus`, the
function will then check the builder's status. Only if that has an `OkStatus`
will the setter call `UpdateStatus`. Effectively, the setter will only save the
error status for the first error and never override an error status.

This restriction is in place so that if two fields are being set and the first
set operation puts the builder into a non `OkStatus`, then the second field's
setting should not override that first error state.

IMPORTANT: Since predicates operate on a per field basis, setting a sub message
requires all its fields and sub fields to be checked. If the sub field is being
set from a builder, then that sub builder should have applied all predicates and
the resulting builder will be in the correct state. However, if the sub message
field gets set from a conversion, then the conversion needs to deal with
potential field predicate violations. Further, if the sub message field gets set
via a plain proto, then no predicates are being applied by default and it is
necessary to provide a predicate that handles all sub fields at once. Using
[validation](validation.md) on the other hand evaluates the whole message before
accessing it, so the sub message field dilemma never applies.

Since the builder's member variable `status_` can be passed to the predicate, it
is possible to implement more elaborate state handling.

> NOTE: The predicate could very well be an immediately invoked lambda:
>
> ```proto
> message Predicate {
> optional int64 number = 1 [(proto_builder.field) = {
>     predicate: "[=]{ return 25 <= @value@ && @value@ < %max%"
>                "         ? absl::OkStatus()"
>                "         : absl::InvalidArgument(\"Value out of range\"); }()"
>     }];
> }
> ```
>
> Notice that the lambda is followed by `()` which means that the lambda will be
> invoked immediately instead of returning the lambda.

NOTE: While predicates offer a very simple and concise way to implement field
data validation, they will only ever work for the builder. If
[validation](validation.md) is needed in other languages or outside of the
builder then other approaches are better.
