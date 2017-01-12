/*Created by JR_Young on 2017/1/4.
 *
 *LoadRawSeqFromSeqDB
 *https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
 *A tool to load raw sequence data from a SeqDB file , written in ANSI C;
 *derived from SeqDB https://bitbucket.org/mhowison/seqdb
 */


#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "blosc.h"
#include "blosc_filter.h"
#include "h5seqdb.h"
#include "seqpack.h"
#include "seq.h"

#define PROGNAME "seqdb"
#include "util.h"

#define BLOSC_LEVEL 4

#ifndef H5SEQDB_MALLOC_ALIGN
#define H5SEQDB_MALLOC_ALIGN 64
#endif

#ifndef H5SEQDB_MALLOC_PAD
#define H5SEQDB_MALLOC_PAD H5SEQDB_MALLOC_ALIGN
#endif

#define H5SEQDB_SEQ_DATASET "seq"
#define H5SEQDB_ID_DATASET "id"
#define H5SEQDB_GROUP "/seqdb"

#define H5SEQDB_ENC_BASE_ATTR "__encode_base__"
#define H5SEQDB_ENC_QUAL_ATTR "__encode_qualilty__"
#define H5SEQDB_DEC_BASE_ATTR "__decode_base__"
#define H5SEQDB_DEC_QUAL_ATTR "__decode_qualilty__"

/*
#define H5CHK(status) if (status < 0)\
   	ERROR("HDF5 error at " << __FILE__ << ":" << __LINE__)  */
#define H5CHK(status) if (status < 0) {\
    fprintf(stderr,"%s: HDF5 error at %s : %d \n",PROGNAME,__FILE__,__LINE__ );\
    exit(EXIT_FAILURE); }

#define H5TRY(call) do{\
	herr_t status = (herr_t)(call);\
	H5CHK(status)\
}while(0);

/* #define H5LOC(file,path) "'" << file << ":" << path << "'" */
#define ERROR_H5LOC(situation,filename,path) do{\
    fprintf(stderr,"%s: %s '%s:%s' \n",PROGNAME,situation,filename,path);\
    exit(EXIT_FAILURE);\
} while(0);


struct H5SeqDB* H5SeqDB_Entity_Create(const char* filename_,
                                      char mode_,
                                      size_t slen_,
                                      size_t ilen_,
                                      size_t blocksize_ ){
    struct H5SeqDB* db= (struct H5SeqDB*)malloc(sizeof(struct H5SeqDB));
    if(db==NULL)
        ERROR("Fail in initializing H5SeqDB")
    /* initialize elements.*/
    db->filename = filename_;
    size_t *p2blsz = ( size_t *) &(db->blocksize);
    *p2blsz = blocksize_;
    db->slen = slen_;
    db->ilen = ilen_;
    db->mode = mode_;
    db->pack = NULL;
    db->pbuf=NULL; db->sbuf=NULL; db->ibuf=NULL;
    db->pbuf_at=NULL; db->sbuf_at=NULL; db->ibuf_at=NULL;
    db->qual_offset = 33;


    if ((db->mode != MODE_READ) && (db->mode != MODE_TRUNCATE) && (db->mode != MODE_APPEND)) {
        ERROR("mode must be READ, TRUNCATE, or APPEND")
    }
    db->pack = SeqPack_Create(db->slen);

    return db;
};

void H5SeqDB_Free(struct H5SeqDB ** p2db){
    struct H5SeqDB *db = *p2db;

    /* free all buffers */
    free(db->pbuf);
    db->pbuf = NULL;
    free(db->ibuf);
    db->ibuf = NULL;
    free(db->sbuf);
    db->sbuf = NULL;

    free(db->pbuf_at);
    db->pbuf_at = NULL;
    free(db->ibuf_at);
    db->ibuf_at = NULL;
    free(db->sbuf_at);
    db->sbuf_at = NULL;

    /*free seqpack*/
    SeqPack_Free(&db->pack);

    *p2db = NULL;
}

void open_hdf5_file(struct H5SeqDB* db,
                    const char* filename,
                    hid_t mode){
    /* Property lists. */
    hid_t fapl = H5P_DEFAULT;
    /*hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);*/
    /*H5CHK(fapl)*/
    /*H5Pset_alignment(fapl, blocksize, 0);*/
    /*H5Pset_meta_block_size(fapl, blocksize);*/

    /* Open. */
    db->h5file = H5Fopen(filename, mode, fapl);
    if (db->h5file < 0)
        ERROR_H5LOC("cannot open file",filename,"")

    /* Cleanup. */
    H5TRY(H5Pclose(fapl))

}

