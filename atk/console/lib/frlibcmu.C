/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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

#include <andrewos.h>
#define NOTFOUND -1
#define AMBIGUOUS -2
#include <ctype.h>

#include <util.h>

#include "console.h"

int stablk (const char  *arg ,const char  * const *table,int  quiet /* ignored */)
{
    int i,ix = 0,count;

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
