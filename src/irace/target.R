#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)

# -------------------- Positional (from irace) --------------------
K1 <- as.integer(args[1])
K2 <- as.integer(args[2])
K3 <- as.integer(args[3])
instance_file <- args[4]

# -------------------- Optional parameters (flags from irace) --------------------
get_arg <- function(flag, default=NULL) {
  idx <- which(args == flag)
  if (length(idx) > 0 && idx < length(args)) {
    return(args[idx + 1])
  }
  return(default)
}

max_it      <- get_arg("--max_it", "50")
max_no_imp  <- get_arg("--max_no_imp", "5")
builder     <- get_arg("--builder", "MFFD")
beta        <- get_arg("--beta", "0.3")
use_qrvnd   <- get_arg("--use_qrvnd", "1")
alpha       <- get_arg("--alpha", "0.1")
gamma       <- get_arg("--gamma", "0.9")
epsilon     <- get_arg("--epsilon", "0.1")

ais_binary <- "../bin/ails_runner"

# -------------------- Resolve instance path --------------------
if (grepl("^t/", instance_file)) {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC/t/MIMT",
                             paste0(sub("^t/", "", instance_file), ".txt"))
} else if (grepl("^u/", instance_file)) {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC/u/MIMT",
                             paste0(sub("^u/", "", instance_file), ".txt"))
} else {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC",
                             paste0(instance_file, ".txt"))
}

# -------------------- Check instance --------------------
if (!file.exists(instance_file)) {
  cat(1e6, "\n")
  quit(status = 0)
}

# -------------------- Build command --------------------
cmd_args <- c(
  instance_file,
  K1, K2, K3,
  "--max_it", max_it,
  "--max_no_imp", max_no_imp,
  "--builder", builder,
  "--beta", beta,
  "--use_qrvnd", use_qrvnd,
  "--alpha", alpha,
  "--gamma", gamma,
  "--epsilon", epsilon
)

# -------------------- Run --------------------
res <- tryCatch(
  system2(ais_binary, args = cmd_args,
          stdout = TRUE, stderr = NULL),
  error = function(e) character(0)
)

# -------------------- Parse result --------------------
score <- suppressWarnings(as.numeric(res))
if (length(score) == 0 || is.na(score)) score <- 1e6

# -------------------- Output (STRICT for irace) --------------------
cat(score, "\n")