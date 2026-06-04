#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace bacpipe {

struct ToolConfig {
    std::string executable;
    std::vector<std::string> extra_args;
};

struct PipelinePathConfig {
    std::string raw_reads{"data/raw/{barcode}"};
    std::string trimmed_reads{"data/trimmed/{barcode}_porechop"};

    std::string assembly_dir{"data/assembly/{barcode}_flye"};
    std::string assembly_fasta{"data/assembly/{barcode}_flye/assembly.fasta"};

    std::string circularization_dir{"data/circularized/{barcode}_circlator"};
    std::string circularization_reads_dir{"data/circularized/{barcode}_circlator/00_reads"};
    std::string combined_trimmed_reads{
        "data/circularized/{barcode}_circlator/00_reads/{barcode}.trimmed.combined.fastq.gz"};
    std::string circlator_output_dir{"data/circularized/{barcode}_circlator/01_circlator"};
    std::string circularized_fasta{
        "data/circularized/{barcode}_circlator/01_circlator/06.fixstart.fasta"};
    std::string circlator_circularize_log{
        "data/circularized/{barcode}_circlator/01_circlator/04.merge.circularise.log"};
};

struct PipelineConfig {
    std::string command;
    std::string barcode;

    std::uint32_t threads;
    std::filesystem::path project_root{"."};
    std::optional<std::filesystem::path> config_file;

    bool skip_existing{true};
    bool stop_on_error{true};

    std::vector<std::string> pipeline_steps{"trim", "assemble", "circularize"};
    PipelinePathConfig paths{};

    ToolConfig porechop{.executable = "porechop", .extra_args = {}};
    ToolConfig flye{.executable = "flye", .extra_args = {}};
    ToolConfig circlator{.executable = "circlator", .extra_args = {}};
};

} // namespace bacpipe
