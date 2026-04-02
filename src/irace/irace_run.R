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

results <- irace(scenario = scenario)

write.csv(results,
          file = "irace_ranked.csv",
          row.names = FALSE)

cat("irace tuning completed.\n")