/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("print.H")

#include <ctype.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <atom.H>
#include <view.H>
#include <environ.H>
#include <hash.H>

#include <print.H>

#include <util.h>

print *print::current_print = NULL;

/* The following defaults are used by the print software */
#define print_INDEXTROFF 10 /*   produce troff index */
#define print_POSTSCRIPTMASK  (128) /* check this bit to see if the print command generates PS or troff in the tmp file. */

static const char *print_formatcommand, *print_pscformatcommand, *print_printcommand, *print_previewcommand, *print_pscprintcommand, *print_pscpreviewcommand;
static const char *print_spoolpath, *print_spooldir, *print_printertype;

/* following ifndefs allow these to be set from the site.h file */
#ifndef print_FORMATCOMMAND
#define print_FORMATCOMMAND "eqn -T$PRINTERTYPE /tmp/%s.n  | troff -T$PRINTERTYPE - |" 
#endif
#ifndef print_PSCFORMATCOMMAND
#define print_PSCFORMATCOMMAND "cat /tmp/%s.n |" 
#endif
#ifndef print_PRINTCOMMAND
#define print_PRINTCOMMAND " lpr -n"  
#endif
#ifndef print_PSCPRINTCOMMAND
#define print_PSCPRINTCOMMAND " lpr"
#endif
#ifndef print_PREVIEWCOMMAND
#define print_PREVIEWCOMMAND " xditview -"
#endif
#ifndef print_PSCPREVIEWCOMMAND
#define print_PSCPREVIEWCOMMAND "cat >/dev/null; gv /tmp/%s.n"  
#endif
#ifndef print_SPOOLPATH
#define print_SPOOLPATH "/usr/spool/" 
#endif
#ifndef print_SPOOLDIR
#define print_SPOOLDIR "lpr" 
#endif
#ifndef print_PRINTERTYPE
#define print_PRINTERTYPE "ps"
#endif
/* The following strings are use for processing the ATK generated index */

#ifndef DIVERTPLAINTROFF
#define DIVERTPLAINTROFF " 2>&1 | sort +0f -2 +2n| indexpro"
#endif
#ifndef DIVERTPRINTTROFF
#define DIVERTPRINTTROFF " 2>&1 | sort +0f -2 +2n | indexpro | troff -ms -T$PRINTERTYPE"
#endif

ATKdefineRegistry(print, ATK, print::InitializeClass);

#if defined(hp9000s300) && HP_OS < 70
static void print_sigAlrm();
#endif /* hp9000s300 */
static void insert(const char  *src,char  *c);
static char *shove(char  *dest,const char  *search,const char  *src);
static void normalize(char  *s);
static void SetPrinterType (char  *printertype, class view *v);
static int ColorHash(const char  *key);
static boolean ParseHexColor(char  *colbuffer, double  *rval , double  *gval , double  *bval);
void NO_DLL_EXPORT print_EnsureClassInitialized();

extern void printp_init();


#if defined(hp9000s300) && HP_OS < 70
static print_sigAlrm()
{ }
#endif /* hp9000s300 */

static const char hexchars[]="0123456789abcdef";
static const class atom *A_printer, *A_tofile, *A_psfile;

static int mystrtol16(char *p, char **pp)
{
    long result=0;
    const char *h;
    while(*p && (h=strchr(hexchars, isupper(*p)?tolower(*p):*p))) {
	result<<=4;
	result+=(h-hexchars);
	p++;
    }
    *pp=p;
    return result;
}

static void insert(const char  *src,char  *c)
{   /* inserts string src into the begining of string c , assumes enough space */
    const char *p;
    char *q, *enddest;
    enddest = c + strlen(c);
    q = enddest + strlen(src);
    while(enddest >= c) *q-- = *enddest-- ;
    for(p = src; *p != '\0';p++)
	*c++ = *p;
}

