#ifndef _scache_h_
#define _scache_h_
#include <atkproto.h>
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libcmenu
 * @{ */

struct scache_el {
    long refcount;
    char str[1];
};

struct scache_node {
    unsigned long els, maxels;
    struct scache_el **scache_el;
};

#define scache_REFCOUNT(string) (*(long *)((string)-(long)((struct scache_el *)0)->str))
/* this hash function is pretty poor as far as magic functions go, but it's not too bad... */


BEGINCPLUSPLUSPROTOS
const char *scache_Hold(const char *str);
void scache_Free(const char *str);
void scache_Collect(void);
ENDCPLUSPLUSPROTOS
/** @} */
#endif
