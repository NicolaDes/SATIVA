plot \
'graphics/UIP.graph' using 1:2 with lines lc rgb "black" lw 2 notitle,\
'graphics/UIP.graph' u 1:2  notitle, \
'graphics/UIP.graph' using 1:2:3 with labels tc rgb "black" offset(0,0)

