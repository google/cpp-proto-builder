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

#include "proto_builder/oss/file.h"

#include <filesystem>

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace file {
namespace oss {
namespace {

namespace fs = ::std::filesystem;

using ::testing::HasSubstr;
using ::testing::status::oss::StatusIs;

static std::string TestName() {
  return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

TEST(FileTest, SetContents) {
  std::error_code ec;
  fs::path global_temp_dir = fs::temp_directory_path(ec);
  ASSERT_EQ(ec.value(), 0);
  fs::path temp_dir = (global_temp_dir / TestName());
  fs::create_directory(temp_dir);
  fs::path temp_file = temp_dir / "foo.txt";
  EXPECT_OK(SetContents(temp_file.string(), "foo"));
  fs::remove_all(temp_dir);
}

TEST(FileTest, Readable) {
  std::error_code ec;
  fs::path global_temp_dir = fs::temp_directory_path(ec);
  ASSERT_EQ(ec.value(), 0);
  fs::path temp_dir = (global_temp_dir / TestName());
  fs::create_directory(temp_dir);
  EXPECT_THAT(Readable(temp_dir.string()),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Open failed: Is a directory")));
  fs::create_directory(temp_dir);
  fs::path temp_file = temp_dir / "foo.txt";
  ASSERT_OK(SetContents(temp_file.string(), "foo"));
  EXPECT_OK(Readable(temp_file.string()));
}

TEST(FileTest, GetContents) {
  std::error_code ec;
  fs::path global_temp_dir = fs::temp_directory_path(ec);
  ASSERT_EQ(ec.value(), 0);
  fs::path temp_dir = (global_temp_dir / TestName());
  fs::create_directory(temp_dir);
  fs::path temp_file = temp_dir / "foo.txt";
  EXPECT_OK(SetContents(temp_file.string(), "foo"));
  std::string file_content;
  EXPECT_OK(GetContents(temp_file.string(), &file_content));
  EXPECT_EQ(file_content, "foo");
  fs::remove_all(temp_dir);
}

TEST(FileTest, IsAbsolutePath) {
  std::error_code ec;
  fs::path global_temp_dir = fs::temp_directory_path(ec);
  ASSERT_EQ(ec.value(), 0);
  EXPECT_TRUE(IsAbsolutePath(global_temp_dir.string()));
}

TEST(FileTest, JoinPath) {
  EXPECT_THAT(JoinPath(""), "");
  EXPECT_THAT(JoinPath("a"), "a");
  EXPECT_THAT(JoinPath("", ""), "");
  EXPECT_THAT(JoinPath("a", ""), "a");
  EXPECT_THAT(JoinPath("", "b"), "b");
  EXPECT_THAT(JoinPath("a", "b"), "a/b");
  EXPECT_THAT(JoinPath("", "", ""), "");
  EXPECT_THAT(JoinPath("a", "", ""), "a");
  EXPECT_THAT(JoinPath("", "b", ""), "b");
  EXPECT_THAT(JoinPath("a", "b", ""), "a/b");
  EXPECT_THAT(JoinPath("", "", "c"), "c");
  EXPECT_THAT(JoinPath("a", "", "c"), "a/c");
  EXPECT_THAT(JoinPath("a", "b", ""), "a/b");
  EXPECT_THAT(JoinPath("a", "b", "c"), "a/b/c");
  EXPECT_THAT(internal::JoinPathSimplify(""), "");
  EXPECT_THAT(internal::JoinPathSimplify("/"), "/");
  EXPECT_THAT(internal::JoinPathSimplify("//"), "/");
  EXPECT_THAT(internal::JoinPathSimplify("///"), "/");
  EXPECT_THAT(internal::JoinPathSimplify("////"), "/");
  EXPECT_THAT(internal::JoinPathSimplify("//a/"), "/a");
  EXPECT_THAT(internal::JoinPathSimplify("//a//b"), "/a/b");
  EXPECT_THAT(internal::JoinPathSimplify("//a//b/"), "/a/b");
  EXPECT_THAT(internal::JoinPathSimplify("//a//b//"), "/a/b");
  EXPECT_THAT(internal::JoinPathSimplify("//a////b"), "/a/b");
  EXPECT_THAT(JoinPath("", "/", ""), "/");
  EXPECT_THAT(JoinPath("", "/", "/"), "/");
  EXPECT_THAT(JoinPath("", "/", "/", "a"), "/a");
  EXPECT_THAT(JoinPath("a/"), "a");
}

}  // namespace
}  // namespace oss
}  // namespace file
