# ProtoBuilder {#top}

https://google.github.io/cpp-proto-builder

<!--#include file="inc/index_oss.md" -->

## Overview

ProtoBuilder is a highly configurable code generator that takes a
[proto2](https://developers.google.com/protocol-buffers/docs/reference/proto2-spec) (or proto 3) protocol
[`Descriptor`](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.descriptor#Descriptor) (or name) and produces a
C++ [Builder pattern](http://en.wikipedia.org/wiki/Builder_pattern) (aka
Speaking or Fluent API) for the identified protocol message type.

The tool allows automatic type conversion, thus for instance allowing to set
field values that express time with time APIs such as `absl::Time` or
`absl::Duration`. It is further possible to fully customize the generated
classes using a template system.

The tool offers [validation support](validation.md).

Since the generated code is automatically created through build rules, the user
does not have to worry about the implementation and protocol buffer flexibility
is fully retained. Further, the generated code is based on a
[customizable code templates](templates.md) which allow arbitrary extensions.

NOTE: By default the tool generates flattened builders with separate builders
for only messages that are used as repeated fields. This default was chosen
because it is more efficient to use and easier to read.

TIP: Check out [quick start](quick_start.md) for a step by step introduction.

[TOC]

Consider:

```proto
message Foo {
  optional bool bar = 1;
  message Baz {
    optional bool sub = 1;
    repeated bool sub_repeated = 2;
  }
  optional Baz baz = 2;
  repeated Baz baz_repeated = 3;
}
```

By default this will give you the following public builder interface:

```c++
class FooBuilder {
 public:
  FooBuilder();
  FooBuilder(const Foo&);
  FooBuilder(Foo&&);

  operator const Foo&() const;

  FooBuilder& SetBar(bool value);
  FooBuilder& SetBaz(const Baz& value);
  FooBuilder& SetBazSub(bool value);
  FooBuilder& AddBazSubRepeated(bool value);
  FooBuilder& AddBazRepeated(const Foo::Baz& value);
};

class Foo_BazBuilder {
  Foo_BazBuilder();
  Foo_BazBuilder(const Foo_Baz&);
  Foo_BazBuilder(FooBaz&&);

  operator const Foo_Baz&() const;

  Foo_BazBuilder& SetSub(bool value);
  Foo_BazBuilder& AddSubRepeated(bool value);
};
```

The [`MessageBuilder`](#MessageBuilder) is highly configurable. Its defaults
simplify handling of small protocol messages to generate code for all transitive
leaf fields and all repeated message fields. In the example above that means the
fields `bar`, `baz`, `baz.sub`, `baz.sub_repeated` and `baz_repeated` (but not
its sub message fields `baz_repeated.sub` and `baz_repeated.sub_repeated`) as
follows below. For each field the header code is shown first and the source code
is second.

*   For field 'bar' the builder will produce:

    ```c++
    FooBuilder& SetBar(bool value);
    ```

    ```c++
    FooBuilder& FooBuilder::SetBar(bool value) {
      data_.set_bar(value);
      return *this;
    }
    ```

*   For field 'baz' the builder will create a new message setter:

    ```c++
    FooBuilder& SetBaz(const Baz& value);
    ```

    ```c++
    FooBuilder& FooBuilder::SetBaz(const Baz& value) {
      *data_.mutable_baz() = value;
      return *this;
    }
    ```

*   For field 'baz' each subfield will create a new setter recursively:

    ```c++
    FooBuilder& SetBazSub(bool value);
    ```

    ```c++
    FooBuilder& FooBuilder::SetBazSub(bool value) {
      data_.mutable_baz()->set_sub(value);
      return *this;
    }
    ```

    NOTE: This will not be present if message recursion is disabled.

*   For repeated fields the generated code changes from 'Set' to 'Add'
    semantics:

    ```c++
    FooBuilder& AddBazSubRepeated(bool value);
    ```

    ```c++
    FooBuilder& FooBuilder::AddBazSubRepeated(bool value) {
      data_.mutable_baz()->add_sub_repeated(value);
      return *this;
    }
    ```

    NOTE: This will not be present if message recursion is disabled.

*   However the repeated field 'baz_repeated' cannot be created with a simple
    value setter but instead it will take Baz messages:

    ```c++
    FooBuilder& AddBazRepeated(const Foo::Baz& value);
    ```

    ```c++
    FooBuilder& FooBuilder::AddBazRepeated(const Foo::Baz& value) {
      *data_.add_baz_repeated() = value;
      return *this;
    }
    ```

Thus if the field `baz_repeated` should be handled via a builder, that builder
needs to be created with a separate [`MessageBuilder`](#MessageBuilder). For
example, if you would like to create a builder for `Foo` and sub-message
`Foo.Baz`, you would declare both messages in the `proto` field of the
[`cc_proto_builder_library`](#cc_proto_builder_library) rules explicitly.

Message types that are never used in a repeated field (i.e., only in an
optional) do not require handling with a separate
[`MessageBuilder`](#MessageBuilder) because you can set their inner fields
directly.

Note that the default recursion behavior can be disabled, so that only setters
for the messages of message fields will be generated.

With the generated code one can set values of the messages in a single C++
expression, rather than multiple separate calls. As a consequence the Builder
pattern is an excellent choice for passing data around, especially to
constructors. Compare the following code that uses a `Foo` to construct `User
user`:

```c++
Foo foo;
foo.set_bar(true);
Foo::Baz* baz = foo.mutable_baz();
baz->set_sub(true);
baz->add_sub_repeated(true);
const User user(foo);
```

With the [Builder pattern](http://en.wikipedia.org/wiki/Builder_pattern), the
construction of `Foo` becomes a parameter. This means the parameter gets
constructed where it needs to and it does not require temporary variables. That
alone is an improvement, since it is obvious what is actually being constructed.

```c++
const User user =
    FooBuilder().SetBar(true).SetBazSub(true).AddBazSubRepeated(true);
```

The above relies on the default [code templates](templates.md) that creates
access to the wrapped type through a conversion operator:

```c++
operator const ::{{PROTO_TYPE}}& () const { return data_; }
```

The builder has [build system integration](usage.md#cc_proto_builder_library),
to fully automate the code generation.

Because sub message `Foo::Baz` is used as a repeated field we also have get the
`Foo_Baz` builder. But any sub message that does not get used as a repeated or
map field will not get a builder by default. Instead its fields will simply be
available in the flattened main builder. The exact messages that result in
builders can be configured, see [finding messages](usage.md#finding_messages).

## State and planned development

ProtoBuilder can generate setter code for all message types and fields. It can
generate both .h and .cc code. It comes with a build rule that simplifies code
generation [`cc_proto_builder_library`](usage.md#cc_proto_builder_library).

Future work:

*   A sub builder `bar` for message `foo` becomes `Foo_Bar` but should also be
    available as `Foo::Bar` to match proto class layout.
*   Add ability to specify base classes including CRTP support.
*   Add a type_map to `MessageBuilderOptions` so that messages can specify their
    own configuration without the need to provide an external config file. Right
    now custom configurations can be provided via the `proto_builder_config`
    rule attribute.
