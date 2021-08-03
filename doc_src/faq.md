# ProtoBuilder FAQ

https://google.github.io/cpp-proto-builder/usage

[TOC]

## FAQ

### Can generated builers inherit some other class?

Yes. You need to provide a custom [template](templates.md).

```txt
{{#BUILDER}}
class {{CLASS_NAME}}
    : public MyBase, public MyInterface, public CRTP<{{CLASS_NAME}}> {
 public:
 //...
};
```

### Can builders be inherited?

Yes. They are normal classes. But it is a bit more complicated and you might
want to read about [inheritance](templates.md#inheritance).

### Can I use a custom textproto config file?

Yes. The best way is to copy the default configuration and then add your
customization. Ideally this is done in a build rule that concatenates the
default and the custom config.

### Why does ProtoBuilder not produce the Java style builders?

You can actually configure ProtoBuilder to do just that. In the
`cc_proto_builder_library` rule set `proto = "**"` and `max_field_depth = 1`.
But you really should not, because it is not necessary and leads to less
efficient and harder to read code. The reason this is more efficient is because
less data has to be copied and easier to read because you see field lines that
can be grouped into their message structure as opposed to having to deal with
separate sub message builder with more boilerplate code.

### Why does ProtoBuilder use flat builders by default?

Because they are more efficient and easier to read.

### Why are my builders broken after I removed a field in my proto?

The builders are just fine, but removing a field results in removing generated
setters. In the same way direct users of your protos that use the removed fields
would be broken. However, you could use a custom template and add the missing
setters back. They can then be marked as deprecated and which should make it
easier to drop them. Meanwhile the proto file can already drop the fields.

### Are builders that use validators compatible with those that are not?

Yes. The default templates are written in a way that you can always pass the
builder to a setter of another builder and all builders have a conversion
operator, so there is no need to explicitly call `Build` or `Consume` which are
not available for all configurations. However, passing a builder with validation
support into one that does not means you lose the error information. If the
passed-in builder has an error state, then it will pass down the default
instance, thus losing all data and state. If the receiving builder has at least
`use_status` set, then it will receive error or data.

### Why are there so many configurations, can I ignore them?

Yes. Builders work perfectly fine out of the box. But we do support some crazy
use cases.

### Why are there not more built-in types?

We were lazy but you are open to suggest them. In the meantime you can just add
types you need to your custom configuration.

### Do I really need C++17 for my ProtoBuilder project?

Yes. The minimum is C++17 and all major C++17 ready compilers should work. The
generated code relies on Google's Abseil library, which may probably should be
at the same level as ProtoBuilder itself and thus you should still use C++17.

### Why do my builders not have a Build method?

The generated builders by default have an implicit conversion operator because
they are meant to be used in place of the protos they hold. Also, the generated
builders always have the proto data available, so there is nothing to build
(very different from Java). However, if you use [validation](validation.md), or
annotate the proto message with
[`use_build`](config.md#MessageBuilderOptions.use_build), then your builders
will have a `absl::StatusOr<ProtoType> Build() const` method which will validate
the proto data before before returning it.

### Should I customize the code templates?

While customizing [templates](templates.md) is fully supported and documented it
should first be considered whether message and field annotations provide enough
customization options. Under [conversion](config.md#conversion) there are plenty
of ways described that allow all kinds of customization. However, these can not
add methods and data members, so those are a prime reason to use custom code
templates. But, it might be better to inherit a builder generated from the
default templates with message as well as field annotations applied. The big
advantage is that this way, the builders are compatible with all other builders.
