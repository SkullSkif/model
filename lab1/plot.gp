# Настройки вывода (можно закомментировать для окна)
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'distribution.png'

# Константы (исправленные для точности)
a = 600.0 / 217.0
b = 1680.0 / 217.0
p_threshold = 147.0 / 217.0

# Определение функций (тернарный оператор condition ? if_true : if_false)
f(x) = (x < 0.3 || x > 1.5) ? 0 : \
       (x < 1.0) ? a * (x - 0.3) : \
       b * (x - 1.5)**2

F(x) = (x < 0.3) ? 0 : \
       (x < 1.0) ? (a/2.0) * (x - 0.3)**2 : \
       (x <= 1.5) ? 0.245*a + (b/3.0) * ((x - 1.5)**3 + 0.125) : 1.0

# Настройки графиков
set grid
set xrange [0 : 2.0]
set yrange [0 : 3.0]
set key top left
set title "Гистограмма распределения"
set xlabel "x"
set ylabel "F(x)"

# Рисуем: 
# 1. Гистограмму из файла data.txt (нужна нормализация через bins)
# 2. Теоретическую плотность
# 3. Теоретическую функцию распределения
# 4. Линию порога

bin_width = 0.015
bin(x, s) = s*int(x/s)

plot "gen_data.txt" using (bin($1, bin_width)):(1.0/(1000000*bin_width)) smooth freq with boxes \
          lc rgb "skyblue" title "Выборка", \
     f(x) with lines lw 3 lc rgb "red" title "Плотность"