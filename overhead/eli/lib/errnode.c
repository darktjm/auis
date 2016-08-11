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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/eli/lib/RCS/errnode.c,v 2.8 1992/12/15 21:01:25 rr2b Stab74 $";
#endif

#include <errnode.h>

void            eliErr_Set(st, e, code, node, loc, unixerr)
EliState_t     *st;
eliErrStuff_t  *e;
int             code;
EliSexp_t      *node;
char           *loc;
{
    EliSexp_t      *tmp = e->badnode;

    e->errnum = code;
    e->badnode = node;
    e->errloc = loc;
    e->unixerr = unixerr;
    if (node)
	eliSexp_IncrRefcount(node);
    if (tmp)
	eliSexp_DecrRefcount(st, tmp);
}

int eliErr_GetUnixErr(e)
eliErrStuff_t *e;
{
    return (e->unixerr);
}

int             eliErr_GetCode(e)
eliErrStuff_t  *e;
{
    return (e->errnum);
}

EliSexp_t      *eliErr_GetNode(e)
eliErrStuff_t  *e;
{
    return (e->badnode);
}

EliCons_t *eliErr_GetBacktrace(e)
eliErrStuff_t *e;
{
    return (e->backtrace);
}

char           *eliErr_GetLoc(e)
eliErrStuff_t  *e;
{
    return (e->errloc);
}

int             eliErr_SexpP(e)
eliErrStuff_t  *e;
{
    return ((e->badnode) != NULL);
}

void            eliErr_Init(e)
eliErrStuff_t  *e;
{
    e->badnode = NULL;
    e->backtrace = NULL;
}
