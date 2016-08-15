/* Copyright 1993 Carnegie Mellon University All rights reserved.
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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>

#include <util.h>

char *AndrewDirStr=NULL;
char *XLibDirStr=NULL;
char *AFSBaseDirStr=NULL;
char *JpegLibDirStr=NULL;
char *TiffLibDirStr=NULL;

char builddir[MAXPATHLEN+1];
char *buildpath=NULL;
char rbuf[MAXPATHLEN+1];
static char ext[]=".rf";
char wd[MAXPATHLEN];
boolean makingdyn=FALSE;

#define HASHMAX 57
struct hasht {
    char *classname;
    struct hashf *lib;
    int needed;
    struct hasht *next;
}; 

static struct hasht *all[HASHMAX];

struct hashf {
    dev_t dev;
    ino_t ino;
    char *pathname;
    struct hashf *next;
    long reflevel, liblevel;
    struct hashf *pnext;
};

/*
void DisplayHashf(struct hashf *h, int line) {
    printf("%d:\tref:%d\tlib:%d\tpath:%s\n", line, h->reflevel,h->liblevel,h->pathname);
}
 */

#define DisplayHashf(h,line)

static struct hashf *havefile[HASHMAX];

#define HASH(n, len) (((n[0]<<10)^(n[len-1]<<5)^(n[len/2]))%HASHMAX)
#define HASHFILE(s) (((s).st_dev^(s).st_ino)%HASHMAX)


static char *GetWord(FILE *fp) {
    static char wordbuf[1024];
    char *p=wordbuf;
    int c;
    wordbuf[0]='\0';
    while(p<wordbuf+sizeof(wordbuf)-2 && !feof(fp) && ((c=fgetc(fp))!=EOF) && !isspace(c)) *p++=c;
    *p='\0';
    if(wordbuf[0]) return wordbuf;
    return NULL;
}


static boolean AddClass(char  *name, struct hashf *sf)
{
    int len=strlen(name);
    int ind=HASH(name, len);
    struct hasht *result;
    if (all[ind]) {
	struct hasht *s=all[ind];
	while (s && strcmp(s->classname, name)) {
	    s=s->next;
	}
	if(s) return FALSE;
    }
    result=(struct hasht *)malloc(sizeof(struct hasht));
    if(result==NULL) {
	fprintf(stderr, "genstatl: out of memory.\n");
	exit(-1);
    }
    result->classname=NewString(name);
    result->lib=sf;
    result->needed=0;
    result->next=all[ind];
    all[ind]=result;
    return TRUE;
}

static struct hashf **handle=NULL;

static struct hashf *AddFile(char  *path, boolean  exists)
{
    struct stat statbuf;
    struct hashf *result;
    int ind;
    boolean flag=FALSE;
#ifdef INODE_NUMBERS_ARE_REAL
    if (path[0]!='-') {
	if(stat(path, &statbuf)!=0) {
	    if(exists) perror(path);
	    handle=NULL;
	    return NULL;
	}
	ind=HASHFILE(statbuf);
    } else 
#else
    /* OS/2 EMX doesn't emulate dev/inode nums.  Just drop through
       * and hash the string name.  Actually, this would be adequate
       * for all platforms!
       */
#endif
    { 
	int len=strlen(path);
	ind=HASH(path, len);
	flag=TRUE;
    }
    if (havefile[ind]) {
	struct hashf *s=havefile[ind];
	handle=(&havefile[ind]);
	while (s && strcmp(s->pathname, path) && ((flag || s->pathname[0]=='-') || (s->dev!=statbuf.st_dev || s->ino!=statbuf.st_ino))) {
	    handle=(&s->next);
	    s=s->next;
	}
	if(s) {
	    return s;
	}
    }
    result=(struct hashf *)malloc(sizeof(struct hashf));
    if(result==NULL) {
	fprintf(stderr, "genstatl: out of memory.\n");
	exit(-1);
    }
    result->reflevel=result->liblevel=0;
    result->pathname=NewString(path);
    DisplayHashf(result,__LINE__);
    result->dev=statbuf.st_dev;
    result->ino=statbuf.st_ino;
    result->next=havefile[ind];
    havefile[ind]=result;
    handle=(&havefile[ind]);
    return result;
}


static int selectfile(DIRENT_TYPE *file)
{
    char *p;
    p=strrchr(file->d_name, '.');
    if(p && strcmp(p+1, "rf")==0) return 1;
    else return 0;
}
    

static void AddLibDir(const char *dir);

static int GetInfo(int argc, const char **argv, boolean user)
{
    int i, j;
    char dbuf[MAXPATHLEN];
    char fbuf[MAXPATHLEN];
    for(i=0;i<argc && argv[i][0]!='-';i++) {
	DIRENT_TYPE **list=NULL;
	FILE *fp;
	int num;
	dbuf[0]='\0';

	if(user) AddLibDir(argv[i]);
	if(argv[i][0]!='/') {
	    strcat(dbuf, wd);
	    if(argv[i][0]!='.' || argv[i][1]!='\0') {
		strcat(dbuf, "/");
		strcat(dbuf, argv[i]);
	    };
	} else strcpy(dbuf, argv[i]);
	num=scandir(dbuf, &list, SCANDIRSELFUNC(selectfile), NULL);
	if(num<0 || list==NULL) {
	    fprintf(stderr, "genstatl: Warning: couldn't read directory %s, skipping...\n", dbuf);
	    continue;
	}
	for(j=0;j<num && list[j];j++) {
	    struct hashf *sf;
	    strcpy(fbuf, dbuf);
	    strcat(fbuf, "/");
	    strcat(fbuf, list[j]->d_name);
	    fp=fopen(fbuf, "r");
	    if(fp==NULL) {
		fprintf(stderr, "genstatl: Warning: Couldn't open %s, skipping...\n", fbuf);
		continue;
	    }
	    sf=AddFile(fbuf, TRUE);
	    if(sf==NULL) {
		fprintf(stderr, "genstatl: Warning: Couldn't add file %s, skipping...\n", fbuf);
		continue;
	    }
	    while(!feof(fp) && fgets(rbuf, sizeof(rbuf)-1-sizeof(ext), fp)) {
		char *p=strrchr(rbuf, '\n');
		if(p) *p='\0';
		if(rbuf[0]=='=') break;
		AddClass(rbuf, sf);
	    }
	    free(list[j]);
	    fclose(fp);
	}
	if(j<num) {
	    fprintf(stderr, "genstatl: Warning: bad filename in directory %s, some files may be skipped...\n", dbuf);
	}
	free(list);
    }
    return i;
}

