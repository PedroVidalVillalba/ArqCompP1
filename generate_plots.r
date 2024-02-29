datos = read.csv(file("stdin"), header = TRUE)
datos = aggregate(cbind(Clocks, ClocksPerAccess) ~ D + L + R, datos, mean)
datos = datos[order(datos$D, datos$L, datos$R), ]
datos

plot(datos$L, datos$ClocksPerAccess, pch = 19, col = factor(datos$D),
     main = "Medidas de rendimiento en accesos a memoria",
     xlab = "Número de líneas caché leídas (L)",
     ylab = "Ciclos de reloj por acceso",
     xaxt = 'n', log = "x")
axis(side = 1, at = unique(datos$L), las = 2)

i = 1
for (D in unique(datos$D)) {
    points(datos[datos$D == D, ]$L, datos[datos$D == D, ]$ClocksPerAccess, type = 'l', col = i)
    i = i + 1
}
