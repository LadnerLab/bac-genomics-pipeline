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

struct AutocyclerConfig {
    std::string executable{"autocycler"};
    std::string genome_size{"auto"};
    std::string read_type{"ont_r10"};
    std::uint32_t subsample_count{4};
    std::vector<std::string> assemblers{"flye", "raven", "miniasm"};
    std::vector<std::string> subsample_extra_args{};
    std::vector<std::string> helper_extra_args{};
    std::vector<std::string> compress_extra_args{};
    std::vector<std::string> cluster_extra_args{};
    std::vector<std::string> trim_extra_args{};
    std::vector<std::string> resolve_extra_args{};
    std::vector<std::string> combine_extra_args{};
};

struct MedakaConfig {
    std::string executable{"medaka_consensus"};
    std::vector<std::string> extra_args{"--bacteria"};
};

struct PipelinePathConfig {
    std::string raw_reads{"data/raw/{barcode}"};
    std::string trimmed_reads{"data/trimmed/{barcode}_porechop"};

    std::string assembly_dir{"data/assembly/{barcode}_flye"};
    std::string assembly_fasta{"data/assembly/{barcode}_flye/assembly.fasta"};

    std::string combined_trimmed_fastq{
        "data/autocycler/{barcode}/reads/{barcode}.trimmed.combined.fastq.gz"};
    std::string autocycler_dir{"data/autocycler/{barcode}/autocycler_out"};
    std::string autocycler_subsampled_reads_dir{"data/autocycler/{barcode}/subsampled_reads"};
    std::string autocycler_input_assemblies_dir{"data/autocycler/{barcode}/assemblies"};
    std::string autocycler_consensus_fasta{
        "data/autocycler/{barcode}/autocycler_out/consensus_assembly.fasta"};
    std::string medaka_dir{"data/autocycler/{barcode}/medaka_consensus"};
    std::string medaka_consensus_fasta{
        "data/autocycler/{barcode}/medaka_consensus/consensus.fasta"};

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

    std::vector<std::string> pipeline_steps{"trim", "assemble", "circularize"};
    PipelinePathConfig paths{};

    ToolConfig porechop{.executable = "porechop", .extra_args = {}};
    ToolConfig flye{.executable = "flye", .extra_args = {}};
    ToolConfig circlator{.executable = "circlator", .extra_args = {}};
    AutocyclerConfig autocycler{};
    MedakaConfig medaka{};
};

} // namespace bacpipe
