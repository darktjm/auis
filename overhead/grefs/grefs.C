/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#include <stdio.h>
#include <refs.h>
#include <util.h>

#include "pathopen.h"

static struct plist {
    char *path;
    struct plist *next;
} *files=NULL;

static char *ipath=NULL;
static int isize=0;

static int FilePtr = 0;
static FILE *FileStack[FILESTACKSIZE];
static char *FileNames[FILESTACKSIZE];

static int done=0;
static struct clist {
    char *cname;
    struct clist *next;
    int def;
} *classes=NULL;

#ifdef FLEX_ENV
extern "C" void *yy_create_buffer(FILE *, int);
extern "C" void yy_switch_to_buffer(void *);
extern char *yytext;
static void *FileBuffers[FILESTACKSIZE];
#else
static void resetlexer() {}
extern char yytext[];
#endif

static void PushFile(char *name, FILE *file)
{
    if (FilePtr >= FILESTACKSIZE) {
	fprintf(stderr, "PushFile stack is full!\n");
        exit(-1);
    }
    FileStack[FilePtr] = file;
    FileNames[FilePtr] = NewString(name);
#ifdef FLEX_ENV
    FileBuffers[FilePtr] = yy_create_buffer(file, 4096);
    yy_switch_to_buffer(FileBuffers[FilePtr]);
#else
    yyin = file;
      resetlexer();
#endif
    FilePtr++;
}

static void DumpRefs() {
    struct clist *c=classes;
    printf("===\n");
    while(c) {
	if(!c->def && c->cname) printf("%s\n",c->cname);
	c=c->next;
    }
}
static void PopFile()  {
    fclose(yyin);
    if (FilePtr < 1) {
	fprintf(stderr, "PopFile stack is empty!\n");
        exit(-1);
    }
    if(FileNames[FilePtr-1]) free(FileNames[FilePtr-1]);
    FilePtr -=1;
    if (FilePtr > 0)  {
#ifdef FLEX_ENV
	yy_switch_to_buffer(FileBuffers[FilePtr-1]);
#else
	yyin = FileStack[FilePtr-1];
	resetlexer();
#endif
    } else {
	done=1;
    }
}

static void AddIncludeDir(const char *dir)
{
    if(dir) {
	if(ipath==NULL) {
	    isize=strlen(dir)+1;
	    ipath=(char *)malloc(isize);
	    if(ipath==NULL) {
		fprintf(stderr, "Failed to add include directory %s\n", dir);
		exit(-1);
	    }
	    strcpy(ipath, dir);
	} else {
	    isize+=strlen(dir)+1;
	    ipath=(char *)realloc(ipath, isize);
	    if(ipath==NULL) {
		fprintf(stderr, "Failed to add include directory %s\n", dir);
		exit(-1);
	    }
	    strcat(ipath, ":");
	    strcat(ipath, dir);
	}
    }
}

static struct clist *LookupClass(char *name) {
    struct clist *c=classes;
    while(c) {
	if(c->cname && strcmp(c->cname, name)==0) {
	    return c;
	}
	c=c->next;
    }
    return NULL;
}

static int NextToken()
{
    int ret;
    if(done) return -1;
#ifdef FLEX_ENV
    /* Does lex work this way too? */
    ret=yylex();
    if (ret == refs_EOF) {
	if(FilePtr>0) PopFile();
	else {
	    done=1;
	    return -1;
	}
    }
#else
    if(feof(yyin)) {
	if(FilePtr>0) PopFile();
	else {
	    done=1;
	    return -1;
	}
    }
    ret=yylex();
#endif
    return ret;
}

static int NextNonWhiteSpace()
{
    int c=(-1);
    while(!done && ((c=NextToken())==refs_WhiteSpace));
    if(c==refs_WhiteSpace) return -1;
    return c;
}


static void DefineClass(char *cname) {
    struct clist *c=LookupClass(cname);
    if(c && c->cname) {
	printf("%s\n", c->cname);
	c->def=TRUE;
    }
}

