#include "bacpipe/core/path_builder.hpp"

namespace bacpipe {

std::filesystem::path PathBuilder::raw_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "raw" / config.barcode;
}

std::filesystem::path PathBuilder::trimmed_reads_dir(const PipelineConfig &config) {
    return config.project_root / "data" / "trimmed" / (config.barcode + "_porechop");
}

} // namespace bacpipe
