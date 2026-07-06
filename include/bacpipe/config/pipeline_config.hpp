#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace bacpipe {

struct ToolConfig {
    std::string executable{};
    std::vector<std::string> extra_args{};

    std::string genome_size{};
    std::string read_type{};
    std::uint32_t subsample_count{0};
    std::vector<std::string> assemblers{};
    std::vector<std::string> subsample_extra_args{};
    std::vector<std::string> helper_extra_args{};
    std::vector<std::string> compress_extra_args{};
    std::vector<std::string> cluster_extra_args{};
    std::vector<std::string> trim_extra_args{};
    std::vector<std::string> resolve_extra_args{};
    std::vector<std::string> combine_extra_args{};
};

struct PipelinePathConfig {
    std::string raw_reads{"data/raw/{barcode}"};
    std::string trimmed_reads{"data/trimmed/{barcode}_porechop"};

    std::string assembly_dir{"data/assembly/{barcode}_autocycler"};
    std::string assembly_fasta{"data/assembly/{barcode}_autocycler/consensus_assembly.fasta"};

    std::string combined_trimmed_fastq{
        "data/assembly/{barcode}_autocycler/reads/{barcode}.trimmed.combined.fastq.gz"};
    std::string assembly_subsampled_reads_dir{
        "data/assembly/{barcode}_autocycler/subsampled_reads"};
    std::string assembly_input_assemblies_dir{"data/assembly/{barcode}_autocycler/assemblies"};
    std::string polish_dir{"data/polish/{barcode}_medaka"};
    std::string polished_fasta{"data/polish/{barcode}_medaka/consensus.fasta"};

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
    bool dry_run{false};

    std::vector<std::string> pipeline_steps{"trim", "assemble", "polish"};
    PipelinePathConfig paths{};

    ToolConfig porechop{.executable = "porechop", .extra_args = {}};
    ToolConfig autocycler{.executable = "autocycler",
                          .extra_args = {},
                          .genome_size = "auto",
                          .read_type = "ont_r10",
                          .subsample_count = 4,
                          .assemblers = {"flye", "raven", "miniasm"},
                          .subsample_extra_args = {},
                          .helper_extra_args = {"--min_depth_rel", "0.1"},
                          .compress_extra_args = {},
                          .cluster_extra_args = {},
                          .trim_extra_args = {},
                          .resolve_extra_args = {},
                          .combine_extra_args = {}};
    ToolConfig medaka{.executable = "medaka_consensus", .extra_args = {"--bacteria"}};
    ToolConfig circlator{.executable = "circlator", .extra_args = {}};
};

} // namespace bacpipe
