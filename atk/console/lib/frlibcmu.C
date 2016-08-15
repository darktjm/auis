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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/frlibcmu.C,v 1.1 1993/05/25 21:29:58 gk5g Stab74 $";
#endif


 

/*  stablk  --  string table lookup
 *
 *  Usage:  i = stablk (arg,table,quiet);
 *
 *	int i;
 *	char *arg,**table;
 *	int quiet;
 *
 *  Stablk looks for a string in "table" which matches
 *  "arg".  Table is declared like this:
 *    char *table[] = {"string1","string2",...,0};
 *  Each string in the table is checked via stablk() to determine
 *  if its initial characters match arg.  If exactly one such
 *  string matches arg, then the index of that string is returned.
 *  If none match arg, or if several match, then -1 (respectively -2)
 *  is returned. 
 *
 */

#define NOTFOUND -1
#define AMBIGUOUS -2
#include <ctype.h>

#include <util.h>

#ifndef NORCSID
#endif
int stablk (char  *arg ,char  **table,int  quiet /* ignored */);


int stablk (char  *arg ,char  **table,int  quiet /* ignored */)
{
    register int i,ix = 0,count;

    count = 0;
    for (i=0; table[i] != 0; i++) {
	if (!lc_strcmp(table[i], arg)){
	    return(i);
	}
	else{
	    if (!lc_strncmp(table[i], arg, strlen(arg))){
		ix = i;
		count++;
	    }
	}
    }
    if (count == 1){
	return (ix);
    }
    else{
	return (count ? AMBIGUOUS : NOTFOUND);
    }
}
