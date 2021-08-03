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

#include "proto_builder/template_builder.h"

#include "gmock/gmock.h"
#include "proto_builder/oss/testing/cpp_pb_gunit.h"

namespace proto_builder {
namespace {

class TemplateBuilderTest : public ::testing::Test {
 protected:
};

TEST_F(TemplateBuilderTest, HeaderGuard) {
  EXPECT_THAT(HeaderGuard("_"), "__");
  EXPECT_THAT(HeaderGuard("."), "__");
  EXPECT_THAT(HeaderGuard("/"), "__");
  EXPECT_THAT(HeaderGuard("foo_bar"), "FOO_BAR_");
  EXPECT_THAT(HeaderGuard("_foo_bar_"), "_FOO_BAR__");
  EXPECT_THAT(HeaderGuard("_foo__bar__"), "_FOO__BAR___");
  EXPECT_THAT(HeaderGuard("foo_bar/baz.h"), "FOO_BAR_BAZ_H_");
}

TEST_F(TemplateBuilderTest, StripPrefixDir) {
  EXPECT_THAT(StripPrefixDir("bla_mid_bla_end", ""), "bla_mid_bla_end");
  EXPECT_THAT(StripPrefixDir("bla_mid_bla_end", "mid"), "bla_mid_bla_end");
  EXPECT_THAT(StripPrefixDir("bla_mid_bla_end", ".*mid"), "_bla_end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*mid/"), "bla/end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*mid"), "bla/end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*bla"), "end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*end"), "");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", "x,.*mid,y"), "bla/end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*bla,.*mid,.*end"), "end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ".*end,.*mid,.*bla"), "");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", ""), "bla/mid/bla/end");
  EXPECT_THAT(StripPrefixDir("bla/mid/bla/end", "bla/mid/bla/end"), "");
  EXPECT_THAT(StripPrefixDir("/mid/bla/", ""), "mid/bla/");
}

}  // namespace
}  // namespace proto_builder
