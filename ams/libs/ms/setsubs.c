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

extern char home[];

MS_SetSubscriptionEntry(FullName, NickName, status)
char *FullName; /* Value passed in to MS */
char *NickName; /* Value passed in to MS */
int status; /* Ditto */
{
    struct MS_Directory *Dir;

    if (status == AMS_UNSUBSCRIBED) {
	char DefaultMailDir[1+MAXPATHLEN],
	     MailRoot[1+MAXPATHLEN];

	sprintf(MailRoot, "%s/%s", home, MS_TREEROOT);
	if (FindDefaultDir(MailRoot, DefaultMailDir)) {
	    if (AMS_ERRNO != ENOENT) {
		return(mserrcode);
	    }
	    DefaultMailDir[0] = '\0';
	}
	if (!strcmp(DefaultMailDir, FullName)) {
	    AMS_RETURN_ERRCODE(EMSFASCISTSUBSCRIPTION, EIN_PARAMCHECK, EVIA_SETSUBSENTRY);
	}
    } else if (ReadOrFindMSDir(FullName, &Dir, MD_OK)) {
	return(mserrcode);
    }
    return(SetSubsEntry(FullName, NickName, status));
}
