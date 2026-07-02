#include "bacpipe/pipeline/polish.hpp"

#include "bacpipe/core/path_builder.hpp"
#include "bacpipe/core/shell.hpp"

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string mkdir_parent_command(const std::filesystem::path &path) {
    const std::filesystem::path parent = path.parent_path();

    if (parent.empty()) {
        return "true";
    }

    return "mkdir -p " + bacpipe::shell_quote(parent.string());
}

std::string build_medaka_command(const std::filesystem::path &combined_reads,
                                 const std::filesystem::path &draft_assembly,
                                 const std::filesystem::path &medaka_dir,
                                 const std::filesystem::path &configured_consensus,
                                 const bacpipe::ToolConfig &tool,
                                 const std::uint32_t threads) {
    const std::filesystem::path native_consensus = medaka_dir / "consensus.fasta";
    std::ostringstream command{};

    command << "mkdir -p " << bacpipe::shell_quote(medaka_dir.string()) << " && "
            << bacpipe::shell_quote(tool.executable) << " -i "
            << bacpipe::shell_quote(combined_reads.string()) << " -d "
            << bacpipe::shell_quote(draft_assembly.string()) << " -o "
            << bacpipe::shell_quote(medaka_dir.string()) << " -t " << threads
            << bacpipe::join_shell_args(tool.extra_args) << " && if [ "
            << bacpipe::shell_quote(native_consensus.string())
            << " != " << bacpipe::shell_quote(configured_consensus.string()) << " ]; then "
            << mkdir_parent_command(configured_consensus) << " && cp "
            << bacpipe::shell_quote(native_consensus.string()) << " "
            << bacpipe::shell_quote(configured_consensus.string()) << "; fi";

    return command.str();
}

void validate_medaka_config(const bacpipe::ToolConfig &medaka) {
    if (medaka.executable.empty()) {
        throw std::runtime_error{"medaka.executable must not be empty"};
    }
}

} // namespace

namespace bacpipe {

std::vector<PipelineStep> build_polish_steps(const PipelineConfig &config) {
    validate_medaka_config(config.medaka);

    const std::filesystem::path combined_reads = PathBuilder::combined_trimmed_fastq(config);
    const std::filesystem::path draft_assembly = PathBuilder::autocycler_consensus_fasta(config);
    const std::filesystem::path medaka_dir = PathBuilder::medaka_dir(config);
    const std::filesystem::path consensus_fasta = PathBuilder::medaka_consensus_fasta(config);

    if (!config.dry_run) {
        if (!std::filesystem::exists(combined_reads)) {
            throw std::runtime_error{"Combined trimmed FASTQ does not exist: " +
                                     combined_reads.string()};
        }

        if (!std::filesystem::exists(draft_assembly)) {
            throw std::runtime_error{"Autocycler consensus FASTA does not exist: " +
                                     draft_assembly.string()};
        }
    }

    return {PipelineStep{.name = "Polish Autocycler assembly with Medaka",
                         .command = build_medaka_command(combined_reads,
                                                         draft_assembly,
                                                         medaka_dir,
                                                         consensus_fasta,
                                                         config.medaka,
                                                         config.threads),
                         .working_directory = config.project_root,
                         .expected_outputs = {consensus_fasta},
                         .skip_when_outputs_exist = true}};
}

} // namespace bacpipe
