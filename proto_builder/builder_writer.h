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

#ifndef PROTO_BUILDER_BUILDER_WRITER_H_
#define PROTO_BUILDER_BUILDER_WRITER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "proto_builder/oss/logging.h"
#include "google/protobuf/descriptor.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"

namespace proto_builder {

using ::google::protobuf::Descriptor;
using ::google::protobuf::EnumDescriptor;
using ::google::protobuf::FileDescriptor;

// READ: https://google.github.io/cpp-proto-builder#Where
enum Where {
  HEADER = 0,     // Target is the header file (.h).
  SOURCE = 1,     // Target contains the function bodies/implementation (.cc).
  INTERFACE = 2,  // Target is interface header file (.interface.h).
};

// Type Where is an internal type and we do not need it to be in a .proto, but
// we do want the name here, so we simulate what the proto compiler would do.
std::string Where_Name(Where where);

// Formats an include for use in an #include line.
std::string FormatInclude(absl::string_view include);

// READ: https://google.github.io/cpp-proto-builder#CodeInfoCollector
// Class that collects information about the generated code.
// This is owned by the innermost BufferWriter, see below.
class CodeInfoCollector {
 public:
  CodeInfoCollector();
  explicit CodeInfoCollector(const std::vector<std::string>& package_path);

  virtual ~CodeInfoCollector() = default;

  // Add 'include' to be used with 'where'.
  // For system includes keep '<' and '>'.
  // For other includes the '"'s will be added as needed.
  // It is possible to add comments (e.g. // IWYU pragma: export) after '>'/'"'.
  virtual void AddInclude(Where where, absl::string_view include);

  // Adds an include for the message type in 'descriptor'.
  // This will always add '// IWYU pragma: export'.
  // The descriptor type must be one of {Descriptor, EnumDescriptor} or any
  // other ::google::protobuf::Descriptor type that had {string fullname(), ::google::protobuf::FileDescriptor* file()}.
  void AddInclude(Where where, const ::google::protobuf::Descriptor& descriptor);
  void AddInclude(Where where, const EnumDescriptor& descriptor);

  // Return all includes for 'where'. This can be called any time, even if no
  // includes have been added (independently of the constructor).
  const std::set<std::string>& GetIncludes(Where where) const;

  // If type is in package_path, then return the relative type name, otherwise
  // the original.
  std::string RelativeType(const std::string& cpp_type) const;

 private:
  CodeInfoCollector(const CodeInfoCollector&) = default;

  void AddInclude(Where where, const std::string& full_name,
                  const ::google::protobuf::FileDescriptor* file);

  const std::vector<std::string> package_path_;
  const std::string namespace_path_;
  std::map<Where, std::set<std::string>> include_map_;
};

// READ: https://google.github.io/cpp-proto-builder#BuilderWriter
// Interface used to write code lines for either HEADER or SOURCE.
class BuilderWriter {
 public:
  virtual ~BuilderWriter() = default;

  // Writes a single line of code. Note that this interface is not supposed
  // to handle new-lines. The receiver will add those after every single call.
  // If a genertor wants to write new line characters then it needs to wrap its
  // writer with the SplitLinesWriter decorator.
  virtual void Write(Where to, const std::string& code) = 0;

  virtual CodeInfoCollector* CodeInfo() = 0;
  virtual const CodeInfoCollector* CodeInfo() const = 0;
};

// READ: https://google.github.io/cpp-proto-builder#BufferWriter
// BuilderWriter that buffers all writes into per target vectors.
class BufferWriter : public BuilderWriter {
 public:
  // Ensure all targets (HEADER, SOURCE) are present and empty.
  explicit BufferWriter(const std::vector<std::string>& package_path = {});

  void Write(Where to, const std::string& line) override;

  // Access to given target as vector.
  const std::vector<std::string>& From(Where from) const {
    return buffer_.at(from);
  }

  // Write the contents 'from' to 'filename'.
  absl::Status WriteFile(Where from, const std::string& filename) const;

  // Move contents 'from' 'to_writer'. Target 'from' will be empty afterwards.
  void MoveContents(Where from, BufferWriter* to_writer);

  // Access to the CodeInfoCollector instance.
  CodeInfoCollector* CodeInfo() final;
  const CodeInfoCollector* CodeInfo() const final;

 private:
  std::map<Where, std::vector<std::string>> buffer_;
  CodeInfoCollector code_info_;
};

// READ: https://google.github.io/cpp-proto-builder#WrappingBuilderWriter
// Base class for all wrapping writers.
// This simplifies management of the wrapped BuilderWriter and automatically
// provides forwarding to the wrapped BuilderWriter's CodeInfoCollector.
class WrappingBuilderWriter : public BuilderWriter {
 public:
  explicit WrappingBuilderWriter(BuilderWriter* wrapped_writer)
      : wrapped_writer_(PBCC_DIE_IF_NULL(wrapped_writer)) {}

  CodeInfoCollector* CodeInfo() final;
  const CodeInfoCollector* CodeInfo() const final;

 protected:
  void WrappedWrite(Where where, const std::string& line);

 private:
  BuilderWriter* const wrapped_writer_;
};

// READ: https://google.github.io/cpp-proto-builder#NoDoubleEmptyLineWriter
// Prevents first and any two consecutive lines from being empty.
class NoDoubleEmptyLineWriter : public WrappingBuilderWriter {
 public:
  using WrappingBuilderWriter::WrappingBuilderWriter;

  void Write(Where to, const std::string& line) override;

 private:
  std::map<Where, bool> last_non_empty_;  // Internal tracking of empty lines.
};

// READ: https://google.github.io/cpp-proto-builder#IndentWriter
// Indents all non empty lines.
class IndentWriter : public WrappingBuilderWriter {
 public:
  explicit IndentWriter(BuilderWriter* wrapped_writer,
                        const std::string& head_indent = "",
                        const std::string& body_indent = "");

  void Write(Where to, const std::string& line) override;

  // Set 'indent' to prefix all lines written for 'to'.
  void SetIndent(Where to, absl::string_view indent);

 private:
  std::map<Where, std::string> indent_;
};

// READ: https://google.github.io/cpp-proto-builder#OwnWrappedWriter
// Changes a 'WrappingWriter' to own its 'wrapped_writer'.
// It is assumed that WrappingWriter is a sub class of WrappingBuilderWriter.
template <class WrappingWriter>
class OwnWrappedWriter : public WrappingWriter {
 public:
  template <class... Args>
  static std::unique_ptr<BuilderWriter> New(
      std::unique_ptr<BuilderWriter> wrapped_writer, Args... args) {
    return absl::WrapUnique<BuilderWriter>(
        new OwnWrappedWriter(std::move(wrapped_writer), args...));
  }

  template <class... Args>
  explicit OwnWrappedWriter(std::unique_ptr<BuilderWriter> wrapped_writer,
                            Args... args)
      : WrappingWriter(wrapped_writer.get(), args...),
        wrapped_writer_(std::move(wrapped_writer)) {}

 private:
  const std::unique_ptr<BuilderWriter> wrapped_writer_;
};

}  // namespace proto_builder

#endif  // PROTO_BUILDER_BUILDER_WRITER_H_
