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

#include <limits>
#include <string>

#include "proto_builder/oss/init_program.h"
#include "proto_builder/descriptor_util.h"
#include "proto_builder/field_builder.h"
#include "proto_builder/message_builder.h"
#include "proto_builder/oss/file.h"
#include "proto_builder/oss/logging_macros.h"
#include "proto_builder/oss/sourcefile_database.h"
#include "proto_builder/oss/util.h"
#include "proto_builder/proto_builder_config.h"
#include "proto_builder/proto_builder_data.h"
#include "proto_builder/template_builder.h"
#include "google/protobuf/descriptor.h"
#include "absl/flags/flag.h"
#include "absl/status/status.h"

ABSL_FLAG(
    std::string, proto, "",
    "Prototype and file describing the message (e.g.: my.Type:/file.proto). "
    "It is possible to provide multiple proto files separated by ','. "
    "However, the tool reads the transitive closure of dependencies. "
    "Thus multiple proto files are only necessary for extensions like "
    "MessageSet. The tool uses $CWD for local filenames (does not work with "
    "bazel run and the tool does not respect --protofiles or --proto_paths. "
    "It is also possible to specify a wildcard (e.g.: *:path/to/file.proto) "
    "to generate builders for ALL top-level messages within the first proto "
    "file.");

ABSL_FLAG(std::string, header, "", "Header file (.h) to write.");

ABSL_FLAG(std::string, interface, "", "Interface header file (.h) to write.");

ABSL_FLAG(std::string, source, "", "Source file (.cc) to write.");

ABSL_FLAG(std::string, template_builder_strip_prefix_dir, "",
          "A comma separated list of directory prefixes to strip from "
          "output files when generating {HEADER_FILE} and {HEADER_GUARD}. "
          "template values. The prefixes are in RE2 notation and will be "
          "anchored to the left (^). If a directory matches and the "
          "remaining suffix starts with a '/', then that will be removed.");

ABSL_FLAG(std::string, header_in, "default",
          "Header input or template file to use.");

ABSL_FLAG(std::string, interface_in, "default",
          "Interface input or template file to use.");

ABSL_FLAG(std::string, source_in, "default",
          "Body input or template file to use.");

ABSL_FLAG(std::string, tpl_value_header, "",
          "This should only be used by proto_builder_test. - "
          "Used to override header related template values. This can be "
          "used to generate files with a different header reference than "
          "automatically derived from --header. It is helpful to solve "
          "cyclic dependencies in proto_builder_test rules where it is not "
          "possible to generate the header with the intended file name.");

ABSL_FLAG(std::string, workdir, "", "Pass in ${PWD} to support bazel run.");

ABSL_FLAG(size_t, max_field_depth, 0u,
          "Maximum message depth (0 = default, "
          "1 = only top level fields and message fields).");

ABSL_FLAG(std::string, conv_deps_file, "", "List of conversion dependencies");

ABSL_FLAG(bool, use_validator, false,
          "Whether Validator code will be generated.");

ABSL_FLAG(std::string, validator_header, "",
          "The validator header to use if any was identifed (automatically "
          "enables --use_validator).");

ABSL_FLAG(bool, make_interface, false,
          "Whether to make an additional interface header file.");

