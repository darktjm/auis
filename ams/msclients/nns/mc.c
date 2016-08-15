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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/msclients/nns/RCS/mc.c,v 2.6 1992/12/15 21:22:34 rr2b Stab74 $";
#endif

/* Methods for the Message Cache data type for the Netnews Snarfer
*/

/* BOGUS: Include relevant AMS header(s) here */

#include <big.h>

static int      MCHash(string)
char           *string;
{
    int             result = 0;
    char           *p = string;

    while (result += *p, *(p++));
    return (result % NUMMCBUCKETS);
}

void            MCInit(mc)
MCache_t       *mc;
{
    int             i;

    mc->HashFn = MCHash;
    for (i = 0; i < NUMMCBUCKETS; ++i)
	MCBInit(&(mc->buckets[i]));
}

void            MCPurge(mc)
MCache_t       *mc;
{
    int             i;

    for (i = 0; i < NUMMCBUCKETS; ++i)
	MCBPurge(&(mc->buckets[i]));
}

int             MCMake(mc, string, Msg)
MCache_t       *mc;
char           *string;
struct MS_Message *Msg;
{
    return (MCBMake(&(mc->buckets[(*(mc->HashFn)) (string)]), string, Msg));
}

void            MCDelete(mc, string)
MCache_t       *mc;
char           *string;
{
    MCBDelete(&(mc->buckets[(*(mc->HashFn)) (string)]), string);
}

struct MS_Message *MCFind(mc, string)
MCache_t       *mc;
char           *string;
{
    return (MCBFind(&(mc->buckets[(*(mc->HashFn)) (string)]), string));
}
