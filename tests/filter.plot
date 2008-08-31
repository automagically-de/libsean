set terminal png
set output "filter.png"
set title "filter results"

set ylabel "detected images"
set xlabel "threshold"
#set yrange [0:90]
#set xrange [56:128]

plot \
	"filter.dat" using 1:2 title "w/o filter" with lines, \
	"filter.dat" using 1:3 title "median filter" with lines, \
	"filter.dat" using 1:4 title "average filter" with lines, \
	"filter.dat" using 1:5 title "average + median" with lines
