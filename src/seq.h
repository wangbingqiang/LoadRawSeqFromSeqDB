/*
 * Created by JR_Young on 2017/1/3.
*/

#ifndef LOADSEQDB_C_SEQ_H
#define LOADSEQDB_C_SEQ_H
#include <stddef.h>
#include <stdio.h>

void printFASTQ(FILE* f, const char* idline, const char* seq, const char* qual, size_t ilen, size_t slen);

#endif /* LOADSEQDB_C_SEQ_H */