static char *shove(char  *dest,const char  *search,const char  *src)
{   /* shove the string src into dest after the string search */
    int searchlen;
    searchlen = strlen(search);
    while(*dest){
	if(*dest == *search && strncmp(dest,search,searchlen) == 0){
	    insert(src,dest + searchlen);
	    return(dest);
	}
	dest++;
    }
    return NULL;
}

static void normalize(char  *s)
{
    char *c;
    for(c = s + strlen(s) - 1; c >= s; c--){
	if(!isalnum(*c)){
	    insert("\\",c);
	}
    }
}

	static boolean 
GeneratePS(char *filename, class view  *v) {
	long pagew, pageh;
	print p;	// printing context
	p.SetFromProperties(v, &pagew, &pageh);

	// pass to PrintPSDoc the page dimensions modified
	// to account for scaling and rotation in PSNewPage
	v->PrintPSDoc(p.GetFILE(), pagew, pageh);

	return p.WritePostScript(filename, NULL);
}

int print::ProcessView(class view  *v, int  print, int  dofork ,
		const char  *DocumentName,const char  *prarg) {
	ATKinit;

    /*  Mostly Gosling Code from PrintDoc in BE 1's BasicIO.c */
    char    PrintCommandFormat[400];
    char    PrintCommand[600];
    char    tname[400],tmpname[400];
    static int  seq = 1;
    char   *p;
    const char   *q, *pp;
    char    dname[400];
    char   *dnameptr;
    FILE   *outf;
    char   pt[128] ;
    struct stat buf;
/*     boolean indexonly = FALSE; */

    if(environ::Get("troffpostprocessor"))
	environ::Delete("troffpostprocessor");
    SetPrinterType(pt, v);
    if ((print == print_PRINTPOSTSCRIPT) 
		&& strncmp(pt,"ps",2) != 0 && strcmp(pt,"postscript") != 0) {
	return (-1);
    }
    if (DocumentName == NULL || *DocumentName == '\0') {
	sprintf(dname, "%d", getpid());
    }
    else {
	q = strrchr(DocumentName, '/');
	q = (q == 0) ? DocumentName
	    : q + 1;
	strcpy(dname, q);
    }
    dnameptr = &dname[strlen(dname)];
    sprintf(tmpname, "/tmp/%s.n", dname);

    /* set tmpname to /tmp/#.n such that neither /tmp/#.n nor /tmp/#.dvi exists. */
    while (1)  {
	char dviname[400];
	while (stat(tmpname, &buf) == 0) {
	    sprintf(dnameptr, ".%d", seq++);
	    sprintf(tmpname, "/tmp/%s.n", dname);
	}
	sprintf(dviname, "/tmp/%s.dvi", dname);
	if (stat(dviname, &buf)/*  && errno == ENOENT */) break;
	sprintf(dnameptr, ".%d", seq++);
	sprintf(tmpname, "/tmp/%s.n", dname);
    }

    if (print & print_POSTSCRIPTMASK) {
#ifndef PSPRINTING_ENV
	/* old way of printing postscript */
	outf = fopen(tmpname,"w");
	if (outf == 0) return(-1);
	(v)->Print(outf,"PostScript","PostScript",1);
	fclose(outf);
#else
	/* new way of printing postscript */
	boolean res = GeneratePS(tmpname, v);
	if (!res) {
	    /* error, or document does not know how to be printed. 
		An error has already been displayed, so we should just 
		delete the temp files and exit quietly. */
	    return (-1);
	}
#endif
    }
    else {
	outf = fopen(tmpname,"w");
	if (outf == 0) return(-1);
	(v)->Print(outf,"troff","PostScript",1);
	fclose(outf);
    }

    /* Now the file `tmpname' contains either troff or postscript code. Great. 
	Phase 2: generate some command to deal with this file. */

    if (!(print==print_PREVIEWPOSTSCRIPT || print==print_PREVIEWTROFF)
	&& v->GetPrintOption(A_tofile)) {
	/* if we're printing, but the to-file option is set, stop processing here and move the tmp file to the desired destination. Note that we do this before piping through {psc}formatcommand -- I think that's the desired behavior. */
	char *destfile = (char *)v->GetPrintOption(A_psfile);
	FILE *inf = NULL;
	outf = NULL;
	int ch;
	if (!destfile) {
	    fprintf(stderr, "Cannot get filename to store %s output in. Output can be found in \"%s\".\n", (print & print_POSTSCRIPTMASK) ? "PostScript" : "troff", tmpname);
	    return 1;
	}
	inf = fopen(tmpname, "r");
	if (!inf) {
	    fprintf(stderr, "Cannot re-read %s output from \"%s\".\n", (print & print_POSTSCRIPTMASK) ? "PostScript" : "troff", tmpname);
	    return 1;
	}
	if (!strcmp(destfile, "-"))
	    outf = stdout;
	else
	    outf = fopen(destfile, "w");
	if (!outf) {
	    fprintf(stderr, "Cannot copy %s output to \"%s\". Output can be found in \"%s\".\n", (print & print_POSTSCRIPTMASK) ? "PostScript" : "troff", destfile, tmpname);
	    fclose(inf);
	    return 1;
	}
	while ((ch=getc(inf)) != EOF) {
	    putc(ch, outf);
	}
	if (!strcmp(destfile, "-")) {
	    /* don't close stdout */
	}
	else
	    fclose(outf);
	fclose(inf);
	unlink(tmpname);
	return 0;
    }

    strcpy(PrintCommandFormat, "");
    if (!(print & print_POSTSCRIPTMASK)) {
	strcpy(PrintCommandFormat, print_formatcommand);
	if((q = environ::Get("TroffArgs")) != NULL){
	    if(q[strlen(q) - 1] != ' '){
		strcpy(PrintCommand,q);
		strcat(PrintCommand," ");
		q = PrintCommand;
	    }
	    shove(PrintCommandFormat,"troff ",q);
#if DEBUG
	    puts(PrintCommandFormat);
	    fflush(stdout);
#endif /* DEBUG */
	}
    }
    else {
	strcpy(PrintCommandFormat, print_pscformatcommand);
    }
    
    if (!(print & print_POSTSCRIPTMASK)) {
	if (print == print_INDEXTROFF || (q = environ::Get("IndexOnly")) != NULL) {
	    /* Set up troff so it only produces the error output,
	     this contains the index information which is them piped through
	     an external program that processes it and pipes it through 
	     another troff process whose output ends up back in the pipe
	     set up by this command */
	    /*	indexonly = TRUE; */
	    if(shove(PrintCommandFormat,"troff ","-z ") == NULL){
		fprintf(stderr,"Can't process index without troff\n");
		fflush(stderr);
	    }
	    else {
		char *mq = strrchr(PrintCommandFormat,'|');
		if(mq != NULL){
		    if(print == print_INDEXTROFF){
			strcpy(mq,DIVERTPLAINTROFF);
		    }
		    else {
			insert(DIVERTPRINTTROFF,mq);
		    }
		}
	    }
	}
	else if((q = environ::Get("troffpostprocessor")) != NULL){
	    char pbuf[2048],*ppp,fbuf[1024];
	    sprintf(fbuf,print_formatcommand,dname, dname, dname, dname, dname);
	    for(ppp = fbuf + strlen(fbuf); ppp > fbuf; ppp--)
		if(*ppp == '|') {*ppp = '\0'; break;}
	    if(shove(fbuf,"troff ","-z ") == NULL){
		fprintf(stderr,"Can't process cross references without troff\n");
		fflush(stderr);
	    }
	    else {
		sprintf(pbuf,"%s %s \"%s\"",q,tmpname,fbuf);
		system(pbuf);
	    }
	}
    }

    normalize(dname); /* This uses backslashes to quote all of the non-alphanumeric characters in dname so the strings used for printing don't get confuse the shell. Instances of tmpname below get quoted with double quotes for the same reason */
#if DEBUG
    printf("PrintCommandFormat = |%s|\n", PrintCommandFormat); fflush(stdout);
#endif /*DEBUG */
    p = &PrintCommandFormat[strlen(PrintCommandFormat)];
    if(print == print_INDEXTROFF){
/*	strcpy(p,print_printcommand); ?????????????????????????????*/
    }	
    else switch(print){	
	case print_PREVIEWTROFF:
	    /* Preview Command */
	    q = environ::GetProfile("previewcommand");
	    if (!q) {
		q = tname;
		if (dofork)
		    sprintf(tname,"%s;rm \"%s\"",print_previewcommand, tmpname);
		else
		    strcpy(tname,print_previewcommand);
	    }
	    strcpy(p, q);
	    break;
	case print_PREVIEWPOSTSCRIPT:
	    /* Postscript Preview Command */
	    q = environ::GetProfile("pscpreviewcommand");
	    if (!q) {
		q = tname;
		if (dofork)
		    sprintf(tname,"%s;rm \"%s\"",print_pscpreviewcommand, tmpname);
		else
		    strcpy(tname,print_pscpreviewcommand);
	    }
	    strcpy(p, q);
	    break;
	case print_PRINTTROFF:
	case print_PRINTPOSTSCRIPT:
	default:
	    /* Print Command */
	    pp = (print == print_PRINTPOSTSCRIPT)? "pscprintcommand" : "printcommand";
	    q = environ::GetProfile(pp);
	    if (!q) {
		pp =  (print == print_PRINTPOSTSCRIPT) ? print_pscprintcommand : print_printcommand ;
		if(prarg == NULL || *prarg == '\0'){
		    q = tname;
		    if(dofork)
			sprintf(tname,"%s;rm \"%s\"",pp,tmpname);
		    else
			strcpy(tname,pp);
		}
		else {
		    q = tname;
		    if(dofork)
			sprintf(tname,"%s %s; rm \"%s\"",pp,prarg,tmpname);
		    else
			sprintf(tname,"%s \"%s\"",pp,prarg);
		}
	    }
	    strcpy(p, q);
	    break;
    }
#if DEBUG
    printf("PrintCommandFormat now = |%s|\n", PrintCommandFormat); fflush(stdout);
#endif /*DEBUG */
    sprintf(PrintCommand, PrintCommandFormat, dname, dname, dname, dname, dname);
#if DEBUG
    printf("PrintCommand = |%s|\n", PrintCommand); fflush(stdout);
#endif /*DEBUG */
    if (dofork) {
#if defined(hp9000s300) && HP_OS < 70
        {
	  WAIT_STATUS_TYPE status;
	  struct sigvec vecAlrm;
	  struct itimerval timer;
	  
	  /** enable an interval timer so we can escape from wait(); **/
	  vecAlrm.sv_handler = print_sigAlrm;
	  vecAlrm.sv_mask = 0;
	  vecAlrm.sv_flags = 0;
	  sigvector(SIGALRM, &vecAlrm, 0);
	  timer.it_value.tv_sec = 0;
	  timer.it_value.tv_usec = 100000;
	  timer.it_interval.tv_sec = 0;
	  timer.it_interval.tv_usec = 100000;
	  setitimer(ITIMER_REAL, &timer, 0);
	  
	  while (wait(&status) > 0) ;
	  
	  /** disable the timer **/
	  timer.it_value.tv_sec = 0;
	  timer.it_value.tv_usec = 0;
	  setitimer(ITIMER_REAL, &timer, 0);
	}
#else /* hp9000s300 */
	pid_t pid;
	WAIT_STATUS_TYPE status;
#ifdef HAVE_WAITPID
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
#else
		while ((pid = wait3 (&status, WNOHANG, 0)) > 0);
#endif
#endif /* hp9000s300 */
	if (osi_vfork() == 0) {
	    int fd;
	    int numfds = FDTABLESIZE();

	    close(0);
	    open("/dev/null", 2, 0777);
	    dup2(0, 1);
	    for (fd = 3; fd < numfds; fd++)
		close(fd);
	    NEWPGRP();
	    execlp("/bin/sh", "sh", "-c", PrintCommand, (char *)NULL);
	    exit(0);
	}
    }
    else  {
	FILE *temp;
#if SY_AIX221 || SY_AIX12 || SY_AIX31 || SY_U54
	signal(SIGCLD, SIG_DFL);
#endif
	if ((temp = popen(PrintCommand, "w")))  {
/*	    while(( c = getc(temp))!= EOF) putc(c,stdout); */
	    if (pclose(temp))  {
		fprintf(stderr, "Print request using the command:\n");
		fprintf(stderr, "\t%s\n", PrintCommand);
		fprintf(stderr, "probably did not complete due - returned the\n");
		fprintf(stderr, "following error message:\n\t");
		perror("");
		unlink(tmpname);
		return(-1);
	    }
	}
	else {
	    fprintf(stderr,"Could not execute the following print command:\n");
	    fprintf(stderr,"\t%s\n", PrintCommand);
	    unlink(tmpname);
	    return(-1);
	}
	unlink(tmpname);
    }
    return(0);
}

