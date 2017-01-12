/*Created by JR_Young on 2017/1/4.
 *
 *LoadRawSeqFromSeqDB
 *https://github.com/wangbingqiang/LoadRawSeqFromSeqDB
 *A tool to load raw sequence data from a SeqDB file , written in ANSI C;
 *derived from SeqDB https://bitbucket.org/mhowison/seqdb
 */

#include <string.h>
#include "seq.h"
#define PROGNAME "seq"

void printFASTQ(
        FILE* f,
        const char* idline,
        const char* seq,
        const char* qual,
        size_t ilen,
        size_t slen) {
    fwrite(idline, 1, strnlen(idline, ilen), f);
    putc('\n', f);
    fwrite(seq, 1, strnlen(seq, slen), f);
    putc('\n', f);
    putc('+', f);
    putc('\n', f);
    fwrite(qual, 1, strnlen(qual, slen), f);
    putc('\n', f);
}

