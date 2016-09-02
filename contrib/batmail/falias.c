#include <stdio.h>
#include <ctype.h>

#include "robin.h"

/* expansion actually contains both expansion & alias, separated by '\0' */
static struct fAlias {
    struct fAlias *next;
    char *alias,expansion[1];
} *folderAliases=NULL;

void falias_Read(fn)
char *fn;
{
    char buf[MAXFOLDERNAME*2];
    FILE *fp;

    fp=fopen(fn,"r");
    if(fp==NULL)
	return;

    while(fgets(buf,sizeof(buf),fp)!=NULL){
	char *al,*exp;

	for(al=buf; *al!='\0' && isspace(*al); al++)
	    ;

	for(exp=al; *exp!='\0' && !isspace(*exp); exp++)
	    ;

	if(*exp!='\0'){
	    char *expEnd;
	    struct fAlias *fa;

	    *exp++='\0';

	    while(isspace(*exp))
		exp++;
	    for(expEnd=exp; *expEnd!='\0' && !isspace(*expEnd); expEnd++)
		;
	    
	    *expEnd='\0';

	    fa=(struct fAlias *)
		malloc(sizeof(struct fAlias)+strlen(al)+expEnd-exp+1);

	    fa->next=folderAliases;
	    folderAliases=fa;

	    fa->alias=fa->expansion+(expEnd-exp)+1;
	    strcpy(fa->expansion,exp);
	    strcpy(fa->alias,al);
	}
    }
}

static char *prefixedTail(pfx,name,sep)
char *pfx,*name,sep;
{
    while(*pfx==*name && *pfx!='\0')
	pfx++, name++;
    if(*pfx=='\0' && (*name==sep || *name=='\0'))
	return name;
    else
	return NULL;
}

char *falias_Unalias(folder)
char *folder;
{
    static char buf[MAXFOLDERNAME];
    struct fAlias *fa;
    for(fa=folderAliases; fa!=NULL; fa=fa->next){
	char *tail=prefixedTail(fa->alias,folder,'.');
	if(tail!=NULL){
	    strcpy(buf,fa->expansion);
	    strcat(buf,tail);
	    return buf;
	}
    }
    return folder;
}

char *falias_Alias(folder)
char *folder;
{
    static char buf[MAXFOLDERNAME];
    struct fAlias *fa;
    for(fa=folderAliases; fa!=NULL; fa=fa->next){
	char *tail=prefixedTail(fa->expansion,folder,'.');
	if(tail!=NULL){
	    strcpy(buf,fa->alias);
	    strcat(buf,tail);
	    return buf;
	}
    }
    return folder;
}
