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

/* Methods for Group List Entries
*/

#include <andrewos.h>
#include <big.h>

void            GLESetBefore(gle, val)
GListEntry_t   *gle;
int             val;
{
    gle->before = val;
}

void            GLESetAhead(gle, val)
GListEntry_t   *gle;
int             val;
{
    gle->ahead = val;
}

char           *GLEGetFilename(gle)
GListEntry_t   *gle;
{
    return (gle->filename);
}

void            GLESet(gle, filename, folder, ahead, before, ignore, time)
GListEntry_t   *gle;
char           *filename, *folder;
int             ahead, before, ignore;
long            time;
{
    gle->filename = filename;
    gle->folder = folder;
    gle->ahead = ahead;
    gle->before = before;
    gle->ignore = ignore;
    gle->time = time;
}

void            GLESetIgnore(gle, ignore)
GListEntry_t   *gle;
int             ignore;
{
    gle->ignore = ignore;
}

int             GLEGetIgnore(gle)
GListEntry_t   *gle;
{
    return (gle->ignore);
}

char           *GLEGetFolder(gle)
GListEntry_t   *gle;
{
    return (gle->folder);
}

int             GLEGetBefore(gle)
GListEntry_t   *gle;
{
    return (gle->before);
}

int             GLEGetAhead(gle)
GListEntry_t   *gle;
{
    return (gle->ahead);
}

int             GLECompare(gle1, gle2)
GListEntry_t   *gle1, *gle2;
{
    int             result = strcmp(gle1->folder, gle2->folder);

    if (!result)
	result = (int) (gle1->time - gle2->time);
    return (result);
}
