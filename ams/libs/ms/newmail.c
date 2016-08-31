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

/*
        newmail.c -- Routines for accessing incoming mail files in the raw.
*/

#include <andrewos.h>
#include <andyenv.h>
#include <ms.h>
#include <hdrparse.h>

extern char    home[];
extern struct MS_Directory *MyOpenDir;

/* As of 23-May, "Code" is only ever "PROCESSNEW_MBOX" */
/* Note: All of the following code is highly bogus.
  * It supports SPEC, which no longer exists, and the
  * old-style recon, which no longer exists.
  * Code, NumDirInsertions and IsReconstruction are all superfluous.
  */

ProcessNewMail(ThisFileName, ParseSpec, Code, UnlinkFailures, EliErrBuf, EliErrBufLim)
char           *ThisFileName, *ParseSpec, *EliErrBuf;
int             Code, EliErrBufLim;
int            *UnlinkFailures;
{
    char *s;
    Boolean         IsReconstruction;
    int             NumDirInsertions = 0;
    struct MS_Message *Msg;

    if (Code == PROCESSNEW_MBOX) {
	IsReconstruction = FALSE;
    }
    else {
	IsReconstruction = TRUE;
    }
    Msg = (struct MS_Message *) malloc(sizeof(struct MS_Message));
    if (Msg == NULL) {
	AMS_RETURN_ERRCODE(ENOMEM, EIN_MALLOC, EVIA_PROCNEWMSGS);
    }
    bzero(Msg, sizeof(struct MS_Message));
    Msg->OpenFD = -1;
    if (ReadRawFile(ThisFileName, Msg, TRUE)
	 || ParseMessageFromRawBody(Msg)
	 || CheckAuthUid(Msg)
	 || (InventID(Msg)
	     || Flames_HandleNewMessage(Msg, ParseSpec,
					&NumDirInsertions,
					(strncmp(ThisFileName, home, strlen(home))) ? FALSE : TRUE,
					ThisFileName,
					EliErrBuf,
					EliErrBufLim))) {        /* Whew! -- bobg */
	FreeMessage(Msg, TRUE);
	return (mserrcode);
    }
    if (!IsReconstruction) {
	AMS_SET_ATTRIBUTE(Msg->Snapshot, AMS_ATT_UNSEEN);
    }
    if (IsReconstruction) {
	s = strrchr(ThisFileName, '/');
	if (s && *++s == '+' && strlen(++s) < AMS_IDSIZE) {
	    strcpy(AMS_ID(Msg->Snapshot), s);
	}
    }

    FreeMessage(Msg, TRUE);
    if (NumDirInsertions == 0) {
	debug(4, ("FLAMES seems to have satisfactorily handled this file, I'll unlink %s\n", ThisFileName));
	unlink(ThisFileName);          /* ignore errors -- might have been
					* renamed out of the way */
    }
    return (0);
}
