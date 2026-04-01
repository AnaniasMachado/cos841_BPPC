# -------------------- weight_irace_run.R --------------------
library(irace)

# --------- Load parameter definitions ---------
parameters <- readParameters("weight_parameters.txt")

# --------- Build scenario ---------
scenario <- defaultScenario()

# Set your custom scenario options:
scenario$targetRunner <- "weight_target.R"      # script to run
scenario$instances <- read.table("instances.txt", stringsAsFactors = FALSE)[[1]]
scenario$maxExperiments <- 200                  # max evaluations
scenario$parallel <- 3                          # number of cores
scenario$logFile <- "weight_irace_progress.Rdata"  # Rdata log
scenario$debugLevel <- 1                        # show basic progress

# Make sure scenario has the parameters
scenario$parameters <- parameters

# Check scenario
checkIraceScenario(scenario)

# --------- Run irace ---------
results <- irace(scenario = scenario)

# --------- Save ranked configurations ---------
write.csv(
  results,   # the returned value of irace() is the data.frame of configurations
  file = "irace_ranked_weights.csv",
  row.names = FALSE
)

cat("irace tuning completed.\nRanked configurations saved in 'irace_ranked_weights.csv'\n")