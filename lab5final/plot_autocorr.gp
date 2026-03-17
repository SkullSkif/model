set terminal png
set output "autocorrelation.png"
set xlabel "Лаг"
set ylabel "Автокорреляция"
plot "autocorr1.dat" with lines title "Матрица 1", "autocorr2.dat" with lines title "Матрица 2"