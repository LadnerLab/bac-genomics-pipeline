#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bacpipe {

struct PipelineStep {
    std::string name{};
    std::string command{};
    std::filesystem::path working_directory{std::filesystem::current_path()};
    std::vector<std::filesystem::path> expected_outputs{};
    bool skip_when_outputs_exist{true};

    [[nodiscard]] bool is_valid() const noexcept;
    [[nodiscard]] bool outputs_exist() const;
};

} // namespace bacpipe
