#pragma once

#include "bacpipe/config/pipeline_config.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include <vector>

namespace bacpipe {

[[nodiscard]] std::vector<PipelineStep> build_autocycler_steps(const PipelineConfig &config);

} // namespace bacpipe