static void SetPrinterType (char  *printertype, class view *v) 
 {
    char   *RealSpoolDir = NULL;
    static char TempSpoolDir[1000];
    struct stat buf;
    char *cp;
    char currentprinter[100];
    const char *SpoolPath = print_spoolpath;
    const char *str;

    cp = (char *) v->GetPrintOption(A_printer);
    strcpy(currentprinter, (cp) ? cp : print_spooldir);
    
    str = environ::GetProfile("print.spoolpath");
    if (str)  {
	SpoolPath=strdup(str);
	if (SpoolPath==NULL) return;
    }
    
    if (strchr(currentprinter, '/') != 0) {
	if (stat(currentprinter, &buf) == 0)
	    RealSpoolDir = currentprinter;
    }
    else {
    
     /* Look at SpoolPath to find the right currentprinter */
    
	const char   *p;
	char   *r;

	r = TempSpoolDir;
	p = SpoolPath;
	while (1) {
	    if (*p == '\0' || *p == ':') {
		if (r != TempSpoolDir) {
		    *r++ = '/';
		    strcpy(r, currentprinter);
		    if (stat(TempSpoolDir, &buf) == 0) {
		    
		     /* Found a spool directory */
		    
			RealSpoolDir = TempSpoolDir;
			break;
		    }
		    r = TempSpoolDir;
		}
		if (*p == '\0')
		    break;
	    }
	    else
		if (r != TempSpoolDir || *p != ' ') {
		    *r++ = *p;
		}
	    p++;
	}
    }
    if (str) free((char *)SpoolPath);

    if (RealSpoolDir) {
	FILE *tfile;

	strcat(RealSpoolDir, "/.PrinterType");
	if ((tfile = fopen(RealSpoolDir, "r")))  {
	    fscanf(tfile, "%s", printertype);
	    fclose (tfile);
	    environ::Put("PRINTERTYPE", printertype);
	    return;
	}
	else
	    environ::Put("PRINTERTYPE", print_printertype);
    }
    else  {
        environ::Put("PRINTERTYPE", print_printertype);
    }
    strcpy(printertype,print_printertype);
}

