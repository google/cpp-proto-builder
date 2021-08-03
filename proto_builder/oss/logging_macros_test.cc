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

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"
#include "absl/status/status.h"

// Last include to override Macros
#include "proto_builder/oss/logging_macros.h"

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

TEST(LoggingTest, CHECK_OK) {
  absl::Status status;
  status = absl::OkStatus();
  CHECK_OK(status);
  EXPECT_TRUE(true);
}

TEST(LoggingTest, CHECK) {
  CHECK(true);
  EXPECT_DEATH(CHECK(false) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, CHECK_EQ) {
  CHECK_EQ(2, 2);
  EXPECT_DEATH(CHECK_EQ(3, 2) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, CHECK_GT) {
  CHECK_GT(3, 2);
  EXPECT_DEATH(CHECK_GT(2, 2) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, CHECK_GE) {
  CHECK_GE(2, 2);
  EXPECT_DEATH(CHECK_GE(1, 2) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, QCHECK) {
  QCHECK(true) << "Ok!";
  EXPECT_DEATH(QCHECK(false) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, QCHECK_OK) {
  absl::Status status;
  status = absl::OkStatus();
  QCHECK_OK(status) << "Ok!";
  absl::Status bad_status;
  bad_status = absl::InternalError("Error!");
  EXPECT_DEATH(QCHECK_OK(bad_status) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, QCHECK_CMP) {
  QCHECK_CMP(2, 3, <) << "Ok!";
  EXPECT_DEATH(QCHECK_CMP(3, 2, <) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, QCHECK_EQ) {
  QCHECK_EQ(42, 42) << "Ok!";
  EXPECT_DEATH(QCHECK_EQ(4, 2) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, QCHECK_GE) {
  QCHECK_GE(4, 2) << "Ok!";
  EXPECT_DEATH(QCHECK_GE(1, 2) << "This is an error",
               HasSubstr("This is an error"));
}

TEST(LoggingTest, LOG_IF_INFO) {
  std::stringstream cout_buffer;
  CoutRedirect cout_redirect(cout_buffer.rdbuf());
  LOG_IF(INFO, false) << "Ok!";
  LOG_IF(INFO, true) << "This is an error";
  EXPECT_THAT(cout_buffer.str(), Not(HasSubstr("Ok!")));
  EXPECT_THAT(cout_buffer.str(), HasSubstr("This is an error"));
}

TEST(LoggingTest, LOG_IF_WARNING) {
  std::stringstream cout_buffer;
  CoutRedirect cout_redirect(cout_buffer.rdbuf());
  LOG_IF(WARNING, false) << "Ok!";
  LOG_IF(WARNING, true) << "This is an error";
  EXPECT_THAT(cout_buffer.str(), Not(HasSubstr("Ok!")));
  EXPECT_THAT(cout_buffer.str(), HasSubstr("This is an error"));
}

TEST(LoggingTest, LOG_IF_ERROR) {
  std::stringstream cerr_buffer;
  CerrRedirect cerr_redirect(cerr_buffer.rdbuf());
  LOG_IF(ERROR, false) << "Ok!";
  LOG_IF(ERROR, true) << "This is an error";
  EXPECT_THAT(cerr_buffer.str(), Not(HasSubstr("Ok!")));
  EXPECT_THAT(cerr_buffer.str(), HasSubstr("This is an error"));
}

TEST(LoggingTest, LOG_IF_FATAL) {
  LOG_IF(FATAL, false) << "Ok!";
  EXPECT_DEATH(LOG_IF(FATAL, true) << "This is an error",
               HasSubstr("This is an error"));
}

}  // namespace
}  // namespace proto_builder::oss
