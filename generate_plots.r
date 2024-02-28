datos = read.csv(file("stdin"), header = TRUE)
datos$Clocks

plot(datos$L, datos$ClocksPerAccess)
