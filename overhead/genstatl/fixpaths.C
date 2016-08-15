/* Copyright 1994 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */
#include <andrewos.h>

#include <util.h>

#ifndef PATHSEP
#ifdef DOS_STYLE_PATHNAMES
#define PATHSEP ';'
#else
#define PATHSEP ':'
#endif
#endif

struct lib {
    char *var;
    char *path;
    struct lib *next;
} *first=NULL, **addpt=&first;

struct var {
    char *var;
    char *path;
    struct var *next;
} *firstd=NULL, **addptd=&firstd;

static void AddVar(const char *var)
{
    char buf[MAXPATHLEN+256];
    strcpy(buf, var);

    char *p=strchr(buf, '=');
    if(p==NULL) {
	fprintf(stderr, "fixpaths warning: Badly formed variable assignment, skipping.");
	return;
    }
    *p++='\0';

    int len=strlen(p);
    if(len && p[len-1]=='/') p[len-1]='\0';

    struct var *result=(struct var *)malloc(sizeof(struct var));
    if(result==NULL) {
	fprintf(stderr, "fixpaths warning: Out of memory.");
	exit(-1);
    }
    result->var=NewString(buf);
    result->path=NewString(p);
    result->next=(*addptd);
    *addptd=result;
    addptd=(&result->next);
}
    
static void AddLibs(const char *path)
{
    char buf[MAXPATHLEN+1];
    
    FILE *fp=fopen(path, "r");
    if(fp==NULL) {
	error:
	fprintf(stderr, "fixpaths warning: skipping libraries from package: %s.\n", path);
	return;
    }
    if(fgets(buf,sizeof(buf)-1,fp)==NULL) goto error;

    char *p=buf, *q;
    while(*p && (isspace(*p) || *p=='\n')) p++;
    q=p;
    while(*q && !(isspace(*q) || *q=='\n')) q++;
    *q='\0'; 

    char *var=NewString(p);

    while(!feof(fp) && fgets(buf,sizeof(buf)-1,fp)) {
	struct lib *result=(struct lib *)malloc(sizeof(struct lib));
	p=strchr(buf, '\n');
	if(p) *p='\0';
	if(result==NULL) goto error;

	result->var=var;
	result->path=NewString(buf);
	result->next=(*addpt);
	*addpt=result;
	addpt=(&result->next);
    }
}


static void AddDir(const char *path)
{
    char buf[MAXPATHLEN+1];
    char buf2[MAXPATHLEN+1];

    strcpy(buf,path);
    strcat(buf, "/defaults");

    FILE *fp=fopen(buf, "r");
    if(fp==NULL) {
	error:
	fprintf(stderr, "fixpaths warning: skipping directory %s.\n", path);
	return;
    }


    while(!feof(fp) && fgets(buf,sizeof(buf)-1,fp)) {

	char *p=buf;
	while(*p) {
	    while(*p && (isspace(*p) || *p=='\n')) p++;

	    char *q=p;
	    while(*q && !(isspace(*q) || *q=='\n')) q++;
	    if(*q) *q++='\0';
	    strcpy(buf2,path);
	    strcat(buf2, "/");
	    strcat(buf2,p);
	    AddLibs(buf2);
	    p=q;
	}
    }
}

static void AddAllPackages() {
    const char *path=getenv("AUISPACKAGESPATH");
    if(path==NULL) path=AndrewDir("/lib/packages");

    char buf[MAXPATHLEN+1];
    const char *p=path, *q=p;

    while(q) {
	q=strchr(p, PATHSEP);
	if(q==NULL) {
	    strcpy(buf,p);
	} else {
	    strncpy(buf,p,q-p);
	    buf[q-p]='\0';
	}
	AddDir(buf);
	if(q) q++;
    }
}

int main(int argc, char **argv) {
    argv++;
    argc--;
    while(argc>0) {
	if(argv[0][0]=='-') {
	    if(argv[0][1]=='d') {
		argv++;
		argc--;
		while(*argv && *argv[0]!='-' && argc-->0) AddDir(*argv++);
		argv++;
		argc--;
		AddAllPackages();
		continue;
	    } else if(argv[0][1]=='v') {
		argv++;
		argc--;
		while(*argv && *argv[0]!='-' && argc-->0) AddVar(*argv++);
		argv++;
		argc--;
		continue;
	    }
	}
	if(first==NULL) AddAllPackages();
	if(argv[0][0]=='-' && argv[0][1]=='L') {
	    struct var *v=firstd;
	    while(v) {
		int plen=strlen(v->path);
		int alen=strlen(argv[0]);
		if(alen-2>=plen) {
		    if(strncmp(argv[0]+2,v->path, plen)==0) {
			printf("-L$%s%s ",v->var,argv[0]+2+plen);
			break;
		    }
		}
		v=v->next;
	    }
	    if(v==NULL) printf("%s ",argv[0]);

	} else {
	    struct lib *l=first;
	    while(l) {
		int plen=strlen(l->path);
		int alen=strlen(argv[0]);
		if(alen>=plen) {
		    if(strcmp(argv[0]+alen-plen, l->path)==0) {
			printf("$%s/%s ",l->var,l->path);
			break;
		    }
		}
		l=l->next;
	    }
	    if(l==NULL) printf("%s ",argv[0]);
	}
	argv++;
	argc--;	    
    }
    printf("\n");
    return 0;
}
	    
	    
