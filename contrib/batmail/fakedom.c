#include <stdio.h>
#include <ctype.h>
#include <regexp.h>

#include "robin.h"

/* actually contains both replacement & domain, separated by '\0' */
static struct fakedom {
    struct fakedom *next;
    struct regexp *domainRegexp;
    char replacement[1];
} *fakedoms=NULL;

void fakedom_Read(fn)
char *fn;
{
    char buf[MAXADDR*2];
    FILE *fp;

    fp=fopen(fn,"r");
    if(fp==NULL)
	return;

    while(fgets(buf,sizeof(buf),fp)!=NULL){
	char *al,*exp;

	for(al=buf; *al!='\0' && isspace(*al); al++)
	    ;

	for(exp=al; *exp!='\0' && !isspace(*exp); exp++)
	    if(isupper(*exp))
		*exp=tolower(*exp);

	if(*exp!='\0'){
	    char *expEnd;
	    struct fakedom *fd;

	    *exp++='\0';

	    while(isspace(*exp))
		exp++;
	    for(expEnd=exp; *expEnd!='\0' && !isspace(*expEnd); expEnd++)
		;
	    
	    *expEnd='\0';

	    fd=(struct fakedom *)malloc(sizeof(struct fakedom)+expEnd-exp+1);

	    fd->next=fakedoms;
	    fakedoms=fd;

	    fd->domainRegexp=(struct regexp *)reg_comp(al);
	    strcpy(fd->replacement,exp);
	}
    }
}

#if 0
static char *suffixStart(sfx,addr,sep)
char *sfx,*addr,sep;
{
    char *sfxEnd=sfx, *addrEnd=addr;

    while(*sfxEnd!='\0')
	sfxEnd++;
    while(*addrEnd!='\0')
	addrEnd++;

    if(addrEnd-addr<sfxEnd-sfx)
	return NULL;

    while(sfxEnd>sfx){
	int addrCh=(*--addrEnd);
	if(isupper(addrCh))
	    addrCh=tolower(addrCh);
	if(*--sfxEnd!=addrCh)
	    return NULL;
    }

    if(addrEnd-->addr && *addrEnd==sep)
	return addrEnd;
    else
    	return NULL;
}
#endif

char *fakedom_ReplaceAddr(addr)
char *addr;
{
    static char buf[MAXADDR];
    struct fakedom *fd;

    for(fd=fakedoms; fd!=NULL; fd=fd->next){
	if(reg_exec(fd->domainRegexp,addr)){
	    reg_sub(fd->domainRegexp,fd->replacement,buf);
	    return buf;
	}
    }

    return addr;
}
