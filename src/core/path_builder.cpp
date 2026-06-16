#include "bacpipe/core/path_builder.hpp"

namespace bacpipe {

std::filesystem::path PathBuilder::raw_reads_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.raw_reads, config);
}

std::filesystem::path PathBuilder::trimmed_reads_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.trimmed_reads, config);
}

std::filesystem::path PathBuilder::assembly_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.assembly_dir, config);
}

std::filesystem::path PathBuilder::assembly_fasta(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.assembly_fasta, config);
}

std::filesystem::path PathBuilder::circularization_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.circularization_dir, config);
}

std::filesystem::path PathBuilder::circularization_reads_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.circularization_reads_dir, config);
}

std::filesystem::path PathBuilder::combined_trimmed_reads_fastq(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.combined_trimmed_reads, config);
}

std::filesystem::path PathBuilder::combined_trimmed_reads_fasta(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.combined_trimmed_reads, config);
}

std::filesystem::path PathBuilder::circlator_output_dir(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.circlator_output_dir, config);
}

std::filesystem::path PathBuilder::circularized_fasta(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.circularized_fasta, config);
}

std::filesystem::path PathBuilder::circlator_circularize_log(const PipelineConfig &config) {
    return bacpipe::expand_config_path(config.paths.circlator_circularize_log, config);
}

} // namespace bacpipe
