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

#include "proto_builder/oss/sourcefile_database.h"

#include "proto_builder/oss/file.h"
#include "proto_builder/oss/get_runfiles_dir.h"
#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder::oss {

namespace {

using ::file::oss::JoinPath;
using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::IsNull;
using ::testing::Not;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Property;

class SourceFileDatabaseTest : public ::testing::Test {
 public:
  static std::string GetFile(bool relative = false) {
    const std::string filename =
        "proto_builder/oss/tests/simple_message.proto";
    return relative ? filename : JoinPath(GetRunfilesDir(), filename);
  }

  static std::string GetBadFile(bool relative = false) {
    const std::string filename =
        "proto_builder/oss/tests/"
        "simple_message_error.proto.bad";
    return relative ? filename : JoinPath(GetRunfilesDir(), filename);
  }
};

TEST_F(SourceFileDatabaseTest, New) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({GetFile()}, {}));
  EXPECT_THAT(sfdb, NotNull());
  EXPECT_TRUE(sfdb->LoadedSuccessfully());
}

TEST_F(SourceFileDatabaseTest, pool) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({GetFile()}, {}));
  ASSERT_THAT(sfdb, NotNull());
  EXPECT_THAT(sfdb->pool(), NotNull());
  EXPECT_TRUE(sfdb->LoadedSuccessfully());
  EXPECT_THAT(sfdb->GetErrors(), IsEmpty());
}

TEST_F(SourceFileDatabaseTest, FindFileByName) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({GetFile(true)}, {GetRunfilesDir()}));
  ASSERT_THAT(sfdb, NotNull());
  ASSERT_THAT(sfdb->pool(), NotNull());
  EXPECT_TRUE(sfdb->LoadedSuccessfully());
  EXPECT_THAT(sfdb->GetErrors(), IsEmpty());
  EXPECT_THAT(sfdb->pool()->FindFileByName(GetFile(true)), NotNull());
}

TEST_F(SourceFileDatabaseTest, FindMessageTypeByName) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({GetFile()}, {}));
  ASSERT_THAT(sfdb, NotNull());
  ASSERT_THAT(sfdb->pool(), NotNull());
  EXPECT_TRUE(sfdb->LoadedSuccessfully());
  EXPECT_THAT(sfdb->pool()->FindMessageTypeByName(
                  "proto_builder.oss.SimpleMessage"),
              NotNull());
}

TEST_F(SourceFileDatabaseTest, MissingFile) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({"does_not_exist.proto"}, {}));
  ASSERT_THAT(
      sfdb, AnyOf(Pointee(Property("GetErrors", &SourceFileDatabase::GetErrors,
                                   Not(IsEmpty()))),
                  IsNull()));
}

TEST_F(SourceFileDatabaseTest, LoadError) {
  std::unique_ptr<SourceFileDatabase> sfdb(
      SourceFileDatabase::New({GetBadFile()}, {}));
  ASSERT_THAT(sfdb, NotNull());
  ASSERT_THAT(sfdb->pool(), NotNull());
  EXPECT_FALSE(sfdb->LoadedSuccessfully());
  EXPECT_THAT(sfdb->GetErrors(), Contains(HasSubstr(GetBadFile(true))));
}

}  // namespace
}  // namespace proto_builder::oss
