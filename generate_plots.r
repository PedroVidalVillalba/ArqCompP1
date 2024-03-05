data = read.csv(file("stdin"), header = TRUE)
data = na.omit(data)  # Eliminar NAs (últimas filas)
means = aggregate(cbind(Clocks, ClocksPerAccess) ~ D + L + R, data, mean)
sds = aggregate(cbind(Clocks, ClocksPerAccess) ~ D + L + R, data, sd)
means = means[order(means$D, means$L, means$R), ]
sds = sds[order(sds$D, sds$L, sds$R), ]

plot(means$L, means$ClocksPerAccess, pch = 19, col = factor(means$D),
     main = "Medidas de rendimiento en accesos a memoria",
     xlab = "Número de líneas caché leídas (L)",
     ylab = "Ciclos de reloj por acceso",
     xaxt = 'n', log = "x")
axis(side = 1, at = unique(means$L), las = 2)

i = 1
for (D in unique(means$D)) {
    x = means[means$D == D, ]$L
    y = means[means$D == D, ]$ClocksPerAccess
    y.sd = sds[sds$D == D, ]$ClocksPerAccess
    points(x, y, type = 'l', col = i)
    arrows(x0 = x, x1 = x, y0 = y - y.sd, y1 = y + y.sd, 
           code = 3, angle = 90, length = 0.1, col = i)
    # legends = c(legends, sprintf("D = %d", D))
    i = i + 1
}

legends = sprintf("D = %d", unique(means$D))
legend("topleft", legends, fill = 1:5)
