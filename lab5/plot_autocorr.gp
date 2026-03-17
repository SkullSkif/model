set terminal png size 800,600
set output 'autocorrelation.png'
set xlabel 'Лаг'
set ylabel 'Автокорреляция'
set grid
plot 'autocorr1.dat' with lines title 'Матрица 1', \
     'autocorr2.dat' with lines title 'Матрица 2'
