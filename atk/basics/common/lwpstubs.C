/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* Stub routines incase we are not linked against the LWP library. */

#ifdef LWP
#include <andrewos.h>

/* this is a dummy iomgr_select.  It is linked in only if IOMGR is not being used */

#ifdef LWP
IOMGR_Select(long  num , long  rfs , long  wfs , long  xfs, struct timeval  *timeout);
LWP_CurrentProcess();
IOMGR_SoftSig();
IOMGR_Cancel();
#endif /* LWP */


IOMGR_Select(long  num , long  rfs , long  wfs , long  xfs, struct timeval  *timeout)
        {
    return select(num, rfs, wfs, xfs, timeout);
}

LWP_CurrentProcess()
{
}

IOMGR_SoftSig()
{
}

IOMGR_Cancel()
{
}
#endif /* LWP */
