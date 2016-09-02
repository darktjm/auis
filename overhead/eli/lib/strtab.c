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

#include  <strtab.h>

/*
 * Functions to handle manipulation of the global string table 
 */

/*
 * This function checks to see if the string is in the table.  If so, its
 * node is returned.  If not, a new one is created, placed in the table, and
 * returned. 
 */

EliStr_t       *eliStringTable_FindOrMake(st, strtab, string)
EliState_t     *st;
eliHashTable_t *strtab;
char           *string;
{
    EliStr_t       *tmp;

    if (!(tmp = eliStringTable_Find(strtab, string))) {
	tmp = eliStringTable_Make(st, strtab, string);
	if (EliErr_ErrP(st))
	    return (NULL);
    }
    return (tmp);
}


/*
 * Like eliStringTable_FindOrMake, but always creates a new node. 
 */

EliStr_t       *eliStringTable_Make(st, strtab, string)
EliState_t     *st;
eliHashTable_t *strtab;
char           *string;
{
    EliStr_t       *tmp;

    if (!(tmp = eliStr_GetNew_trace(st, EliTraceStk(st), string)))
	return (NULL);		/* Observe that this test is completely
				 * unnecessary */
    return (tmp);
}


/*
 * Find the strnode whose name is name, in the global table WARNING: Assumes
 * all data in ht are strnodes! 
 */

EliStr_t       *eliStringTable_Find(strtab, name)
eliHashTable_t *strtab;
char           *name;
{
    EliSexp_t      *tmp = eliHT_Find(strtab, name);

    return (tmp ? tmp->data.datum.strval : NULL);
}
