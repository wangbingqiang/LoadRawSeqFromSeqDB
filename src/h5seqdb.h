/*
 * Created by JR_Young on 2017/1/3.
 *
 *This file contains declarations of struct H5SeqDB, and functions that are realated to H5SeqDB.
*/

#ifndef LOADSEQDB_C_H5SEQDB_H
#define LOADSEQDB_C_H5SEQDB_H


#include <stdio.h>
#include <stddef.h>
/* #include "seqdb-version.h"*/
#include "seqpack.h"
#include <hdf5.h>
/* #include "fastq.h"*/

#define TOTAL_SECTIONS 24


static const char MODE_READ = 0x00;
static const char MODE_TRUNCATE = 0x01;
static const char MODE_APPEND = 0x02;


struct H5SeqDB{

    /* data */
    const char* filename;
    size_t nrecords;
    size_t nread;
    size_t slen;
    size_t ilen;
    int qual_offset;
    char mode;
    struct SeqPack* pack;

    /* */

    /* data */
    const size_t blocksize;

    /* buffers for block reading and writing */
    uint8_t* pbuf;
    size_t pbufsize;

    char* sbuf;
    size_t sbufsize;

    char* ibuf;
    size_t ibufsize;

    /* buffers for random-access reads */
    uint8_t* pbuf_at;
    char* sbuf_at;
    char* ibuf_at;


    /* HDF5 objects */
    hid_t h5file;
    hid_t sdset;
    hid_t sspace;
    hid_t smemspace;
    hid_t idset;
    hid_t ispace;
    hid_t imemspace;
};

/*return an address of a new H5SeqDB, just like a constructor*/
struct H5SeqDB* H5SeqDB_Entity_Create(const char* filename_, char mode_, size_t slen_, size_t ilen_, size_t blocksize_);

/*free the memory , just like a destructor*/
void H5SeqDB_Free(struct H5SeqDB ** p2db);

void alloc_buffers(struct H5SeqDB* db);

void clear_buffers(struct H5SeqDB* db);

void clear_buffers_at(struct H5SeqDB* db);

void flush_reads(struct H5SeqDB* db);

void open_hdf5_file(struct H5SeqDB* db, const char* filename, hid_t mode);

void read_array_attribute(struct H5SeqDB* db, const char* name, hid_t type, hsize_t n, void* array);

void open_datasets(struct H5SeqDB* db,const char* path);

struct H5SeqDB* H5SeqDB_Create(const char* filename, char mode, size_t slen, size_t ilen, size_t blocksize);

struct H5SeqDB* open_input(const char* filename);

void reset_range(struct H5SeqDB *db);

void flush_reads(struct H5SeqDB* db);

void export_block(struct H5SeqDB* db, FILE* f, hsize_t count);

void exportFASTQ(struct H5SeqDB* db, FILE* f);

void exportFASTQ_PARTLY(struct H5SeqDB* db, FILE* f);

#endif /*LOADSEQDB_C_H5SEQDB_H*/
