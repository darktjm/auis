/* Symbol table manager for Bison,
   Copyright (C) 1984, 1989 Free Software Foundation, Inc.

This file is part of Bison, the GNU Compiler Compiler.

Bison is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Bison is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bison; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <stdio.h>
#include "andrewos.h"
#include "new.h"
#include "symtab.h"
#include "gram.h"
#include "proto.h"

static bucket **symtab;
bucket *firstsymbol;
static bucket *lastsymbol;


static int
hash(const char *key)
{
  const char *cp;
  int k;

  cp = key;
  k = 0;
  while (*cp)
    k = ((k << 1) ^ (*cp++)) & 0x3fff;

  return (k % TABSIZE);
}



static char *
copys(const char *s)
{
  char *result;

  result = xmalloc((unsigned int)strlen(s) + 1);
  strcpy(result, s);
  return (result);
}


void
tabinit(void)
{
/*   int i; JF unused */

  symtab = NEW2(TABSIZE, bucket *);

  firstsymbol = NULL;
  lastsymbol = NULL;
}


bucket *
getsym(const char *key)
{
  int hashval;
  bucket *bp;
  int found;

  hashval = hash(key);
  bp = symtab[hashval];

  found = 0;
  while (bp != NULL && found == 0)
    {
      if (strcmp(key, bp->tag) == 0)
	found = 1;
      else
	bp = bp->link;
    }

  if (found == 0)
    {
      nsyms++;

      bp = NEW(bucket);
      bp->link = symtab[hashval];
      bp->next = NULL;
      bp->tag = copys(key);
      bp->class = SUNKNOWN;

      if (firstsymbol == NULL)
	{
	  firstsymbol = bp;
	  lastsymbol = bp;
	}
      else
	{
	  lastsymbol->next = bp;
	  lastsymbol = bp;
	}

      symtab[hashval] = bp;
    }

  return (bp);
}


void
free_symtab(void)
{
  int i;
  bucket *bp,*bptmp;/* JF don't use ptr after free */

  for (i = 0; i < TABSIZE; i++)
    {
      bp = symtab[i];
      while (bp)
	{
	  bptmp = bp->link;
#if 0 /* This causes crashes because one string can appear more than once.  */
	  if (bp->type_name)
	    FREE(bp->type_name);
#endif
	  FREE(bp);
	  bp = bptmp;
	}
    }
  FREE(symtab);
}
