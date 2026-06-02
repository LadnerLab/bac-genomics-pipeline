#include "bacpipe/core/path_builder.hpp"

namespace bacpipe {

std::filesystem::path PathBuilder::raw_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "raw" / config.barcode;
}

std::filesystem::path PathBuilder::trimmed_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "trimmed" / (config.barcode + "_porechop");
}

std::filesystem::path PathBuilder::assembly_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "assembly" / (config.barcode + "_flye");
}

std::filesystem::path PathBuilder::assembly_fasta(const PipelineConfig &config) {
    return assembly_dir(config) / "assembly.fasta";
}

std::filesystem::path PathBuilder::circularization_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "circularized" / (config.barcode + "_circlator");
}

std::filesystem::path PathBuilder::circularization_reads_dir(const PipelineConfig &config) {
    return circularization_dir(config) / "00_reads";
}

std::filesystem::path PathBuilder::combined_trimmed_reads_fastq(const PipelineConfig &config) {
    return circularization_reads_dir(config) / (config.barcode + ".trimmed.combined.fastq.gz");
}

std::filesystem::path PathBuilder::circlator_output_dir(const PipelineConfig &config) {
    return circularization_dir(config) / "01_circlator";
}

std::filesystem::path PathBuilder::circularized_fasta(const PipelineConfig &config) {
    return circlator_output_dir(config) / "06.fixstart.fasta";
}

std::filesystem::path PathBuilder::circlator_circularize_log(const PipelineConfig &config) {
    return circlator_output_dir(config) / "04.merge.circularise.log";
}

} // namespace bacpipe
