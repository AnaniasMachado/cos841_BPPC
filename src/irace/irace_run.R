library(irace)

parameters <- readParameters("parameters.txt")

scenario <- defaultScenario()

scenario$targetRunner <- "target.R"
scenario$targetRunnerArgs <- ""

scenario$instances <- read.table("stratified_sample.txt",
                                 stringsAsFactors = FALSE)[[1]]

scenario$maxExperiments <- 200
scenario$parallel <- 6
scenario$logFile <- "irace_progress.Rdata"
scenario$debugLevel <- 1
scenario$maxTime <- 1200

scenario$parameters <- parameters

checkIraceScenario(scenario)

if (file.exists(scenario$logFile)) {
  cat("Resuming previous irace run...\n")
  results <- tryCatch(
    irace(scenario = scenario,
          parameters = parameters,
          recoveryFile = scenario$logFile),
    error = function(e) {
      cat("Recovery failed, starting fresh...\n")
      irace(scenario = scenario,
            parameters = parameters)
    }
  )
} else {
  cat("Starting new irace run...\n")
  results <- irace(scenario = scenario,
                   parameters = parameters)
}

write.csv(results,
          file = "irace_ranked.csv",
          row.names = FALSE)

cat("irace tuning completed.\n")