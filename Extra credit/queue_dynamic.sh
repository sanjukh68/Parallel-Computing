#!/bin/sh
set -x
RESULTDIR=result/
h=`hostname`
. ./params.sh 
if [ "$h" = "mba-i1.uncc.edu"  ];
then
    echo Do not run this on the headnode of the cluster, use qsub!
    exit 1
fi
if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi
make reduce_dynamic
for proc in ${PROC}}
do
	for n in ${N}
	do 
		echo "mpirun -np ${proc} ./reduce_dynamic ${n} 2> ${RESULTDIR}/reduce_dynamic_${N}_${PROC}  >/dev/null"
	done 
done