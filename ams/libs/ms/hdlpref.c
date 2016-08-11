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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/hdlpref.c,v 2.8 1993/06/15 03:56:02 rr2b Stab74 $";
#endif

#include <ms.h>
#include <util.h>

MS_HandlePreference(prog, pref, InVal, OutVal, OutLim, opcode, resulti, defaulti)
char *prog, *pref, *InVal; /* Passed IN */
char *OutVal; /* Passed OUT */
int OutLim, opcode, defaulti; /* Passed IN */
int *resulti; /* Passed OUT */
{
    char *s, *key;

    key = malloc(2+strlen(prog) + strlen(pref));
    if (!key) {
	AMS_RETURN_ERRCODE(ENOMEM, EIN_MALLOC, EVIA_HANDLEPREFERENCE);
    }
    sprintf(key, "%s.%s", prog, pref);
    switch(opcode) {
	case AMS_GETPROFILESTRING:
	    s = getprofile(key);
	    if (s) {
		strncpy(OutVal, s, OutLim);
	    } else {
		OutVal[0] = '\0';
	    }
	    break;
	case AMS_GETPROFILEINT:
	    *resulti = getprofileint(key, defaulti);
	    break;
	case AMS_GETPROFILESWITCH:
	    *resulti = getprofileswitch(key, defaulti);
	    break;
	case AMS_SETPROFILESTRING:
	    if (setprofilestring(prog, pref, InVal)) {
		free(key);
		AMS_RETURN_ERRCODE(errno, EIN_SETPROF, EVIA_HANDLEPREFERENCE);
	    }
	    break;
	default:
	    free(key);
	    AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_HANDLEPREFERENCE);
    }
    free(key);
    return(0);
}
