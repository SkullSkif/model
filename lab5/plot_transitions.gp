set terminal png size 800,600
set output 'transitions.png'
set xlabel 'Номер перехода'
set ylabel 'Состояние'
set grid
plot 'states1.dat' with lines title 'Матрица 1', \
     'states2.dat' with lines title 'Матрица 2'
