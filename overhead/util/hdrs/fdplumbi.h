/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

#define FDLEAK_OPENCODE_OPEN 1
#define FDLEAK_OPENCODE_FOPEN 2
#define FDLEAK_OPENCODE_POPEN 3
#define FDLEAK_OPENCODE_QOPEN 4
#define FDLEAK_OPENCODE_TOPEN 5
#define FDLEAK_OPENCODE_T2OPEN 6
#define FDLEAK_OPENCODE_CREAT 7
#define FDLEAK_OPENCODE_DUP 8
#define FDLEAK_OPENCODE_PIPE 9
#define FDLEAK_OPENCODE_SOCKET 10
#define FDLEAK_OPENCODE_OPENDIR 11

void RegisterOpenFile(int fd, const char *path, int Code);
void RegisterCloseFile(int fd);

ENDCPLUSPLUSPROTOS

