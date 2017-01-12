vpath %.c ./src
vpath %.h ./src
objects = loadseqdb.o h5seqdb.o seqpack.o seq.o \
		  blosc_filter.o blosc.o blosclz.o shuffle.o

 CFLAGS = -Wall -fopenmp
 cc = gcc
LoadSeqDB : $(objects)
	cc -lhdf5 -lpthread -fopenmp -I./obj -o LoadSeqDB $(objects); rm ./*.o


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
		rm LoadSeqDB $(objects)
