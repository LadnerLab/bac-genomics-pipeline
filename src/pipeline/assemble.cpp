#include "bacpipe/pipeline/assemble.hpp"

#include "bacpipe/core/file_discovery.hpp"
#include "bacpipe/core/path_builder.hpp"
#include "bacpipe/core/shell.hpp"

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string mkdir_command(const std::filesystem::path &directory) {
    if (directory.empty()) {
        return "true";
    }

    return "mkdir -p " + bacpipe::shell_quote(directory.string());
}

std::string mkdir_parent_command(const std::filesystem::path &path) {
    const std::filesystem::path parent = path.parent_path();
    return mkdir_command(parent);
}

std::string sample_id(const std::uint32_t index) {
    std::ostringstream id{};
    id << std::setw(2) << std::setfill('0') << index;
    return id.str();
}

std::filesystem::path genome_size_file(const std::filesystem::path &assembly_dir) {
    if (assembly_dir.empty()) {
        return "genome_size.txt";
    }

    return assembly_dir / "genome_size.txt";
}

bool uses_auto_genome_size(const bacpipe::ToolConfig &autocycler) {
    return autocycler.genome_size == "auto";
}

std::string genome_size_argument(const bacpipe::ToolConfig &autocycler,
                                 const std::filesystem::path &genome_size_path) {
    if (uses_auto_genome_size(autocycler)) {
        return "\"$(cat " + bacpipe::shell_quote(genome_size_path.string()) + ")\"";
    }

    return bacpipe::shell_quote(autocycler.genome_size);
}

std::filesystem::path subsampled_read_path(const std::filesystem::path &subsampled_reads_dir,
                                           const std::uint32_t index) {
    return subsampled_reads_dir / ("sample_" + sample_id(index) + ".fastq");
}

std::filesystem::path assembly_output_prefix(const std::filesystem::path &assemblies_dir,
                                             std::string_view assembler,
                                             const std::uint32_t sample_index) {
    return assemblies_dir / (std::string{assembler} + "_" + sample_id(sample_index));
}

std::filesystem::path assembly_fasta_path(const std::filesystem::path &assemblies_dir,
                                          std::string_view assembler,
                                          const std::uint32_t sample_index) {
    return assemblies_dir / (std::string{assembler} + "_" + sample_id(sample_index) + ".fasta");
}

std::vector<std::filesystem::path>
expected_subsample_outputs(const std::filesystem::path &subsampled_reads_dir,
                           const std::uint32_t count) {
    std::vector<std::filesystem::path> outputs{};
    outputs.reserve(count);

    for (std::uint32_t i = 1; i <= count; ++i) {
        outputs.push_back(subsampled_read_path(subsampled_reads_dir, i));
    }

    return outputs;
}

std::string build_combine_reads_command(const std::vector<std::filesystem::path> &trimmed_reads,
                                        const std::filesystem::path &combined_reads) {
    std::ostringstream command{};

    command << mkdir_parent_command(combined_reads) << " && (for read in";

    for (const auto &read_file : trimmed_reads) {
        command << " " << bacpipe::shell_quote(read_file.string());
    }

    command << "; do case \"$read\" in *.gz) gzip -cd -- \"$read\" ;; *) cat -- \"$read\" ;; "
               "esac; done) | gzip -c > "
            << bacpipe::shell_quote(combined_reads.string());

    return command.str();
}

std::string build_genome_size_command(const std::filesystem::path &combined_reads,
                                      const std::filesystem::path &genome_size_path,
                                      const bacpipe::ToolConfig &tool,
                                      const std::uint32_t threads) {
    std::ostringstream command{};

    command << mkdir_parent_command(genome_size_path) << " && "
            << bacpipe::shell_quote(tool.executable) << " helper genome_size"
            << " --reads " << bacpipe::shell_quote(combined_reads.string()) << " --threads "
            << threads << " > " << bacpipe::shell_quote(genome_size_path.string());

    return command.str();
}

std::string build_subsample_command(const std::filesystem::path &combined_reads,
                                    const std::filesystem::path &subsampled_reads_dir,
                                    const bacpipe::ToolConfig &tool,
                                    const std::filesystem::path &genome_size_path) {
    std::ostringstream command{};

    command << mkdir_command(subsampled_reads_dir) << " && "
            << bacpipe::shell_quote(tool.executable) << " subsample"
            << " --reads " << bacpipe::shell_quote(combined_reads.string()) << " --out_dir "
            << bacpipe::shell_quote(subsampled_reads_dir.string()) << " --genome_size "
            << genome_size_argument(tool, genome_size_path) << " --count " << tool.subsample_count
            << bacpipe::join_shell_args(tool.subsample_extra_args);

    return command.str();
}

