/*
 * 20 April 1987 - Miles Bader
 * Last edit by Miles Bader (bader) on Thu,  4 Jun 1987 -  7:39pm
 */

#include <ctype.h>

#include "com.h"
#include "robin.h"
#include "hash.h"

HASHTABLE   *hash_New(df)
VOID	(*df)();
{
    int	    i;
    HASHTABLE	*table=NEW(HASHTABLE);

    table->deleteFunc=df;
    for(i=0;i<HASHSIZE;i++)
	table->table[i]=NULL;

    return table;
}

#define hashVal(ohv,c) ((((ohv)<<5)&0x0fff)^(c))

VOID	*hash_Find(table,key)
HASHTABLE   *table;
char	    *key;
{
    char	    *str=key;
    register int    hv=0;
    HASHENTRY	    *pl;

    while(*str){
	register char	c= *str++;
	if(isupper(c))
	    c=tolower(c);
	hv=hashVal(hv,c);
    }

    pl=table->table[hv%HASHSIZE];
    while(pl!=NULL)
	if(lcstrcmp(pl->name,key)==0)
	    return pl->value;
	else
	    pl=pl->next;

    return NULL;
}

VOID hash_Install(table,key,value)
HASHTABLE   *table;
char	    *key;
VOID	    *value;
{
    register int    hv=0;
    HASHENTRY	    **plP;
    char	    *str=key;

    while(*str){
	register char	c= *str++;
	if(isupper(c))
	    c=tolower(c);
	hv=hashVal(hv,c);
    }

    plP= &table->table[hv%HASHSIZE];
    while(*plP!=NULL)
	if(lcstrcmp((*plP)->name,str)==0){
	    if((*plP)->value!=NULL && table->deleteFunc!=NULL)
		(*table->deleteFunc)((*plP)->value);
	    (*plP)->value=value;
	    return;
	}else
	    plP= &(*plP)->next;

    *plP=NEW(HASHENTRY);
    (*plP)->next=NULL;
    (*plP)->name=(char *)strcpy((char *)malloc(strlen(key)+1),key);
    (*plP)->value=value;
}