static struct hasht *LookupClass(char  *name)
{
    int len=strlen(name);
    int ind=HASH(name, len);
    struct hasht *s=all[ind];
    while (s && strcmp(s->classname, name)) {
	s=s->next;
    }
    return s;
}

static struct hashf *LookupLib(char  *name)
{
    int i;
    char buf[1024];
    strcpy(buf, name);
    strcat(buf, ".rf");
    for(i=0;i<HASHMAX;i++) {
	struct hashf *f=havefile[i];
	while(f) {
	    char *p=strrchr(f->pathname, '/');
	    if(p==NULL) p=f->pathname;
	    else p=p+1;
	    if(strcmp(p, buf)==0) return f;
	    f=f->next;
	}
    }
    return NULL;
}

static boolean dolibsinrf=FALSE;
static boolean dodefsinrf=TRUE;
static boolean doclassesinrf=TRUE;
void processMainFile(FILE *mainfp) {
    if(mainfp) {
	char libToRegister[MAXPATHLEN];
	char *p;
	int i;
	if(dodefsinrf) {
	    for(i=0;i<HASHMAX;i++) {
	    struct hasht *s=all[i];
	    while(s) {
		if((s->needed) && (s->lib->reflevel==0))
		    fprintf(mainfp, "%s\n", s->classname);
		s=s->next;
	    }
	    }
	}
	if(doclassesinrf) {
	    fprintf(mainfp,"===\n");
	    for(i=0;i<HASHMAX;i++) {
		struct hasht *s=all[i];
		while(s) {
		    if((s->needed) && (s->lib->reflevel>0))
			fprintf(mainfp, "%s\n", s->classname, s->lib->reflevel);
		    s=s->next;
		}
	    }
	}
	if(dolibsinrf) {
	    fprintf(mainfp, "---\n");
	    for(i=0;i<HASHMAX;i++) {
		struct hashf *f=havefile[i];
		for(;f; f=f->next) {
		    if(f->reflevel!=0) {
			strcpy(libToRegister, f->pathname);
			if((p = strrchr(libToRegister, '.'))) {
			    if(!strcmp(p, ".rf"))
				*p = '\0';
			    else
				continue;
			}
			else
			    continue;
			char *p=strrchr(libToRegister, '/');
			if(p==NULL) p=libToRegister;
			else p=p+1;
			fprintf(mainfp, "%s\n", p);
		    }
		}
	    }
	}
    }
}

#undef MAX
#define MAX(x,y) (((x)<(y))?(y):(x))

static long ProcessReferences(FILE *f);
static void ProcessLib(struct hashf *lib) {
    FILE *fp2;
    int localrefs=0;
    char *wbuf;
    printf("Scanning %s.\n", lib->pathname);
    lib->reflevel=1;
    DisplayHashf(lib,__LINE__);
    fp2=fopen(lib->pathname, "r");
    if(fp2==NULL) {
	fprintf(stderr, "genstatl: Warning couldn't open refs file %s\n", lib->pathname);
	exit(-1);
    }
    while(wbuf=GetWord(fp2)) {
	struct hasht *st;
	st=LookupClass(wbuf);
	if(st) st->needed=1;
	if(strcmp(wbuf, "===")==0) {
	    localrefs=ProcessReferences(fp2);
	    break;
	}
    }
    fclose(fp2);
    lib->reflevel=localrefs;
    DisplayHashf(lib,__LINE__);
}


static void ProcessEverything()
{
    int i;
    for(i=0;i<HASHMAX;i++) {
	struct hashf *f=havefile[i];
	while(f) {
	    ProcessLib(f);
	    f=f->next;
	}
    }
}

static long ProcessName(char *wbuf, long refs) {
    struct hasht *s;
    struct hashf *lib;
    if(wbuf[0]!='@') {
	s=LookupClass(wbuf);
	if(s==NULL) {
	    fprintf(stderr, "genstatl: Warning: undefined class %s referenced\n", wbuf);
	} else {
	    if(s->needed==0) {
		s->needed=1;
		if(s->lib->reflevel==0) ProcessLib(s->lib);
	    }
	    refs=MAX(refs,s->lib->reflevel+1);
	}
    } else {
	lib=LookupLib(wbuf+1);
	if(lib==NULL) fprintf(stderr, "genstatl: Warning: unknown library %s referenced.\n", wbuf+1);
	else {
	    if(lib->reflevel==0) ProcessLib(lib);
	    refs=MAX(refs,lib->reflevel+1);
	}
    }
    return refs;
}

