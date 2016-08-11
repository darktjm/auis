/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
static char *dofix_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/dynlink/ix86_LinuxAout/RCS/dofix.C,v 1.4 1994/08/26 15:35:23 rr2b Stab74 $";

/* 
	dofix.c - convert .o file into .do file

	Author:  John H Howard - April 9, 1987
 */
#include <andrewos.h>
#include <fstream.h>

#include <ATKSymTab.H>

#include <a.out.h>
#include <setjmp.h>
#define NEED_DOLOAD_ENVIRONMENT 1
#include <doload.h>
#define RP_LENGTH( rp ) ( rp->r_length )
#define IS_RP_EXTERN( rp ) ( rp->r_extern )
#define IS_RP_PC_REL( rp ) ( rp->r_pcrel )
#define SYM_TYPE( sp ) ( sp->n_type & N_TYPE )
#define IS_EXTERN_SYM( sp ) ( sp->n_type & N_EXT )
#define IS_EXPORTABLE(sp) (IS_EXTERN_SYM(sp) && (SYM_TYPE(sp)!=N_UNDF))
#include <sys/stat.h>

int doload_trace=0;

#include "../common/safe.h"

static void doload_setup(struct doload_environment *e, int inFD, doload_mode mode)
{
    e->mode = mode;
    e->fd = inFD;
    e->problems = 0;
    e->text = NULL;
    e->rtab = NULL;
    e->symtab = NULL;
    e->stringtab = NULL;
    e->newsym = NULL;
    e->newsymcount = 0;
    e->firstfixup=NULL;
    return;
}

/* tear down environment */

static void doload_cleanup(struct doload_environment *e)
{
    if (e->problems > 0) {
	e->problems = -e->problems;
	doload_punt(e, "Errors while processing");
    }
    safe_free((char *)e->rtab);
    safe_free((char *)e->symtab);
    safe_free(e->stringtab);
    safe_free((char *)e->newsym);
    return ;
}

/* read module into memory */

static void doload_read(struct doload_environment *e)
{
    long stringlen;	/* length of string table */

    /* read header */

    safe_read(e, (char *)&(e->header), (long)sizeof e->header);
    if (e->mode == List)
	printf( "\nHEADER\n  magic= %x\n  text = %x\n  data = %x\n\
  bss  = %x\n  syms = %x\n  entry= %x\n  trsize=%x\n  drsize=%x\n",
		N_MAGIC(e->header), e->header.a_text, e->header.a_data,
		e->header.a_bss, e->header.a_syms, e->header.a_entry,
		e->header.a_trsize, e->header.a_drsize);
    if (N_BADMAG(e->header))
	doload_punt(e, "file not in loader format");

    /* read text plus data */
    e->text = safe_malloc( e,
		 (long)(e->header.a_text + e->header.a_data + e->header.a_bss));
    e->data = e->text + e->header.a_text;
    safe_lseek(e, (long)N_TXTOFF(e->header), 0);
    safe_read(e, e->text, (long)(e->header.a_text + e->header.a_data));
    bzero(e->data + e->header.a_data, e->header.a_bss);

    /* read relocation information */

    if (e->header.a_trsize + e->header.a_drsize > 0) {
	long rsize;		/* size of relocation info */

	rsize = e->header.a_trsize + e->header.a_drsize;
	e->rtab = (struct relocation_info *)safe_malloc(e, rsize);
	safe_read(e, (char *)e->rtab, rsize);
    }

    /* read symbol table */
    /* Hope symbol table comes Right after data relocations !! */
    e->symtab = (struct nlist *)safe_malloc(e, (long)e->header.a_syms);
    safe_read(e, (char *)e->symtab, (long)e->header.a_syms);

    /* read string table */
    /* Hope string table comes Right after symbol table !! */
    if (read(e->fd, (char *)&stringlen, sizeof stringlen) == sizeof stringlen) {
	e->stringtab = safe_malloc(e, stringlen);
	safe_read( e, e->stringtab + sizeof stringlen,
		   stringlen - sizeof stringlen);
	bcopy((char *)&stringlen, e->stringtab, sizeof stringlen);
    }
}

