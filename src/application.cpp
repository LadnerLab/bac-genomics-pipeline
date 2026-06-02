#include "bacpipe/application.hpp"

#include "bacpipe/core/logger.hpp"
#include "bacpipe/core/runner.hpp"
#include "bacpipe/core/thread_resolver.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include "bacpipe/pipeline/assemble.hpp"
#include "bacpipe/pipeline/trim.hpp"

#include <exception>
#include <iostream>
#include <vector>

namespace {

std::vector<bacpipe::PipelineStep> build_pipeline_steps(const bacpipe::PipelineConfig &config) {
    const std::string barcode = config.barcode;
    const std::filesystem::path project_root = config.project_root;

    if (config.command == "trim") {
        return bacpipe::build_trim_steps(config);
    }

    if (config.command == "assemble") {
        return bacpipe::build_assemble_steps(config);
    }

    if (config.command == "circularize") {
        return {bacpipe::PipelineStep{.name = "Circularize trimmed assembly with Circlator",
                                      .command = "echo Circlator would run for " + barcode,
                                      .working_directory = project_root}};
    }

    if (config.command == "run") {
        std::vector<bacpipe::PipelineStep> steps = bacpipe::build_trim_steps(config);
        const std::vector<bacpipe::PipelineStep> assemble_steps =
            bacpipe::build_assemble_steps(config);
        steps.insert(steps.end(), assemble_steps.begin(), assemble_steps.end());
        steps.push_back(bacpipe::PipelineStep{.name = "Circularize assembly with Circlator",
                                              .command = "echo circlator would run for " + barcode,
                                              .working_directory = project_root});
        return steps;
    }

    return {};
}

} // namespace

namespace bacpipe {

Application::Application(std::span<char *> args) : args_{args} {}

int Application::run() const {
    const PipelineConfig config = parse_args();

    if (config.command == "help") {
        print_help();
        return 0;
    }

    if (!is_known_command(config.command)) {
        Logger::error("Unknown command: " + config.command);
        print_help();
        return 1;
    }

    if (config.barcode.empty()) {
        Logger::error("Missing barcode argument");
        print_help();
        return 1;
    }

    try {
        print_run_summary(config);

        const std::vector<PipelineStep> steps = build_pipeline_steps(config);

        if (steps.empty()) {
            Logger::error("No pipeline steps were built");
            return 1;
        }

        const Runner runner{
            RunnerOptions{.dry_run = true, .skip_existing = true, .stop_on_error = true}};

        const std::vector<bacpipe::RunnerResult> results = runner.run_all(steps);

        return 0;
    } catch (const std::exception &error) {
        Logger::error(error.what());
        return 1;
    }
}

PipelineConfig Application::parse_args() const {
    PipelineConfig config;
    config.threads = ThreadResolver::resolve_default();

    if (args_.size() >= 2) {
        config.command = args_[1];
    }

    // likely calling other command and passing barcode
    if (args_.size() >= 3) {
        config.barcode = args_[2];
    }

    return config;
}

bool Application::is_known_command(std::string_view command) {
    return command == "trim" || command == "assemble" || command == "circularize" ||
           command == "run";
}

void Application::print_help() {
    std::cout << "bac-genomics-pipeline\n\n"
              << "Usage:\n"
              << "  bacpipe trim <barcode>\n"
              << "  bacpipe assemble <barcode>\n"
              << "  bacpipe circularize <barcode>\n"
              << "  bacpipe run <barcode>\n\n"
              << "Examples:\n"
              << "  bacpipe trim barcode05\n"
              << "  bacpipe run barcode05\n";
}

void Application::print_run_summary(const PipelineConfig &config) {
    Logger::info("bac-genomics-pipeline configuration");
    Logger::info("Command: " + config.command);
    Logger::info("Barcode: " + config.barcode);
    Logger::info("Threads: " + std::to_string(config.threads));
    Logger::info("Project root: " + config.project_root.string());
}

} // namespace bacpipe
