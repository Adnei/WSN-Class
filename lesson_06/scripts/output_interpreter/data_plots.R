source("parse_data.R")
################################################################################
#                            Delivery Rate By Execution                        #
################################################################################
max_exec <- max(parse_data.df$EXEC)

recv_rate <- c()
for(i_exec in c(1:max_exec)){
  rate <- length(parse_data.df[parse_data.df$EXEC == i_exec &
    parse_data.df$RECEIVED == TRUE, ]$RECEIVED) /
  length(parse_data.df[parse_data.df$EXEC == i_exec, ]$RECEIVED)
  recv_rate <- c(recv_rate, rate)
}

plot_info.df <- data.frame(x_axis=c(1:max_exec), y_axis=recv_rate)

delivery_rate_exec.plot <- ggplot(plot_info.df, aes(x=x_axis,y=y_axis, color=x_axis)) +
  geom_line() +
  geom_point() +
  ylab("Taxa de entrega") +
  xlab("ID de execução") +
  ggtitle("Entrega de pacotes") +
  theme_minimal() +
  scale_x_continuous(breaks=seq(1,max(plot_info.df$x_axis),1) ) +
  scale_y_continuous(limits=c(0, 1) )

ggsave(filename='delivery_rate_exec.pdf', plot=delivery_rate_exec.plot)
################################################################################
#                         Packet Delay Avg By Execution                        #
################################################################################
delay_avg_exec.df <- aggregate(DELAY ~ EXEC, data=parse_data.df, FUN=mean)
delay_exec.plot <- ggplot(delay_avg_exec.df, aes(x=EXEC,y=DELAY/time_unit)) +
  geom_line() +
  geom_point() +
  ylab("Atraso médio (s)") + # OLD -> µs
  xlab("ID de execução") +
  ggtitle("Atraso de entrega de pacotes") +
  theme_minimal() +
  scale_x_continuous(breaks=seq(1,max(plot_info.df$x_axis),1) )
ggsave(filename='delay_exec.pdf', plot=delay_exec.plot)
################################################################################
#                         Power mW By Mote                                     #
################################################################################
source("parse_data_power.R")

lt_sec_precision.df <- listen_transmit.df
lt_sec_precision.df$Time <- as.integer(lt_sec_precision.df$Time/time_unit/5)

energest_avg_by_sec <- aggregate(list(
    LISTEN=lt_sec_precision.df$LISTEN,
    TRANSMIT=lt_sec_precision.df$TRANSMIT),
  by=list(Time=lt_sec_precision.df$Time),
  FUN=mean)

power_avg_sec.plot <- ggplot(energest_avg_by_sec, aes(x = Time)) +
  geom_line(aes(y=LISTEN* 0.33 * 3 / 32768 / 5, colour="RX")) +
  geom_line(aes(y=TRANSMIT* 0.33 * 3 / 32768 / 5, colour="TX")) +
  ylab("Energia (mW)") +
  xlab("Tempo (s)") +
  ggtitle("Consumo de energia") +
  theme_minimal() #+
  # scale_x_continuous(breaks=seq(1,max(plot_info.df$x_axis),1) )
ggsave(filename='power_avg_sec.pdf', plot=power_avg_sec.plot)
