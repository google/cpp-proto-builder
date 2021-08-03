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

#include "proto_builder/oss/qcheck.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::oss {
namespace {

using ::testing::HasSubstr;

class CoutRedirect {
 public:
  explicit CoutRedirect(std::streambuf* buffer)
      : my_buffer_(std::cout.rdbuf(buffer)) {}

  ~CoutRedirect() { std::cout.rdbuf(my_buffer_); }

 private:
  std::streambuf* my_buffer_;
};

class CerrRedirect {
 public:
  explicit CerrRedirect(std::streambuf* buffer)
      : my_buffer_(std::cerr.rdbuf(buffer)) {}

  ~CerrRedirect() { std::cerr.rdbuf(my_buffer_); }

 private:
  std::streambuf* my_buffer_;
};

TEST(QCheckTest, NoError) {
  QCheck(true, "true") << "Ok!";
  EXPECT_TRUE(true);
}

TEST(QCheckTest, ErrorWithAbort) {
  EXPECT_DEATH(QCheck(false, "false") << "This is an error",
               HasSubstr("This is an error"));
}

TEST(QCheckTest, ErrorWithoutAbort) {
  QCheck(false, "false",
         QCheckOptions::kDefault & ~QCheckOptions::kAbortOnError)
      << "This is an error";
  EXPECT_TRUE(true);
}

TEST(QCheckTest, ErrorWithoutAbortOnCerr) {
  std::stringstream cerr_buffer;
  CerrRedirect cerr_redirect(cerr_buffer.rdbuf());
  QCheck(false, "false",
         QCheckOptions::kDefault & ~QCheckOptions::kAbortOnError)
      << "This is an error";
  EXPECT_THAT(cerr_buffer.str(), HasSubstr("This is an error"));
}

TEST(QCheckTest, ErrorWithoutAbortOnCout) {
  std::stringstream cout_buffer;
  CoutRedirect cout_redirect(cout_buffer.rdbuf());
  QCheck(false, "false",
         QCheckOptions::kDefault & ~QCheckOptions::kAbortOnError &
             ~QCheckOptions::kPrintToStandardError)
      << "This is an error";
  EXPECT_THAT(cout_buffer.str(), HasSubstr("This is an error"));
}

}  // namespace
}  // namespace proto_builder::oss
