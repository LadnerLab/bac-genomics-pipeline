#include "bacpipe/application.hpp"

#include "bacpipe/core/runner.hpp"
#include "bacpipe/core/thread_resolver.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include <iostream>
#include <vector>

namespace {

std::vector<bacpipe::PipelineStep> build_pipeline_steps(const bacpipe::PipelineConfig &config) {
    const std::string barcode = config.barcode;
    const std::filesystem::path project_root = config.project_root;

    if (config.command == "trim") {
        return {bacpipe::PipelineStep{.name = "Trim reads with Porechop",
                                      .command = "echo Porechop would run for " + barcode,
                                      .working_directory = project_root}};
    }

    if (config.command == "assemble") {
        return {bacpipe::PipelineStep{.name = "Assemble reads with Flye",
                                      .command = "echo Flye would run for " + barcode,
                                      .working_directory = project_root}};
    }

    if (config.command == "circularize") {
        return {bacpipe::PipelineStep{.name = "Circularize trimmed assembly with Circlator",
                                      .command = "echo Circlator would run for " + barcode,
                                      .working_directory = project_root}};
    }

    if (config.command == "run") {
        return {bacpipe::PipelineStep{.name = "Trim reads with Porechop",
                                      .command = "echo porechop would run for " + barcode,
                                      .working_directory = project_root},
                bacpipe::PipelineStep{.name = "Assemble reads with Flye",
                                      .command = "echo flye would run for " + barcode,
                                      .working_directory = project_root},
                bacpipe::PipelineStep{.name = "Circularize assembly with Circlator",
                                      .command = "echo circlator would run for " + barcode,
                                      .working_directory = project_root}};
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
        std::cerr << "Unknown command: " << config.command << '\n';
        print_help();
        return 1;
    }

    if (config.barcode.empty()) {
        std::cerr << "Missing barcode argument\n";
        print_help();
        return 1;
    }

    print_run_summary(config);

    const std::vector<PipelineStep> steps = build_pipeline_steps(config);

    if (steps.empty()) {
        std::cerr << "No pipeline steps were built" << std::endl;
        return 1;
    }

    const Runner runner{
        RunnerOptions{.dry_run = true, .skip_existing = true, .stop_on_error = true}};

    runner.run_all(steps);

    return 0;
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
    std::cout << "bac-genomics-pipeline configuration\n\n"
              << "Command: " << config.command << '\n'
              << "Barcode: " << config.barcode << '\n'
              << "Threads: " << config.threads << '\n'
              << "Project root: " << config.project_root.string() << '\n';
}

} // namespace bacpipe
