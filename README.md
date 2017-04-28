# MPI_LoadSeqDB 
version:0.0.1
https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
MPI_LoadSeqDB aims at loading raw seqeuence data parallel from a SeqDB file,written in C and derived from SeqDB source code
This work is mainly done by Juion Young, an intern from South China University of Technology
It prints fastq format sequences on stdout, you can redirect ouput to file.
usage : ./MPI_LoadSeqDB seqdbfile
output files need to be set inside the program file "loadseqdb.c", since this program is unfinished.