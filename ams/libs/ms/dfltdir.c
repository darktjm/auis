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

#include <andrewos.h>
#include <ms.h>

FindDefaultDir(Root, Name)
char *Root, *Name;
{
    int NumGood, NumBad;
    struct MS_Directory *Dir;

    sprintf(Name, "%s/%s", Root, AMS_DEFAULTMAILDIR);
    mserrcode = ReadOrFindMSDir(Name, &Dir, MD_OK);
    if (mserrcode == 0) {
	return(0);
    }
    if (AMS_ERRNO == EMSBADDIRFORMAT) {
	NonfatalBizarreError("Your mail directory was corrupted and is being automatically reconstructed.  Please wait...");
	return(MS_ReconstructDirectory(Name, &NumGood, &NumBad, TRUE));
    }
    if (AMS_ERRNO != ENOENT) {
	return(mserrcode);
    }
    sprintf(Name, "%s/misc", Root);
    mserrcode = ReadOrFindMSDir(Name, &Dir, MD_OK);
    if (mserrcode == 0) {
	return(0);
    }
    if (AMS_ERRNO == EMSBADDIRFORMAT) {
	NonfatalBizarreError("Your misc directory was corrupted and is being automatically reconstructed.  Please wait...");
	return(MS_ReconstructDirectory(Name, &NumGood, &NumBad, TRUE));
    }
    if (AMS_ERRNO != ENOENT) {
	return(mserrcode);
    }
    /* Could try other things here eventually */
    return(mserrcode);
}
