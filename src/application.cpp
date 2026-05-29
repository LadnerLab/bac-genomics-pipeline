#include "bacpipe/application.hpp"
#include "bacpipe/core/thread_resolver.hpp"

#include <iostream>

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

    return 0;
}

PipelineConfig Application::parse_args() const {
    PipelineConfig config;
    config.threads = ThreadResolver::resolve_default();

    // likely "help"
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
