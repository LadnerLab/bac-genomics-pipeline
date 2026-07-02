#pragma once

#include "bacpipe/config/pipeline_config.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include <vector>

namespace bacpipe {

[[nodiscard]] std::vector<PipelineStep> build_polish_steps(const PipelineConfig &config);

} // namespace bacpipe
