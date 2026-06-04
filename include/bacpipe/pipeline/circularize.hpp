#pragma once

#include "bacpipe/config/pipeline_config.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include <vector>

namespace bacpipe {

std::vector<PipelineStep> build_circularize_steps(const PipelineConfig &config);

} // namespace bacpipe