const char *print::GetPrintCmd(int  print)
{
	ATKinit;

    const char *q;
    switch(print){
	case print_PREVIEWTROFF:
	    if((q = environ::GetProfile("previewcommand")) != NULL)
		return q;
	    return print_previewcommand;
	case print_PREVIEWPOSTSCRIPT:
	    if((q = environ::GetProfile("pscpreviewcommand")) != NULL)
		return q;
	    return print_pscpreviewcommand;
	case print_PRINTPOSTSCRIPT:
	    if((q = environ::GetProfile("pscprintcommand")) != NULL) return q;
	    return print_pscprintcommand;
	case print_PRINTTROFF:
	default:
	    if((q = environ::GetProfile("printcommand")) != NULL) return q;
	    return print_printcommand;
    }
}

boolean print::InitializeClass()
{
    const char *foo;
    A_printer = atom::Intern("printer");
    A_psfile = atom::Intern("psfile");
    A_tofile = atom::Intern("tofile");

    if((foo =environ::GetConfiguration("printcommand")) == NULL)
	print_printcommand = print_PRINTCOMMAND;
    else
	print_printcommand = strdup(foo);

    if((foo =environ::GetConfiguration("pscprintcommand")) == NULL)
	print_pscprintcommand = print_PSCPRINTCOMMAND;
    else
	print_pscprintcommand = strdup(foo);

    if((foo =environ::GetConfiguration("previewcommand")) == NULL)
	print_previewcommand = print_PREVIEWCOMMAND;
    else
	print_previewcommand = strdup(foo);

    if((foo =environ::GetConfiguration("pscpreviewcommand")) == NULL)
	print_pscpreviewcommand = print_PSCPREVIEWCOMMAND;
    else
	print_pscpreviewcommand = strdup(foo);

    if(((foo = environ::GetProfile("formatcommand")) != NULL) ||
	((foo =environ::GetConfiguration("formatcommand")) != NULL))
	print_formatcommand = strdup(foo);
    else print_formatcommand = print_FORMATCOMMAND;

    if(((foo = environ::GetProfile("pscformatcommand")) != NULL) ||
	((foo =environ::GetConfiguration("pscformatcommand")) != NULL))
	print_pscformatcommand = strdup(foo);
    else print_pscformatcommand = print_PSCFORMATCOMMAND;

    if((foo =environ::GetConfiguration("spoolpath")) == NULL)
	print_spoolpath = print_SPOOLPATH;
    else
	print_spoolpath = strdup(foo);

    if((foo =environ::GetConfiguration("printer")) == NULL &&
       (foo =environ::GetConfiguration("spooldir")) == NULL)
	print_spooldir = print_SPOOLDIR;
    else
	print_spooldir = strdup(foo);

    if((foo =environ::GetConfiguration("printertype")) == NULL)
	print_printertype = print_PRINTERTYPE;
    else
	print_printertype = strdup(foo);

    printp_init(); /* initialize stuff in printp.C */
    return TRUE;
}

