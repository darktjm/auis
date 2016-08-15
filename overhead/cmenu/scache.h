#ifndef _scache_h_
#define _scache_h_
#include <atkproto.h>
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


struct scache_el {
    long refcount;
    char str[1];
};

struct scache_node {
    unsigned long els, maxels;
    struct scache_el **scache_el;
};

#define scache_REFCOUNT(string) (*(long *)((string)-(long)((struct scache_el *)0)->str))
/* this hash function is pretty poor as far as magic functions go, but it's not too bad... */


#if (defined(__STDC__) && !defined(ibm032)) || defined(__cplusplus)
BEGINCPLUSPLUSPROTOS
const char *scache_Hold(const char *str);
void scache_Free(const char *str);
void scache_Collect(void);
ENDCPLUSPLUSPROTOS
#else
char *scache_Hold();
void scache_Free();
void scache_Collect();
#endif
#endif
