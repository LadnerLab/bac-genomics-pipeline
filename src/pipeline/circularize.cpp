#include "bacpipe/pipeline/circularize.hpp"
#include "bacpipe/core/file_discovery.hpp"
#include "bacpipe/core/path_builder.hpp"
#include "bacpipe/core/shell.hpp"

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string build_combine_reads_command(const std::vector<std::filesystem::path> &trimmed_reads,
                                        const std::filesystem::path &combined_reads,
                                        const std::filesystem::path &reads_dir) {
    std::ostringstream command{};

    command << "mkdir -p " << bacpipe::shell_quote(reads_dir.string()) << " && cat";

    for (const auto &read_file : trimmed_reads) {
        command << " " << bacpipe::shell_quote(read_file.string());
    }

    command << " > " << bacpipe::shell_quote(combined_reads.string());

    return command.str();
}

std::string build_circlator_command(const std::filesystem::path &assembly_fasta,
                                    const std::filesystem::path &combined_reads,
                                    const std::filesystem::path &output_dir,
                                    std::uint32_t threads) {
    std::ostringstream command{};

    command << "mkdir -p " << bacpipe::shell_quote(output_dir.string()) << " && circlator all"
            << " --threads " << threads << " --verbose "
            << bacpipe::shell_quote(assembly_fasta.string()) << " "
            << bacpipe::shell_quote(combined_reads.string()) << " "
            << bacpipe::shell_quote(output_dir.string());

    return command.str();
}

} // namespace

namespace bacpipe {

std::vector<PipelineStep> build_circularize_steps(const PipelineConfig &config) {
    const std::filesystem::path trimmed_dir = PathBuilder::trimmed_reads_dir(config);
    const std::filesystem::path assembly_fasta = PathBuilder::assembly_fasta(config);
    const std::filesystem::path reads_dir = PathBuilder::circularization_reads_dir(config);
    const std::filesystem::path combined_reads = PathBuilder::combined_trimmed_reads_fastq(config);
    const std::filesystem::path circlator_dir = PathBuilder::circlator_output_dir(config);
    const std::filesystem::path final_fasta = PathBuilder::circularized_fasta(config);
    const std::filesystem::path circularize_log = PathBuilder::circlator_circularize_log(config);

    if (!std::filesystem::exists(assembly_fasta)) {
        throw std::runtime_error{"Assembly FASTA does not exist: " + assembly_fasta.string()};
    }

    const std::vector<std::filesystem::path> trimmed_reads =
        FileDiscovery::find_fastq_files(trimmed_dir);

    if (trimmed_reads.empty()) {
        throw std::runtime_error{"No trimmed FASTQ files were found for barcode: " +
                                 config.barcode + " in " + trimmed_dir.string()};
    }

    return {PipelineStep{.name = "Combine trimmed reads for Circlator",
                         .command =
                             build_combine_reads_command(trimmed_reads, combined_reads, reads_dir),
                         .working_directory = config.project_root,
                         .expected_outputs = {combined_reads},
                         .skip_when_outputs_exist = true},
            PipelineStep{.name = "Circularize assembly with Circlator",
                         .command = build_circlator_command(assembly_fasta,
                                                            combined_reads,
                                                            circlator_dir,
                                                            config.threads),
                         .working_directory = config.project_root,
                         .expected_outputs = {final_fasta, circularize_log},
                         .skip_when_outputs_exist = true}};
}

} // namespace bacpipe
