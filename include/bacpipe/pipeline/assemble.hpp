#pragma once

#include "bacpipe/pipeline/pipeline_step.hpp"
#include "bacpipe/pipeline_config.hpp"

#include <vector>

namespace bacpipe {

[[nodiscard]] std::vector<PipelineStep> build_assemble_steps(const PipelineConfig &config);

} // namespace bacpipe
