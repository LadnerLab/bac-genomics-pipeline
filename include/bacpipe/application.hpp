#pragma once

#include "bacpipe/config/pipeline_config.hpp"

#include <span>
#include <string_view>

namespace bacpipe {

class Application {
  public:
    explicit Application(std::span<char *> args);

    int run() const;

  private:
    std::span<char *> args_;

    PipelineConfig parse_args() const;

    static bool is_known_command(std::string_view command);

    static void print_help();
    static void print_run_summary(const PipelineConfig &config);
};

} // namespace bacpipe
