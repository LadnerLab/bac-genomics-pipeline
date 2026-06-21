# bac-genomics-pipeline

`bacpipe` is a C++ command-line orchestrator for assembling bacterial genomes from
barcoded Oxford Nanopore FASTQ reads. The current pipeline runs three stages:

1. adapter trimming with Porechop;
2. de novo assembly with Flye; and
3. read conversion with SeqKit followed by assembly circularization with Circlator.

Polishing and quality-control reporting are not implemented yet. Generated commands
use POSIX shell features, so the supported execution environment is Linux. The
primary deployment target is a Slurm-based HPC cluster (i.e., Monsoon).

## HPC quick start

### Prerequisites

- Conda (the cluster script loads the `anaconda3` module)
- Slurm for array execution
- CMake 3.20 or newer
- GCC 12 or another C++23-capable compiler
- vcpkg with the repository's `tomlplusplus` manifest dependency

### Create the runtime environment

The pipeline invokes four external programs at runtime. Create the environment on
the Linux cluster with the `conda-forge` and `bioconda` channels in strict-priority
mode:

```bash
module load anaconda3
source "$(conda info --base)/etc/profile.d/conda.sh"

conda create --name bacpipe \
  --channel conda-forge \
  --channel bioconda \
  --strict-channel-priority \
  python=3.10 porechop flye circlator seqkit

conda activate bacpipe
```

Confirm that every runtime executable is available (optional):

```bash
for tool in porechop flye circlator seqkit; do
  command -v "$tool" || exit 1
done
```

