#include "bacpipe/config/config_loader.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <toml++/toml.hpp>
#include <vector>

namespace {

void read_string(const toml::table &table, std::string_view key, std::string &target) {
    if (const auto value = table[key].value<std::string>()) {
        target = *value;
    }
}

void read_path(const toml::table &table, std::string_view key, std::filesystem::path &target) {
    if (const auto value = table[key].value<std::string>()) {
        target = *value;
    }
}

void read_bool(const toml::table &table, std::string_view key, bool &target) {
    if (const auto value = table[key].value<bool>()) {
        target = *value;
    }
}

void read_threads(const toml::table &table, std::uint32_t &target) {
    const auto value = table["threads"].value<std::int64_t>();

    if (!value)
        return;
    if (*value <= 0)
        return;
    if (*value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error{"runtime.threads is too large"};
    }

    target = static_cast<std::uint32_t>(*value);
}

void read_uint32(const toml::table &table, std::string_view key, std::uint32_t &target) {
    const auto value = table[key].value<std::int64_t>();

    if (!value)
        return;
    if (*value <= 0) {
        throw std::runtime_error{std::string{key} + " must be greater than zero"};
    }
    if (*value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error{std::string{key} + " is too large"};
    }

    target = static_cast<std::uint32_t>(*value);
}

std::optional<std::vector<std::string>> read_string_array(const toml::table &table,
                                                          std::string_view key) {
    const toml::array *array = table[key].as_array();

    if (array == nullptr)
        return std::nullopt;

    std::vector<std::string> values{};
    values.reserve(array->size());

    for (const toml::node &node : *array) {
        const auto value = node.value<std::string>();

        if (!value) {
            throw std::runtime_error{"Expected string array for key: " + std::string{key}};
        }

        values.push_back(*value);
    }

    return values;
}

void read_tool_config(const toml::table &table, bacpipe::ToolConfig &tool) {
    read_string(table, "executable", tool.executable);
    read_string(table, "genome_size", tool.genome_size);
    read_string(table, "read_type", tool.read_type);
    read_uint32(table, "subsample_count", tool.subsample_count);

    if (auto extra_args = read_string_array(table, "extra_args")) {
        tool.extra_args = *extra_args;
    }
    if (auto assemblers = read_string_array(table, "assemblers")) {
        tool.assemblers = *assemblers;
    }
    if (auto extra_args = read_string_array(table, "subsample_extra_args")) {
        tool.subsample_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "helper_extra_args")) {
        tool.helper_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "compress_extra_args")) {
        tool.compress_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "cluster_extra_args")) {
        tool.cluster_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "trim_extra_args")) {
        tool.trim_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "resolve_extra_args")) {
        tool.resolve_extra_args = *extra_args;
    }
    if (auto extra_args = read_string_array(table, "combine_extra_args")) {
        tool.combine_extra_args = *extra_args;
    }
}

} // namespace

namespace bacpipe {

PipelineConfig ConfigLoader::load(const std::filesystem::path &config_file, PipelineConfig config) {
    toml::table root;

    try {
        root = toml::parse_file(config_file.string());
    } catch (const toml::parse_error &error) {
        throw std::runtime_error{"Failed to parse config TOML file: " + std::string{error.what()}};
    }

    config.config_file = config_file;

    // parse project section
    if (const auto *project = root["project"].as_table()) {
        read_path(*project, "root", config.project_root);
    }

    // parse runtime section
    if (const auto *runtime = root["runtime"].as_table()) {
        read_threads(*runtime, config.threads);
        read_bool(*runtime, "skip_existing", config.skip_existing);
        read_bool(*runtime, "stop_on_error", config.stop_on_error);
    }

    // parse pipeline section
    if (const auto *pipeline = root["pipeline"].as_table()) {
        if (auto steps = read_string_array(*pipeline, "steps")) {
            config.pipeline_steps = *steps;
        }
    }

    // parse paths
    if (const auto *paths = root["paths"].as_table()) {
        read_string(*paths, "raw_reads", config.paths.raw_reads);
        read_string(*paths, "trimmed_reads", config.paths.trimmed_reads);
        read_string(*paths, "assembly_dir", config.paths.assembly_dir);
        read_string(*paths, "assembly_fasta", config.paths.assembly_fasta);

        read_string(*paths, "combined_trimmed_fastq", config.paths.combined_trimmed_fastq);
        read_string(*paths,
                    "assembly_subsampled_reads_dir",
                    config.paths.assembly_subsampled_reads_dir);
        read_string(*paths,
                    "assembly_input_assemblies_dir",
                    config.paths.assembly_input_assemblies_dir);
        read_string(*paths, "polish_dir", config.paths.polish_dir);
        read_string(*paths, "polished_fasta", config.paths.polished_fasta);

        read_string(*paths, "circularization_dir", config.paths.circularization_dir);
        read_string(*paths, "circularization_reads_dir", config.paths.circularization_reads_dir);
        read_string(*paths, "combined_trimmed_reads", config.paths.combined_trimmed_reads);
        read_string(*paths, "circlator_output_dir", config.paths.circlator_output_dir);
        read_string(*paths, "circularized_fasta", config.paths.circularized_fasta);
        read_string(*paths, "circlator_circularize_log", config.paths.circlator_circularize_log);
    }

    // parse tools
    if (const auto *tools = root["tools"].as_table()) {
        if (const auto *porechop = (*tools)["porechop"].as_table()) {
            read_tool_config(*porechop, config.porechop);
        }

        if (const auto *autocycler = (*tools)["autocycler"].as_table()) {
            read_tool_config(*autocycler, config.autocycler);
        }

        if (const auto *medaka = (*tools)["medaka"].as_table()) {
            read_tool_config(*medaka, config.medaka);
        }

        if (const auto *circlator = (*tools)["circlator"].as_table()) {
            read_tool_config(*circlator, config.circlator);
        }
    }

    return config;
}

} // namespace bacpipe