static int DoClasses(int argc, char **argv)
{
    int i;
    for(i=0;i<argc && argv[i][0]!='-';i++) {
	ProcessName(argv[i], 1);
    }
    return i;
}
static long ProcessReferences(FILE *fp)
{
    long refs=1;
    char *wbuf;
    while(wbuf=GetWord(fp)) {
	if(wbuf[0]=='=') break;
	refs=ProcessName(wbuf, refs);
    }
    return refs;
}

static char *FixSeg(char *arg)
{
    char *p;
    p=(char *)strchr(arg, ' ');
    if(p==NULL) return arg+strlen(arg);
    *p='\0';
    return p+1;
}

struct dirlist {
    char *path;
    struct dirlist *next;
} *dirs=NULL, **lastp=NULL;

static void FreeDirList()
{
    while(dirs) {
	struct dirlist *next=dirs->next;
	free(dirs);
	dirs=next;
    }
    lastp=(&dirs);
}

static void AddDirToList(char *dir)
{
    struct dirlist *result=(struct dirlist *)malloc(sizeof(struct dirlist));
    if(result==NULL) {
	fprintf(stderr, "genstatl: Out of memory...\n");
	return;
    }
    result->next=NULL;
    result->path=dir;
    *lastp=result;
    lastp=(&result->next);
}

char *libexts[]={
#ifdef ATKLIBEXT
    ATKLIBEXT,
#endif
    "a",
    NULL
};

static char realpath_linuxsucks[MAXPATHLEN];
static char *FindLib(char *lib)
{
    struct dirlist *d=dirs;
    while(d) {
	char *p;
	char **libext=libexts;
	strcpy(realpath_linuxsucks, d->path);
	strcat(realpath_linuxsucks, "/lib");
	strcat(realpath_linuxsucks, lib);
	p=realpath_linuxsucks+strlen(realpath_linuxsucks);
	while(*libext) {
	    strcpy(p, ".");
	    strcat(p, *libext);
	    if(access(realpath_linuxsucks, R_OK)==0) return realpath_linuxsucks;
	    libext++;
	}
	d=d->next;
    }
    return NULL;
}


static char *TranslatePath(const char *q)
{
    char *p;
    static char filebuf[MAXPATHLEN+1];
    filebuf[0]='\0';
    if(strncmp(q, "ANDREWDIR", 9)==0) {
	strcpy(filebuf, AndrewDirStr);
    } else if(strncmp(q, "XLIBDIR", 7)==0) {
	strcpy(filebuf, XLibDirStr);
    } else if(strncmp(q, "AFSBASEDIR", 10)==0) {
	strcpy(filebuf, AFSBaseDirStr);
    } else if(strncmp(q, "JPEGLIBDIR", 10)==0) {
	strcpy(filebuf, JpegLibDirStr);
    } else if(strncmp(q, "TIFFLIBDIR", 10)==0) {
	strcpy(filebuf, TiffLibDirStr);
    } else {
	char *v;
	v=(char *)getenv(q);
	if(v) strcpy(filebuf, v);
	else strcpy(filebuf, q);
    }
    p=(char *)strchr(q, '/');
    if(p==NULL) return filebuf;
    else strcat(filebuf,p);
    return filebuf;
}

static void AddLibList(char *p, char *ltpath, int defaultlevel)
{
    char *q;
    int liblevel=0;
    FreeDirList();
    while(*p) {
	struct hashf *sf;
	if(*p=='-') {
	    if(p[1]=='L') {
		char *dir=p+2;
		q=p;
		p=FixSeg(dir);
		if(dir[0]!='$') {
		    AddDirToList(dir);
		    sf=AddFile(q, TRUE);
		} else {
		    char buf[MAXPATHLEN+1];
		    dir=TranslatePath(dir+1);
		    AddDirToList(dir);
		    strcpy(buf,"-L");
		    strcat(buf,dir);
		    sf=AddFile(buf, TRUE);
		}
		if(sf) {
		    if(sf->reflevel>=0) {
			sf->reflevel=defaultlevel;
			sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
		    } else {
			if(sf->reflevel<defaultlevel) {
			    sf->reflevel=defaultlevel;
			    sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
			}
		    }
		}
	    } else if(p[1]=='l') {
		char *lib=p+2;
		char *path;
		q=p;
		p=FixSeg(lib);
		path=FindLib(lib);
		if(path==NULL) {
		    sf=AddFile(q, TRUE);
		} else {
		    sf=AddFile(path, TRUE);
		}
		if(sf) {
		    if(sf->reflevel>=0) {
			sf->reflevel=defaultlevel;
			sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
		    } else {
			if(sf->reflevel<defaultlevel) {
			    sf->reflevel=defaultlevel;
			    sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
			}
		    }
		}
	    } else {
		p=FixSeg(p);
		fprintf(stderr, "genstatl: Warning: unkown flag in libs list %s.\n", ltpath);
	    }
	} else {
	    char *q=p;
	    p=FixSeg(p);
	    if(*q) {
		if(*q=='$') q=TranslatePath(q+1);
		sf=AddFile(q, TRUE);
		if(sf) {
		    if(sf->reflevel>=0) {
			sf->reflevel=defaultlevel;
			sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
		    } else {
			if(sf->reflevel<defaultlevel) {
			    sf->reflevel=defaultlevel;
			    sf->liblevel=liblevel++;
			DisplayHashf(sf,__LINE__);
			}
		    }
		}
	    }
	}
    }
}

