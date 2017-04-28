vpath %.c ./src
vpath %.h ./src
objects = loadseqdb.o h5seqdb.o seqpack.o seq.o \
		  blosc_filter.o blosc.o blosclz.o shuffle.o

 CFLAGS = -Wall -std=gnu99  -fopenmp -I/HOME/nscc-gz_junrongyang/hdf5/hdf5_parallel/include -I/usr/local/mpi3/include/
 cc = /usr/local/mpi3/bin/mpicc 
 
MPI_LoadSeqDB : $(objects)
	/usr/local/mpi3/bin/mpicc -lpthread -lz -lsz -lc -fopenmp -I./obj -o MPI_LoadSeqDB $(objects) -lhdf5


loadseqdb.o : h5seqdb.h
h5seqdb.o : blosc.h blosc_filter.h h5seqdb.h seqpack.h util.h seq.h
seqpack.o : seqpack.h util.h
seq.o : seq.h
blosc_filter.o : blosc.h blosc_filter.h
blosc.o : blosc.h blosclz.h shuffle.h
blosclz.o : blosclz.h
shuffle.o : shuffle.h

.PHONY : clean
clean : 
		rm MPI_LoadSeqDB $(objects)
