#include "bacpipe/core/path_builder.hpp"

namespace bacpipe {

std::filesystem::path PathBuilder::raw_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "raw" / config.barcode;
}

std::filesystem::path PathBuilder::trimmed_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "trimmed" / (config.barcode + "_porechop");
}

std::filesystem::path PathBuilder::assembly_dir(const PipelineConfig &config) {
    return config.project_root / "assembly" / (config.barcode + "_flye");
}

std::filesystem::path PathBuilder::assembly_fasta(const PipelineConfig &config) {
    return assembly_dir(config) / "assembly.fasta";
}

} // namespace bacpipe