static void ProcessLibFile(char *ltpath, long reflevel)
{
    FILE *fp=fopen(ltpath, "r");
    char *buf, *p;
    struct stat sbuf;
    if(fp==NULL) {
	fprintf(stderr, "genstatl: Couldn't open libs list %s, punting...\n", ltpath);
	return;
    }
    if(fstat(fileno(fp), &sbuf)<0) {
	fprintf(stderr, "genstatl: Couldn't stat libs list %s, punting...\n", ltpath);
	fclose(fp);
	return;
    }
    buf=(char *)malloc(sbuf.st_size+1);
    if(buf==NULL) {
	fprintf(stderr, "genstatl: Couldn't allocate space for lisbs list %s, punting...\n", ltpath);
	fclose(fp);
	return;
    }
    if(fgets(buf, sbuf.st_size+1, fp)==NULL) {
	fclose(fp);
	fprintf(stderr, "genstatl: Failed reading libs list %s, punting...\n", ltpath);
	return;
    }
    p=buf;
    while(*p) {
	if(*p=='\n') *p=' ';
	p++;
    }
    fclose(fp);
    if(*buf && *buf!='\n') AddLibList(buf, ltpath, reflevel);  
}

#ifndef ATKDYNOBJS_ARE_LIBS
static char *DynPath(char *spath, char *lpath) {
    char *p=strrchr(lpath, '/');
    char *q;
    if(p) {
	q=strrchr(spath, '/');
	strcpy(p+1,q+1);
	q=strrchr(p+1,'.');
    } else {
	strcpy(lpath, wd);
	strcat(lpath, "/");
	strcat(lpath, spath);
	q=strrchr(lpath, '.');
    }
    if(!q) {
	fprintf(stderr, "genstatl: Missing '/' in %s.\n", lpath);
	fprintf(stderr, "genstatl: Unexpected internal failure(%d).\n",__LINE__);
	exit(-1);
    }
    strcpy(q+1, ATKDYNEXT);
    if(access(lpath, R_OK)==0) return lpath;
    return NULL;
}
#else
#define DynPath(x,y) LibPath(x,y)
#endif

static char *LibPath(char *spath, char *lpath) {
    char *p=strrchr(lpath, '/');
    char *q;
    char **libext=libexts;
    if(p) {
	strcpy(p+1, "lib");
	q=strrchr(spath, '/');
	strcat(p+4, q+1);
	q=strrchr(p+4, '.');
    } else {
	strcpy(lpath, wd);
	strcat(lpath, "/lib");
	strcat(lpath, spath);
	q=strrchr(lpath, '.');
    }
    if(!q) {
	fprintf(stderr, "genstatl: Missing '/' in %s.\n", lpath);
	fprintf(stderr, "genstatl: Unexpected internal failure(%d).\n",__LINE__);
	exit(-1);
    }
    while(*libext) {
	strcpy(q+1, *libext);
	if(access(lpath, R_OK)==0) return lpath;
	libext++;
    }
    return NULL;
}

static void AddLibraries()
{
    int i;
    char *p=NULL, *q;
 
    for(i=0;i<HASHMAX;i++) {
	struct hashf *sf=havefile[i];
	while(sf) {
	    if(sf->reflevel>0) {
		long rlevel=(-sf->reflevel-1);
		struct hashf *sf2;
		char *libpath, buf[MAXPATHLEN+2];
		char ltpath[MAXPATHLEN];
		libpath=buf+2;
		strcpy(ltpath, sf->pathname);
		p=strrchr(ltpath, '.');
		if(p) strcpy(p+1, "lt");
		else {
		    fprintf(stderr, "genstatl: Could't find extension on path %s\n", ltpath);
		    goto error;
		}
		strcpy(libpath, sf->pathname);
		if(makingdyn) {
#if defined(HAVE_DYNAMIC_INTERLINKING) && !SY_OS2
		    p=DynPath(sf->pathname,libpath);
		    if(p==NULL)
#endif
			p=LibPath(sf->pathname,libpath);
		} else {
		    p=LibPath(sf->pathname,libpath);
		    if(p==NULL) p=DynPath(sf->pathname,libpath);
		}
		if(p==NULL) {
		    fprintf(stderr, "genstatl: Couldn't find path to library or dynamic object corresponding to:\n%s\n", sf->pathname);
		    goto error;
		}
		sf2=AddFile(libpath, TRUE);
		if(sf2) sf2->reflevel=rlevel;
			DisplayHashf(sf2,__LINE__);
#if 0
		p=strrchr(libpath, '/');
		if(p) {
		    *p='\0';
		    buf[0]='-';
		    buf[1]='L';
		    sf2=AddFile(buf, FALSE);
		    if(sf2) sf2->reflevel=(-1);
			DisplayHashf(sf2,__LINE__);
		}
#endif
		sf->reflevel=0;
			DisplayHashf(sf,__LINE__);
		ProcessLibFile(ltpath, rlevel);
	    }
	    sf=sf->next;
	}
    }
    return;
    error:
      fprintf(stderr, "genstatl: Unexpected internal failure(%d).\n",__LINE__);
      exit(-1);
}
static void OrderLibraries(char *orderfile)
{
    int i;
    long reflevel=0;
    FILE *fp=fopen(orderfile, "r");
    if(fp==NULL) {
	fprintf(stderr, "genstatl: Couldn't open library ordering file %s.\n", orderfile);
	exit(-1);
    }
    FreeDirList();
    while(!feof(fp) && fgets(rbuf, sizeof(rbuf)-1-sizeof(ext), fp)) {
	char filebuf[MAXPATHLEN+1];
	char *p=strrchr(rbuf, '\n');
	char *q=rbuf;
	struct hashf *result=NULL;
	reflevel++;
	if(p) *p='\0';
	if(rbuf[0]=='\0') continue;
	if(q[0]=='-' && q[1]=='l') {
	    p=FindLib(q+2);
	    if(p) strcpy(filebuf,  p);
	    else strcpy(filebuf, q);
	} else {
	    p=strchr(rbuf, '/');
	    if(!p) continue;
	    filebuf[0]='\0';
	    if(q[0]=='-' && q[1]=='L') {
		strcat(filebuf, "-L");
		q+=2;
	    }
	    if(q[0]=='$') {
		q++;
		if(strncmp(q, "ANDREWDIR", 9)==0) {
		    strcat(filebuf, AndrewDirStr);
		} else if(strncmp(q, "XLIBDIR", 7)==0) {
		    strcat(filebuf, XLibDirStr);
		} else if(strncmp(q, "AFSBASEDIR", 10)==0) {
		    strcat(filebuf, AFSBaseDirStr);
		} else {
		    char *v;
		    p[0]='\0';
		    v=(char *)getenv(q);
		    if(v) strcat(filebuf, v);
		    else p=(--q);
		}
	    } else p=q;
	    strcat(filebuf, p);
	    if(filebuf[0]=='-' && filebuf[1]=='L') {
		AddDirToList(filebuf+2);
	    }
	}
	result=AddFile(filebuf, filebuf[0]=='-' && filebuf[1]=='l');
	if(result && result->reflevel<=-1) {
	    result->reflevel=reflevel;
			DisplayHashf(result,__LINE__);
	}
    }
    for(i=0;i<HASHMAX;i++) {
	struct hashf *sf=havefile[i];
	while(sf) {
	    if(sf->reflevel==-1) {
		sf->reflevel=++reflevel;
			DisplayHashf(sf,__LINE__);
	    }
	    sf=sf->next;
	}
    }

}