std::string build_helper_command(const std::filesystem::path &subsampled_read,
                                 const std::filesystem::path &out_prefix,
                                 std::string_view assembler,
                                 const bacpipe::ToolConfig &tool,
                                 const std::filesystem::path &genome_size_path,
                                 const std::uint32_t threads) {
    std::ostringstream command{};

    command << mkdir_command(out_prefix.parent_path()) << " && "
            << bacpipe::shell_quote(tool.executable) << " helper "
            << bacpipe::shell_quote(assembler) << " --reads "
            << bacpipe::shell_quote(subsampled_read.string()) << " --out_prefix "
            << bacpipe::shell_quote(out_prefix.string()) << " --threads " << threads
            << " --genome_size " << genome_size_argument(tool, genome_size_path) << " --read_type "
            << bacpipe::shell_quote(tool.read_type)
            << bacpipe::join_shell_args(tool.helper_extra_args);

    return command.str();
}

std::string build_compress_command(const std::filesystem::path &assemblies_dir,
                                   const std::filesystem::path &assembly_dir,
                                   const bacpipe::ToolConfig &tool,
                                   const std::uint32_t threads) {
    std::ostringstream command{};

    command << mkdir_parent_command(assembly_dir) << " && "
            << bacpipe::shell_quote(tool.executable) << " compress"
            << " -i " << bacpipe::shell_quote(assemblies_dir.string()) << " -a "
            << bacpipe::shell_quote(assembly_dir.string()) << " -t " << threads
            << bacpipe::join_shell_args(tool.compress_extra_args);

    return command.str();
}

std::string build_cluster_command(const std::filesystem::path &assembly_dir,
                                  const bacpipe::ToolConfig &tool) {
    std::ostringstream command{};

    command << bacpipe::shell_quote(tool.executable) << " cluster"
            << " -a " << bacpipe::shell_quote(assembly_dir.string())
            << bacpipe::join_shell_args(tool.cluster_extra_args);

    return command.str();
}

std::string build_trim_resolve_command(const std::filesystem::path &assembly_dir,
                                       const bacpipe::ToolConfig &tool,
                                       const std::uint32_t threads) {
    const std::filesystem::path qc_pass_dir = assembly_dir / "clustering" / "qc_pass";
    std::ostringstream command{};

    command << "for c in " << bacpipe::shell_quote(qc_pass_dir.string()) << "/cluster_*; do "
            << "[ -d \"$c\" ] || { echo 'No Autocycler QC-pass clusters found' >&2; exit 1; }; "
            << bacpipe::shell_quote(tool.executable) << " trim -c \"$c\""
            << " -t " << threads << bacpipe::join_shell_args(tool.trim_extra_args) << " && "
            << bacpipe::shell_quote(tool.executable) << " resolve -c \"$c\""
            << bacpipe::join_shell_args(tool.resolve_extra_args) << " || exit 1; done";

    return command.str();
}

std::string build_combine_command(const std::filesystem::path &assembly_dir,
                                  const std::filesystem::path &configured_consensus,
                                  const bacpipe::ToolConfig &tool) {
    const std::filesystem::path native_consensus = assembly_dir / "consensus_assembly.fasta";
    const std::filesystem::path qc_pass_dir = assembly_dir / "clustering" / "qc_pass";

    std::ostringstream command{};

    command << bacpipe::shell_quote(tool.executable) << " combine"
            << " -a " << bacpipe::shell_quote(assembly_dir.string()) << " -i "
            << bacpipe::shell_quote(qc_pass_dir.string()) << "/cluster_*/5_final.gfa"
            << bacpipe::join_shell_args(tool.combine_extra_args);

    if (native_consensus != configured_consensus) {
        command << " && " << mkdir_parent_command(configured_consensus) << " && cp "
                << bacpipe::shell_quote(native_consensus.string()) << " "
                << bacpipe::shell_quote(configured_consensus.string());
    }

    return command.str();
}

void validate_autocycler_config(const bacpipe::ToolConfig &autocycler) {
    if (autocycler.executable.empty()) {
        throw std::runtime_error{"autocycler.executable must not be empty"};
    }

    if (autocycler.genome_size.empty()) {
        throw std::runtime_error{"autocycler.genome_size must not be empty"};
    }

    if (autocycler.read_type.empty()) {
        throw std::runtime_error{"autocycler.read_type must not be empty"};
    }

    if (autocycler.subsample_count == 0) {
        throw std::runtime_error{"autocycler.subsample_count must be greater than zero"};
    }

    if (autocycler.assemblers.empty()) {
        throw std::runtime_error{"autocycler.assemblers must contain at least one assembler"};
    }

    for (const std::string &assembler : autocycler.assemblers) {
        if (assembler.empty()) {
            throw std::runtime_error{"autocycler.assemblers must not contain empty values"};
        }
    }
}

} // namespace

