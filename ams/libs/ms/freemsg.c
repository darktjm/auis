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
#include <hdrparse.h>

/* 2 routines for backward compatibility */

FreeMessage(Msg, FreeSnapshot)
struct MS_Message *Msg;
Boolean FreeSnapshot;
{
    debug(1, ("FreeMessage\n"));
    if (Msg) {
	if (Msg->OpenFD >= 0) {
	    if (Msg->OpenFD == 0) {
		CriticalBizarreError("Warning!  FreeMessage is closing file descriptor zero, which is *probably* a mistake!");
	    }
	    close(Msg->OpenFD);
	    Msg->OpenFD = -1; /* Just for sheer paranoia */
	}
	FreeMessageContents(Msg, FreeSnapshot);
	if (Msg->AuthCell) free(Msg->AuthCell);
	if (Msg->AuthName) free(Msg->AuthName);
	free(Msg);
    }
    return(0);
}

FreeMessageContents(Msg, FreeSnapshot)
struct MS_Message *Msg;
Boolean FreeSnapshot;
{
    if (!Msg) return(0);
    if (FreeSnapshot && Msg->Snapshot) {free (Msg->Snapshot); Msg->Snapshot = NULL;}
    if (Msg->RawBits) {free (Msg->RawBits); Msg->RawBits = NULL;}
    if (Msg->ReplyTo) {free (Msg->ReplyTo); Msg->ReplyTo = NULL;}
    if (Msg->WideReply) {free (Msg->WideReply); Msg->WideReply = NULL;}
    FreeParsedStuff(Msg);
    return(0);
}

FreeParsedStuff(Msg)
struct MS_Message *Msg;
{
    int i;
    if (Msg->ParsedStuff) {
	for (i = HP_END_PREDEFINED; Msg->ParsedStuff->HeadName[i]; ++i) {
	    free(Msg->ParsedStuff->HeadName[i]);
	}
	free(Msg->ParsedStuff->HeadName);
	free(Msg->ParsedStuff->HeadBody);
	free(Msg->ParsedStuff->HeadBodyLen);
	free(Msg->ParsedStuff->IsDuplicated);
	free (Msg->ParsedStuff);
	Msg->ParsedStuff = NULL;
    }
    return(0);
}

