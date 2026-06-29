#!/bin/bash

#SBATCH --job-name=bacpipe
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=32GB
#SBATCH --time=06:00:00
#SBATCH --output=/scratch/%u/bacpipe_slurm/output/%x_%A_%a.out
#SBATCH --error=/scratch/%u/bacpipe_slurm/error/%x_%A_%a.err

usage() {
  echo "Usage:"
  echo "  ./scripts/run_bacpipe_array.sh"
  echo ""
  echo "Optional environment variables:"
  echo "  PROJECT_ROOT          default: /projects/ladner_lab/bac_genomics/fastq_pass"
  echo "  BARCODE_ROOT          default: /projects/ladner_lab/bac_genomics/fastq_pass"
  echo "  BACPIPE_BIN           default: \${PROJECT_ROOT}/build/bacpipe"
  echo "  CONFIG_FILE           default: \${PROJECT_ROOT}/bacpipe.toml"
  echo "  COMMAND               default: run"
  echo "  BARCODES_TO_RUN       default: empty; space-separated subset, e.g. \"barcode11 barcode12\""
  echo "  DRY_RUN               default: 0"
  echo "  CONDA_ENV             default: bacpipe"
  echo "  MAX_CONCURRENT_JOBS   default: 4"
  echo "  CPUS_PER_TASK         default: 8"
  echo "  MEMORY                default: 32G"
  echo "  TIME_LIMIT            default: 24:00:00"
  echo ""
  echo "Examples:"
  echo "  ./scripts/run_bacpipe_array.sh"
  echo "  DRY_RUN=1 ./scripts/run_bacpipe_array.sh"
  echo "  COMMAND=trim DRY_RUN=1 ./scripts/run_bacpipe_array.sh"
  echo "  CPUS_PER_TASK=8 MEMORY=32G MAX_CONCURRENT_JOBS=2 ./scripts/run_bacpipe_array.sh"
  echo "  BARCODES_TO_RUN=\"barcode11 barcode12 barcode13\" COMMAND=run ./scripts/run-bacpipe-array.sh"
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

PROJECT_ROOT="${PROJECT_ROOT:-/projects/ladner_lab/bac_genomics/fastq_pass}"
BARCODE_ROOT="${BARCODE_ROOT:-/projects/ladner_lab/bac_genomics/fastq_pass}"
BACPIPE_BIN="${BACPIPE_BIN:-/projects/ladner_lab/bac_genomics/bac-genomics-pipeline/build/bacpipe}"
CONFIG_FILE="${CONFIG_FILE:-/projects/ladner_lab/bac_genomics/bac-genomics-pipeline/bacpipe.toml}"

BARCODES_TO_RUN="${BARCODES_TO_RUN:-}"
COMMAND="${COMMAND:-run}"
DRY_RUN="${DRY_RUN:-0}"
CONDA_ENV="${CONDA_ENV:-bacpipe}"

MAX_CONCURRENT_JOBS="${MAX_CONCURRENT_JOBS:-4}"
CPUS_PER_TASK="${CPUS_PER_TASK:-8}"
MEMORY="${MEMORY:-32G}"
TIME_LIMIT="${TIME_LIMIT:-03:00:00}"

SLURM_LOG_ROOT="${SLURM_LOG_ROOT:-/scratch/$USER/bacpipe_slurm}"
SLURM_OUT_DIR="${SLURM_LOG_ROOT}/output"
SLURM_ERR_DIR="${SLURM_LOG_ROOT}/error"

load_barcodes() {
  BARCODES=()

  if [ -n "${BARCODES_TO_RUN}" ]; then
    for barcode in ${BARCODES_TO_RUN}; do
      if [[ ! "${barcode}" =~ ^barcode[0-9]+$ ]]; then
        echo "[bacpipe-error] invalid barcode name in BARCODES_TO_RUN: ${barcode}"
        exit 1
      fi

      if [ ! -d "${BARCODE_ROOT}/${barcode}" ]; then
        echo "[bacpipe-error] requested directory does not exist: ${BARCODE_ROOT}/${barcode}"
        exit 1
      fi

      BARCODES+=("${barcode}")
    done
  else
    mapfile -t BARCODES < <(
      find "${BARCODE_ROOT}" \
        -mindepth 1 \
        -maxdepth 1 \
        -type d \
        -name "barcode*" \
        -printf "%f\n" \
        | sort -V
    )
  fi
}

# --------------------------------------------------------------------
# SUBMIT MODE
# This runs when you execute the script directly from the login node
# It scans barcode folders, computes the array size, and submits itself
# i.e., ./run-bacpipe-array.sh
# --------------------------------------------------------------------

if [ -z "${SLURM_JOB_ID:-}" ]; then
  mkdir -p "${SLURM_OUT_DIR}" "${SLURM_ERR_DIR}"

  if [ ! -d "${BARCODE_ROOT}" ]; then
    echo "[bacpipe-submit-error] BARCODE_ROOT does not exist: ${BARCODE_ROOT}"
    exit 1  
  fi

  if [ ! -x "${BACPIPE_BIN}" ]; then
    echo "[bacpipe-submit-error] BACPIPE_BIN does not exist or is non-executable: ${BACPIPE_BIN}"
    exit 1
  fi

  load_barcodes || exit 1
  BARCODE_COUNT="${#BARCODES[@]}"

  if [ "${BARCODE_COUNT}" -eq 0 ]; then
    echo "[bacpipe-submit-error] no barcode directories found in: ${BARCODE_ROOT}"
    exit 1
  fi

  LAST_ARRAY_INDEX="$((BARCODE_COUNT - 1))"

  echo "[bacpipe-submit] project_root=${PROJECT_ROOT}"
  echo "[bacpipe-submit] barcode_root=${BARCODE_ROOT}"
  echo "[bacpipe-submit] barcodes=${BARCODES[*]}"
  echo "[bacpipe-submit] barcodes_to_run=${BARCODES_TO_RUN:-all}"
  echo "[bacpipe-submit] barcode_count=${BARCODE_COUNT}"
  echo "[bacpipe-submit] array=0-${LAST_ARRAY_INDEX}%${MAX_CONCURRENT_JOBS}"
  echo "[bacpipe-submit] cpus_per_task=${CPUS_PER_TASK}"
  echo "[bacpipe-submit] memory=${MEMORY}"
  echo "[bacpipe-submit] time_limit=${TIME_LIMIT}"
  echo "[bacpipe-submit] command=${COMMAND}"
  echo "[bacpipe-submit] dry_run=${DRY_RUN}"

  sbatch \
    --array="0-${LAST_ARRAY_INDEX}%${MAX_CONCURRENT_JOBS}" \
    --cpus-per-task="${CPUS_PER_TASK}" \
    --mem="${MEMORY}" \
    --time="${TIME_LIMIT}" \
    --export=ALL,PROJECT_ROOT="${PROJECT_ROOT}",BARCODE_ROOT="${BARCODE_ROOT}",BACPIPE_BIN="${BACPIPE_BIN}",CONFIG_FILE="${CONFIG_FILE}",COMMAND="${COMMAND}",DRY_RUN="${DRY_RUN}",CONDA_ENV="${CONDA_ENV}" \
    "$0"

  exit 0
fi

# --------------------------------------------------------------------
# WORKER MODE
# This runs inside each SLURM array task.
# This section loads the environment and calls bacpipe.
# i.e., runs on ONE array task: ./bacpipe run barcode03
# --------------------------------------------------------------------

mkdir -p "${SLURM_OUT_DIR}" "${SLURM_ERR_DIR}"

load_barcodes || exit 1
BARCODE_COUNT="${#BARCODES[@]}"
TASK_ID="${SLURM_ARRAY_TASK_ID:-0}"

if [ "${BARCODE_COUNT}" -eq 0 ]; then
  echo "[bacpipe-error] no barcode directories found in: ${BARCODE_ROOT}"
  exit 1
fi

if [ "${TASK_ID}" -ge "${BARCODE_COUNT}" ]; then
  echo "[bacpipe-error] SLURM_ARRAY_TASK_ID=${TASK_ID} is out of range for barcode_count=${BARCODE_COUNT}"
  exit 1
fi

BARCODE="${BARCODES[$TASK_ID]}"
BARCODE_DIR="${BARCODE_ROOT}/${BARCODE}"

echo "[bacpipe] job_id=${SLURM_JOB_ID}"
echo "[bacpipe] array_job_id=${SLURM_ARRAY_JOB_ID:-unset}"
echo "[bacpipe] array_task_id=${SLURM_ARRAY_TASK_ID}"
echo "[bacpipe] barcode_count=${BARCODE_COUNT}"
echo "[bacpipe] barcode=${BARCODE}"
echo "[bacpipe] barcode_dir=${BARCODE_DIR}"
echo "[bacpipe] barcodes=${BARCODES[*]}"
echo "[bacpipe] barcodes_to_run=${BARCODES_TO_RUN:-all}"
echo "[bacpipe] project_root=${PROJECT_ROOT}"
echo "[bacpipe] bacpipe_bin=${BACPIPE_BIN}"
echo "[bacpipe] config_file=${CONFIG_FILE}"
echo "[bacpipe] command=${COMMAND}"
echo "[bacpipe] dry_run=${DRY_RUN}"
echo "[bacpipe] conda_env=${CONDA_ENV}"
echo "[bacpipe] cpus_per_task=${SLURM_CPUS_PER_TASK:-unset}"
echo "[bacpipe] node_list=${SLURM_JOB_NODELIST:-unset}"

module purge
module load anaconda3
source $(conda info --base)/etc/profile.d/conda.sh
conda activate ${CONDA_ENV}

echo "[bacpipe] active_conda_env=${CONDA_DEFAULT_ENV:-unset}"
for tool in porechop flye autocycler medaka_consensus raven miniasm minimap2 minipolish circlator seqkit; do
  echo "[bacpipe] ${tool}=$(command -v "${tool}" || echo missing)"
done

BACPIPE_ARGS=(
  "${COMMAND}"
  "${BARCODE}"
)

if [ -f "${CONFIG_FILE}" ]; then
  BACPIPE_ARGS+=("--config" "${CONFIG_FILE}")
fi

if [ "${DRY_RUN}" -eq 1 ]; then
  BACPIPE_ARGS+=("--dry-run")
fi

cd "${PROJECT_ROOT}"

echo "[bacpipe] running: srun ${BACPIPE_BIN} ${BACPIPE_ARGS[*]}"

srun "${BACPIPE_BIN}" "${BACPIPE_ARGS[@]}"

echo "[bacpipe] done barcode=${BARCODE}"
