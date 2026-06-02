#pragma once

#include "bacpipe/pipeline_config.hpp"

#include <filesystem>

namespace bacpipe {

class PathBuilder {
  public:
    [[nodiscard]] static std::filesystem::path raw_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path trimmed_reads_dir(const PipelineConfig &config);

    [[nodiscard]] static std::filesystem::path assembly_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path assembly_fasta(const PipelineConfig &config);

    [[nodiscard]] static std::filesystem::path circularization_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    circularization_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    combined_trimmed_reads_fastq(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path circlator_output_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path circularized_fasta(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path
    circlator_circularize_log(const PipelineConfig &config);
};

} // namespace bacpipe
