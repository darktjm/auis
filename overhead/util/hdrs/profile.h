/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

struct proFile {
    long lastmod;
    struct proFile *next;
    char name[1];
};

/* MISSING DEFINITION *proFileList() */
struct proFile  *proFileList();

ENDCPLUSPLUSPROTOS

