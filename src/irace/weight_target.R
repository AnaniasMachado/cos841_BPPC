#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)

K1 <- as.integer(args[1])
K2 <- as.integer(args[2])
K3 <- as.integer(args[3])
instance_file <- args[4]

ais_binary <- "../bin/ails_weight_runner"

# Determine full path to instance
if (grepl("^t/", instance_file)) {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC/t/MIMT", paste0(sub("^t/", "", instance_file), ".txt"))
} else if (grepl("^u/", instance_file)) {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC/u/MIMT", paste0(sub("^u/", "", instance_file), ".txt"))
} else {
  instance_file <- file.path("../../instances/BPPC_test_instances/BPPC", paste0(instance_file, ".txt"))
}

# Ensure instance exists
if (!file.exists(instance_file)) {
  cat(1e6, "\n")
  quit(status = 0)
}

# Run AILS, redirect stderr to avoid warnings printed
res <- tryCatch(
  system2(ais_binary, args = c(instance_file, K1, K2, K3),
          stdout = TRUE, stderr = NULL),
  error = function(e) character(0)
)

score <- suppressWarnings(as.numeric(res))
if (length(score) == 0 || is.na(score)) score <- 1e6

# Only print the numeric score
cat(score, "\n")