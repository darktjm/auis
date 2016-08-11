/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/hdrs/RCS/profile.h,v 2.6 1993/09/28 17:54:35 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/hdrs/RCS/profile.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_h = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/hdrs/RCS/profile.h,v 2.6 1993/09/28 17:54:35 rr2b Stab74 $ ";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

struct proFile {
    long lastmod;
    struct proFile *next;
    char name[1];
};

/* MISSING DEFINITION *proFileList() */
struct proFile  *proFileList();

ENDCPLUSPLUSPROTOS

