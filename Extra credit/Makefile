CXX=mpicxx
CC=mpicc
LD=mpicxx
LC=mpicc
CFLAGS=-fopenmp
CXXFLAGS=-O3 -std=c++11 -g -fopenmp
LDFLAGS=-fopenmp
ARCHIVES=libgen.a 

all: sample mergesort mergempi mpimerge2 quicksort compresser_static compresser_sequential compresser reduce_static reduce_seq reduce_dynamic

sample: sample.o
	$(LD) $(LDFLAGS) sample.o -o sample
	
mergesort: mergesort.o
	$(LD) $(LDFLAGS) mergesort.o -o mergesort

mergempi: mergempi.o
	$(LC) $(LDFLAGS) mergempi.o -o mergempi

mpimerge2: mpimerge2.o
	$(LC) $(LDFLAGS) mpimerge2.o -o mpimergempi
	
quicksort: quicksort.o
	$(LC) $(LDFLAGS) quicksort.o -o quicksort
	
compresser: compresser.o
	$(LD) $(LDFLAGS) compresser.o -o compresser

compresser_static: compresser_static.o
	$(LD) $(LDFLAGS) compresser_static.o -o compresser_static

compresser_sequential: compresser_sequential.o
	$(LD) $(LDFLAGS) compresser_sequential.o -o compresser_sequential

reduce_seq: reduce_seq.o
	$(LD) $(LDFLAGS) reduce_seq.o $(ARCHIVES) -o reduce_seq
	
reduce_static: reduce_static.o
	$(LD) $(LDFLAGS) reduce_static.o $(ARCHIVES) -o reduce_static

reduce_dynamic: reduce_dynamic.o
	$(LD) $(LDFLAGS) reduce_dynamic.o $(ARCHIVES) -o reduce_dynamic
	
libgen.a: gen_lib.o
	ar rcs libgen.a gen_lib.o
	
test : compresser_static
	mpirun ./compresser_static 10
	mpirun ./compresser_static 20
	mpirun ./compresser_static 30

all : compresser

clean :
	- rm -rf compresser *.o core.* compresser_static compresser_sequential