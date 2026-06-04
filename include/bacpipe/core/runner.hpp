#pragma once

#include "bacpipe/pipeline/pipeline_step.hpp"

#include <string>
#include <vector>

namespace bacpipe {

struct RunnerOptions {
    bool dry_run{false};
    bool skip_existing{true};
    bool stop_on_error{true};
};

struct RunnerResult {
    std::string step_name{};
    int exit_code{0};
    bool skipped{false};
};

class Runner {
  public:
    explicit Runner(RunnerOptions options = RunnerOptions{});

    [[nodiscard]] RunnerResult run(const PipelineStep &step) const;
    [[nodiscard]] std::vector<RunnerResult> run_all(const std::vector<PipelineStep> &steps) const;

  private:
    RunnerOptions options_;
    [[nodiscard]] static int execute(const PipelineStep &step);
};

} // namespace bacpipe
