set terminal png
set output "transitions.png"
set xlabel "Номер перехода"
set ylabel "Номер состояния"
plot "states1.dat" with lines title "Матрица 1", "states2.dat" with lines title "Матрица 2"