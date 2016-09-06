/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>
#define GAP 1
#define BUFSIZE 5120
#define NUMSIZE 512
#define DCHAR '+'
static const char header[] =
    ".SH \n"
    "Index\n"
    ".LP \n"
    ".nr PS 8\n"
    ".nr VS 9\n"
    ".MC 1.9i\n"
    ".na	\n"
    ".de XX	\n"
    ".br	\n"
    ".ti -.2i\n"
    ".ne 2	\n"
    "..\n"
    ".de YY 	\n"
    ".sp 1.5\n"
    ".ne 3\n"
    ".ce 1\n"
    "- \\\\$1 -\n"
    ".sp .5\n"
    "..\n"
    ".in .2i	\n"
    ".hy 0";

static int cmp(int  *a,int  *b)
{
    return(*a - *b);
}
static char lbuf[512];
static void  output(char  *buf,int  *n,int  *np)
{
    int *tp;
    static char lastc = ' ';
    char *lc;
    int head = 0;
    if(lastc != *buf) {
	printf(".XX\n.YY %c\n.XX\n",*buf);
	lastc = *buf;
	head++;
    }
    if(np > n + 2) qsort((char *)n,np - n,sizeof(int),QSORTFUNC(cmp));
    if((lc = (char *)strchr(buf,DCHAR)) != NULL && lc[1] == DCHAR &&
	lc[2] != ' ' && lc[2] != '\0'){
	unsigned int len = lc - buf;
	if((strlen(lbuf) == len) && strncmp(lbuf,buf,(len)) == 0){
	    buf = lc + 2;
	}
	else {
	    strncpy(lbuf,buf,len);
	    lbuf[len] = '\0';
	    if (head == 0) puts(".XX\n");
	    puts(lbuf);
	    buf = lc + 2;
	}
	putchar(' ');
    }
    else {
	if (head == 0) puts(".XX\n");
	strcpy(lbuf,buf);
    }
    fputs(buf,stdout);
    putchar(',');putchar(' ');
    while(n < np){
	printf("%d",*n);
	if(n + 1 == np) break;
	for(tp = n + 1;tp != np; tp++){
	    if(*tp == *n) continue;
	    if(*tp <= *(tp - 1) + GAP) continue;
	    break;
	}
	if(tp == n + 1){
	    putchar(',');
	    n++;
	    continue;
	}
	tp--;
	if(*tp != *n) printf("-%d",*(tp));
	n = tp + 1;
	if(n != np) putchar(',');
    }
    putchar('\n');
}

int main(int  argc,char  *argv[])
{
    char buf[BUFSIZE],rbuf[BUFSIZE],*begin, *end,*c;
    int num[NUMSIZE],*np;
    puts(header);
    *buf = '\0';
    *lbuf = '\0';
    np = num;
    begin = rbuf + 4; /* skip over "ix: " */
    while(fgets(rbuf,sizeof(rbuf),stdin) != NULL){
	rbuf[sizeof(rbuf)-1] = 0;
	end = buf + strlen(rbuf);
	if(end > rbuf && end[-1] == '\n')
	    end[-1] = 0;
	end = strrchr(rbuf,'\t');
	if(end == NULL) continue;
	for(c = end ; isspace(*c) && c > rbuf; c--)
	    *c = '\0';
	end++;
	if(islower(*begin)) *begin = toupper(*begin);
	if(*buf == '\0')
	    strcpy(buf,begin);
	else if(strcmp(begin,buf) != 0){
	    output(buf,num,np);
	    np = num;
	    strcpy(buf,begin);
	}
	*np++ = atoi(end);
    }
    if(*buf) output(buf,num,np);
    exit(0);
}

	
