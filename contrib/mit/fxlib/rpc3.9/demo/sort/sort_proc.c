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

#include <rpc/rpc.h>
#include "sort.h"

static int
comparestrings(sp1, sp2)
    char **sp1, **sp2;
{
    return (strcmp(*sp1, *sp2));
}

struct sortstrings *
sort_1(ssp)
    struct sortstrings *ssp;
{
    static struct sortstrings ss_res;

    if (ss_res.ss.ss_val != (str *)NULL)
        free(ss_res.ss.ss_val);

    qsort(ssp->ss.ss_val, ssp->ss.ss_len, sizeof (char *), comparestrings);
    ss_res.ss.ss_len = ssp->ss.ss_len;
    ss_res.ss.ss_val = (str *)malloc(ssp->ss.ss_len * sizeof(str *));
    bcopy(ssp->ss.ss_val, ss_res.ss.ss_val,
        ssp->ss.ss_len * sizeof(str *));
    return(&ss_res);
}
