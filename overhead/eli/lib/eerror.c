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

#include  <eerror.h>

static const char    * const eliErrStrs[] =
{
    "Error from ELI client",
    "Out of Memory",
    "Bad Syntax",
    "Unbound Atom",
    "Undefined Function",
    "Bad Function Parameter",
    "Bad Argument",
    "Symbol Does Not Exist",
    "Wrong Number of Arguments to Function",
    "System Error",
    "User Error"
};

/* Definition of EliError, the all-purpose error-handler */

void            EliError(st, errtype, node, loc, unixerr)
EliState_t     *st;
int             errtype;
EliSexp_t      *node;
char           *loc;
int unixerr;
{
    void (*fn)();

    eliErr_Set(st, EliErrNode(st), errtype, node, loc, unixerr);
    st->g_errflag = TRUE;
    if (errtype & EliCatchMask(st)) {
        fn = EliCatchFn(st);
        (*fn)(st);
    }
}

char           *EliErrStr(code)        /* If code is a power of two <= 128,
                                        * this will return a string from
                                        * eliErrStrs. */
int             code;
{
    int             i, j;

    for (i = code, j = 0; i; (i = (i >> 1)), ++j);
    return (eliErrStrs[j]);
}

void            eliyyerror(s)
char           *s;
{
}

int             eliyywrap()
{
    EliProcessInfo.u_wrap = TRUE;
    return (1);
}
