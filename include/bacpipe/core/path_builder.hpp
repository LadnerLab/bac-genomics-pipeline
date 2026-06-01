#pragma once

#include "bacpipe/pipeline_config.hpp"

#include <filesystem>

namespace bacpipe {

class PathBuilder {
  public:
    [[nodiscard]] static std::filesystem::path raw_reads_dir(const PipelineConfig &config);
    [[nodiscard]] static std::filesystem::path trimmed_reads_dir(const PipelineConfig &config);
};

} // namespace bacpipe
