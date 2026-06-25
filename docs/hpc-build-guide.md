# HPC Build Notes for `bacpipe`

## Purpose

Build `bacpipe` directly on the HPC so the executable matches the cluster compiler, runtime libraries, and dependency environment.

## Load Modules

```bash
module load gcc/12
module load cmake
```

Verify:

```bash
g++ --version
cmake --version
```

Tested setup:

```text
g++ 12.2.0
cmake 3.24.3
```

## First-Time Configure

From the repository root:

```bash
cd /projects/ladner_lab/bac_genomics/bac-genomics-pipeline

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$(which g++)" \
  -DCMAKE_TOOLCHAIN_FILE=/projects/ladner_lab/bac_genomics/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Build

```bash
cmake --build build --parallel
```

Executable location:

```bash
./build/bacpipe
```

## Normal Update Workflow

Use this after new changes are pushed to `main`:

```bash
cd /projects/ladner_lab/bac_genomics/bac-genomics-pipeline

module load gcc/12
module load cmake

git pull origin main
cmake --build build --parallel
```

## Reconfigure From Scratch

Only needed if `CMakeLists.txt`, compiler modules, dependencies, build type, or the vcpkg toolchain path changes.

```bash
rm -rf build

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$(which g++)" \
  -DCMAKE_TOOLCHAIN_FILE=/projects/ladner_lab/bac_genomics/tools/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build build --parallel
```

## Quick Test

```bash
./build/bacpipe help
./build/bacpipe trim barcode05 --config bacpipe.toml --dry-run
```

Run `autocycler_assemble`, `medaka_polish`, or the full configured `run` dry run
only after the prerequisite outputs for those stages exist.

## Notes

Do not commit the `build/` directory.

`tomlplusplus` is resolved externally through the HPC `vcpkg` installation, not vendored into the repository.
