/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* 
	doload.c - dynamic loader for class system

	Author:  John H Howard - April 4, 1987
 */

#include <andrewos.h>

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


int doload_trace=0;		/* nonzero if debugging */

#include "../common/safe.h"

/* initialize state */

static ATKSymTab DynamicGlobals(sizeof(long));

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
	printf( "\nHEADER\n  magic= %x\n  text = %x\n  data = %x\n"
		"  bss  = %x\n  syms = %x\n  entry= %x\n  trsize=%x\n"
		"  drsize=%x\n",
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

struct globaltab {
    char *entryname;
    long entrypoint;
};
#ifndef DEFINE_GLOBALS
extern struct globaltab globals[];
extern long globalcount;
#else
struct globaltab globals[1];
long globalcount=0;
#endif

/* preset global symbols */

static char *symtypename[] = {"UNDF", "ABS ", "TEXT", "DATA", "BSS ", "????" };


static char *RelocType(int i)
{
    i = (i & N_TYPE) >> 1;
    return symtypename[i <= 4 ? i : 5];
}


/* compute relocation adjustment */

static long adjust(struct doload_environment *e, long tw, struct relocation_info *rp, char *format)
{
    if (e->mode == List)
	printf("  %s", format);
    if (IS_RP_EXTERN( rp )) {
	register struct nlist *sp = e->symtab + rp->r_symbolnum;
	const char *np = ((sp->n_un.n_strx)
		 ? (e->stringtab + sp->n_un.n_strx) :  "<<noname>>");
	if (e->mode == List) {
	    if (tw)
		(void) printf("%x+", tw);
	}
	if ( SYM_TYPE(sp) == N_UNDF && e->mode == Load)
	    doload_punt(e,
		"Internal botch - should have resolved in doload_preset");
	if (e->mode == List)
	    (void) printf( "%s=%x<%s>", np, sp->n_value,
		    RelocType(sp->n_type));
	else {
	    tw += sp->n_value;
	    switch ( SYM_TYPE(sp) ) {
	    case N_DATA:
	    case N_BSS:
	    case N_TEXT:
		tw += (long) e->text;
	    case N_ABS:
		break;
	    case N_UNDF:
		if (IS_EXTERN_SYM( sp ))
		    break;
	    default:
		fprintf(stderr, "doload:  Unknown relocation in symbol.\n");
		fprintf( stderr, "  reltab:  %.8x %.6x %.2x\n",
			 rp->r_address, rp->r_symbolnum, *((char *)rp + 7));
		fprintf( stderr,
			 "  symtab[%.6x]: %.8x %.2x %.2x %.4x %.8x %s\n",
			 rp->r_symbolnum, sp->n_un.n_strx, sp->n_type,
			 sp->n_other, sp->n_desc, sp->n_value, np);
		e->problems++;
	    }
	}
    } /* endif IS_RP_EXTERN( rp ) */
    else {
	if (e->mode == List)
	    printf( "%x<%s>", tw,
		    RelocType(rp->r_symbolnum));
	switch (rp->r_symbolnum & N_TYPE)
	{
	    case N_DATA:
	    case N_BSS:
	    case N_TEXT:
		tw += (long) e->text;
		break;
	    case N_ABS:
		if ((tw & 0xf00f0000) == 0xa0080000) {
		    register int i = (tw >> 20) & 0xFF;
		    const char *np = (i < globalcount)
			     ? globals[i].entryname : "**INDEX TOO LARGE**";
		    if (e->mode == List)
			printf("  >>%s<<", np);
		    else if (i >= globalcount) {
			fprintf(stderr, "doload:  special index invalid\n");
			e->problems++;
		    }
		    else {
			tw = globals[i].entrypoint;
		    }
		}
		break;
	    default:
		doload_punt(e, "unknown symbol type");
	} /* end switch */
    } /* end else */
    return tw;
}

/* relocate one item */

static void doload_relocate(struct doload_environment *e, char *cp, struct relocation_info  *rp)
{
    register long tw;

    switch (RP_LENGTH( rp )) {
    case 0:	/* 1 byte */
	tw = *cp;
  	if (IS_RP_PC_REL( rp )) {
  	    tw += rp->r_address;
  	    tw = adjust(e, tw, rp, "(pcrel)");
  	    tw -= (long)cp;
  	}
  	else
	    tw = adjust(e, tw, rp, "(char)");
	if (e->mode == Load) {
	    if (tw < -128 || tw > 127)
		doload_punt(e, "byte displacement overflow");
	    *cp = tw;
	}
	break;
    case 1:	/* 2 bytes */
	tw = *(short *)cp;
	if (IS_RP_PC_REL( rp ))
	    doload_punt(e, "pc relative short relocation");
	tw = adjust(e, tw, rp, "(short)");
	if (e->mode == Load) {
	    if (tw < -32768 || tw > 32767)
		doload_punt(e, "short displacement overflow");
	    *(short *)cp = tw;
	}
	break;
    case 2:	/* 4 bytes */
	tw = *(long *)cp;
	if (IS_RP_PC_REL( rp )) {

	/* the following kludge is taken from 4.2A's ld.c */

	    tw += rp->r_address;
	    tw = adjust(e, tw, rp, "(pcrel)");
	    tw -= (long)cp;
	} /* if IS_RP_PC_REL( rp ) */
	else
	{
	    tw = adjust(e, tw, rp, "(word)");
	}
	if (e->mode == Load) {
	    *(long *)cp = tw;
	}
	break;
    default:
	doload_punt(e, "unknown relocation length");
    }
    return ;
}

static struct setfixup *FindFixup(struct doload_environment *e, const char *name) {
    struct setfixup *s=e->firstfixup;
    while(s) {
	if(strcmp(s->name, name)==0) break;
	s=s->next;
    }
    return s;
}

static long FindFixupAddr(struct doload_environment *e, const char *name) {
    struct setfixup *s=FindFixup(e, name);
    if(s==NULL) doload_punt(e, "bad fixup");
    return (long)s->list;
}

static long NewFixup(struct doload_environment *e, const char *name, long value) {
    struct setfixup *s=FindFixup(e, name);
    if(e) {
	if(s==NULL) {
	    s=(struct setfixup *)safe_malloc(e, sizeof(struct setfixup));
	    s->next=e->firstfixup;
	    e->firstfixup=s;
	    s->name=(char *)name; /* I guess this better be safe */
	    s->list=(long *)safe_malloc(e, sizeof(long)*2);
	    s->list[0]=0;
	    s->list[1]=0;
	}
	s->list[0]++;
	s->list=(long *)safe_realloc(e, (char *)s->list, (s->list[0]+2)*sizeof(long));
	s->list[s->list[0]]=value;
	s->list[s->list[0]+1]=0;
    }
    if(s) return (long) s->list;
    else return 0;
}
    
static void doload_preset(struct doload_environment *e)
{
    register struct nlist *sp;
    register struct nlist *sbound;
    void *sym;
    sbound = (struct nlist *)((char *)e->symtab+ e->header.a_syms);
	
    for (sp=e->symtab; sp < sbound; sp++) {
	const char *np = ((sp->n_un.n_strx)
		 ? (e->stringtab + sp->n_un.n_strx) : "<<noname>>" ) ;

	if (e->mode == List) {
	    printf( " %.2x %.2x %.4x %.8x  %s %s %s\n",
		    sp->n_type, sp->n_other, sp->n_desc, sp->n_value,
		    RelocType(sp->n_type),
		    ( IS_EXTERN_SYM( sp ) ? "EXT " : "    "),
		    np );
	}
	else if ( SYM_TYPE(sp) == N_UNDF) {
	    register int i;

	    for (i = globalcount;
		 --i >= 0 && strcmp(globals[i].entryname, np) != 0; ) ;
	    if (i >= 0)
		sp->n_value = globals[i].entrypoint;
	    else {
		if (sp->n_value > 0) {

		    unsigned long length = sp->n_value;

		    sp->n_value = (unsigned long)safe_malloc(e, length);
		    bzero((void *)sp->n_value, length);
		    void *sym=DynamicGlobals.AddSymbol(np);
		    *((void **)sym)=(void *)sp->n_value;
		}
		else {
		    void *sym=DynamicGlobals.FindSymbol(np);
		    if(sym) {
			sp->n_value = *(long *)sym;
		    } else {
			fprintf(stderr, "doload:  Undefined symbol: %s\n", np);
			e->problems++;
		    }
		}
		
	    }
	    sp->n_type = N_ABS + N_EXT;
	} /* endif N_UNDF */ else {
	    switch(SYM_TYPE(sp)&~N_EXT) {
		case N_SETA:
		    NewFixup(e, np, sp->n_value);
		    if(IS_EXPORTABLE(sp)) {
			sym=DynamicGlobals.AddSymbol(np);
			if(sym) *((void **)sym)=(void *)sp->n_value;
		    }
		    break;
		case N_SETT:
		case N_SETD:
		case N_SETB:
		    sp->n_value=(long)e->text+sp->n_value;
		    NewFixup(e, np, sp->n_value);
		    if(IS_EXPORTABLE(sp)) {
			sym=DynamicGlobals.AddSymbol(np);
			if(sym) *((void **)sym)=(void *)sp->n_value;
		    }
		    break;
		case N_ABS:
		    if(IS_EXPORTABLE(sp)) {
			sym=DynamicGlobals.AddSymbol(np);
			if(sym) *((void **)sym)=(void *)(sp->n_value);

		    }
		    break;
		case N_DATA:
		case N_BSS:
		case N_TEXT:
		    if(IS_EXPORTABLE(sp)) {
			sym=DynamicGlobals.AddSymbol(np);
			if(sym) *((void **)sym)=(void *)(sp->n_value+e->text);

		    }
		    break;
	  /*  case N_SETD:
		sp->n_value=(long)e->data+sp->n_value;
		NewFixup(e, np, sp->n_value);
	    case N_SETB:
		sp->n_value=(long)e->text+sp->n_value;
		NewFixup(e, np, sp->n_value);
		break; */
	    }
	}
    }
    for(sp=e->symtab;sp<sbound;sp++) {
	void *sym;
	const char *np = ((sp->n_un.n_strx)
		    ? (e->stringtab + sp->n_un.n_strx) : "<<noname>>" ) ;
	 switch(SYM_TYPE(sp)&~N_EXT) {
	     case N_SETA:
	     case N_SETT:
	     case N_SETD:
	     case N_SETB:
		sp->n_value=FindFixupAddr(e, np);
		 sp->n_type=N_ABS|N_EXT;
		 /* FALLTHROUGH */
	   /*  case N_SETT:
		sp->n_value=FindFixupAddr(e, np);
		sp->n_type=N_ABS|N_EXT;
		break;
	    case N_SETD:
		sp->n_value=FindFixupAddr(e, np);
		sp->n_type=N_ABS|N_EXT;
	    case N_SETB:
		sp->n_value=FindFixupAddr(e, np);
		sp->n_type=N_ABS|N_EXT;
		break; */
		 if(IS_EXPORTABLE(sp)) {
		     sym=DynamicGlobals.AddSymbol(np);
		     if(sym) *((void **)sym)=(void *)sp->n_value;
		 }
		break;
	}
	
    }
}
#include <ATKDoLoad.H>


static const char registrystr[]="_ATKregistry_";

static int Exportable(const char *np) {
    const char *p;
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
static int find_global(const char *np) {
	int i;
	    for (i = globalcount;
		 --i >= 0 && strcmp(globals[i].entryname, np) != 0; ) ;
	if(i>=0) return 1;
	else return 0;
}

static int LoadDependencies(struct doload_environment *e, int debug) {
    register struct nlist *sp;
    register struct nlist *sbound;
    void *sym;
    sbound = (struct nlist *)((char *)e->symtab + e->header.a_syms);

    for (sp=e->symtab; sp < sbound; sp++) {
	const char *np = ((sp->n_un.n_strx)
		    ? (e->stringtab + sp->n_un.n_strx) : "<<noname>>" ) ;
	if(SYM_TYPE(sp)==N_UNDF && Exportable(np)==2 && ! find_global(np)) {
	    char buf[MAXPATHLEN];
	    int len=strlen(np)-sizeof(registrystr)+1;
	    strncpy(buf,np,len);
	    buf[len]='\0';
	    if(ATKDynamicLoader(buf+1,debug)==NULL) {
		fprintf(stderr, "doload: couldn't load %s\n", np+6);
		return -1;
	    }
	}
    }
    return 0;
}


/* read and relocate module */
doload_entry ATKDoLoadI(int inFD, const char *name, char **bp, long *lenP, const char *path, int debug)
{
    struct doload_environment E;
    register struct doload_environment *e;
    unsigned long n;	/* number of relocation items */
    struct relocation_info *rp;

    /* set up environment */

    if(path) {
	inFD=open(path, O_RDONLY, 0);
	if(inFD==-1) {
	    perror("doload");
	    fprintf(stderr, "Error: couldn't open %s.\n", path);
	    return NULL;
	}
    }
    doload_setup(e = &E, inFD, Load);
    if (setjmp(e->errorJump)) {
	doload_cleanup(e);
	return NULL;
    }

    /* read module into memory */

    doload_read(e);

    /* do relocation */
    
    if (e->header.a_syms) {
	if(LoadDependencies(e,debug)!=0) return NULL;
	doload_preset(e);
    }
   
    rp = e->rtab;
    for (n = (e->header.a_trsize)/(sizeof *rp); n > 0; n--, rp++) {
	doload_relocate(e, e->text + rp->r_address, rp);
    }
    for (n = (e->header.a_drsize)/(sizeof *rp); n > 0; n--, rp++) {
	doload_relocate(e, e->data + rp->r_address, rp);
    }

    /* all done */

    if (doload_trace)
	printf( " %s: text = 0x%.8x  data = 0x%.8x  entry = 0x%.8x\n",
		name, e->text, e->data, e->text + e->header.a_entry);

    if(bp!=NULL) *bp = e->text;
    if(lenP!=NULL) *lenP = e->header.a_text;

    doload_cleanup(e);

    return (doload_entry) (e->text + e->header.a_entry);
}

extern "C" int (*ATKDoLoad(const char *path, int debug))(int argc, char **argv) {
    return (int (*)(int argc, char **argv))ATKDoLoadI(-1, path, NULL, NULL, path, debug);
}

