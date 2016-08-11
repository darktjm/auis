#include <stdio.h>
#include <com.h>

#define BUFLEN	5000

main()
{
    int c;
    char    buf[BUFLEN], *p=buf;

    headfilter_init(TRUE,"from:date:subject");
    headfilter_begin(stdout,79,TRUE);

    while(!feof(stdin)){
	p=buf;
	while((p-buf)<BUFLEN && (c=getchar())!=EOF)
	    *p++=c;
	if(headfilter_filter(buf,p-buf)!=NULL)
	    break;
    }

    headfilter_end();
}
