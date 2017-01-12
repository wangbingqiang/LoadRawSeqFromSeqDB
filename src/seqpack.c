/*Created by JR_Young on 2017/1/4.
 *
 *LoadRawSeqFromSeqDB
 *https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
 *A tool to load raw sequence data from a SeqDB file , written in ANSI C;
 *derived from SeqDB https://bitbucket.org/mhowison/seqdb
 */

#include "seqpack.h"
#include <string.h>
#include <stdlib.h>
#define PROGNAME "seqpack"
#include "util.h"

static char bases[5] = { 'N', 'A', 'T', 'C', 'G' };

/* hard-coded phred33 quality scores */
static char quals[51] = { '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S' };


struct SeqPack* SeqPack_Create(int length_){
    struct SeqPack* sp = (struct SeqPack *)malloc(sizeof(struct SeqPack));
    if(sp==NULL)
        ERROR("failed in initilizing SeqPack")
    /* initialize elements */
    sp->length=length_;
    memset(sp->enc_base, 0x00, SEQPACK_ENC_SIZE);
    memset(sp->enc_qual, 0x00, SEQPACK_ENC_SIZE);
    memset(sp->dec_base, 0x00, SEQPACK_DEC_SIZE);
    memset(sp->dec_qual, 0x00, SEQPACK_DEC_SIZE);

    /* create encoding tables */
    for (uint8_t i=0; i<=4; i++) {
        int j = (int)bases[i];
        if (j < 0 || j > 127) ERROR("internal error building encoding table")
        sp->enc_base[j] = i;
    }
    for (uint8_t i=1; i<=51; i++) {
        int j = (int)quals[i-1];
        if (j < 0 || j > 127) ERROR("internal error building encoding table")
        sp->enc_qual[j] = i;
    }

    /* create decoding tables */
    for (int i=0; i<=4; i++) {
        for (int j=1; j<=51; j++) {
            sp->dec_base[i*51 + j] = bases[i];
            sp->dec_qual[i*51 + j] = quals[j-1];
        }
    }

    return sp;
}

void SeqPack_Free(struct SeqPack **p2sp){
    free(*p2sp);
    *p2sp=NULL;
    return ;
}

void setLength(struct SeqPack* sp, int length_){
    sp->length = length_;
}

void setEncBase(struct SeqPack* sp, const uint8_t* _enc_base, size_t len){
    if (len != SEQPACK_ENC_SIZE) ERROR("incorrect size for encoding table")
    memcpy(sp->enc_base, _enc_base, SEQPACK_ENC_SIZE);
}

void setEncQual(struct SeqPack* sp, const uint8_t* _enc_qual, size_t len){
    if (len != SEQPACK_ENC_SIZE) ERROR("incorrect size for encoding table")
    memcpy(sp->enc_qual, _enc_qual, SEQPACK_ENC_SIZE);
}

void setDecBase(struct SeqPack* sp, const char* _dec_base, size_t len){
    if (len != SEQPACK_DEC_SIZE) ERROR("incorrect size for decoding table")
    memcpy(sp->dec_base, _dec_base, SEQPACK_DEC_SIZE);
}

void setDecQual(struct SeqPack* sp, const char* _dec_qual, size_t len){
    if (len != SEQPACK_DEC_SIZE) ERROR("incorrect size for decoding table")
    memcpy(sp->dec_qual, _dec_qual, SEQPACK_DEC_SIZE);
}

void unpack(struct SeqPack* sp, const uint8_t* record, char* seq, char* qual){
    for (int i=0; i<(sp->length); i++) {
        uint8_t j = record[i];
        seq[i] = sp->dec_base[j];
        qual[i] = sp->dec_qual[j];
    }
}

void parunpack(	struct SeqPack* sp, size_t n, const uint8_t* src, char* dst){
    #pragma omp parallel for
    for (size_t i=0; i<n; i++) {
        unpack(sp, src + i*sp->length, dst + 2*i*sp->length, dst + (2*i+1)*sp->length);
    }
}

