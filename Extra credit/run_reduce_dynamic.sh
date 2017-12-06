#!/bin/sh

RESULTDIR=result/
h=`hostname`

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi


N="10000"
PROCS="1 2 4 8 12 16"

make reduce_dynamic
make reduce_seq

for n in $N;
do
    for t in $PROCS;
    do
    echo "mpirun ./reduce_dynamic $n $t >/dev/null 2> ${RESULTDIR}/reduce_dynamic_${n}_${t}"
    echo "mpirun ./reduce_seq $n >/dev/null 2> ${RESULTDIR}/reduce_dynamic_${n}_seq"
    done
done

for n in $N;
do
    for t in $PROCS;
    do
    #output in format "thread seq par"
    echo ${t} \
         $(cat ${RESULTDIR}/reduce_dynamic_${n}_seq) \
         $(cat ${RESULTDIR}/reduce_dynamic_${n}_${t})
    done   > ${RESULTDIR}/speedup_reduce_dynamic_${n}
done

gnuplot <<EOF
set terminal pdf
set output 'reduce_dynamic_plots.pdf'
set style data linespoints
set key top left;
set xlabel 'threads';
set ylabel 'speedup';
set xrange [1:20];
set yrange [0:20];
set title 'n=$N';
        plot '${RESULTDIR}/speedup_reduce_dynamic_${N}' u 1:(\$2/\$3) t 'static' lc 1
EOF