static void DumpStatl()
{
    FILE *sfp;
    int i;
    sfp=fopen("statl.C.new", "w");
    if(sfp==NULL) {
	perror("statl.C.new");
	exit(-1);
    }
    fprintf(sfp, "#include <andrewos.h>\n#include <ATK.H>");
    
    fprintf(sfp, "\n\nvoid doStaticLoads() {\n");
    for(i=0;i<HASHMAX;i++) {
	struct hasht *s=all[i];
	while(s) {
	    if(s->needed) fprintf(sfp, "ATKregister(%s);\n", s->classname);
	    s=s->next;
	}
    }
    fprintf(sfp, "}\n");
    if(fclose(sfp)!=0) {
	fprintf(stderr, "genstatl: genstatl: error writing statl.C.new...\n");
	exit(-1);
    }
    if(access("statl.C", R_OK)==0) {
	char cbuf[1024];
	int result;
	sprintf(cbuf, "diff statl.C statl.C.new");
	result=system(cbuf);
	if(result==127) {
	    fprintf(stderr, "genstatl: WARNING: Couldn't exec shell to run '%s'.\n", cbuf);
	}
	if(result==-1) {
	    fprintf(stderr, "genstatl: WARNING: Couldn't fork to run '%s'.\n", cbuf);
	}
	if(result==0) {
	    fprintf(stderr, "genstatl: statl.C and statl.C.new are identical, retaining statl.C.\n");
	    unlink("statl.C.new");
	    return;
	}
	if(unlink("statl.C")!=0) {
	    fprintf(stderr, "genstatl: Couldn't unlink old statl.C to replace it with the new version.\n");
	    exit(-1);
	}
    }
    if(rename("statl.C.new","statl.C")!=0) {
	fprintf(stderr, "genstatl: Couldn't replace the old statl.C with the new version(statl.C.new).\n");
	exit(-1);
    }
}

static int libsort(struct hashf **a, struct hashf **b)
{
    if((*a)->pathname[0]=='-' && (*b)->pathname[0]!='-') return 1;
    if((*a)->pathname[0]!='-' && (*b)->pathname[0]=='-') return -1; 
    if((*a)->reflevel-(*b)->reflevel!=0) return (*a)->reflevel-(*b)->reflevel;
    else return (*a)->liblevel-(*b)->liblevel;
}

static void DumpLibs(FILE *ifp)
{
    int i, j;
    int count=0;
    struct hashf **asf=NULL;
    int syslibs=0;
    fprintf(ifp, "\nLIBS = ");
    for(i=0;i<HASHMAX;i++) {
	struct hashf *sf=havefile[i];
	while(sf) {
	    if(sf->reflevel!=0) count++;
	    sf=sf->next;
	}
    }
    asf=(struct hashf **)malloc(sizeof(struct hashf *)*count);
    for(i=0, j=0;i<HASHMAX;i++) {
	struct hashf *sf=havefile[i];
	while(sf) {
	    if(sf->reflevel!=0) {
		asf[j++]=sf;
	    }
	    sf=sf->next;
	}
    }
    qsort(asf, count, sizeof(struct hashf *), QSORTFUNC(libsort));
    for(j=0;j<count;j++) {
	if(asf[j]->pathname[0]=='-' && !syslibs) {
	    fprintf(ifp, "\n\nSYSLIBS = ");
	    syslibs=1;
	}
	fprintf(ifp,"\t%s%s\n", asf[j]->pathname, j!=count-1?((syslibs || asf[j+1]->pathname[0]!='-')?" \\":""):"");
    }
}

static const char *fakeargv[]={
    NULL,
    NULL
};

