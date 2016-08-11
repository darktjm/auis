/*
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/com_err/RCS/init_et.c,v 1.4 1994/02/02 21:44:20 sa3e Stab74 $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/com_err/RCS/init_et.c,v $
 * $Locker:  $
 *
 * Copyright 1986, 1987, 1988 by MIT Information Systems and
 *	the MIT Student Information Processing Board.
 *
 * For copyright info, see mitcopyright.h.
 */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/com_err/RCS/init_et.c,v 1.4 1994/02/02 21:44:20 sa3e Stab74 $";
#endif

#include <stdio.h>
#include "error_table.h"
#include "mitcopyright.h"

#ifndef __STDC__
#define const
#endif



extern char *malloc(), *realloc();

struct foobar {
    struct et_list etl;
    struct error_table et;
};

extern struct et_list * _et_list;

int init_error_table(msgs, base, count)
    const char * const * msgs;
    int base;
    int count;
{
    struct foobar * new_et;

    if (!base || !count || !msgs)
	return 0;

    new_et = (struct foobar *) malloc(sizeof(struct foobar));
    if (!new_et)
	return errno;	
    new_et->etl.table = &new_et->et;
    new_et->et.msgs = msgs;
    new_et->et.base = base;
    new_et->et.n_msgs= count;

    new_et->etl.next = _et_list;
    _et_list = &new_et->etl;
    return 0;
}
