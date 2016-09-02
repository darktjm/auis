#include <stdio.h>
#include <ctype.h>
#include <regexp.h>

#define TRUE 1
#define FALSE 0

char *caseIgnored(src,buf,len)
char *src,*buf;
int len;
{
    char *p;
    int lower;

    p=src;
    while(*p!='\0')
	if(*p=='\\' && p[1]!='\0')
	    if(isalpha(p[1])){
		lower=FALSE;
		break;
	    }else
		p+=2;
	else if(*p=='[')
	    while(p[1]!='\0' && *p!=']') /* skip char classes */
		p++;
	else if(isupper(*p)){
	    lower=FALSE;
	    break;
	}else
	    p++;

    if(lower){
	char *q=buf;

	p=src;
	while(*p!='\0' && q-buf<len-4)
	    if(*p=='\\' && p[1]!='\0'){
		if(!isalpha(p[1]))
		    *q++=*p++;
		else
		    p++;
		*q++=*p++;
	    }else if(*p=='[')
		while(p[1]!='\0' && *p!=']') /* skip char classes */
		    *q++=*p++;
	    else if(isalpha(*p)){
		int c=(isupper(*p) ? tolower(*p) : *p);
		*q++='[';
		*q++=toupper(c);
		*q++=c;
		*q++=']';
		p++;
	    }else
		*q++=*p++;
	return buf;
    }else
	return src;
}

main(argc,argv)
int argc;
char **argv;
{
    char newpat[200];
    struct regexp *re;
    char *str,*res;
    int n,count=10000;
    int result;

    if(argc<3){
	fprintf(stderr,"Usage: %s pattern string [count]\n",argv[0]);
	exit(-1);
    }

    res=caseIgnored(argv[1],newpat,200);
    re=regcomp(res);

    str=argv[2];

    if(argv[3]!=NULL)
	count=atoi(argv[3]);

    printf("%d iterations of regexec(\"%s\",\"%s\"): ",count,res,str);
    fflush(stdout);

    for(n=count; n>0; n--)
	result=regexec(re,str);

    puts(result?"matched":"didn't match");

    exit(0);
}
