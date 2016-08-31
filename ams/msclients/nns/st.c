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

/* Methods for the String Table data type for the Netnews Snarfer
*/

#include <andrewos.h>
#include <big.h>

static int      STHash(string)
char           *string;
{
    int             result = 0;
    char           *p = string;

    while (result += *p, *(p++));
    return (result % NUMSTBUCKETS);
}

void            STInit(st)
STable_t       *st;
{
    int             i;

    st->HashFn = STHash;
    for (i = 0; i < NUMSTBUCKETS; ++i)
	STBInit(&(st->buckets[i]));
}

void            STPurge(st)
STable_t       *st;
{
    int             i;

    for (i = 0; i < NUMSTBUCKETS; ++i)
	STBPurge(&(st->buckets[i]));
}

char           *STFindOrMake(st, string)
STable_t       *st;
char           *string;
{
    char           *tmp = STFind(st, string);

    if (!tmp)
	tmp = STMake(st, string);
    return (tmp);
}

char           *STFind(st, string)
STable_t       *st;
char           *string;
{
    int             hash = (*(st->HashFn)) (string);
    char           *tmp = STBFind(&(st->buckets[hash]), string);

    return (tmp);
}

char           *STMake(st, string)
STable_t       *st;
char           *string;
{
    return (STBMake(&(st->buckets[(*(st->HashFn)) (string)]), string));
}