/* set entry point */

static int FixEntryPoint(struct doload_environment *e, char *EntryPointName)
{
    register struct nlist *sp;
    register struct nlist *sbound;
    if (EntryPointName == NULL || *EntryPointName == '\0')
	return e->problems;
    sp = e->symtab;
    sbound = (struct nlist *)((char *)sp + e->header.a_syms);
    for (; sp < sbound; sp++) {
	if ( SYM_TYPE( sp ) != N_UNDF
	 && (IS_EXTERN_SYM( sp ) && !(sp->n_type & N_STAB))
	 && strcmp( EntryPointName,
			((sp->n_un.n_strx)
			? (e->stringtab + sp->n_un.n_strx)
			: "<<noname>>") ) == 0 )
	    {
	    switch ( SYM_TYPE(sp) ) {
	    case N_DATA:
	    case N_TEXT:
		e->header.a_entry = sp->n_value;
		break;
	    default:
		fprintf( stderr,
		 "dofix:  invalid entry point relocation %x\n", SYM_TYPE(sp) ) ;
		e->problems++;
	    } /* end of switch */
	    return e->problems;
	} /* end of name match */
    } /* end of loop */
    fprintf(stderr, "dofix:  entry point %s undefined\n", EntryPointName);
    e->problems++;
    return e->problems;
}

/* fix up one relocation table entry */

static void FixRelocation(struct doload_environment *e,struct relocation_info *rp)
{
    register int i;
    register int j;

    if ( IS_RP_EXTERN( rp ) ) {
	register struct nlist *sp = e->symtab + rp->r_symbolnum;
	char *np = ((sp->n_un.n_strx) ? ( e->stringtab + sp->n_un.n_strx )
		    : "<<noname>>" ) ;
	if (SYM_TYPE(sp) == N_UNDF  ) {
#if 0
	    if (sp->n_value == 0) {
		for (i = globalcount; --i >= 0 ;)
		    if (strcmp(globals[i].entryname, np) == 0)
			break;
		if (i < 0 && !(e->ast && e->ast->FindSymbol(np))) {
		    fprintf(stderr, "dofix:  Undefined:  %s\n", np);
		    e->problems++;
		}
	    }
#endif
	    for ( j = e->newsymcount ;
		 --j >= 0
		   && strcmp(e->stringtab + e->newsym[j].n_un.n_strx, np) != 0; ) ;
	    if (j < 0) {
		j = e->newsymcount++;
		e->newsym = (struct nlist *)
		  safe_realloc( e, (char *) e->newsym,
			       e->newsymcount * sizeof *(e->newsym));
		bcopy(sp, e->newsym + j, sizeof *sp);
	    }
	    if (sp->n_value > e->newsym[j].n_value)
		e->newsym[j].n_value = sp->n_value;
	    rp->r_symbolnum = j;
	} /* endif N_UNDF */
	else if ( SYM_TYPE(sp) != N_ABS) {
	    fprintf( stderr,
		    "dofix:  Relocatable symbol value (%s = %d, type %d)\n",
		    np, sp->n_value, SYM_TYPE(sp) );
	    e->problems++;
	}
	else if (sp->n_value != 0) {
	    fprintf( stderr, "dofix:  Nonzero symbol value (%s = %d)\n",
		    np, sp->n_value);
	    e->problems++;
	}
    }
    return;
}
/* write new symbol table */

