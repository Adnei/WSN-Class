library(ggplot2)
library(dplyr)

log_data.df <- read.csv("../all_logs.log", sep="|", header=T, stringsAsFactors = F)

time_unit <- 1000000

listen_transmit.df <- log_data.df[
    !is.na(log_data.df$LISTEN) &
    !is.na(log_data.df$TRANSMIT),-4] #-4 removes column 4 (Message)
