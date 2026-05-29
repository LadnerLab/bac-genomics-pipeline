#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace bacpipe {

struct PipelineConfig {
    std::string command{"help"};
    std::string barcode{};
    std::uint32_t threads{1};
    std::filesystem::path project_root{std::filesystem::current_path()};
};

} // namespace bacpipe