static void usage()
{

    fprintf(stderr, "usage: genstatl [flags, see below]\n");
    fprintf(stderr, "note: text in <> should be replaced with the appropriate\nfilename or word, text in quotes should be replaced with an\nappropriate string surrounded by quotes.\n\n");
    fprintf(stderr, "\t-a\nSupresses loading <executable-name>app into the application.\n(not needed with -L)\n\n");
    fprintf(stderr, "\t-all\nCauses all available classes to be included in the application.\n\n");
    fprintf(stderr, "\t-d <directory>\nSpecifies the directory where the Imakefile and/or statl.C file will be created.\nThe directory is created if necessary.\n\n");
    fprintf(stderr, "\t-E\nForces an executable to be produced even if -L is also used.\n\n");
    fprintf(stderr, "\t-e <executable/library-name>\nSets the name of the application or library to be created.\n\n");
    fprintf(stderr, "\t-i <imakefile-name>\nSets the name of the imakefile to be created.\n\n");
    fprintf(stderr, "\t-L [<libraries...>]\nSepcifies that a library is to be built, [<libraries...>] lists additional\nlibraries to link this library against.\n\n");
    fprintf(stderr, "\t-l <library-order-file>\nSelects an alternate library ordering file.\n\n");
    fprintf(stderr, "\t-m foo1.o foo2.o etc...\nSelects alternate object files for main() in an application,\nor for inclusion in the library.\n\n");
    fprintf(stderr, "\t-n\nSupresses reading library information from\n%s.\n\n", AndrewDir("/lib/atk"));
    fprintf(stderr, "\t-o\nSpecifies the directory where the object files given with -m will be found.\n\n");
    fprintf(stderr, "\t-r <dir1> [<dir2> ...]\nReads library information from additional directories.\n\n");
    fprintf(stderr, "\t-c <class1> [<class2> ...]\nSelects the classes to be loaded into the application or library.\nThe class <executable-name>app is loaded by default when creating\nan application.\n\n");
    fprintf(stderr, "\t-f <class-list-file>\nReads the specified file and selects the classes to be loaded into\nthe application or library. A single - may be used to indicate stdin. (once)\n\n");
    fprintf(stderr, "\t-s[cnl]*\nCreates a file with a list of the classes defined (-s[cl]*),\nthe classes referenced (-sc[nl]*), and/or the libraries needed for the\nreferenced classes (-sl[cn]*). An 'n' will supress the list of defined classes.\n"); 
    exit(-1);
}

static char *normalobjects[2]={
    "runapp.o",
    NULL
};

static const char **libdirs=NULL;
static int libdirsc=0;

static void AddLibDir(const char *dir) {
    libdirsc++;
    if(libdirs==NULL) {
	libdirs=(const char **)malloc(libdirsc*sizeof(char *));
    } else libdirs=(const char **)realloc(libdirs,libdirsc*sizeof(char *));
    if(libdirs==NULL) {
	fprintf(stderr, "Out of memory adding library directory.\n");
	exit(-1);
    }
    libdirs[libdirsc-1]=dir;
}

static void MakeRunapp(FILE *ifp, char *odir, char **objects, int ocount, char *executablename) {
    int i;
    fprintf(ifp, "NormalObjectRule()\nNormalATKRule()\n");

    if(libdirs!=NULL) {
	fprintf(ifp, "LOCAL_SHARED_LIB_PATH=\\\n");
	const char **p=libdirs;
	int i;
	for(i=0;i<libdirsc;i++) {
	    fprintf(ifp, "\tATK_SHARED_LIB_DIR(%s)%s\n", libdirs[i], i<libdirsc-1?" \\":"");
	}
	fprintf(ifp, "\n");
    }
    
    if(objects==NULL) {
	fprintf(ifp, "runapp.C: $(BASEDIR)/lib/genstatl/runapp.C\n");
	fprintf(ifp, "\t$(RM) runapp.C\n");
	fprintf(ifp, "\tcp $(BASEDIR)/lib/genstatl/runapp.C .\n");
	fprintf(ifp, "clean::\n\t$(RM) runapp.C\n");
	objects=normalobjects;
	ocount=1;
    }
    
    fprintf(ifp, "OBJS=");

    for(i=0;i<ocount;i++) {
	fprintf(ifp, "\t%s%s%s%s\n", odir, odir[0]?"/":"", objects[i], (i<ocount-1)?" \\":"");
    }    
    DumpLibs(ifp);
     
    fprintf(ifp, "\nCPPDynProgramTarget(%s, statl.o $(OBJS), $(LIBS), $(SYSLIBS))\n", executablename);
}

#ifndef MAX
#define MAX(x,y) ((x>y)?(x):(y))
#endif

static char *liblist=NULL;
static int liblistsize=0;
static int DoLibraries(int argc, char **argv) {
    int args=0;
    if(liblist) {
	free(liblist);
	liblist=NULL;
	liblistsize=0;
    }
    while(argc>0 && argv[0] && (argv[0][0]!='-' || argv[0][1]=='l' || argv[0][1]=='L')) {
	int len=strlen(argv[0]);
	args++;
	if(liblist==NULL) {
	    liblistsize=MAX(len+1, 1024);
	    liblist=(char *)malloc(liblistsize);
	    if(liblist==NULL) {
		argc--;
		argv++;
		fprintf(stderr, "genstatl: Out of memory, skipping argument...\n");
		continue;
	    }
	    strcpy(liblist, argv[0]);
	} else {
	    if(strlen(liblist)+len+1>liblistsize) {
		liblistsize+=MAX(len+1, 1024);
		liblist=(char *)realloc(liblist, liblistsize);
		if(liblist==NULL) {
		    argc--;
		    argv++;
		    fprintf(stderr, "genstatl: Out of memory, skipping argument...\n");
		    continue;
		}
	    }
	    strcat(liblist, " ");
	    strcat(liblist, argv[0]);
	}
	argc--;
	argv++;
    }
    if(liblist) AddLibList(liblist, "on command line", -32768);
    return args;
}

