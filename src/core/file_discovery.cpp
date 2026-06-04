#include "bacpipe/core/file_discovery.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace {

bool has_fastq_extension(const std::filesystem::path &path) {
    const std::string filename = path.filename().string();

    return filename.ends_with(".fastq") || filename.ends_with(".fastq.gz");
}

} // namespace

namespace bacpipe {

std::vector<std::filesystem::path>
FileDiscovery::find_fastq_files(const std::filesystem::path &directory) {
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error{"Input directory does not exist: " + directory.string()};
    }

    if (!std::filesystem::is_directory(directory)) {
        throw std::runtime_error{"Input directory is not a directory: " + directory.string()};
    }

    std::vector<std::filesystem::path> files{};
    for (const auto &entry : std::filesystem::directory_iterator{directory}) {
        if (!entry.is_regular_file()) {
            continue;
        }

        // append all files with .fastq or .fastq.gz extensions only
        if (has_fastq_extension(entry.path())) {
            files.push_back(entry.path());
        }
    }

    // sort files in ascending order
    std::ranges::sort(files, [](const auto &left, const auto &right) {
        return left.string() < right.string();
    });

    if (files.empty()) {
        throw std::runtime_error{"No FASTQ files found in: " + directory.string()};
    }

    return files;
}

} // namespace bacpipe
