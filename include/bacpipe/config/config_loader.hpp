#pragma once

#include "bacpipe/config/pipeline_config.hpp"

#include <filesystem>

namespace bacpipe {

class ConfigLoader {
  public:
    static PipelineConfig load(const std::filesystem::path &config_file, PipelineConfig config);
};

} // namespace bacpipe
