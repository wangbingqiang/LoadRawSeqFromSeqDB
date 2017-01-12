/*
 * Created by JR Young on 17-1-5.
 *
 *LoadRawSeqFromSeqDB
 *https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
 *
 *A tool to load raw sequence data from a SeqDB file , written in ANSI C;
 *derived from SeqDB https://bitbucket.org/mhowison/seqdb
*/
#define _POSIX_C_SOURCE 2
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "h5seqdb.h"

#define PROGNAME "LoadSeqDB"
#include "util.h"

void print_usage() {
    printf("usage: %s SeqDB_File \nConverts the SEQDB input file to FASTQ format, printing to stdout.\n",PROGNAME);
    printf("Example usage : \n%s 1.seqdb > 1.fastq \n",PROGNAME);
}

int main(int argc, char** argv){

    int c;
    while ((c = getopt(argc, argv, "h")) != -1)
        switch (c) {
            case 'h':
            default:
                print_usage();
                return EXIT_SUCCESS;
        }

    if (optind >= argc)
        ARG_ERROR("you must specify an input file\n")

    struct H5SeqDB *db = open_input(argv[optind]);

    setvbuf(stdout, NULL, _IOFBF, 1024*1024);

    exportFASTQ(db, stdout);

    H5SeqDB_Free(&db);

    return EXIT_SUCCESS;
}