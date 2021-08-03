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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <string>

#include "proto_builder/oss/file.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"

namespace file {
namespace oss {

absl::Status SetContents(absl::string_view file_name, absl::string_view content,
                         const Options& options) {
  // Use POSIX C APIs instead of C++ iostreams to avoid exceptions.
  int fd = open(std::string(file_name).c_str(),
                O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0664);
  if (fd == -1) {
    return absl::UnknownError(
        absl::StrFormat("Unable to open file: '%s'", file_name));
  }

  int64_t written = 0;
  while (written < static_cast<int64_t>(content.size())) {
    ssize_t n = write(fd, content.data() + written, content.size() - written);
    if (n < 0) {
      if (errno == EAGAIN) {
        continue;
      }
      close(fd);
      return absl::UnknownError(
          absl::StrFormat("Unable to write to file: '%s'", file_name));
    }
    written += n;
  }
  if (close(fd) != 0) {
    return absl::UnknownError(
        absl::StrFormat("Unable to close file: '%s'", file_name));
  }
  return absl::OkStatus();
}

absl::Status Readable(absl::string_view file_name, const Options& options) {
  if (!std::filesystem::exists(file_name)) {
    return absl::NotFoundError(
        absl::StrFormat("File does not exist: '%s'.", file_name));
  }
  if (std::filesystem::is_directory(file_name)) {
    return absl::FailedPreconditionError(
        absl::StrFormat("Open failed: Is a directory: '%s'", file_name));
  }
  // Use POSIX C APIs instead of C++ iostreams to avoid exceptions.
  std::string str_file_name(file_name);  // need zero termination
  int fd = open(str_file_name.c_str(), O_RDONLY);
  if (!fd) {
    return absl::FailedPreconditionError(
        absl::StrFormat("Unable to read file: '%s'", file_name));
  }
  if (close(fd) != 0) {
    return absl::UnknownError(
        absl::StrFormat("Unable to close file: '%s'", file_name));
  }
  return absl::OkStatus();
}

absl::Status GetContents(absl::string_view file_name, std::string* output,
                         const Options& options) {
  std::string str_file_name(file_name);  // need zero termination
  // Use POSIX C APIs instead of C++ iostreams to avoid exceptions.
  int fd = open(str_file_name.c_str(), O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
    return absl::UnknownError(
        absl::StrFormat("Unable to open file: '%s'", file_name));
  }
  char buf[4096];
  output->clear();
  while (ssize_t n = read(fd, buf, sizeof(buf))) {
    if (n < 0) {
      if (errno == EAGAIN) {
        continue;
      }
      close(fd);
      return absl::UnknownError(
          absl::StrFormat("Unable to read from file: '%s'", file_name));
    }
    output->append(buf, n);
  }
  if (close(fd) != 0) {
    return absl::UnknownError(
        absl::StrFormat("Unable to close file: '%s'", file_name));
  }
  return absl::OkStatus();
}

bool IsAbsolutePath(absl::string_view path) {
  std::filesystem::path fs_path = path;
  return fs_path.is_absolute();
}

}  // namespace oss
}  // namespace file
