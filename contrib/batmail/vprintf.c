#include <stdio.h>
#include <varargs.h>

/* sigh. some benighted architectures haven't got _iobuf (!)
so we use a file,  write, rewind, and reread.  
Maybe this won't actually do i/o  (maybe the earth is flat) */


#define MAXLEN 200	/* max output for normal vsprintf */

/* this is a non-standard version of vsprintf that takes a max output length */
void vsnprintf(buf,maxlen,format,args)
char	*buf,*format;
int 	maxlen;
va_list	*args;
{
    long finish, i;
    static FILE *tmp = NULL;
    static char *tmpname = "/tmp/My G0l1y < This=odd!";

    if (tmp == NULL) {
	tmp = fopen(tmpname, "w+");
	unlink(tmpname);
    }
    else rewind(tmp);

    _doprnt(format,args,tmp);
    putc('\0',tmp);
    rewind(tmp);
    for (i = 0; i < maxlen && (buf[i] = getc(tmp)); i++)
	{}
}

#if 0

void vsprintf(buf,format,args)
char	*buf,*format;
va_list	*args;
{
    vsnprintf(buf,MAXLEN,format,args);
}

void vprintf(format,args)
char	*format;
va_list	*args;
{
    _doprnt(format,args,stdout);
}

void vfprintf(stream,format,args)
FILE	*stream;
char	*format;
va_list	*args;
{
    _doprnt(format,args,stream);
}

#endif /* 0 */