void alloc_buffers(struct H5SeqDB* db){

    db->pbufsize = db->slen * db->blocksize;
    CHECK_ERR(
            posix_memalign((void**)&(db->pbuf),
                           H5SEQDB_MALLOC_ALIGN, db->pbufsize + H5SEQDB_MALLOC_PAD))

    db->sbufsize = 2 * db->slen * db->blocksize;
    CHECK_ERR(
            posix_memalign((void**)&(db->sbuf),
                           H5SEQDB_MALLOC_ALIGN, db->sbufsize + H5SEQDB_MALLOC_PAD))

    db->ibufsize = db->ilen * db->blocksize;
    CHECK_ERR(
            posix_memalign((void**)&(db->ibuf),
                           H5SEQDB_MALLOC_ALIGN, db->ibufsize + H5SEQDB_MALLOC_PAD))

    CHECK_ERR(
            posix_memalign((void**)&(db->pbuf_at),
                           H5SEQDB_MALLOC_ALIGN, db->slen + H5SEQDB_MALLOC_PAD))

    CHECK_ERR(
            posix_memalign((void**)&(db->sbuf_at),
                           H5SEQDB_MALLOC_ALIGN, 2*db->slen + H5SEQDB_MALLOC_PAD))

    CHECK_ERR(
            posix_memalign((void**)&(db->ibuf_at),
                           H5SEQDB_MALLOC_ALIGN, db->ilen + H5SEQDB_MALLOC_PAD))
}

void clear_buffers(struct H5SeqDB* db){
    memset(db->pbuf, 0x00, db->pbufsize);
    memset(db->sbuf, 0x00, db->sbufsize);
    memset(db->ibuf, 0x00, db->ibufsize);
}

void clear_buffers_at(struct H5SeqDB* db){
    memset(db->pbuf_at, 0x00, db->slen);
    memset(db->sbuf_at, 0x00, 2 * db->slen);;
    memset(db->ibuf_at, 0x00, db->ilen);
}

void flush_reads(struct H5SeqDB* db){
    hsize_t remainder = db->blocksize;
    if ((db->nrecords - db->nread) < db->blocksize) {
        remainder = db->nrecords - db->nread;
    }

#ifdef DEBUG
    /*NOTIFY("flushing " << remainder << " reads at offset " << nread)*/
    fprintf(stderr,"%s : flushing %zu reads at offset %zu \n",PROGNAME,remainder,nread);
#endif

    /* Property lists. */
    hid_t dxpl = H5P_DEFAULT;

    /* Select. */
    hsize_t offset[2] = { db->nread, 0 };
    hsize_t scount[2] = { remainder, db->slen };
    hsize_t icount[2] = { remainder, db->ilen };
    H5TRY(H5Sselect_hyperslab(db->sspace, H5S_SELECT_SET, offset, NULL, scount, NULL))
    H5TRY(H5Sselect_hyperslab(db->ispace, H5S_SELECT_SET, offset, NULL, icount, NULL))
    offset[0] = 0;
    H5TRY(H5Sselect_hyperslab(db->smemspace, H5S_SELECT_SET, offset, NULL, scount, NULL))
    H5TRY(H5Sselect_hyperslab(db->imemspace, H5S_SELECT_SET, offset, NULL, icount, NULL))

    /* Read. */
    clear_buffers(db);
    H5TRY(H5Dread(db->sdset, H5T_NATIVE_UINT8, db->smemspace, db->sspace, dxpl, db->pbuf))
    H5TRY(H5Dread(db->idset, H5T_NATIVE_CHAR, db->imemspace, db->ispace, dxpl, db->ibuf))

    /* Unpack. */
    parunpack(db->pack, remainder, db->pbuf, db->sbuf);
}

void read_array_attribute(struct H5SeqDB* db,
                          const char* name,
                          hid_t type,
                          hsize_t n,
                          void* array){

    hid_t attr = H5Aopen(db->h5file, name, H5P_DEFAULT);
    H5CHK(attr)

    H5TRY(H5Aread(attr, type, array))

    /* Cleanup. */
    H5TRY(H5Aclose(attr))
}