static int AddClass(char *cname, int tok) {
    struct clist *result=NULL;
    int isatk=(tok==refs_LeftBrace);
    while(tok==refs_Name) {
	if(strcmp(yytext, "ATK")==0 || (result=LookupClass(yytext))) {
	    isatk=1;
	    break;
	}
	tok=NextNonWhiteSpace();
	if(tok!=refs_Comma) break;
	tok=NextNonWhiteSpace();
    }
    if(!isatk) return tok;
    result=(struct clist *)malloc(sizeof(struct clist));
    if(result==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    result->cname=NewString(cname);
    if(result->cname==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    result->next=classes;
    result->def=FALSE;
    classes=result;	
    return tok;
}

boolean verbose=FALSE;
extern char pathopen_realpath[];
static void DoInclude(char *file, int current)
{
    FILE *fp=NULL;
    char lpath[MAXPATHLEN];
    struct plist *p=files;
    while(p) {
	if(strcmp(p->path, file)==0) return;
	p=p->next;
    }
    if(current) {
	if(FilePtr>0) {
	    char *p;
	    strcpy(lpath, FileNames[FilePtr-1]);
	    p=strrchr(lpath, '/');
	    if(p) {
		strcpy(p+1, file);
	    } else strcpy(lpath, file);
	}  else strcpy(lpath, file);
	fp=fopen(lpath, "r");
    } else strcpy(lpath, file);
    
    if(fp==NULL && ipath) {
	fp=pathopen(ipath, file, "r");
    } else strcpy(pathopen_realpath, lpath);
    
    p=(struct plist *)malloc(sizeof(struct plist));
    if(p==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    p->next=files;
    files=p;
    p->path=strdup(file);
    if(p->path==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    if(fp==NULL) {
	if(verbose) {
	    perror(pathopen_realpath);
	    fprintf(stderr, "Failed to find included file %s.\n", file);
	    if(FilePtr>0) fprintf(stderr, "While processing file %s\n", FileNames[FilePtr-1]);
	}
	return;
    }
    PushFile(pathopen_realpath, fp);
}

static void usage() {
    fprintf(stderr, "grefs [-Idirectory ...] [-e .ext1.ext2.etc] -v[erbose] files\n");
    exit(-1);
}

int main(int argc, char **argv) {
    char classname[1024];
    int tok;
    int i;
    char *p;
    const char *exts=".C.cc.c.i";
    for(i=1;i<argc;i++) {
	if(argv[i][0]!='-') break;
	switch(argv[i][1]) {
	    case 'I':
		AddIncludeDir(argv[i]+2);
		break;
	    case 'e':
		if(i+1>=argc) usage();
		exts=argv[i+1];
		i++;
		break;
	    case 'v':
		verbose=TRUE;
		break;
	    default:
		usage();
	}
    }
    AddIncludeDir(getenv("ATKINCLUDEPATH"));
    AddIncludeDir("/usr/include");
    while(i<argc) {
	char buf[256];
	FILE *fp=NULL;
	char *q, *r=strrchr(argv[i], '.');;
	done=0;
	p=buf;
	strcpy(buf, exts);
	if(r && r[1]=='o' && r[2]=='\0') {
	    char file[MAXPATHLEN];
	    *r='\0';
	    while(fp==NULL && p && *p) {
		strcpy(file, argv[i]);
		p=strchr(p, '.');
		if(p==NULL) break;
		q=strchr(p+1, '.');
		if(q) *q='\0';
		strcat(file, p);
		p=q+1;
		fp=fopen(file, "r");
	    }
	    *r='.';
	} else fp=fopen(argv[i], "r");
	if(fp==NULL) {
	    perror(argv[i]);
	    i++;
	    continue;
	}
	PushFile(argv[i], fp);
	tok=NextToken();
	while(!done) {
	    switch(tok) {
		case refs_Define:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_ATKdefineRegistry) continue;
		    break;
		case refs_Extern:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_ATKregistryEntry) continue;
		    break;
		case refs_ATKregistryEntry:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_ATKregistry_) continue;
		    p=strchr(yytext, '_');
		    if(p) *p='\0';
		    strcpy(classname, yytext);
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_Equal) continue;
		    DefineClass(classname);
		    break;
		case refs_ATKdefineRegistry:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_LeftParen) continue;
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_Name) continue;
		    DefineClass(yytext);
		    break;
		case refs_Include:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_String && tok!=refs_LessThan) continue;
		    if(tok==refs_String) {
			yytext[strlen(yytext)-1]='\0';
			DoInclude(yytext+1, 1);
			continue;
		    } else {
			char filename[MAXPATHLEN];
			int len=0;
			filename[0]='\0';
			tok=NextNonWhiteSpace();
			while(!done && tok!=refs_GreaterThan && len+strlen(yytext)<sizeof(filename)-1) {
			    strcat(filename, yytext);
			    tok=NextToken();
			}
			DoInclude(filename, 0);
			continue;
		    }
		    break;
		case refs_Class:
		    tok=NextNonWhiteSpace();
		    if(tok!=refs_Name) continue;
		    if(strlen(yytext)>sizeof(classname)-1) {
			fprintf(stderr, "Identifier %s too long, skipped...\n", yytext);
			break;
		    }
		    strcpy(classname, yytext);
		    tok=NextNonWhiteSpace();
		    if(tok==refs_LeftBrace && strcmp(classname, "ATK")==0) AddClass(classname, tok);
		    else {
			if(tok!=refs_Colon) continue;
			tok=NextNonWhiteSpace();
			while(tok==refs_Inheritance) tok=NextNonWhiteSpace();
			if(tok!=refs_Name) continue;
			tok=AddClass(classname, tok);
			if(tok!=refs_LeftBrace) continue;
		    }
		    break;
		default:
		    break;
	    }
	    tok=NextToken();
	}
	i++;
    }
    DumpRefs();
    return 0;
}