The package names above are documented by
[Bioconda](https://bioconda.github.io/index.html).

### Build `bacpipe`

The cluster-tested build uses GCC 12, CMake, and the lab vcpkg checkout:

```bash
module load gcc/12
module load cmake

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$(command -v g++)" \
  -DCMAKE_TOOLCHAIN_FILE=/projects/ladner_lab/bac_genomics/tools/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build build --parallel
./build/bacpipe help
```

See the [HPC build guide](docs/hpc-build-guide.md) for the normal update workflow
and reconfiguration guidance.  

Note: the repository is already cloned and built at `/projects/ladner_lab/bac_genomics/bac-genomics-pipeline` on Monsoon.

## Input layout and pipeline outputs

Each barcode must have its own directory containing regular `.fastq` or `.fastq.gz`
files.

```text
BARCODE_ROOT/
├── barcode01/
│   ├── reads-01.fastq.gz
│   └── reads-02.fastq.gz
└── barcode02/
    └── reads-01.fastq.gz
```

Paths and filenames are controlled by the TOML configuration:

| Stage | Input | Program | Expected output |
| --- | --- | --- | --- |
| `trim` | Raw FASTQ files | Porechop | One `*.trimmed.fastq.gz` per input file |
| `assemble` | Trimmed FASTQ files | Flye | `assembly_fasta` |
| `circularize` preparation | Trimmed FASTQ files | `seqkit fq2fa` | `combined_trimmed_reads` |
| `circularize` | Assembly plus converted reads | Circlator | `circularized_fasta` and `circlator_circularize_log` |

`seqkit fq2fa` writes FASTA content. The current example configuration gives
`combined_trimmed_reads` a `.fastq.gz` suffix despite that content format; downstream
usage should treat the file as FASTA.

## CLI usage

```text
./build/bacpipe <command> <barcode> [--config <path>] [--dry-run]
```

Commands:

| Command | Behavior |
| --- | --- |
| `trim` | Trim every discovered raw FASTQ file for one barcode. |
| `assemble` | Assemble all discovered trimmed reads for one barcode. |
| `circularize` | Convert trimmed reads and circularize an existing assembly. |
| `run` | Execute the configured pipeline steps in order. |

Use the positional `help` command to print built-in usage.

```bash
./build/bacpipe help
./build/bacpipe trim barcode05 --config bacpipe.toml --dry-run
./build/bacpipe assemble barcode05 --config bacpipe.toml
./build/bacpipe circularize barcode05 --config bacpipe.toml
./build/bacpipe run barcode05 --config bacpipe.toml
```

Supplying a configuration is strongly recommended because it selects the project
paths and tool arguments, including the Flye read mode used by this project.

### Runtime behavior

- `runtime.skip_existing = true` skips a step only when all of its expected outputs
  already exist. Trimming is evaluated independently for each input file.
- `runtime.stop_on_error = true` stops at the first nonzero tool exit. Setting it to
  `false` allows later steps to be attempted.
- `runtime.threads`, when set to a positive integer, takes precedence over
  `SLURM_CPUS_PER_TASK`. Without either value, hardware concurrency is used, falling
  back to one thread.
- `--dry-run` prints commands without executing them, but still discovers inputs,
  validates required directories and intermediate files, and honors
  `skip_existing`. A full dry run on a new barcode cannot synthesize the trimmed
  reads and assembly required by later stages; validate new data one stage at a
  time.

## Configuration

Adapt [bacpipe.toml](bacpipe.toml) for the target run. Its checked-in
paths are specific to the Ladner Lab cluster and should not be assumed to exist
elsewhere.

Active configuration sections are:

| Section | Keys | Purpose |
| --- | --- | --- |
| `[pipeline]` | `steps` | Ordered steps used by the `run` command. |
| `[project]` | `root` | Base directory and command working directory. |
| `[runtime]` | `threads`, `skip_existing`, `stop_on_error` | Resource and runner behavior. |
| `[paths]` | Input, intermediate, output, and log templates | Filesystem layout for each barcode. |
| `[tools.porechop]` | `executable`, `extra_args` | Porechop command customization. |
| `[tools.flye]` | `executable`, `extra_args` | Flye command customization. |
| `[tools.circlator]` | `executable`, `extra_args` | Circlator command customization. |

Single-stage CLI commands run their requested stage directly; `[pipeline].steps`
only controls `run`. The top-level `version` value is currently informational and
is not validated by the loader.

Path templates support `{barcode}` and `{project_root}`. Absolute paths are used as
written. Relative paths are resolved beneath `[project].root`, which is also the
working directory used to launch each external command.

For example:

```toml
[pipeline]
steps = ["trim", "assemble", "circularize"]

[project]
root = "/projects/ladner_lab/bac_genomics/fastq_pass"

[runtime]
threads = 16
skip_existing = true
stop_on_error = true

[paths]
raw_reads = "{project_root}/{barcode}"
trimmed_reads = "{project_root}/trimmed/{barcode}_porechop"

[tools.flye]
executable = "flye"
extra_args = ["--nano-hq"]
```

The complete example contains every path required by assembly and circularization.

## Slurm array execution

Run the wrapper from a login node. With no barcode selection it discovers immediate
`barcode*` directories under `BARCODE_ROOT`, sorts them naturally, and submits one
array task per barcode:

```bash
./scripts/run-bacpipe-array.sh
```

Common overrides:

```bash
# validate the configured pipeline without executing tool commands
DRY_RUN=1 ./scripts/run-bacpipe-array.sh

# run one stage for selected barcodes
BARCODES_TO_RUN="barcode01 barcode02" \
  COMMAND=trim \
  ./scripts/run-bacpipe-array.sh

# change the Slurm allocation and concurrency limit
CPUS_PER_TASK=16 \
  MEMORY=64G \
  TIME_LIMIT=06:00:00 \
  MAX_CONCURRENT_JOBS=2 \
  ./scripts/run-bacpipe-array.sh
```

Explicit values in `BARCODES_TO_RUN` must match `barcode[0-9]+` and must already
exist beneath `BARCODE_ROOT`.

### Wrapper variables and actual defaults

| Variable | Default | Purpose |
| --- | --- | --- |
| `PROJECT_ROOT` | `/projects/ladner_lab/bac_genomics/fastq_pass` | Directory used by the worker as its current directory. |
| `BARCODE_ROOT` | `/projects/ladner_lab/bac_genomics/fastq_pass` | Directory scanned for barcode folders. |
| `BACPIPE_BIN` | `/projects/ladner_lab/bac_genomics/bac-genomics-pipeline/build/bacpipe` | Executable invoked through `srun`. |
| `CONFIG_FILE` | `/projects/ladner_lab/bac_genomics/bac-genomics-pipeline/bacpipe.toml` | TOML file passed with `--config` when it exists. |
| `BARCODES_TO_RUN` | empty | Space-separated subset; empty means all discovered barcodes. |
| `COMMAND` | `run` | CLI command executed for each barcode. |
| `DRY_RUN` | `0` | Set to `1` to append `--dry-run`. |
| `CONDA_ENV` | `bacpipe` | Environment activated in each worker. |
| `MAX_CONCURRENT_JOBS` | `4` | Maximum simultaneously running array tasks. |
| `CPUS_PER_TASK` | `16` | CPUs requested per task and exposed to `bacpipe`. |
| `MEMORY` | `32G` | Memory requested per task. |
| `TIME_LIMIT` | `03:00:00` | Wall time requested per task. |

Slurm output and error logs use:

```text
/scratch/$USER/bacpipe_slurm/output/bacpipe_<job>_<task>.out
/scratch/$USER/bacpipe_slurm/error/bacpipe_<job>_<task>.err
```

The table above reflects the variables currently assigned by the script.

## Troubleshooting

- **A tool is missing:** activate `bacpipe` and check all four executables with the
  verification loop in the quick start. The wrapper currently prints checks for
  Porechop, Flye, and Circlator but not SeqKit.
- **An input directory or FASTQ file is not found:** confirm the selected barcode,
  `[project].root`, and `[paths]` templates. Files in nested subdirectories are not
  discovered.
- **A step is skipped:** all configured expected outputs already exist and
  `skip_existing` is enabled. Review those files before intentionally rerunning the
  underlying tool. Ensure if `skip_existing` is set to `true`, you remove old directories
  before running the step again.
- **A full dry run fails on new data:** run `trim`, `assemble`, and `circularize` dry
  runs separately as their prerequisites become available.
- **A barcode is rejected by the wrapper:** explicit selections must use the
  `barcodeNN` form and refer to directories directly beneath `BARCODE_ROOT`.