static void MakeLibrary(FILE *ifp, char *odir, char **objects, int ocount, char *executablename) {
    
    if(libdirs!=NULL) {
	fprintf(ifp, "LOCAL_SHARED_LIB_PATH=\\\n");
	const char **p=libdirs;
	int i;
	for(i=0;i<libdirsc;i++) {
	    fprintf(ifp, "\tATK_SHARED_LIB_DIR(%s)%s\n", libdirs[i], i<libdirsc-1?" \\":"");
	}
	fprintf(ifp, "\n");
    }
    
    if(objects==NULL) {
	fprintf(ifp, "OBJS = ");
    } else {
	int i;
	fprintf(ifp, "OBJS = ");
	for(i=0;i<ocount;i++) {
	    fprintf(ifp, "\t%s%s%s%s\n", odir, odir[0]?"/":"", objects[i], (i<ocount-1)?" \\":"");
	}
    }    
    DumpLibs(ifp);

    fprintf(ifp, "\nATKLibraryTarget(%s, $(OBJS), $(LIBS), $(SYSLIBS))\n", executablename);
}

static void MakeDynObj(FILE *ifp, char *odir, char **objects, int ocount, char *executablename, char *extras) {
    
    if(libdirs!=NULL) {
	fprintf(ifp, "LOCAL_SHARED_LIB_PATH=\\\n");
	const char **p=libdirs;
	int i;
	for(i=0;i<libdirsc;i++) {
	    fprintf(ifp, "\tATK_SHARED_LIB_DIR(%s)%s\n", libdirs[i], i<libdirsc-1?" \\":"");
	}
	fprintf(ifp, "\n");
    }
    
    if(objects==NULL) {
	fprintf(ifp, "OBJS = ");
    } else {
	int i;
	fprintf(ifp, "OBJS = ");
	for(i=0;i<ocount;i++) {
	    fprintf(ifp, "\t%s%s%s%s\n", odir, odir[0]?"/":"", objects[i], (i<ocount-1)?" \\":"");
	}
    }    
    DumpLibs(ifp);

    fprintf(ifp, "\nDynamicMultiObjectTarget(%s,%s,$(OBJS), $(LIBS), $(SYSLIBS))\n", executablename, extras);
}

static void AtLeastTwo(int argc, char **argv)
{
    if(argc<2) {
	fprintf(stderr, "genstatl: %s flag requires an argument.\n");
	usage();
	exit(-1);
    };
}

int DoExtraClasses(int argc, char **argv, char **extras)
{
    int args=0;
    while(argc && *argv && argv[0][0]!='-') {
	*extras=(char *)realloc(*extras, strlen(*extras)+strlen(argv[0])+2);
	if(*extras==NULL) {
	    fprintf(stderr, "genstatl: Out of memory.\n");
	    exit(-1);
	}
	strcat(*extras, argv[0]);
	strcat(*extras, " ");
	argv++;
	argc--;
	args++;
    }
    return args;
}

