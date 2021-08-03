// Copyright 2021 The CPP Proto Builder Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// READ: https://google.github.io/cpp-proto-builder

#ifndef PROTO_BUILDER_MESSAGE_BUILDER_H_
#define PROTO_BUILDER_MESSAGE_BUILDER_H_

#include <memory>
#include <string>
#include <tuple>

#include "proto_builder/field_builder.h"
#include "proto_builder/proto_builder.pb.h"
#include "google/protobuf/descriptor.h"
#include "absl/container/flat_hash_set.h"

namespace proto_builder {

using ::google::protobuf::Descriptor;
using ::google::protobuf::FieldDescriptor;

std::pair<std::string, std::string> GetPackageAndClassName(
    const ::google::protobuf::Descriptor* descriptor);

// Class to write code for a single protocol message type.
// READ: https://google.github.io/cpp-proto-builder#MessageBuilder
class MessageBuilder {
 public:
  struct Options {
    const ProtoBuilderConfigManager& config;
    BuilderWriter* writer;         // The writer will not be owned
    const ::google::protobuf::Descriptor& descriptor;  // Target message for the builder
    size_t max_field_depth;        // Maximum message depth (1 = this only)
    bool use_validator = false;    // Whether to generate Validator code
    bool make_interface = false;   // Whether to make an Interface
  };

  explicit MessageBuilder(Options options);

  void WriteBuilder();

  const ::google::protobuf::Descriptor& root_descriptor() const { return root_descriptor_; }
  const MessageBuilderOptions& root_options() const { return root_options_; }
  const std::string& class_name() const { return class_name_; }

 private:
  FieldData MakeFieldData(const FieldBuilderOptions& options,
                          const FieldDescriptor& field_descriptor,
                          const std::string& data_parent,
                          const std::string& name_parent,
                          bool first_method = false) const;

  void WriteMethod(const FieldData& field_data);

  bool WriteField(const FieldDescriptor& field_descriptor,
                  const std::string& data_parent,
                  const std::string& name_parent);

  void WriteMessage(const ::google::protobuf::Descriptor& descriptor,
                    const std::string& data_parent,
                    const std::string& name_parent, int depth);

  // Tracks the set of messages that have been used in a sub-field setter in
  // the current WriteMessage(...) recursive call stack.
  absl::flat_hash_set<const ::google::protobuf::Descriptor*> messages_in_subfield_setter_stack_;

  const Options options_;
  const std::unique_ptr<BuilderWriter> writer_;
  const ::google::protobuf::Descriptor& root_descriptor_;
  const MessageBuilderOptions root_options_;
  const std::string class_name_;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_MESSAGE_BUILDER_H_
