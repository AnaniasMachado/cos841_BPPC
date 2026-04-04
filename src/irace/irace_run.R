library(irace)

parameters <- readParameters("parameters.txt")
scenario <- defaultScenario()
scenario$instances <- read.table("stratified_sample.txt", stringsAsFactors = FALSE)[[1]]
scenario$parameters <- parameters

scenario$maxExperiments <- 1200
scenario$parallel <- 6
scenario$debugLevel <- 1
scenario$logFile <- "irace_progress2.Rdata"
scenario$recoveryFile <- "irace_progress.Rdata"
scenario$softRestart <- TRUE
scenario$firstTest <- 5
scenario$targetRunner <- "target.R"

# Run irace (it will resume automatically if log exists)
cat("Starting or resuming irace run...\n")
results <- irace(scenario = scenario)

# Save final results
save(results, file = "irace_results.Rdata")
write.csv(results, "irace_ranked.csv", row.names = FALSE)
cat("irace tuning completed.\n")