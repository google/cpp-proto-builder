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

#ifndef PROTO_BUILDER_OSS_SOURCEFILE_DATABASE_H_
#define PROTO_BUILDER_OSS_SOURCEFILE_DATABASE_H_

#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/descriptor.h"

namespace proto_builder::oss {

// Class to interface with protocol buffer metadata from files.
class SourceFileDatabase {
 public:
  // Create a new protocol database. proto_files should be a list of additional
  // files to load; these may be either relative paths to any proto root, or
  // absolute paths. proto_paths is an ordered list of the roots of the proto
  // directory; "." is always implicit.
  //
  // Returns pointer to newly created instance.
  static std::unique_ptr<SourceFileDatabase> New(
      const std::vector<std::string>& proto_files,
      const std::vector<std::string>& proto_paths);

  // Returns `--protofiles` as a vector.
  static std::vector<std::string> GetProtoFilesFlag();

  // Returns `--proto_paths` as a vector.
  static std::vector<std::string> GetProtoPathsFlag();

  virtual ~SourceFileDatabase() = default;

  virtual const ::google::protobuf::DescriptorPool* pool() const = 0;
  virtual bool LoadedSuccessfully() const = 0;
  virtual std::vector<std::string> GetErrors() const = 0;

 protected:
  // The class must be overridden and may not be instantiated.
  SourceFileDatabase() = default;
};

}  // namespace proto_builder::oss

#endif  // PROTO_BUILDER_OSS_SOURCEFILE_DATABASE_H_
