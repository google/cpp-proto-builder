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

#include <filesystem>
#include <memory>

#include "google/protobuf/compiler/importer.h"
#include "proto_builder/oss/file.h"
#include "proto_builder/oss/sourcefile_database.h"
#include "absl/flags/flag.h"
#include "absl/memory/memory.h"
#include "absl/strings/substitute.h"

ABSL_FLAG(std::string, protofiles, "",
          ".proto files to load into the default SourceFileDatabase");

ABSL_FLAG(std::string, proto_paths, "",
          "Comma-separate list of paths to search for proto files. The CWD is "
          "always searched first.");

ABSL_FLAG(bool, use_global_db, false, "DO NOT USE.");

namespace proto_builder::oss {

using ::google::protobuf::compiler::DiskSourceTree;
using ::google::protobuf::compiler::Importer;
using ::google::protobuf::compiler::MultiFileErrorCollector;

static auto& sourcefile_database_list =
    *new std::vector<std::unique_ptr<SourceFileDatabase>>;

class SilentErrorCollector : public MultiFileErrorCollector {
 public:
  SilentErrorCollector() = default;
  ~SilentErrorCollector() override = default;

  // implements ErrorCollector ---------------------------------------
  void AddError(const std::string& filename, int line, int column,
                const std::string& message) override {
    errors_.emplace_back(
        absl::Substitute("$0:$1:$2: $3\n", filename, line, column, message));
  }

  std::vector<std::string> errors() const { return errors_; }

 private:
  std::vector<std::string> errors_;
};

class SourceFileDatabaseImpl : public SourceFileDatabase {
 public:
  SourceFileDatabaseImpl(const std::vector<std::string>& proto_files,
                         const std::vector<std::string>& proto_paths);
  ~SourceFileDatabaseImpl() override = default;

  SourceFileDatabaseImpl(SourceFileDatabaseImpl&&) = default;
  SourceFileDatabaseImpl& operator=(SourceFileDatabaseImpl&&) = default;

  const ::google::protobuf::DescriptorPool* pool() const override { return importer_->pool(); }

  bool LoadedSuccessfully() const override { return loaded_successfully_; }
  std::vector<std::string> GetErrors() const override {
    // We hid exactly what MultiFileErrorCollector implementation we use from
    // the header, so now we have to cast.
    return static_cast<const SilentErrorCollector*>(error_collector_.get())
        ->errors();
  }

 private:
  std::unique_ptr<DiskSourceTree> source_tree_;
  std::unique_ptr<MultiFileErrorCollector> error_collector_;
  std::unique_ptr<Importer> importer_;
  bool loaded_successfully_;
};

/*static*/
std::unique_ptr<SourceFileDatabase> SourceFileDatabase::New(
    const std::vector<std::string>& proto_files,
    const std::vector<std::string>& proto_paths) {
  // files are readable
  for (absl::string_view proto_file : proto_files) {
    if (!file::oss::Readable(proto_file).ok()) {
      return nullptr;
    }
  }
  return std::make_unique<SourceFileDatabaseImpl>(proto_files, proto_paths);
}

std::vector<std::string> SourceFileDatabase::GetProtoFilesFlag() {
  return absl::StrSplit(absl::GetFlag(FLAGS_protofiles), ',',
                        absl::SkipEmpty());
}

std::vector<std::string> SourceFileDatabase::GetProtoPathsFlag() {
  return absl::StrSplit(absl::GetFlag(FLAGS_proto_paths), ',',
                        absl::SkipEmpty());
}

SourceFileDatabaseImpl::SourceFileDatabaseImpl(
    const std::vector<std::string>& proto_files,
    const std::vector<std::string>& proto_paths)
    : source_tree_(std::make_unique<DiskSourceTree>()),
      error_collector_(std::make_unique<SilentErrorCollector>()),
      importer_(std::make_unique<Importer>(source_tree_.get(),
                                           error_collector_.get())),
      loaded_successfully_(true) {
  std::string root_path = std::filesystem::current_path().root_path().string();
  source_tree_->MapPath("", ".");
  for (const std::string& path : proto_paths) {
    source_tree_->MapPath("", path);
  }
  source_tree_->MapPath(root_path, root_path);
  for (const std::string& proto_file : proto_files) {
    if (importer_->Import(proto_file) == nullptr) {
      loaded_successfully_ = false;
      break;
    }
  }
}

}  // namespace proto_builder::oss
