#include "bacpipe/core/runner.hpp"
#include "bacpipe/core/logger.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <system_error>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace {

class WorkingDirectoryGuard {
  public:
    explicit WorkingDirectoryGuard(const std::filesystem::path &next)
        : previous_{std::filesystem::current_path()} {
        if (!next.empty()) {
            std::filesystem::create_directories(next);
            std::filesystem::current_path(next);
        }
    }

    ~WorkingDirectoryGuard() {
        std::error_code error{};
        std::filesystem::current_path(previous_, error);
    }

  private:
    std::filesystem::path previous_;
};

int normalize_exit_code(const int status) {
    if (status == -1) {
        return -1;
    }

#ifndef _WIN32
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
#endif

    return status;
}

} // namespace

namespace bacpipe {

Runner::Runner(RunnerOptions options) : options_{options} {}

RunnerResult Runner::run(const PipelineStep &step) const {
    if (!step.is_valid()) {
        throw std::invalid_argument{"Pipeline step must have a name and command"};
    }

    // skip run-step if data already existing for current step
    if (options_.skip_existing && step.skip_when_outputs_exist && step.outputs_exist()) {
        Logger::skip(step.name);

        return RunnerResult{.step_name = step.name, .exit_code = 0, .skipped = true};
    }

    Logger::step(step.name);
    Logger::command(step.command);

    // return early on dry-runs
    if (options_.dry_run) {
        return RunnerResult{.step_name = step.name, .exit_code = 0, .skipped = false};
    }

    const int exit_code = execute(step);

    if (exit_code != 0 && options_.stop_on_error) {
        throw std::runtime_error{"Pipeline step failed with exit code " +
                                 std::to_string(exit_code) + ": " + step.name};
    }

    return RunnerResult{.step_name = step.name, .exit_code = exit_code, .skipped = false};
}

std::vector<RunnerResult> Runner::run_all(const std::vector<PipelineStep> &steps) const {
    std::vector<RunnerResult> results{};
    results.reserve(steps.size());

    for (const auto &step : steps) {
        results.push_back(run(step));
    }

    return results;
}

int Runner::execute(const PipelineStep &step) {
    WorkingDirectoryGuard working_directory_guard{step.working_directory};

    const int status = std::system(step.command.c_str());

    return normalize_exit_code(status);
}

} // namespace bacpipe