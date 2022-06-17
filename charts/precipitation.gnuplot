set datafile separator ','

set xtics nomirror
set ytics nomirror
set grid

set xdata time
set timefmt "%s"
set format x "%d-%m-%Y"
set xtics rotate by -45 offset 0,0
set rmargin 10

set style line 1 lt 1 lw 2 ps 0 lc "#003366"
plot 'cmake-build-debug/bin/datasets/raw_test.csv' using 1:2 with linespoints linestyle 1 title 'Test Series'
