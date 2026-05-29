#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>

struct PipelineConfig {
    std::string command{"help"};
    std::string barcode{};
    std::uint32_t threads{1};
    std::filesystem::path project_root{std::filesystem::current_path()};
};

class ThreadResolver {
  public:
    static std::uint32_t resolve_default() {
        // get number of SLURM-allocated threads
        if (const auto slurm_threads = read_positive_env("SLURM_CPUS_PER_TASK")) {
            return *slurm_threads;
        }

        // get number of hardware threads
        const auto hardware_threads = std::thread::hardware_concurrency();
        if (hardware_threads == 0) {
            return 1;
        }

        return hardware_threads;
    }

  private:
    static std::optional<std::uint32_t> read_positive_env(const char *name) {
        const char *value = std::getenv(name);
        if (value == nullptr) {
            return std::nullopt;
        }
        return parse_pos_int(value);
    }

    static std::optional<std::uint32_t> parse_pos_int(std::string_view text) {
        std::uint32_t result{};
        const auto *begin = text.data();
        const auto *end = text.data() + text.size();

        const auto [ptr, err] = std::from_chars(begin, end, result);
        if (err != std::errc{} || ptr != end || result == 0) {
            return std::nullopt;
        }
        return result;
    }
};

class Application {
  public:
    explicit Application(std::span<char *> args) : args_{args} {}

    int run() const {
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

  private:
    std::span<char *> args_;

    PipelineConfig parse_args() const {
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

    static bool is_known_command(std::string_view command) {
        return command == "trim" || command == "assemble" || command == "circularize" ||
               command == "run";
    }

    static void print_help() {
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

    static void print_run_summary(const PipelineConfig &config) {
        std::cout << "bac-genomics-pipeline configuration\n\n"
                  << "Command: " << config.command << '\n'
                  << "Barcode: " << config.barcode << '\n'
                  << "Threads: " << config.threads << '\n'
                  << "Project root: " << config.project_root.string() << '\n';
    }
};

int main(int argc, char *argv[]) {
    const Application app{std::span<char *>{argv, static_cast<std::size_t>(argc)}};
    return app.run();
}