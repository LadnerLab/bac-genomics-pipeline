#pragma once

#include "bacpipe/config/pipeline_config.hpp"

#include <filesystem>
#include <string_view>

namespace bacpipe {

std::filesystem::path expand_config_path(std::string_view path_template,
                                         const PipelineConfig &config);

} // namespace bacpipe
