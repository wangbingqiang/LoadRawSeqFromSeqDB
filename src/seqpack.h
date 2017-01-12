/*Created by JR_Young on 2017/1/4.
 *
 *LoadRawSeqFromSeqDB
 *https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
 *A tool to load raw sequence data from a SeqDB file , written in ANSI C;
 *derived from SeqDB https://bitbucket.org/mhowison/seqdb
 * This file contains declarations of struct SeqPack, and functions that are realated to SeqPack.
 */


#ifndef LOADSEQDB_C_SEQPACK_H
#define LOADSEQDB_C_SEQPACK_H

#include <stdlib.h>
#include <inttypes.h>

#define SEQPACK_ENC_SIZE 128
#define SEQPACK_DEC_SIZE 256


struct SeqPack{
    int length;    /* sequence length */
    int qual_offset;
    uint8_t enc_base[SEQPACK_ENC_SIZE];
    uint8_t enc_qual[SEQPACK_ENC_SIZE];
    char dec_base[SEQPACK_DEC_SIZE];
    char dec_qual[SEQPACK_DEC_SIZE];

};

/*return an address of a new SeqPack, just like a constructor*/
struct SeqPack* SeqPack_Create(int length_);

/*free the memory , just like a destructor*/
void SeqPack_Free(struct SeqPack **p2sp);

void setLength(struct SeqPack* sp, int length_);

void setEncBase(struct SeqPack* sp, const uint8_t* _enc_base, size_t len);

void setEncQual(struct SeqPack* sp, const uint8_t* _enc_qual, size_t len);

void setDecBase(struct SeqPack* sp, const char* _dec_base, size_t len);

void setDecQual(struct SeqPack* sp, const char* _dec_qual, size_t len);

void unpack(struct SeqPack* sp, const uint8_t* record, char* seq, char* qual);

void parunpack(	struct SeqPack* sp, size_t n, const uint8_t* src, char* dst);



#endif /*LOADSEQDB_C_SEQPACK_H*/
