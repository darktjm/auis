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

/* Methods for Message Cache Buckets
*/

/* BOGUS: include relevant AMS header(s) here */

#include <andrewos.h>
#include <big.h>

void            MCBInit(mcb)
MCacheBucket_t *mcb;
{
    *mcb = NULL;
}

struct MS_Message *MCBFind(mcb, string)
MCacheBucket_t *mcb;
char           *string;
{
    MCacheBucketEntry_t *mcbe = *mcb;
    struct MS_Message *result = NULL;

    while (mcbe && !result) {
	if (strcmp(MCBEGetFilename(mcbe), string))
	    mcbe = MCBEGetNext(mcbe);
	else
	    result = MCBEGetMsg(mcbe);
    }
    return (result);
}

int             MCBMake(mcb, string, Msg)
MCacheBucket_t *mcb;
char           *string;
struct MS_Message *Msg;
{
    MCacheBucketEntry_t *tmp = (MCacheBucketEntry_t *) malloc(sizeof(MCacheBucketEntry_t));

    if (tmp) {
	MCBESet(tmp, string, Msg, *mcb);
	*mcb = tmp;
	return (TRUE);
    }
    return (FALSE);
}

void            MCBDelete(mcb, string)
MCacheBucket_t *mcb;
char           *string;
{
    MCacheBucketEntry_t *mcbe = *mcb, *prevmcbe = NULL;

    while (mcbe) {
	if (strcmp(MCBEGetFilename(mcbe), string)) {
	    prevmcbe = mcbe;
	    mcbe = MCBEGetNext(mcbe);
	}
	else {
	    if (prevmcbe)
		MCBESetNext(prevmcbe, MCBEGetNext(mcbe));
	    else
		*mcb = MCBEGetNext(mcbe);
	    free(mcbe);
	    mcbe = NULL;
	}
    }
}

void            MCBPurge(mcb)
MCacheBucket_t *mcb;
{
    MCacheBucketEntry_t *mcbe = *mcb, *next;

    while (mcbe) {
	next = MCBEGetNext(mcbe);
	free(mcbe);
	mcbe = next;
    }
    *mcb = NULL;
}
