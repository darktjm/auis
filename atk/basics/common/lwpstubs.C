/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/lwpstubs.C,v 3.2 1993/07/29 20:15:08 rr2b Stab74 $";
#endif


 

/* Stub routines incase we are not linked against the LWP library. */

#ifdef LWP

/* this is a dummy iomgr_select.  It is linked in only if IOMGR is not being used */

#ifndef NORCSID
#endif
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
