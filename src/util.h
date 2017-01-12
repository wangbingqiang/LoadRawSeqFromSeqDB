/*
 * Created by JR_Young on 2017/1/3.
*/

#ifndef LOADSEQDB_C_UTIL_H
#define LOADSEQDB_C_UTIL_H


#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>


/* workaround to get variable as a string */
#define STRINGIFY(x) #x



#define NOTIFY(msg) fprintf(stderr, "%s: %s \n",PROGNAME,msg);
#define ERROR(msg) do{NOTIFY(msg) exit(EXIT_FAILURE);}while(0);
#define PERROR(msg) do{\
	NOTIFY(msg) perror(PROGNAME); exit(EXIT_FAILURE);\
}while(0);
#define ARG_ERROR(msg) do{NOTIFY(msg) print_usage(); exit(EXIT_FAILURE);}while(0);

#define CHECK_ERR(call) do{\
	errno = 0;\
	call;\
	if (errno > 0) { \
        fprintf(stderr, "%s: error at %s : %d\n",PROGNAME,__FILE__,__LINE__);\
        exit(EXIT_FAILURE); }\
}while(0);

#define PRINT_VERSION printf("SeqDB %s", PACKAGE_VERSION );



#endif /*LOADSEQDB_C_UTIL_H*/
