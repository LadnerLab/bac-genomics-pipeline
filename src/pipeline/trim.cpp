#include "bacpipe/pipeline/trim.hpp"
#include "bacpipe/core/file_discovery.hpp"
#include "bacpipe/core/path_builder.hpp"
#include "bacpipe/core/shell.hpp"

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string remove_fastq_suffix(std::string filename) {
    constexpr std::string_view fastq_gz{".fastq.gz"};
    constexpr std::string_view fastq{".fastq"};

    if (filename.ends_with(fastq_gz)) {
        filename.erase(filename.size() - fastq_gz.size());
    } else if (filename.ends_with(fastq)) {
        filename.erase(filename.size() - fastq.size());
    }

    return filename;
}

std::filesystem::path make_trimmed_output_path(const std::filesystem::path &output_dir,
                                               const std::filesystem::path &input_file) {
    const std::string base_name = remove_fastq_suffix(input_file.filename().string());
    return output_dir / (base_name + ".trimmed.fastq.gz");
}

std::string build_porechop_command(const std::filesystem::path &input_file,
                                   const std::filesystem::path &output_file,
                                   const std::filesystem::path &output_dir,
                                   const bacpipe::ToolConfig &tool,
                                   std::uint32_t threads) {
    std::ostringstream command{};

    command << "mkdir -p " << bacpipe::shell_quote(output_dir.string()) << " && "
            << bacpipe::shell_quote(tool.executable) << bacpipe::join_shell_args(tool.extra_args)
            << " -i " << bacpipe::shell_quote(input_file.string()) << " -o "
            << bacpipe::shell_quote(output_file.string()) << " --threads " << threads;

    return command.str();
}

} // namespace

namespace bacpipe {

std::vector<PipelineStep> build_trim_steps(const PipelineConfig &config) {
    const std::filesystem::path input_dir = PathBuilder::raw_reads_dir(config);
    const std::filesystem::path output_dir = PathBuilder::trimmed_reads_dir(config);

    const std::vector<std::filesystem::path> input_files =
        FileDiscovery::find_fastq_files(input_dir);

    std::vector<PipelineStep> steps{};
    steps.reserve(input_files.size());

    for (const auto &input_file : input_files) {
        const std::filesystem::path output_file = make_trimmed_output_path(output_dir, input_file);

        steps.push_back(
            PipelineStep{.name = "Trim reads with Porechop: " + input_file.filename().string(),
                         .command = build_porechop_command(input_file,
                                                           output_file,
                                                           output_dir,
                                                           config.porechop,
                                                           config.threads),
                         .working_directory = config.project_root,
                         .expected_outputs = {output_file},
                         .skip_when_outputs_exist = true});
    }

    return steps;
}

} // namespace bacpipe