namespace proto_builder {

absl::Status WriteProtoBuilderFiles() {
  if (!absl::GetFlag(FLAGS_conv_deps_file).empty()) {
    QCHECK_OK(CheckConversionDependencies(absl::GetFlag(FLAGS_conv_deps_file)))
        << absl::GetFlag(FLAGS_conv_deps_file);
  }
  auto [status, descriptor_util] = UnpackStatusOrDefault(  //
      DescriptorUtil::Load(absl::GetFlag(FLAGS_proto),
                           oss::SourceFileDatabase::GetProtoFilesFlag(),
                           oss::SourceFileDatabase::GetProtoPathsFlag()),
      DescriptorUtil::DefaultWillNotWork::kDefaultWillNotWork);
  if (!status.ok()) {
    return status;
  }
  const size_t max_field_depth =
      absl::GetFlag(FLAGS_max_field_depth)
          ? absl::GetFlag(FLAGS_max_field_depth)
          : (descriptor_util.search_mode() == MessageSearchMode::kTransitiveAll
                 ? 1
                 : std::numeric_limits<size_t>::max());

  const std::string header =
      StripPrefixDir(absl::GetFlag(FLAGS_tpl_value_header).empty()
                         ? absl::GetFlag(FLAGS_header)
                         : absl::GetFlag(FLAGS_tpl_value_header),
                     absl::GetFlag(FLAGS_template_builder_strip_prefix_dir));
  const std::string interface =
      StripPrefixDir(absl::GetFlag(FLAGS_interface),
                     absl::GetFlag(FLAGS_template_builder_strip_prefix_dir));

  const ProtoBuilderConfigManager global_config;
  BufferWriter writer;
  const std::string header_template =
      absl::GetFlag(FLAGS_header_in) == "default"
          ? DefaultHeaderTemplate()
          : *file::oss::GetContents(absl::GetFlag(FLAGS_header_in));
  const std::string interface_template =
      absl::GetFlag(FLAGS_interface_in) == "default"
          ? DefaultInterfaceTemplate()
          : *file::oss::GetContents(absl::GetFlag(FLAGS_interface_in));
  const std::string source_template =
      absl::GetFlag(FLAGS_source_in) == "default"
          ? DefaultSourceTemplate()
          : *file::oss::GetContents(absl::GetFlag(FLAGS_source_in));
  const std::string validator_header = absl::GetFlag(FLAGS_validator_header);
  if (auto s = TemplateBuilder(
                   {
                       .config = global_config,
                       .writer = &writer,
                       .descriptors = descriptor_util.descriptors(),
                       .header = header,
                       .tpl_head = header_template,
                       .tpl_body = source_template,
                       .max_field_depth = max_field_depth,
                       .use_validator = absl::GetFlag(FLAGS_use_validator) ||
                                        !validator_header.empty(),
                       .validator_header = validator_header,
                       .make_interface = absl::GetFlag(FLAGS_make_interface),
                       .tpl_iface = interface_template,
                       .interface_header = interface,
                   })
                   .WriteBuilder();
      !s.ok()) {
    return s;
  }

  if (auto s = writer.WriteFile(HEADER, absl::GetFlag(FLAGS_header)); !s.ok()) {
    return s;
  }
  if (auto s = writer.WriteFile(SOURCE, absl::GetFlag(FLAGS_source)); !s.ok()) {
    return s;
  }
  if (absl::GetFlag(FLAGS_make_interface)) {
    if (auto s = writer.WriteFile(INTERFACE, absl::GetFlag(FLAGS_interface));
        !s.ok()) {
      return s;
    }
  }
  return absl::OkStatus();
}

}  // namespace proto_builder

namespace {
const char* UsageMessage() {
  return R"(

proto_builder --proto <my.Type:/path/file.proto> --header <file> --source <file>

For details see: https://google.github.io/cpp-proto-builder

Read the provided proto file and generate a C++ Builder pattern. The
declaration will be saved in --header <file> and the implementation in
--source <file>.
)";
}

}  // namespace

int main(int argc, char** argv) {
  InitProgram(UsageMessage(), &argc, &argv, true);
  QCHECK(!absl::GetFlag(FLAGS_proto).empty());
  QCHECK(!absl::GetFlag(FLAGS_header).empty());
  QCHECK(!absl::GetFlag(FLAGS_source).empty());
  if (!absl::GetFlag(FLAGS_workdir).empty()) {
    QCHECK(file::oss::IsAbsolutePath(absl::GetFlag(FLAGS_workdir)));
    QCHECK_EQ(::chdir(absl::GetFlag(FLAGS_workdir).c_str()), 0);
  }
  QCHECK_OK(::proto_builder::WriteProtoBuilderFiles());
  return 0;
}
