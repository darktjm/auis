#ifndef _scache_h_
#define _scache_h_
#include <atkproto.h>
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libcmenu
 * @{ */

BEGINCPLUSPLUSPROTOS
const char *scache_Hold(const char *str);
void scache_Free(const char *str);
void scache_Collect(void);
ENDCPLUSPLUSPROTOS
/** @} */
#endif
