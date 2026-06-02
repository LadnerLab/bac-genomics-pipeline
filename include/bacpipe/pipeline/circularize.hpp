#pragma once

#include "bacpipe/pipeline/pipeline_step.hpp"
#include "bacpipe/pipeline_config.hpp"

#include <vector>

namespace bacpipe {

std::vector<PipelineStep> build_circularize_steps(const PipelineConfig &config);

} // namespace bacpipe
