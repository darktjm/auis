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
	gtvpwent.c
	getvpwent() is like getpwent(), but for Vice IDs
\* ************************************************************ */
#include <andrewos.h> /* syslog.h */
#include <andyenv.h>
#include <stdio.h>
#include <errno.h>
#include <util.h>
#ifdef WHITEPAGES_ENV
#include <pwd.h>
#include <wp.h>
#endif /* WHITEPAGES_ENV */
#include <svcconf.h>

#ifdef WHITEPAGES_ENV
static int wpOpenedHere = 0;
static char *LastPK = NULL;
static int fldEK = -1;

int setvpwent()
{/* The equivalent of wp_Initialize() */
    CheckServiceConfiguration();
    if (AMS_UseWP) {
	if (LastPK != NULL) {free(LastPK); LastPK = NULL;}
	if (wpOpenedHere == 0) {
		if (_pw_OpenWP() != 0) wpOpenedHere = 1;
	}
	return wpOpenedHere;
    } else {
	(void) setpwent();
	return 1;
    }
}

int endvpwent()
{/* The equivalent of wp_Terminate() */
    CheckServiceConfiguration();
    if (AMS_UseWP) {
	if (LastPK != NULL) {free(LastPK); LastPK = NULL;}
	if (wpOpenedHere) _pw_CloseWP();
	wpOpenedHere = 0;
	return 0;
    } else {
	(void) endpwent();
	return(0);
    }
}

struct passwd *getvpwent()
{/* Return the next Vice struct passwd */
    wp_ErrorCode Res;
    char *Value;
    extern struct passwd *_pw_getvpwkey();

    CheckServiceConfiguration();
    if (AMS_UseWP) {
	if (wpOpenedHere == 0) {
		if (setvpwent() == 0) {
			errno = ETIMEDOUT;
			return NULL;
		}
	}
	if (fldEK < 0) {
	    fldEK = wp_FieldNameToIndex("EK");
	}
	for (;;) {
		Res = wp_Generate(&LastPK);
		if (Res != wperr_NoError) {endvpwent(); errno = ETIMEDOUT; return NULL;}
		if (LastPK == NULL) {errno = ENOENT; return NULL;}
		if (fldEK < 0) break;
		else {
			Res = wp_Read(LastPK, fldEK, &Value);
			if (Res != wperr_NoError) break;
			if ((atoi(Value) & 1) != 0) break;
		}
	}
	return _pw_getvpwkey(LastPK);
    } else {
	return(getpwent());
    }
}
#endif /* WHITEPAGES_ENV */