static int SavedKey;

static int ColorHash(const char  *key)
{
    return SavedKey;
}

/* helper function for print__LookUpColor(). rval, gval, bval must be nonNULL, and have undefined values if FALSE is returned. The contents of colbuffer get hacked up. */
static boolean ParseHexColor(char  *colbuffer, double  *rval , double  *gval , double  *bval)
{
    int ix, jx;
    long val;
    char *cx;
    double divval;

    ix = strlen(colbuffer);
    switch (ix) {
	case 3:
	    divval = 15.0;
	    break;
	case 6:
	    divval = 255.0;
	    break;
	case 9:
	    divval = 4095.0;
	    break;
	case 12:
	    divval = 65535.0;
	    break;
	default:
	    return FALSE;
    }

    /* the following is kind of icky, but saves space and time. */
    jx = ix/3;

    colbuffer[jx*3] = '\0';
    val = mystrtol16(colbuffer+jx*2, &cx);
    if (cx!=colbuffer+jx*3) return FALSE;
    *bval = (double)val / divval;

    colbuffer[jx*2] = '\0';
    val = mystrtol16(colbuffer+jx*1, &cx);
    if (cx!=colbuffer+jx*2) return FALSE;
    *gval = (double)val / divval;

    colbuffer[jx*1] = '\0';
    val = mystrtol16(colbuffer+jx*0, &cx);
    if (cx!=colbuffer+jx*1) return FALSE;
    *rval = (double)val / divval;

    return TRUE;
}

