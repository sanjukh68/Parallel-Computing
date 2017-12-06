#!/bin/sh

RESULTDIR=result/
h=`hostname`

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi


N="1000000"
THREADS="1 2 4 8 12 16"

make compresser_static
make compresser_sequential

for n in $N;
do
    for t in $THREADS;
    do
    ./compresser_static $n $t >/dev/null 2> ${RESULTDIR}/compression_${n}_${t}_static
    ./compresser_sequential $n >/dev/null 2> ${RESULTDIR}/compression_${n}_seq
    done
done

for n in $N;
do
    for t in $THREADS;
    do
    #output in format "thread seq par"
    echo ${t} \
         $(cat ${RESULTDIR}/compression_${n}_seq) \
         $(cat ${RESULTDIR}/compression_${n}_${t}_static)
    done   > ${RESULTDIR}/speedup_compression_${n}
done

gnuplot <<EOF
set terminal pdf
set output 'compression_plots.pdf'
set style data linespoints
set key top left;
set xlabel 'threads';
set ylabel 'speedup';
set xrange [1:20];
set yrange [0:20];
set title 'n=$N';
        plot '${RESULTDIR}/speedup_compression_${N}' u 1:(\$2/\$3) t 'static' lc 1
EOF
