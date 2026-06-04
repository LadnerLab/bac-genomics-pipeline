#include "bacpipe/pipeline/pipeline_step.hpp"

#include <algorithm>

namespace bacpipe {

bool PipelineStep::is_valid() const noexcept {
    return !name.empty() && !command.empty();
}

bool PipelineStep::outputs_exist() const {
    if (expected_outputs.empty()) {
        return false;
    }
    return std::ranges::all_of(expected_outputs,
                               [](const auto &output) { return std::filesystem::exists(output); });
}

} // namespace bacpipe
