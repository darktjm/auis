/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/wpcname.c,v 2.10 1993/06/12 12:29:34 rr2b Stab74 $";
#endif

/* ************************************************************ *\
	wpcname.c
	Compatibility routines for non-cellular White Pages lookups.
	Include file ``wp.h'' declares the procedures for clients.
\* ************************************************************ */

#include <stdio.h>
#include <andyenv.h>
#include <andrewos.h>		/* sys/file.h sys/types.h strings.h*/
#include <ctype.h>
#include <errno.h>
#include <truth.h> /* itc.h -> truth.h DMT */
#include <pwd.h>
#ifdef WHITEPAGES_ENV  /* avoid makedepend "errors" */
#include <bt.h>
#include <wp.h>
#include <btwp.h>
#endif /* WHITEPAGES_ENV   */

#if !POSIX_ENV
extern int errno;
#endif

/* Compatibility routines for non-cellular use */
extern struct wp_CD *ThisCellDir;

wp_ErrorCode wp_Search(SrchToken, MaxResults, MaxQuality, MatchQuality, PKPtr)
wp_SearchToken SrchToken; int MaxResults;
int MaxQuality, *MatchQuality; wp_PrimeKeySetPtr *PKPtr;
{
    if (ThisCellDir == NULL) return wperr_NotInited;
    return cwp_Search((struct wp_cd *) ThisCellDir, SrchToken, MaxResults,
		       MaxQuality, MatchQuality, PKPtr);
}

wp_ErrorCode wp_Lookup(SrchToken, MinMatchesFound, MaxQuality, MatchQuality, PKPtr)
wp_SearchToken SrchToken; int *MinMatchesFound;
int MaxQuality, *MatchQuality; wp_PrimeKey *PKPtr;
{
    if (ThisCellDir == NULL) return wperr_NotInited;
    return cwp_Lookup((struct wp_cd *) ThisCellDir, SrchToken, MinMatchesFound,
		       MaxQuality, MatchQuality, PKPtr);
}
