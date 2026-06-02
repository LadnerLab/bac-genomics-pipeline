#include "bacpipe/pipeline/assemble.hpp"
#include "bacpipe/core/file_discovery.hpp"
#include "bacpipe/core/path_builder.hpp"
#include "bacpipe/core/shell.hpp"

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string build_flye_command(const std::vector<std::filesystem::path> &trimmed_reads,
                               const std::filesystem::path &output_dir,
                               std::uint32_t threads) {
    std::ostringstream command{};

    command << "mkdir -p " << bacpipe::shell_quote(output_dir.parent_path().string()) << " && flye"
            << " --nano-hq";

    for (const auto &read_file : trimmed_reads) {
        command << " " << bacpipe::shell_quote(read_file.string());
    }

    command << " --out-dir " << bacpipe::shell_quote(output_dir.string()) << " --threads "
            << threads;

    return command.str();
}

} // namespace

namespace bacpipe {

std::vector<PipelineStep> build_assemble_steps(const PipelineConfig &config) {
    const std::filesystem::path trimmed_dir = PathBuilder::trimmed_reads_dir(config);
    const std::filesystem::path output_dir = PathBuilder::assembly_dir(config);
    const std::filesystem::path expected_assembly = PathBuilder::assembly_fasta(config);

    const std::vector<std::filesystem::path> trimmed_reads =
        FileDiscovery::find_fastq_files(trimmed_dir);

    return {PipelineStep{.name = "Assemble reads with Flye",
                         .command = build_flye_command(trimmed_reads, output_dir, config.threads),
                         .working_directory = config.project_root,
                         .expected_outputs = {expected_assembly},
                         .skip_when_outputs_exist = true}};
}

} // namespace bacpipe
