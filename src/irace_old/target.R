#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)

# -------------------- Check enough arguments --------------------
if (length(args) < 4) {
  stop("Not enough arguments: irace should pass at least 4 arguments (run_id, seed, experiment, instance_file)")
}

# -------------------- Positional arguments --------------------
# Skip irace system arguments: run_id, seed, experiment
instance_file <- args[4]

# Remaining arguments (flags) for optional parameters
optional_args <- if (length(args) > 4) args[5:length(args)] else character(0)

# -------------------- Helper to get optional flags --------------------
get_arg <- function(flag, default=NULL) {
  idx <- which(optional_args == flag)
  if (length(idx) > 0 && idx < length(optional_args)) {
    return(optional_args[idx + 1])
  }
  return(default)
}

# -------------------- Required flags for weights --------------------
K1 <- as.integer(get_arg("--K1"))
K2 <- as.integer(get_arg("--K2"))
K3 <- as.integer(get_arg("--K3"))

if (any(is.na(c(K1, K2, K3)))) {
  stop("Error: --K1, --K2, and --K3 must be provided and numeric")
}

# -------------------- Optional parameters --------------------
max_it      <- get_arg("--max_it", "50")
max_no_imp  <- get_arg("--max_no_imp", "5")
acceptance  <- get_arg("--acceptance", "BEST")
improvement <- get_arg("--improvement", "BI")
use_ucb     <- get_arg("--use_ucb", "1")
c           <- get_arg("--c", "0.25")
builder     <- get_arg("--builder", "MFFD")
beta        <- get_arg("--beta", "0.3")
use_qrvnd   <- get_arg("--use_qrvnd", "1")
alpha       <- get_arg("--alpha", "0.1")
gamma       <- get_arg("--gamma", "0.9")
epsilon     <- get_arg("--epsilon", "0.1")
time_limit  <- get_arg("--time_limit", "3600")
verbose     <- get_arg("--verbose", "0")

# -------------------- Resolve instance path --------------------
if (grepl("^t/", instance_file)) {
  # Instances starting with "t/" go into t/MIMT subfolder
  instance_file <- file.path(
    "../../instances/BPPC_test_instances/BPPC",
    "t", "MIMT", paste0(sub("^t/", "", instance_file), ".txt")
  )
} else if (grepl("^u/", instance_file)) {
  # Instances starting with "u/" go into u/MIMT subfolder
  instance_file <- file.path(
    "../../instances/BPPC_test_instances/BPPC",
    "u", "MIMT", paste0(sub("^u/", "", instance_file), ".txt")
  )
} else {
  # All other instances directly in BPPC folder
  instance_file <- file.path(
    "../../instances/BPPC_test_instances/BPPC",
    paste0(instance_file, ".txt")
  )
}

# -------------------- Check instance file exists --------------------
if (!file.exists(instance_file)) {
  stop("Error: instance file not found: ", instance_file)
}

# -------------------- Build command for ails_runner --------------------
ais_binary <- "../bin/ails_runner"

cmd_args <- c(
  instance_file,
  K1, K2, K3,
  "--max_it", max_it,
  "--max_no_imp", max_no_imp,
  "--acceptance", acceptance,
  "--improvement", improvement,
  "--use_ucb", use_ucb,
  "--c", c,
  "--builder", builder,
  "--beta", beta,
  "--use_qrvnd", use_qrvnd,
  "--alpha", alpha,
  "--gamma", gamma,
  "--epsilon", epsilon,
  "--time_limit", time_limit,
  "--verbose", verbose
)

# -------------------- Run AILS --------------------
res <- tryCatch(
  system2(ais_binary, args = cmd_args, stdout = TRUE, stderr = TRUE),
  error = function(e) stop("Failed to run ails_runner: ", e$message)
)

# -------------------- Parse numeric result strictly --------------------
numeric_res <- suppressWarnings(as.numeric(res))
numeric_res <- numeric_res[!is.na(numeric_res)]

if (length(numeric_res) != 1) {
  stop("Error: ails_runner returned non-numeric or multiple outputs:\n",
       paste(res, collapse = "\n"))
}

score <- numeric_res

# -------------------- Output strictly for irace --------------------
cat(score, "\n")