#include <ctype.h>

#ifndef NULL
#define NULL 0
#endif

char *skipTo(p,c)
char *p,c;
{
    while(*p!='\0' && *p!=c)
	p++;
    return p;
}

/* finds an inline contiguous address */

char *findTokenAddr(buf,begP,endP)
char *buf,**begP,**endP;
{
    char *end=NULL, *begAddr=NULL, *endAddr=NULL;

    while(end==NULL && *buf!='\0')
	switch(*buf){
	    case ',':
		end=buf;
		break;
	    case '(':
		if(begAddr!=NULL && endAddr==NULL)
		    endAddr=buf;
		buf=skipTo(buf+1,')');
		if(*buf==')')
		    buf++;
		break;
	    case '"':
		if(begAddr==NULL)
		    begAddr=buf+1;
		buf=skipTo(buf+1,'"');
		if(*buf=='"'){
		    if(endAddr==NULL)
			endAddr=buf;
		    buf++;
		}
		break;
	    case '<':
		begAddr=buf+1;
		buf=skipTo(buf+1,'>');
		if(*buf=='>'){
		    endAddr=buf;
		    buf++;
		}
		break;
	    default:
		if(begAddr==NULL && (isalpha(*buf) || *buf=='.' || *buf=='_'))
		    begAddr=buf;
		buf++;
	}

    if(begAddr!=NULL && endAddr==NULL)
	endAddr=buf;

    if(end==NULL)
	end=buf;

    if(begAddr!=NULL){
	while(begAddr<endAddr && isspace(*begAddr))
	    begAddr++;
	while(endAddr>begAddr && isspace(endAddr[-1]))
	    endAddr--;
    }

    *begP=begAddr;
    *endP=endAddr;

    return end;
}

#ifdef TEST
main(argc,argv)
int argc;
char **argv;
{
    char addr[200];
    do{
	printf("Input an address: ");
	gets(addr);
	if(addr[0]!='\0'){
	    int i=0;
	    char *p=addr;
	    while(*p!='\0'){
		char *beg,*end;
		p=findTokenAddr(p,&beg,&end);
		if(beg!=NULL){
		    printf("Address[%d]: <",i++);
		    while(beg<end)
			putchar(*beg++);
		    putchar('>');
		    putchar('\n');
		}
		if(*p!='\0')
		    p++; /* skip the comma */
	    }
	}
    }while(addr[0]!='\0');
}
#endif
