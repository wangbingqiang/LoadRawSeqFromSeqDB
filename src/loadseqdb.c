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
//#include <mpi.h>
#define PROGNAME "LoadSeqDB"
#include "util.h"

#define OUTPUT_PATH "/WORK/nscc-gz_junrongyang/SEQDB/fastq/subset/"
#define OUTPUT_PREFIX "a.fastq"

int mpi_size, mpi_rank;
MPI_Comm comm  = MPI_COMM_WORLD;
MPI_Info info  = MPI_INFO_NULL;


void print_usage() {
    printf("usage: %s SeqDB_File \nConverts the SEQDB input file to FASTQ format, printing to stdout.\n",PROGNAME);
    printf("Example usage : \n%s 1.seqdb > 1.fastq \n",PROGNAME);
}

int main(int argc, char** argv){

    //system("date +%s.%N");
    int c;
/*    size_t section=0;
 *    int part_flag=0;
 */
    /*
 *  *      * MPI variables
 *   *           */

    /*
 *  *      * Initialize MPI
 *   *           */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);  
 
    char file_output[256];
    sprintf(file_output,"%s%s_%d",OUTPUT_PATH,OUTPUT_PREFIX,mpi_rank+1);
    FILE *f_output=NULL;
    //f_output=fopen(file_output,"w");
    f_output=fopen("/dev/null","w");
    //f_output=popen("wc -c","w");

    if(f_output==NULL){
	fprintf(stderr,"ERROR: Can't open file:%s",file_output);
	return -1;
    }


    //while ((c = getopt(argc, argv, "hs:")) != -1)
    while ((c = getopt(argc, argv, "h")) != -1)
        switch (c) {
/*
            case 's':
                section=(size_t)strtoul(optarg, NULL, 10);
                part_flag=1;
                if(section>TOTAL_SECTIONS || section<1)
                    ERROR("session too large")
                fprintf(stderr,"section:%zu \n",section);
                break;
*/
            case 'h':
            default:
                print_usage();
                return EXIT_SUCCESS;
        }

    if (optind >= argc)
        ARG_ERROR("you must specify an input file\n")

    struct H5SeqDB *db = open_input(argv[optind]);

    setvbuf(f_output, NULL, _IOFBF, 1024*1024);
/*
    if(part_flag==1){
    reset_range(db,section);
    exportFASTQ_PARTLY(db, stdout);
    }else if(part_flag==0)
        exportFASTQ(db,stdout);
*/
    reset_range(db);
    exportFASTQ_PARTLY(db, f_output);

    fclose(f_output);
    
    //pclose(f_output);

    H5SeqDB_Free(&db);

    MPI_Finalize();

    //system("date +%s.%N");
    return EXIT_SUCCESS;
}
