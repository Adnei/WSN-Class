library(ggplot2)
library(dplyr)
library(readr)

log_data.df <- read.csv("../all_logs.log", sep="|", header=T, stringsAsFactors = F)

time_unit <- 1000000

send_recv <- log_data.df[grepl('DATA', log_data.df$Message), -c(5,6)] #-c(5,6) removes columns 5 and 6
send_recv <- send_recv[order(send_recv$Time), ]
sent <- send_recv[grepl('sent', send_recv$Message),]
recv <- send_recv[grepl('recv', send_recv$Message),]


EXEC <- c()
SOURCE <- c()
DEST <- c()
RECEIVED <- c()
SEQN <- c()
SEND_TIME <- c()
RECV_TIME <- c()
DELAY <- c()


for(exec_id in c(min(sent$EXEC):max(sent$EXEC))) {
  for(mote_id in c(min(sent$ID):max(sent$ID))) {
    splitted <- unlist(strsplit(
      sent[sent$ID == mote_id & sent$EXEC == exec_id, ]$Message, split="seqn "))
    seqn.arr <- as.integer(splitted[seq(2,length(splitted), by=2)])
    #parse number gets the ID from the string "DATA sent to X", in which the ID is X
    sink_id.arr <- parse_number(splitted[seq(1,length(splitted), by=2)])
    send_time.arr <- sent[sent$ID == mote_id & sent$EXEC == exec_id, ]$Time
    for(arr_idx in c(1:length(seqn.arr))) {
      recv_element <- recv[recv$EXEC == exec_id &
        grepl(paste('from', mote_id, sep=' '), recv$Message) &
        grepl(paste('seqn', seqn.arr[arr_idx], sep=' '), recv$Message), ]
      recv_time <- recv_element$Time[1]
      delay <- recv_time - send_time.arr[arr_idx]
      received <- TRUE

      if(length(recv_element[,1]) == 0){
        recv_time <- NA
        delay <- NA
        received <- FALSE
      }

      EXEC <- c(EXEC, exec_id)
      SOURCE <- c(SOURCE, mote_id)
      DEST <- c(DEST, sink_id.arr[arr_idx])
      RECEIVED <- c(RECEIVED, received)
      SEQN <- c(SEQN, seqn.arr[arr_idx])
      SEND_TIME <- c(SEND_TIME, send_time.arr[arr_idx])
      RECV_TIME <- c(RECV_TIME, recv_time)
      DELAY <- c(DELAY, delay)
    }
  }
}

parse_data.df <- data.frame(
  EXEC=EXEC,
  SOURCE=SOURCE,
  DEST = DEST,
  RECEIVED=RECEIVED,
  SEQN=SEQN,
  SEND_TIME=SEND_TIME,
  RECV_TIME=RECV_TIME,
  DELAY=DELAY
)
