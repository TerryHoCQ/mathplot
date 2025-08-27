#!/bin/bash

# Count lines of code in mathplot
find . \( -path './*.[h]' -or -path './*.hpp' -or -path './*.cpp' \) \
     -or \( -path './mplot/jcvoronoi' -or -path './build' -or -path './mplot/fonts' -or -path './mplot/lodepng.h' -or -path './include' -or -path './mplot/healpix' \) -prune \
     | xargs wc -l
