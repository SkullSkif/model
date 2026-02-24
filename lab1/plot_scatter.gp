set key top left
set title "Scatter"
set xlabel "x"
set ylabel "f(x), F(x)"

a = 600.0 / 217.0
b = 1680.0 / 217.0
N = 50000.0       
bin_width = 0.04

f(x) = (x < 0.3 || x > 1.5) ? 0 : \
       (x < 1.0) ? a * (x - 0.3) : \
       b * (x - 1.5)**2

F(x) = (x < 0.3) ? 0 : \
       (x < 1.0) ? (a/2.0) * (x - 0.3)**2 : \
       (x <= 1.5) ? 0.245*a + (b/3.0) * ((x - 1.5)**3 + 0.125) : 1.0

set terminal pngcairo size 1200,800 enhanced font "Arial,12"
set output "distribution_plot.png"

plot "plot_data" using 1:(f($1) * rand(0)) with points \
     pt 7 ps 0.2 lc rgb "royalblue" title "50k dots", \
     f(x) with lines lw 3 lc rgb "red" title "f(x)", \
     F(x) with lines lw 2 lc rgb "dark-green" dt 2 title "F(x)"

set arrow from 1,0 to 1,f(1) nohead lc rgb "black" dt 3

plot "data.txt" using 1:2 with points pt 7 ps 0.5 lc rgb "blue" title

plot "data.txt" using 1:(f($1) * rand(0)) with points \
     pt 7 ps 0.2 lc rgb "royalblue" title "Выборка", \

set grid
set xrange [-0.3 : 2.3]
set yrange [0 : 1.1]
set samples 200000
set style fill transparent solid 0.4 noborder



bin(x, s) = s * floor(x/s) + s/2.0

plot "data.txt" using (bin($1, bin_width)):(1.0/(N*bin_width)) smooth freq with boxes \
          lc rgb "royalblue" title "Выборка (50k)", \
     f(x) with lines lw 3 lc rgb "red" title "Теоретическая плотность f(x)", \
     F(x) with lines lw 2 lc rgb "dark-green" dt 2 title "Функция распределения F(x)"