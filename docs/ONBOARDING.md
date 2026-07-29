# Onboarding `bacpipe`

`bacpipe` orchestrates bacterial genome assembly from barcoded Oxford Nanopore
FASTQ reads. It is designed for Linux and is normally run on the Monsoon Slurm
cluster. This guide gets a new researcher to a safe first run and gives a new
developer enough context to navigate the codebase.

For the complete operational reference, see the [README](../README.md). For
cluster compiler and vcpkg details, see the [HPC build guide](hpc-build-guide.md).

## Before you start

You need a Linux environment with CMake 3.20 or later, a C++23-capable compiler,
and vcpkg providing `tomlplusplus`. Pipeline runs also require Conda packages for
Porechop, Autocycler and its helper assemblers, Medaka, and, when circularizing,
Circlator and SeqKit.

The repository includes two configuration examples:

- `bacpipe.toml` contains Ladner Lab cluster paths and is the configuration used
  by the Slurm wrapper by default.
- `bacpipe.local.toml` uses repository-local `data/` paths and is appropriate for
  local development or a small test run after its settings have been reviewed.

Do not assume the cluster paths in `bacpipe.toml` exist outside Monsoon. Keep raw
reads and generated outputs under an ignored data directory; do not add them to
Git.

## Researcher first run

1. Build the executable using the commands in the README or the HPC build guide.
2. Place each sample's reads in a direct barcode directory. Only regular
   `.fastq` and `.fastq.gz` files directly inside that directory are discovered.

   ```text
   data/
   `-- barcode05/
       |-- reads-01.fastq.gz
       `-- reads-02.fastq.gz
   ```

3. Review `bacpipe.local.toml`. Set `[project].root`, the `[paths]` templates,
   and tool arguments for the environment where the run will execute. Relative
   paths are resolved below `[project].root`.
4. Confirm the planned commands before running tools:

   ```bash
   ./build/bacpipe trim barcode05 --config bacpipe.local.toml --dry-run
   ./build/bacpipe run barcode05 --config bacpipe.local.toml --dry-run
   ```

   Dry-run still verifies input directories and discovers FASTQ files, but it
   does not create outputs or execute external tools. A full dry run requires
   trimmed reads to already be discoverable before it can plan assembly.
5. Run an individual stage or the configured sequence after reviewing the
   generated commands:

   ```bash
   ./build/bacpipe run barcode05 --config bacpipe.local.toml
   ```

The default configured sequence is `trim`, `assemble`, then `polish`. Use
`circularize` explicitly only when an assembly and the Circlator reference
configuration are ready. `skip_existing = true` prevents re-running stages whose
expected outputs already exist; inspect or remove obsolete outputs before a
deliberate rerun.

## Running on Monsoon

Use the checked-in `bacpipe.toml` only after confirming its project paths and
tool options are correct for the run. Submit barcode arrays from a login node:

```bash
./scripts/run-bacpipe-array.sh
```

For a non-executing validation submission, use:

```bash
DRY_RUN=1 ./scripts/run-bacpipe-array.sh
```

The wrapper discovers direct `barcode*` directories, submits one array task per
barcode, activates the `bacpipe` Conda environment, and invokes `bacpipe` through
`srun`. Review the README before changing resource requests, selecting barcodes,
or using a custom configuration file.

## Developer orientation

The project builds one C++23 executable, `bacpipe`, with CMake and the
`tomlplusplus` dependency. A debug-oriented VS Code configuration is included in
`.vscode/` for Linux or WSL environments.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/tools/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --parallel
./build/bacpipe help
```

The main implementation areas are:

| Area | Responsibility |
| --- | --- |
| `src/application.cpp` | Parses CLI arguments, loads configuration, and dispatches commands. |
| `src/config/` | Loads TOML values and expands `{barcode}` and `{project_root}` path templates. |
| `src/core/` | Discovers reads, resolves threads, quotes shell arguments, and executes steps. |
| `src/pipeline/` | Builds the Porechop, Autocycler, Medaka, and Circlator command sequences. |
| `include/bacpipe/` | Public project headers grouped by the same areas. |

Use `--dry-run` while changing command construction or configuration behavior to
inspect the exact POSIX shell commands without running external bioinformatics
tools. The repository currently has no automated test suite; validate changes by
building the executable and exercising the affected command with representative
local data or dry-run input directories.

## Working conventions

- Keep generated builds, FASTQ files, assemblies, and logs out of Git; the
  repository's `.gitignore` already covers the normal locations and extensions.
- Prefer configuration changes over hard-coded path or tool changes when adapting
  a run to another project or cluster.
- Preserve the direct-child barcode input model when changing file discovery or
  documentation: nested FASTQ files are intentionally not included.
- Update the README and this guide whenever a user-facing command, configuration
  key, runtime dependency, or expected pipeline output changes.
