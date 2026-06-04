#include "bacpipe/config/path_expander.hpp"

#include <string>

namespace {

void replace_all(std::string &text, std::string_view from, std::string_view to) {
    std::size_t position{0};
    while ((position = text.find(from, position)) != std::string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

} // namespace

namespace bacpipe {

std::filesystem::path expand_config_path(std::string_view path_template,
                                         const PipelineConfig &config) {
    std::string expanded{path_template};

    replace_all(expanded, "{barcode}", config.barcode);
    replace_all(expanded, "{project_root}", config.project_root.string());

    std::filesystem::path path{expanded};

    if (path.is_absolute()) {
        return path;
    }

    return config.project_root / path;
}

} // namespace bacpipe