int main(int argc, char **argv)
{
    boolean makeexe=FALSE;
    boolean makedyn=FALSE;
    boolean makelib=FALSE;
    boolean doatklib=TRUE;
    boolean dodefaultapp=TRUE;
    FILE *ifp;
    char *executablename="runapp";
    char **objects=NULL;
    char *extras=NewString("");
    int ocount=0;
    char *directory=NULL;
    char *odir="";
    char *orderfilename;
    char *imakefile=NULL;
    char mainFile[MAXPATHLEN];
    FILE *mfp=NULL; /*file to write entry point function 'libName_main_()' in*/
    char ibuf[MAXPATHLEN];
    AndrewDirStr=NewString((char *)AndrewDir(""));
    XLibDirStr=NewString((char *)XLibDir(""));
    AFSBaseDirStr=(char *)getenv("AFSBASEDIR");
    if(AFSBaseDirStr==NULL) AFSBaseDirStr=AFSBASEDIR;
    AFSBaseDirStr=NewString(AFSBaseDirStr);
    JpegLibDirStr=(char *)getenv("JPEGLIBDIR");
    if(JpegLibDirStr) JpegLibDirStr=JPEGLIBDIR;
    TiffLibDirStr=(char *)getenv("TIFFLIBDIR");
    if(TiffLibDirStr) TiffLibDirStr=TIFFLIBDIR;
    
    orderfilename=NewString( (char *)AndrewDir("/lib/genstatl/liborder"));

    getwd(wd);
    
    argv++;
    argc--;
    if(argc<=0) usage();
    while(argc>0) {
	if(argv[0][0]=='-') {
	    int args;
	    switch(argv[0][1]) {
		case 'E':
		    makeexe=TRUE;
		    argv++;
		    argc--;
		    break;
		case 'D':
		    makingdyn=TRUE;
		    makedyn=TRUE;
		    argv++;
		    argc--;
		    args=DoExtraClasses(argc, argv, &extras);
		    argc-=args;
		    argv+=args;
		    break;
		case 'L':
		    argv++;
		    argc--;
		    args=DoLibraries(argc, argv);
		    argc-=args;
		    argv+=args;
		    makelib=TRUE;
		    break;
		case 'a':
		    if(argv[0][2]!='l') {
			dodefaultapp=FALSE;
		    } else {
			if(doatklib) {
			    fakeargv[0]=AndrewDir("/lib/atk");
			    GetInfo(1, fakeargv, FALSE);
			    doatklib=0;
			}
			ProcessEverything();
		    }
		    argv++;
		    argc--;
		    break;
		case 'd':
		    AtLeastTwo(argc, argv);
		    directory=argv[1];
		    argv+=2;
		    argc-=2;
		    break;
		case 'n':
		    doatklib=FALSE;
		    argv++;
		    argc--;
		    break;
		case 'e':
		    AtLeastTwo(argc, argv);
		    executablename=argv[1];
		    argv+=2;
		    argc-=2;
		    break;
		case 'l':
		    AtLeastTwo(argc, argv);
		    orderfilename=argv[1];
		    argv+=2;
		    argc-=2;
		    break;
		case 'i':
		    AtLeastTwo(argc, argv);
		    imakefile=argv[1];
		    argv+=2;
		    argc-=2;
		    break;
		case 'm':
		    argv++;
		    argc--;
		    objects=argv;
		    while(argc>0 && argv[0] && argv[0][0]!='-') {
			ocount++;
			argv++;
			argc--;
		    }
		    break;
		case 'o':
		    AtLeastTwo(argc, argv);
		    odir=argv[1];
		    argv+=2;
		    argc-=2;
		    break;
		case 'r':
		    argv++;
		    argc--;
		    if(argc<=0) usage();
		    args=GetInfo(argc, (const char **)argv, TRUE);
		    argc-=args;
		    argv+=args;
		    break;
		case 'c':
		    argv++;
		    argc--;
		    if(argc<=0) usage();
		    if(doatklib) {
			fakeargv[0]=AndrewDir("/lib/atk");
			GetInfo(1, fakeargv, FALSE);
			doatklib=0;
		    }
		    args=DoClasses(argc, argv);
		    argc-=args;
		    argv+=args;
		    break;
		case 'f': 
		    if(doatklib) {
			fakeargv[0]=AndrewDir("/lib/atk");
			GetInfo(1, fakeargv, FALSE);
			doatklib=0;
		    }
		    argv++;
		    argc--;
		    if(argc<=0) usage();
		    if(argv[0][0]=='-' && argv[0][1]=='\0') { ProcessReferences(stdin);
		    } else {
			FILE *fp=fopen(argv[0], "r");
			if(fp==NULL) {
			    perror(argv[0]);
			} else {
			    ProcessReferences(fp);
			    fclose(fp);
			    argv++;
			    argc--;
			}
		    }
		    argv++;
		    argc--;
		    break;
		case 's':
		    AtLeastTwo(argc, argv);
		    /*Generate a file with the verified references/defines info.
		     With -sc  same as the .rf file except that classes which weren't found
		     are removed and all classes in the used libraries are listed.  With -s
		     only the classes defined in the lib or dynamic object are listed.  With
		     -sl the defined classes and the ATK libs needed for the referenced
		     classes are listsed.  (In a section starting with ---.)  With -scl all
		     sections are present. */
		    if(makelib) {
			if(strchr(argv[0], 'n')) dodefsinrf=FALSE;
			else dodefsinrf=TRUE;
			if(strchr(argv[0], 'l')) dolibsinrf=TRUE;
			else dolibsinrf=FALSE;
			if(strchr(argv[0], 'c')) doclassesinrf=TRUE;
			else doclassesinrf=FALSE;
			strcpy(mainFile, argv[1]);
			mfp=fopen(mainFile, "w");
			if(mfp!=NULL) {
			}
			else {
			    perror(mainFile);
			    exit(-1);
			}
		    }
		    argv+=2;
		    argc-=2;
		    break;
		default:
		    usage();
	    }
	} else usage();
    }
    if(makeexe) {
	makelib=makedyn=makingdyn=FALSE;
    }
    if(dodefaultapp && makelib) {
	struct hashf libf;
	char buf[1024];
	if(doatklib) {
	    fakeargv[0]=AndrewDir("/lib/atk");
	    GetInfo(1, fakeargv, FALSE);
	    doatklib=0;
	}
	strcpy(buf, executablename);
	strcat(buf, ".rf");
	libf.pathname=buf;
	ProcessLib(&libf);
	if(mfp)
	    processMainFile(mfp);
    } else if(dodefaultapp && strcmp(executablename, "runapp")!=0) {
	char *p, buf[1024];
	if(doatklib) {
	    fakeargv[0]=AndrewDir("/lib/atk");
	    GetInfo(1, fakeargv, FALSE);
	    doatklib=0;
	}
	strcpy(buf, executablename);
	if (p = strchr(buf, '.'))
	    *p = '\0';	/* remove file extension (if any) */
	strcat(buf, "app");
	ProcessName(buf, 1);
    }
    if(directory==NULL && imakefile==NULL) {
	fprintf(stderr, "genstatl: Error: no directory specified and no Imakefile name specified, use the -d <directory> flag or the -i <Imakefile name> flag.\n");
	exit(-1);
    }
    if(imakefile==NULL) imakefile="Imakefile";
    AddLibraries();
    OrderLibraries(orderfilename);
    if(directory && chdir(directory)) {
	if(mkdir(directory, 0000777)!=0 || chdir(directory)!=0) {
	    perror(directory);
	    exit(-1);
	}
    }
    strcpy(ibuf, imakefile);
    strcat(ibuf, ".BAK");
    if(rename(imakefile, ibuf)!=0 && errno!=ENOENT) {
	fprintf(stderr, "genstatl: Warning: unable to save a backup copy of the previous %s.\n", imakefile);
    }
    ifp=fopen(imakefile, "w");
    if(ifp==NULL) {
	perror(imakefile);
	exit(-1);
    }
    if(!makelib) {
	MakeRunapp(ifp, odir, objects, ocount, executablename);
	DumpStatl();
    } else {
	if(!makedyn) {
	    MakeLibrary(ifp, odir, objects, ocount, executablename);
	} else MakeDynObj(ifp, odir, objects, ocount, executablename, extras);
    }
    if(fclose(ifp)!=0) {
	perror(imakefile);
	exit(-1);
    }

    if(mfp!=NULL)
	if(fclose(mfp)!=0) {
	    perror(mainFile);
	    exit(-1);
	}
    return 0;
}
