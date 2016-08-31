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

/* ************************************************************ *\
	wpcbase.c
	Compatibility routines that choose a cell to use.
	Include file ``wp.h'' declares the procedures for clients.
\* ************************************************************ */

#include <andrewos.h>		/* sys/file.h sys/types.h sys/time.h strings.h */
#include <andyenv.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <truth.h> /* itc.h -> truth.h DMT */
#include <sys/stat.h>
#include <pwd.h>
#include <util.h>
#ifdef WHITEPAGES_ENV  /* avoid makedepend "errors" */
#include <bt.h>
#include <wp.h>
#include <btwp.h>
#endif /* WHITEPAGES_ENV   */
#include <svcconf.h>


/* Compatibility routines for non-cellular use */
struct wp_CD *ThisCellDir = NULL;

wp_ErrorCode wp_Initialize()
{
    wp_ErrorCode wpErr;
    /* char DomainName[200]; */
    int rc;

    if (ThisCellDir != NULL && ThisCellDir->Tag != wpcdTag) ThisCellDir = NULL;
    if (ThisCellDir == NULL) {	/* Need to default this from somewhere. */
	CheckServiceConfiguration();
	/* rc = GetCurrentWSCell(DomainName, sizeof(DomainName)); */
	/* if (rc != 0) return (rc + wperr_FileSystemErrorBegin); */
	/* Try to get the WP for this domain. */
	/* wpErr = wp_InitializeCell(DomainName, (struct wp_cd **) &ThisCellDir); */
	wpErr = wp_InitializeCell(ThisDomain, (struct wp_cd **) &ThisCellDir);
	return wpErr;	/* First initialization: Done as much as possible. */
    }

    /* Subsequent times: just bump the use count. */
    ++(ThisCellDir->TimesInited);
    return wperr_NoError;
}

wp_ErrorCode wp_Terminate()
{
    int OldInited;
    wp_ErrorCode res;

    if (ThisCellDir == NULL) return wperr_NotInited;
    OldInited = ThisCellDir->TimesInited;
    res = cwp_Terminate((struct wp_cd *) ThisCellDir);
    if (OldInited <= 1) ThisCellDir = NULL;	/* Was freed */
    return res;
}

wp_ErrorCode wp_GetNIDOnly(NID, PKPtr)
int NID; wp_PrimeKey *PKPtr;
{
    if (ThisCellDir == NULL) return wperr_NotInited;
    return cwp_GetNIDOnly((struct wp_cd *) ThisCellDir, NID, PKPtr);
}

wp_ErrorCode wp_GetUIDOnly(UID, PKPtr)
char *UID; wp_PrimeKey *PKPtr;
{
    if (ThisCellDir == NULL) return wperr_NotInited;
    return cwp_GetUIDOnly((struct wp_cd *) ThisCellDir, UID, PKPtr);
}

wp_ErrorCode wp_Read(PKey, FieldIx, FValPtr)
wp_PrimeKey PKey; wp_FieldIndex FieldIx; char **FValPtr;
{
    if (ThisCellDir == NULL) return wperr_NotInited;
    return cwp_Read((struct wp_cd *) ThisCellDir, PKey, FieldIx, FValPtr);
}
