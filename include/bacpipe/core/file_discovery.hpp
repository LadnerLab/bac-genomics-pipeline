#pragma once

#include <filesystem>
#include <vector>

namespace bacpipe {

class FileDiscovery {
  public:
    [[nodiscard]] static std::vector<std::filesystem::path>
    find_fastq_files(const std::filesystem::path &directory);
};

} // namespace bacpipe
