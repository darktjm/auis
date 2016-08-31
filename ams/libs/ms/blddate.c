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

#include <andrewos.h> /* sys/time.h */
#include <ms.h>
#include <hdrparse.h>

BuildDateField(Msg, datetype)
struct MS_Message *Msg;
int datetype;
{
    unsigned long when;
    char DateBuf[250];
    struct tm TmBuf;

    debug(1, ("Build date field\n"));
    switch(datetype) {
	case DATETYPE_FROMHEADER:
	    if (Msg->ParsedStuff->HeadBody[HP_DATE]) {
		when = Msg->ParsedStuff->HeadBodyLen[HP_DATE];
		if (when > (sizeof(DateBuf)-1)) when = (sizeof(DateBuf)-1);
		strncpy(DateBuf, Msg->ParsedStuff->HeadBody[HP_DATE], when);
		DateBuf[when] = '\0';
		if (!parsedateheader(DateBuf, &TmBuf, 1, 1, 1, 0 /* &when */)) {
		    when = (unsigned long) gtime(&TmBuf);
		    if (when <= ((unsigned long) time(0) + 7*24*60*60) && when <= (unsigned long) 0xc0000000) break;
		}
	    } /* drop through */
	case DATETYPE_FROMFILE:
	    when = (unsigned long) Msg->RawFileDate;
	    if (when <= ((unsigned long) time(0) + 7*24*60*60) && when <= (unsigned long) 0xc0000000) break;
	case DATETYPE_CURRENT:
	default:
	    when = (unsigned long) time(0);
	    if (when > ((unsigned long) time(0) + 7*24*60*60) || when > (unsigned long) 0xc0000000) {
		AMS_RETURN_ERRCODE(EMSCLOCKBOGUS, EIN_PARAMCHECK, EVIA_BUILDDATE);
	    }
	    break;
    }
    strcpy(AMS_DATE(Msg->Snapshot), convlongto64(when, 0));
    return(0);
}
