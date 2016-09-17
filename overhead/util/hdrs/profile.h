/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */
struct proFile {
    long lastmod;
    struct proFile *next;
    char name[1];
};

/* MISSING DEFINITION *proFileList() */
struct proFile  *proFileList();

ENDCPLUSPLUSPROTOS

