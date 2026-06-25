#pragma once

#include "bacpipe/config/path_expander.hpp"
#include "bacpipe/config/pipeline_config.hpp"

#include <filesystem>

namespace bacpipe {

class PathBuilder {
  public:
    [[nodiscard]] static std::filesystem::path raw_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path trimmed_reads_dir(const PipelineConfig &config);

    [[nodiscard]] static std::filesystem::path assembly_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path assembly_fasta(const PipelineConfig &config);

    [[nodiscard]] static std::filesystem::path combined_trimmed_fastq(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path autocycler_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    autocycler_subsampled_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    autocycler_input_assemblies_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    autocycler_consensus_fasta(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path medaka_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path medaka_consensus_fasta(const PipelineConfig &config);

    [[nodiscard]] static std::filesystem::path circularization_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    circularization_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    combined_trimmed_reads_fastq(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path combined_trimmed_reads_fasta(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path circlator_output_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path circularized_fasta(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    circlator_circularize_log(const PipelineConfig &config);
};

} // namespace bacpipe