void open_datasets(struct H5SeqDB* db,const char* path){
    /* Property lists. */
    hid_t dapl = H5P_DEFAULT;
    /*hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
     *H5CHK(dapl)
     *H5Pset_chunk_cache(dapl, );*/

    /* Open. */
    hid_t group = H5Gopen(db->h5file, path, H5P_DEFAULT);
    if (group < 0)
        ERROR_H5LOC("cannot open group at",db->filename,path)

    db->sdset = H5Dopen(group, H5SEQDB_SEQ_DATASET, dapl);
    if (db->sdset < 0)
        ERROR_H5LOC("cannot open sequence dataset at ",db->filename,path)

    db->idset = H5Dopen(group, H5SEQDB_ID_DATASET, dapl);
    if (db->idset < 0)
        ERROR_H5LOC("cannot open ID dataset at " ,db->filename,path)

    /* Get data space. */
    db->sspace = H5Dget_space(db->sdset);
    H5CHK(db->sspace)
    db->ispace = H5Dget_space(db->idset);
    H5CHK(db->ispace)

    /* Get dimensions. */
    hsize_t sdims[2] = { 0, 0 };
    hsize_t idims[2] = { 0, 0 };
    H5TRY(H5Sget_simple_extent_dims(db->sspace, sdims, NULL))
    H5TRY(H5Sget_simple_extent_dims(db->ispace, idims, NULL))
    if (sdims[0] != idims[0])
        ERROR_H5LOC("sequence and ID datasets have differend sizes at",db->filename,path)

    db->nrecords = sdims[0];
    fprintf(stderr,"found %zu records at %s : %s\n",db->nrecords,db->filename,path);

    db->slen = sdims[1];
    db->ilen = idims[1];
    fprintf(stderr, "sequence length is %zu , ID length is %zu\n",db->slen , db->ilen );

    /* Read SeqPack tables from attributes. */
    uint8_t enc[SEQPACK_ENC_SIZE];
    read_array_attribute(db, H5SEQDB_ENC_BASE_ATTR,
                         H5T_NATIVE_UINT8, SEQPACK_ENC_SIZE, enc);
    setEncBase(db->pack, enc, SEQPACK_ENC_SIZE);
    read_array_attribute(db, H5SEQDB_ENC_QUAL_ATTR,
                         H5T_NATIVE_UINT8, SEQPACK_ENC_SIZE, enc);
    setEncQual(db->pack, enc, SEQPACK_ENC_SIZE);

    char dec[SEQPACK_DEC_SIZE];
    read_array_attribute(db, H5SEQDB_DEC_BASE_ATTR,
                         H5T_NATIVE_CHAR, SEQPACK_DEC_SIZE, dec);
    setDecBase(db->pack, dec, SEQPACK_DEC_SIZE);
    read_array_attribute(db, H5SEQDB_DEC_QUAL_ATTR,
                         H5T_NATIVE_CHAR, SEQPACK_DEC_SIZE, dec);
    setDecQual(db->pack, dec, SEQPACK_DEC_SIZE);

    /* Cleanup. */
    /*H5TRY(H5Pclose(dapl))*/
    H5TRY(H5Gclose(group))

}

struct H5SeqDB* H5SeqDB_Create(const char* filename,
                               char mode,
                               size_t slen,
                               size_t ilen,
                               size_t blocksize){

    /*return an entity of H5SeqDB*/
    struct H5SeqDB* db = H5SeqDB_Entity_Create(filename,mode,slen,ilen,blocksize);
    /*H5Eset_auto(H5E_DEFAULT, hdf5_exit_on_error, NULL);*/
    char* version;
    char* date;
    register_blosc(&version, &date);
    int threads = omp_get_max_threads();   /*OpenMP*/
    blosc_set_nthreads(threads);

    fprintf(stderr, "%s: using BLOSC %s ( %s ) with %d thread(s)\n",PROGNAME,version,date,threads);

    if (db->mode == MODE_READ) {

        open_hdf5_file(db, filename, H5F_ACC_RDONLY);
        /* Opening the datasets will update slen and ilen. */
        open_datasets(db, H5SEQDB_GROUP);
        db->nread = 0;
        setLength(db->pack, db->slen);
    } else {
        /*
        if (mode == MODE_TRUNCATE) {
            create_file(filename, H5F_ACC_TRUNC);
        } else {
            open_file(filename, H5F_ACC_RDWR);
        }
        create_datasets(H5SEQDB_GROUP);
        nrecords = 0;
         */
        ERROR("Assessing Mode Wrong.");
    }

    if (db->slen <= 0 || db->ilen <= 0)
        ERROR("sequence or ID length is non-positive")

    alloc_buffers(db);

    hsize_t sdims[2] = { blocksize, db->slen };
    db->smemspace = H5Screate_simple(2, sdims, NULL);
    H5CHK(db->smemspace)

    hsize_t idims[2] = { blocksize, db->ilen };
    db->imemspace = H5Screate_simple(2, idims, NULL);
    H5CHK(db->imemspace)

    return db;
}

struct H5SeqDB* open_input(const char* filename){
    if (H5Fis_hdf5(filename)) {
        size_t blocksize = 16*1024;
        char* s = getenv("SEQDB_BLOCKSIZE");
        if (s != NULL) {
            blocksize = atoi(s);
            fprintf(stderr,"%s : overriding default HDF5 blocksize to %zu\n",PROGNAME,blocksize);
        }
        return H5SeqDB_Create(filename, MODE_READ, 0, 0, blocksize);
    } else {
        ERROR("cannot determine storage layer for the input file")
    }
}

void export_block(struct H5SeqDB* db, FILE* f, hsize_t count){
    /* Load the next block. */
    flush_reads(db);
    /* ibuf and sbuf have been set in flush_read(); */
    const char* i = db->ibuf;
    const char* s = db->sbuf;

    /* Print each sequence. */
    for (hsize_t j=0; j<count; j++) {
        printFASTQ(f, i, s, s + db->slen, db->ilen, db->slen);
        /* Advance the buffers. */
        i += db->ilen;
        s += 2*db->slen;
    }
    db->nread += count;
}

void exportFASTQ(struct H5SeqDB* db, FILE* f){
    /* Iterate over the full read blocks. */
    hsize_t nblocks = db->nrecords / db->blocksize;
    for (hsize_t i=0; i<nblocks; i++) export_block(db,f, db->blocksize);

    /* Export any remaining records. */
    hsize_t remainder = db->nrecords % db->blocksize;
    if (remainder) export_block(db,f, remainder);
}