static void WriteNewSym(struct doload_environment *e, int outFD)
{
    register int i;
    register char *newcp;
    long newstringsize;
    char *newstrings = NULL;

    /* allocate new string table */

    for (newstringsize = sizeof newstringsize, i = e->newsymcount; --i >= 0; )
	newstringsize += strlen(e->stringtab + e->newsym[i].n_un.n_strx) + 1;
    if (doload_trace)
	printf( " new symbol count %d, new string size %d\n",
		e->newsymcount, newstringsize ) ;
    newcp = newstrings = safe_malloc(e, newstringsize);
    *(long *)newcp = newstringsize;
    newcp += sizeof newstringsize;

    /* make a new string table */

    for (i = 0; i < e->newsymcount; i++) {
	register char *oldcp ;
	register int n ;

	oldcp = e->stringtab + e->newsym[i].n_un.n_strx ;
	n = strlen(oldcp) + 1;
	bcopy(oldcp, newcp, n);
	e->newsym[i].n_un.n_strx = newcp - newstrings;
	newcp += n;
    }

    /* write symbols and strings */

    safe_write( e, outFD, (char *) e->newsym,
		e->newsymcount * sizeof (struct nlist) );
    safe_write(e, outFD, newstrings, newstringsize);

    /* clean up */

    safe_free(newstrings);

    return;
}

static void FixSets(struct doload_environment *e) {

    register struct nlist *sp;
    register struct nlist *sbound;

    sp = e->symtab;
    sbound = (struct nlist *)((char *)sp + e->header.a_syms);

    for (; sp < sbound; sp++) {
	char *np = ((sp->n_un.n_strx)
		    ? (e->stringtab + sp->n_un.n_strx) : "<<noname>>" ) ;
	/* for now we assume that the constructors for any shared libraries will be run by the main executable...
	 later we should figure out how to tell if a given absolute symbol refers to a location
	 in a shared library NOT ilinked into the main executable.  -rr2b */
	if(/* SYM_TYPE(sp)==N_SETA || */SYM_TYPE(sp)==N_SETT || SYM_TYPE(sp)==N_SETB || SYM_TYPE(sp)==N_SETD) {
	    int j;
	    sp->n_type=SYM_TYPE(sp)|N_EXT;
	    j = e->newsymcount++;
	    e->newsym = (struct nlist *)
	      safe_realloc( e, (char *) e->newsym,
			    e->newsymcount * sizeof *(e->newsym));
	    bcopy(sp, e->newsym + j, sizeof *sp);	    
	}
    }
}

static char registrystr[]="_ATKregistry_";

static int Exportable(char *np) {
    char *p;
    int len=strlen(np);
    if(len>=sizeof(registrystr)) {
	p=np+len-sizeof(registrystr)+1;
	if(strcmp(p,registrystr)==0) return 2;
    }
    p=np;
    while(p[0] && p[1] && p[2]) if((p[0]=='_' || p[0]=='$')&& p[1]=='_' && isdigit(p[2])) return 1;
    else p++;
    return 0;
}
/* read, fix, and write out module */