/* this takes the color colname and figures out its color components (red, green, blue.) The color name matching is not case-sensitive.
If the color is found, the procedure will return TRUE; the three values are returned in *rval, *gval, *bval. Each will be a real number from 0 (black) to 1 (full intensity). 
If the color is not found, the procedure will return FALSE, and *rval, *gval, *bval will each be set to 0 (pure black.) 
Any or all of rval, gval, bval may be NULL if you don't care about that component. */
boolean print::LookUpColor(const char  *colname, double  *rval , double  *gval , double  *bval)
{
	ATKinit;

#define NUMBASICCOLORS (2)
    struct basic_colors_t {
	const char *name;
	double r, g, b;
    };
    struct hash_colors_t {
	boolean found;
	double r, g, b;
    };

    /* basic_colors are a bunch of colors so common that we can assume their values. */
    static const struct basic_colors_t basic_colors[NUMBASICCOLORS] = {
	{"black",   0.0, 0.0, 0.0},
	{"white",   1.0, 1.0, 1.0}
    };

    static class hash *htbl = NULL;

    int ix;
    int key;
    char lowercolor[128];
    char colbuffer[128];
    const char *cx;
    char *dx;
    struct hash_colors_t *colv;
    int rtmp, gtmp, btmp;
    FILE *fc;

    if (!colname || colname[0]=='\0') {
	if (rval) *rval = 0.0;
	if (gval) *gval = 0.0;
	if (bval) *bval = 0.0;
	return FALSE;
    }

    for (ix=0; ix<NUMBASICCOLORS; ix++) {
	if (FoldedEQ(colname, basic_colors[ix].name)) {
	    if (rval) *rval = basic_colors[ix].r;
	    if (gval) *gval = basic_colors[ix].g;
	    if (bval) *bval = basic_colors[ix].b;
	    return TRUE;
	}
    }

    if (!htbl) {
	htbl = new hash;
	(htbl)->SetHash( ColorHash);
    }

    /* generate hash key and lowercase version of name */
    key = 0;
    for (cx = colname, dx = lowercolor; 
	  *cx && dx - lowercolor < (int)sizeof(lowercolor)-1; 
	  cx++) {
	*dx = (isupper(*cx)) ? tolower(*cx) : *cx;
	key += *dx++;
    }
    *dx = '\0';

    SavedKey = key % hash_BUCKETS;

    colv = (struct hash_colors_t *)(htbl)->Lookup( lowercolor);
    if (colv == NULL) {
	/* find color in rgb.txt */
	colv = (struct hash_colors_t *)malloc(sizeof(struct hash_colors_t));
	colv->found = TRUE;

	if (lowercolor[0]=='#') {
	    strcpy(colbuffer, lowercolor+1);
	    ix = ParseHexColor(colbuffer, &colv->r, &colv->g, &colv->b);
	    if (!ix) {
		colv->r = 0.0;
		colv->g = 0.0;
		colv->b = 0.0;		
		colv->found = FALSE;
	    }
	    (htbl)->Store( lowercolor, (char *)colv);
	}
	else {
	    fc = fopen(XLibDir("/X11/rgb.txt"), "r");
	    if (fc) while (fscanf(fc, " %d %d %d %[^\n]\n", 
				  &rtmp, &gtmp, &btmp, colbuffer) == 4) {
		if (FoldedEQ(colbuffer,  colname))
		    break;
	    }
	    if (fc == NULL || feof(fc)) {
		/* default:  black */
		rtmp = 0, gtmp = 0, btmp = 0; 
		colv->found = FALSE;
	    }  
	    if (fc) fclose(fc);

	    /* save color in hash table */
	    colv->r = (double)rtmp / 255.0;
	    colv->g = (double)gtmp / 255.0;
	    colv->b = (double)btmp / 255.0;		
	    (htbl)->Store( lowercolor, (char *)colv);
	}
    }

    /* extract color numbers from *colv */
    if (rval) *rval = colv->r;
    if (gval) *gval = colv->g;
    if (bval) *bval = colv->b;
    return colv->found;
}

/* sigh */
void print_EnsureClassInitialized()
{
    ATKinit;
}