namespace bacpipe {

std::vector<PipelineStep> build_assemble_steps(const PipelineConfig &config) {
    validate_autocycler_config(config.autocycler);

    const std::filesystem::path trimmed_dir = PathBuilder::trimmed_reads_dir(config);
    const std::filesystem::path combined_reads = PathBuilder::combined_trimmed_fastq(config);
    const std::filesystem::path assembly_dir = PathBuilder::assembly_dir(config);
    const std::filesystem::path subsampled_reads_dir =
        PathBuilder::assembly_subsampled_reads_dir(config);
    const std::filesystem::path assemblies_dir =
        PathBuilder::assembly_input_assemblies_dir(config);
    const std::filesystem::path consensus_fasta = PathBuilder::assembly_fasta(config);
    const std::filesystem::path genome_size_path = genome_size_file(assembly_dir);

    const std::vector<std::filesystem::path> trimmed_reads =
        FileDiscovery::find_fastq_files(trimmed_dir);

    std::vector<PipelineStep> steps{};
    steps.push_back(
        PipelineStep{.name = "Combine trimmed reads for Autocycler and Medaka",
                     .command = build_combine_reads_command(trimmed_reads, combined_reads),
                     .working_directory = config.project_root,
                     .expected_outputs = {combined_reads},
                     .skip_when_outputs_exist = true});

    if (uses_auto_genome_size(config.autocycler)) {
        steps.push_back(PipelineStep{.name = "Estimate genome size for Autocycler",
                                     .command = build_genome_size_command(combined_reads,
                                                                          genome_size_path,
                                                                          config.autocycler,
                                                                          config.threads),
                                     .working_directory = config.project_root,
                                     .expected_outputs = {genome_size_path},
                                     .skip_when_outputs_exist = true});
    }

    steps.push_back(
        PipelineStep{.name = "Subsample reads for Autocycler",
                     .command = build_subsample_command(combined_reads,
                                                        subsampled_reads_dir,
                                                        config.autocycler,
                                                        genome_size_path),
                     .working_directory = config.project_root,
                     .expected_outputs =
                         expected_subsample_outputs(subsampled_reads_dir,
                                                    config.autocycler.subsample_count),
                     .skip_when_outputs_exist = true});

    for (const std::string &assembler : config.autocycler.assemblers) {
        for (std::uint32_t i = 1; i <= config.autocycler.subsample_count; ++i) {
            const std::filesystem::path read_file = subsampled_read_path(subsampled_reads_dir, i);
            const std::filesystem::path out_prefix =
                assembly_output_prefix(assemblies_dir, assembler, i);
            const std::filesystem::path output_fasta =
                assembly_fasta_path(assemblies_dir, assembler, i);

            steps.push_back(PipelineStep{
                .name = "Run Autocycler helper: " + assembler + "_" + sample_id(i),
                .command = build_helper_command(read_file,
                                                out_prefix,
                                                assembler,
                                                config.autocycler,
                                                genome_size_path,
                                                config.threads),
                .working_directory = config.project_root,
                .expected_outputs = {output_fasta},
                .skip_when_outputs_exist = true});
        }
    }

    steps.push_back(PipelineStep{.name = "Compress Autocycler input assemblies",
                                 .command = build_compress_command(assemblies_dir,
                                                                   assembly_dir,
                                                                   config.autocycler,
                                                                   config.threads),
                                 .working_directory = config.project_root,
                                 .expected_outputs = {assembly_dir / "input_assemblies.gfa"},
                                 .skip_when_outputs_exist = true});

    steps.push_back(
        PipelineStep{.name = "Cluster Autocycler input contigs",
                     .command = build_cluster_command(assembly_dir, config.autocycler),
                     .working_directory = config.project_root,
                     .expected_outputs = {assembly_dir / "clustering" / "qc_pass"},
                     .skip_when_outputs_exist = true});

    steps.push_back(PipelineStep{
        .name = "Trim and resolve Autocycler QC-pass clusters",
        .command = build_trim_resolve_command(assembly_dir, config.autocycler, config.threads),
        .working_directory = config.project_root,
        .expected_outputs = {},
        .skip_when_outputs_exist = false});

    steps.push_back(PipelineStep{
        .name = "Combine Autocycler resolved clusters",
        .command = build_combine_command(assembly_dir, consensus_fasta, config.autocycler),
        .working_directory = config.project_root,
        .expected_outputs = {consensus_fasta},
        .skip_when_outputs_exist = true});

    return steps;
}

} // namespace bacpipe
