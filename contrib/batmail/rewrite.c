#include <andrewos.h>

static void moveDown(buf,len,by)
char *buf;
int len,by;
{
    len-=by;
    while(len-->0){
	*buf=buf[by];
	buf++;
    }
}

static void moveUp(buf,len,by)
char *buf;
int len,by;
{
    buf+=len-1;
    len-=by;
    while(len-->0){
	*buf=buf[-by];
	buf--;
    }
}

static void replaceInString(buf,len,str)
char *buf;
int len;
char *str;
{
    int slen=strlen(str), blen=strlen(buf);
    if(slen<len)
	moveDown(buf+slen,blen-slen+1,len-slen);
    else
	moveUp(buf+len,blen-len+(slen-len)+1,(slen-len));
    memmove(buf,str,slen);
}

char *rewriteAddrs(buf,func,rock)
char *buf;
char *(*func)();
char *rock;
{
    char *start=buf;
    int blen=(-1);
    int extra=0;

    while(*buf!='\0'){
	char *beg,*end,temp,*newAddr,*findTokenAddr();

	buf=findTokenAddr(buf,&beg,&end);
	if(*buf==',')
	    buf++;

	temp=(*end);
	*end='\0';

	newAddr=(*func)(beg,rock);
	*end=temp;/* restore */

	if(newAddr!=beg){
	    int len=end-beg;
	    int extend=strlen(newAddr)-len;

	    if(extend-extra>0){
		/* extend the buffer if necessary */
		int offs=beg-start;
		if(blen<0){
		    char *new=(char *)malloc((blen=strlen(start)+extend)+1);
		    strcpy(new,start);
		    start=new;
		}else
		    start=(char *)realloc(start,(blen+=extend)+1);
		beg=start+offs;
	    }else
		extra-=extend;

	    replaceInString(beg,len,newAddr);
	}
    }

    return start;
}
