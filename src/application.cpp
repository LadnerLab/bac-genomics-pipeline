#include "bacpipe/application.hpp"

#include "bacpipe/config/config_loader.hpp"

#include "bacpipe/core/logger.hpp"
#include "bacpipe/core/runner.hpp"
#include "bacpipe/core/thread_resolver.hpp"
#include "bacpipe/pipeline/pipeline_step.hpp"

#include "bacpipe/pipeline/assemble.hpp"
#include "bacpipe/pipeline/autocycler.hpp"
#include "bacpipe/pipeline/circularize.hpp"
#include "bacpipe/pipeline/medaka.hpp"
#include "bacpipe/pipeline/trim.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::vector<bacpipe::RunnerResult> run_steps(const bacpipe::Runner &runner,
                                             const std::vector<bacpipe::PipelineStep> &steps) {
    if (steps.empty()) {
        throw std::runtime_error{"No pipeline steps were built"};
    }
    return runner.run_all(steps);
}

std::vector<bacpipe::RunnerResult> run_single_step(const bacpipe::PipelineConfig &config,
                                                   const bacpipe::Runner &runner,
                                                   std::string_view step_name) {
    if (step_name == "trim") {
        return run_steps(runner, bacpipe::build_trim_steps(config));
    }

    if (step_name == "assemble") {
        return run_steps(runner, bacpipe::build_assemble_steps(config));
    }

    if (step_name == "autocycler_assemble") {
        return run_steps(runner, bacpipe::build_autocycler_steps(config));
    }

    if (step_name == "medaka_polish") {
        return run_steps(runner, bacpipe::build_medaka_steps(config));
    }

    if (step_name == "circularize") {
        return run_steps(runner, bacpipe::build_circularize_steps(config));
    }

    throw std::runtime_error{"Unknown or unsupported pipeline step in configuration: " +
                             std::string{step_name}};
}

std::vector<bacpipe::RunnerResult> run_requested_pipeline(const bacpipe::PipelineConfig &config,
                                                          const bacpipe::Runner &runner) {
    std::vector<bacpipe::RunnerResult> results{};

    auto append_results = [&results](std::vector<bacpipe::RunnerResult> next_results) {
        results.insert(results.end(), next_results.begin(), next_results.end());
    };

    // step-by-step commands
    if (config.command == "trim" || config.command == "assemble" ||
        config.command == "autocycler_assemble" || config.command == "medaka_polish" ||
        config.command == "circularize") {
        append_results(run_single_step(config, runner, config.command));
        return results;
    }

    // full e2e pipeline
    if (config.command == "run") {
        for (const auto &step_name : config.pipeline_steps) {
            append_results(run_single_step(config, runner, step_name));
        }
        return results;
    }

    return results;
}

bool has_option(std::span<char *> args, std::string_view option) {
    std::size_t i;
    for (i = 1; i < args.size(); ++i) {
        if (std::string_view{args[i]} == option) {
            return true;
        }
    }
    return false;
}

bool is_option(std::string_view arg) {
    return arg.starts_with("--");
}

} // namespace

namespace bacpipe {

Application::Application(std::span<char *> args) : args_{args} {}

int Application::run() const {
    try {
        const PipelineConfig config = parse_args();

        if (config.command == "help") {
            print_help();
            return 0;
        }

        if (config.barcode.empty()) {
            Logger::error("Missing barcode argument, see help below:\n");
            print_help();
            return 1;
        }

        if (!is_known_command(config.command)) {
            Logger::error("Unknown command: '" + config.command + "', see help below:\n");
            print_help();
            return 1;
        }

        print_run_summary(config);
        Logger::info(std::string{"Dry run: "} + (config.dry_run ? "true" : "false"));

        const Runner runner{RunnerOptions{.dry_run = config.dry_run,
                                          .skip_existing = config.skip_existing,
                                          .stop_on_error = config.stop_on_error}};

        const std::vector<bacpipe::RunnerResult> results = run_requested_pipeline(config, runner);

        if (results.empty()) {
            Logger::error("No pipeline steps were ran");
            return 1;
        }

        Logger::info("Pipeline completed successfully");
        return 0;
    } catch (const std::exception &error) {
        Logger::error(error.what());
        return 1;
    }
}

PipelineConfig Application::parse_args() const {
    PipelineConfig config;
    config.threads = ThreadResolver::resolve_default();
    config.dry_run = has_option(args_, "--dry-run");

    std::optional<std::filesystem::path> config_file{};
    std::vector<std::string_view> positionals{};

    std::size_t i;
    for (i = 1; i < args_.size(); ++i) {
        const std::string_view arg{args_[i]};

        if (arg == "--dry-run") {
            continue;
        }

        if (arg == "--config") {
            if (i + 1 >= args_.size()) {
                throw std::runtime_error{"Missing file after --config"};
            }

            // grab file then skip path arg
            config_file = std::filesystem::path{args_[i + 1]};
            ++i;
            continue;
        }

        if (is_option(arg)) {
            throw std::runtime_error{"Unknown option: " + std::string{arg}};
        }

        positionals.push_back(arg);
    }

    if (config_file) {
        config = ConfigLoader::load(*config_file, config);
    }

    if (!positionals.empty()) {
        config.command = std::string{positionals[0]};
    }

    if (positionals.size() >= 2) {
        config.barcode = std::string{positionals[1]};
    }

    if (positionals.size() > 2) {
        throw std::runtime_error{"Too many positional arguments"};
    }

    return config;
}

bool Application::is_known_command(std::string_view command) {
    return command == "trim" || command == "assemble" || command == "autocycler_assemble" ||
           command == "medaka_polish" || command == "circularize" || command == "run";
}

void Application::print_help() {
    std::cout << "bac-genomics-pipeline\n\n"
              << "Usage:\n"
              << "  bacpipe trim <barcode> [--config <path>] [--dry-run]\n"
              << "  bacpipe assemble <barcode> [--config <path>] [--dry-run]\n"
              << "  bacpipe autocycler_assemble <barcode> [--config <path>] [--dry-run]\n"
              << "  bacpipe medaka_polish <barcode> [--config <path>] [--dry-run]\n"
              << "  bacpipe circularize <barcode> [--config <path>] [--dry-run]\n"
              << "  bacpipe run <barcode> [--config <path>] [--dry-run]\n\n"
              << "Options:\n"
              << "  --config <path>    Read pipeline settings from a TOML file\n"
              << "  --dry-run          Print commands without executing them\n\n"
              << "Examples:\n"
              << "  bacpipe trim barcode05 --dry-run\n"
              << "  bacpipe run barcode05 --config bacpipe.toml --dry-run\n";
}

void Application::print_run_summary(const PipelineConfig &config) {
    Logger::info("bac-genomics-pipeline configuration");
    Logger::info("Command: " + config.command);
    Logger::info("Barcode: " + config.barcode);
    Logger::info("Threads: " + std::to_string(config.threads));
    Logger::info("Project root: " + config.project_root.string());

    if (config.config_file) {
        Logger::info("Config file: " + config.config_file->string());
    }
}

} // namespace bacpipe
