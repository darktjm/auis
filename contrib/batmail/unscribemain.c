#include <stdio.h>

#define BUFLEN	5000

main()
{
    int c;
    char    buf[BUFLEN], *p=buf;

    unscribe_init();
    unscribe_begin(stdout,79);

    while(!feof(stdin)){
	p=buf;
	while((p-buf)<BUFLEN && (c=getchar())!=EOF)
	    *p++=c;
	unscribe_filter(buf,p-buf);
    }

    unscribe_end();
}