static int FixIt(int inFD, int outFD, char *EntryPointName)
{
    struct doload_environment E;
    register struct doload_environment *e;
    unsigned long n;	/* number of relocation items */
    struct relocation_info *rp;

    if (doload_trace)
	printf("FixIt(%d, %d, %s)\n", inFD, outFD, EntryPointName);

    /* set up environment */

    doload_setup(e = &E, inFD, Fix);
    if (setjmp(e->errorJump)) {
	int problems=e->problems;
	doload_cleanup(e);
	return problems;
    }

    /* read module into memory */
    doload_read(e);


      if(1) {
	int j;
	register struct nlist *sp;
	register struct nlist *sbound;

	sp = e->symtab;
	sbound = (struct nlist *)((char *)sp + e->header.a_syms);
	for(;sp<sbound;sp++) {
	    char *np = ((sp->n_un.n_strx) ? ( e->stringtab + sp->n_un.n_strx )
			: "<<noname>>" ) ;
#if 0
	    if(SYM_TYPE(sp)==N_UNDF && np[0]=='A' && strncmp(np, "ATKLD_",6)==0) {
		
	    if(e->ast) {
		char *rpath;
		char sym[MAXPATHLEN];
		strcpy(sym,np+6);
		strcat(sym,".+");
		int found=ATKFindObject(sym,&rpath);
		if(found==0) {
		    doload_entry result=ATKDoLoadI(-1,np+6,NULL,NULL,rpath,e->ast,0);
		    if(result==NULL) return -1;
		}
	    } else {
		int result=ATKDynamicLoad(np+6,0);
		if(result!=0) return result;
	    }
	    }
#endif
	    if(IS_EXTERN_SYM(sp) && (SYM_TYPE(sp)&~N_EXT)!=N_ABS && ((SYM_TYPE(sp)&~N_EXT)!=N_UNDF||(Exportable(np)==2)) && strncmp("__PLT",np,5) && strncmp("__GOT",np,5)  &&Exportable(np)) {
		for ( j = e->newsymcount ;
		     --j >= 0
		       && strcmp(e->stringtab + e->newsym[j].n_un.n_strx, np) != 0; ) ;
		if (j < 0) {
		    j = e->newsymcount++;
		    e->newsym = (struct nlist *)
		      safe_realloc( e, (char *) e->newsym,
				   e->newsymcount * sizeof *(e->newsym));
		    bcopy(sp, e->newsym + j, sizeof *sp);
		}
	    }
	}
    }

    FixSets(e);
    /* repair relocation tables */
    rp = e->rtab;
    for ( n = (e->header.a_trsize + e->header.a_drsize)/(sizeof *rp);
	  n > 0;
	  n--, rp++) {
	FixRelocation(e, rp);
    }
  
    /* get entry point */
    if(FixEntryPoint(e, EntryPointName)==0) {

	/* write out result */
	e->header.a_syms = e->newsymcount * sizeof(struct nlist);
	safe_write(e, outFD, (char *)&(e->header), (long)sizeof e->header);
	if (lseek(outFD, (long)N_TXTOFF(e->header), 0) < 0)
	    doload_punt(e, "seek to write text failed");
	safe_write(e, outFD, e->text, (long)(e->header.a_text + e->header.a_data));
	safe_write(e, outFD, (char *)e->rtab,
		   e->header.a_trsize + e->header.a_drsize);
	WriteNewSym(e, outFD);

    }
    int problems=e->problems;
    doload_cleanup(e);
    return problems;
}

static char *ComputeOutputFileName (char *InputFileName, char *extension)
{
    static char name[1024];
    register char  *p, *q;
    char   *ext;

 /* copy the input name and look for the last '.' */

    for (p = InputFileName, q = name, ext = NULL; *p != '\0';) {
	if (*p == '/')		/* ignore period if '/' follows */
	    p++, q = name, ext = NULL;
	else
	    if ((*q++ = *p++) == '.')
		ext = q - 1;
    }
    if (ext == NULL)
	ext = q;
    *ext = '\0';

 /* overwrite the extension with new extension */

    strncat(name, extension, 1024);
    if (strcmp(InputFileName, name) == 0)
	strncat(name, extension, 1024);
    return name ;
}

char *ATKDoFix(char *path) {
    char *outname=ComputeOutputFileName(path, ".+");
    int infd, outfd;

    infd = open(path, O_RDONLY, 0);

    if (infd < 0) {
	fprintf(stderr, "dofix:  File %s not found\n", path);
	return NULL;
    } else {

	outfd = open(outname, O_WRONLY+O_CREAT+O_TRUNC, 0644);

	if (outfd < 0) {

	    fprintf(stderr, "dofix:  Can not write file %s\n", outname);
	    perror("dofix");
	    return NULL;
	}
	else {

	    if(FixIt(infd, outfd, "_main")!=0) {

		close(outfd);
		return NULL;
	    }
	    if(close(outfd)!=0) return NULL;
	}
	close(infd);
    }
    return outname;
}
