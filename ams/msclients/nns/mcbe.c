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

/* Methods for Message Cache Bucket Entries
*/

#include <andrewos.h>
#include <big.h>

char           *MCBEGetFilename(mcbe)
MCacheBucketEntry_t *mcbe;
{
    return (mcbe->filename);
}

MCacheBucketEntry_t *MCBEGetNext(mcbe)
MCacheBucketEntry_t *mcbe;
{
    return (mcbe->next);
}

struct MS_Message *MCBEGetMsg(mcbe)
MCacheBucketEntry_t *mcbe;
{
    return (mcbe->Msg);
}

void            MCBESet(mcbe, filename, Msg, next)
MCacheBucketEntry_t *mcbe;
char           *filename;
struct MS_Message *Msg;
MCacheBucketEntry_t *next;
{
    mcbe->filename = filename;
    mcbe->Msg = Msg;
    mcbe->next = next;
}

void            MCBESetNext(mcbe, next)
MCacheBucketEntry_t *mcbe, *next;
{
    mcbe->next = next;
}
