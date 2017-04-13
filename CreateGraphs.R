args<-commandArgs(TRUE)

data <- read.csv(args[1])
#print(data)

#png(file = "line_chart.png", width=1200, height=700)

# Plot the bar chart. 
#plot(data[, 1], type = "p", col = "blue", ylab = 'Throughput/#Threads', xlab = 'Time (sec)', main = 'The Main Title') # throughput
#lines(data[, 2], type = "l", col = "red")

#legend("topright", legend=c("Throughput", "# Threads"), col=c("blue", "red"), lty=1:2, cex=0.8)

# Save the file.
#dev.off()

library(ggplot2)

ggplot(data, aes(x = Time)) + 
  geom_point(aes(y = Throughput, colour="Throughput"), size=1, shape=4) + 
  geom_line(aes(y = Threads, colour="Threads")) +
  labs(title = "", x = "Time (sec)", y = "Throughput/#Threads", color = "") +
  scale_colour_manual(values=c("red", "blue"))

ggsave(args[2], width=280, height=140, units="mm")
