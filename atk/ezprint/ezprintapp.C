/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/file.h */
ATK_IMPL("ezprintapp.H")

#include <sys/param.h> /* For MAXPATHLEN. */

/*
*	ezprint - A program for printing ez documents
*
*	Modified from bx and be1 code.
*
*
*
*/

#include <atom.H>
#include <im.H>
#include <text.H>
#include <textview.H>
#include <readscr.H>
#include <texttroff.H>
#include <view.H>
#include <dataobject.H>
#include <environ.H>
#include <filetype.H>
#include <attribs.h>

#include <ezprintapp.H>

/* doc_Read(dd,file) */
#include <signal.h>
#include <ctype.h>
#include <print.H>
#define print_INDEXTROFF (10) /* copied from support/print.C */
#define print_POSTSCRIPTMASK  (128) /* check this bit to see if the print command generates PS or troff in the tmp file. */
#include <keymap.H>
#ifdef hpux
#include <unistd.h>
#endif /* hpux */
/* output options */
#define PREVIEW 0
#define PRINT 1
#define TROFF 2

static boolean quiet;
#ifdef PageOffsetOpt

static char *topts[] = {
    "1.25",
    "6",
    ".fi",
};

#endif /* PageOffsetOpt */

static struct view_printopt po_printer, po_psfile, po_tofile, po_landscape, po_scale, po_papersize;
static struct view_printopt po_enumcontents, po_swapheaders, po_endnotes, po_doindex, po_docontents;

ATKdefineRegistry(ezprintapp, application, ezprintapp::InitializeClass);

static void usage(class ezprintapp  *self);


static const char *progname;

boolean ezprintapp::InitializeClass()
{
    class atom *A_boolean = atom::Intern("boolean");

    po_printer.name = atom::Intern("printer");
    po_printer.type = atom::Intern("string");
    po_printer.label = NULL;
    po_psfile.name = atom::Intern("psfile");
    po_psfile.type = atom::Intern("file");
    po_psfile.label = NULL;
    po_tofile.name = atom::Intern("tofile");
    po_tofile.type = A_boolean;
    po_tofile.label = NULL;
    po_landscape.name = atom::Intern("landscape");
    po_landscape.type = A_boolean;
    po_landscape.label = NULL;
    po_scale.name = atom::Intern("scale");
    po_scale.type = atom::Intern("int");
    po_scale.label = NULL;
    po_papersize.name = atom::Intern("papersize");
    po_papersize.type = atom::Intern("string");
    po_papersize.label = NULL;
    po_docontents.name = atom::Intern("docontents");
    po_docontents.type = A_boolean;
    po_docontents.label = NULL;
    po_doindex.name = atom::Intern("doindex");
    po_doindex.type = A_boolean;
    po_doindex.label = NULL;
    po_endnotes.name = atom::Intern("endnotes");
    po_endnotes.type = A_boolean;
    po_endnotes.label = NULL;
    po_swapheaders.name = atom::Intern("swapheaders");
    po_swapheaders.type = A_boolean;
    po_swapheaders.label = NULL;
    po_enumcontents.name = atom::Intern("enumcontents");
    po_enumcontents.type = A_boolean;
    po_enumcontents.label = NULL;
    return TRUE;
}

void ezprintapp::ReadInitFile()
{
}

static void usage(class ezprintapp  *self)
{
    if(!quiet){
	(self)->PrintVersionNumber();
    }
    fprintf(stderr,"usage: %s <-P PrinterName> <-preview> <-stdin> <-o OutputFileTitle> <-f outputfilename> <-M ps|troff> <-v ScribeVersion> <-contents> <-Landscape> <-%% scale> <-g papersize> <-troff> <-zapafterprinting> <-Enumerate> <-IndexOnly> <-quiet> file\n", progname);
    exit(2);
}

boolean ezprintapp::ParseArgs (int  argc, const char  **argv)
/* Since we are non-interactive, everything is done in this function so we
 can handle the args as they come */
{
    int i;
    int ix;
    const char *DocumentName, *c, *printargs, *outputfile;
    FILE *f;
#if 0 // use if0'd out below
    FILE *ofile;
    boolean indexflag;
#endif
    class dataobject *d = NULL;
    class view *v = NULL;
    const char *objectName,*viewName;
    long objectID;
    int ffl = 0;
    int popt = PRINT;
#ifdef PSPRINTING_ENV
    int mechanism = print_POSTSCRIPTMASK;
#else
    int mechanism = 0;
#endif
    int print_scale = 0; /* 0 means use default */
    boolean orient_landscape = FALSE;
    const char *papersize = NULL;
    int status;
    int LockAndDelete = 0; 
    class view *ff;
    const char *ScribeVersion = NULL;
    const char *printer = NULL;
    const char *currentfile;
#if 0 // use if0'd out below
    indexflag = FALSE;
    ofile = stdout;
#endif
    boolean docontents=FALSE;
    boolean enumcontents=FALSE;
    boolean printendnotes=FALSE;
    /* initialize as if ez */
    ((class application * )this)->name = "ez";
    (this)->application::ReadInitFile();
    ((class application * )this)->name = "ezprint";
    d = NULL;
    printargs = "";
    DocumentName = NULL;
    outputfile = NULL;
    quiet = FALSE;
    f = NULL;
    progname = argv[0];
    ff = (class view *) ATK::NewObject("frame");

    for (i=1;i<argc;i++){
	if (argv[i][0] == '-'){
	    const char *env,*val;
	    char *enew = NULL;
	    switch(argv[i][1]){
		case 'q':
		    quiet = TRUE;
		    break;
		case 'i':
		case 'I':
		    environ::Put("IndexOnly",NULL);
#if 0 // use if0'd out below
		    indexflag = TRUE;
#endif
		    break;
		case 'e':
		    if(argv[i][2] != '\0')
			env = &argv[i][2];
		    else if(++i  < argc){
			env = argv[i];
		    }
		    else {
			usage(this);
			break;
		    }
		    if((val = strchr(env,'=')) != NULL){
			enew = strdup(env);
			if(!enew)
			    exit(1);
			enew[(int)(val - env)] = 0;
			env = enew;
			if(*++val == '\0'){
			    environ::Delete(env);
			    free(enew);
			    break;
			}
		    }
		    environ::Put(env,val);
		    if(enew) free(enew);
		    break;
		case 'E': /* enumerate */
		    enumcontents=TRUE;
		//    v->SetPrintOption(&po_enumcontents, TRUE);
		    break;
		case 'F':
		    val = "";
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    else if(++i  < argc){
			val = argv[i];
		    }
		    switch(*val){
			case 'E':
			case 'e':
			    printendnotes=TRUE;
			  //  v->SetPrintOption(&po_endnotes, TRUE);
			    break;
			default:
			    printendnotes=FALSE;
			    //v->SetPrintOption(&po_endnotes, FALSE);
			    break;
		    }
		case 'c':
		    val = "";
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    docontents= !((*val)=='f' || (*val)=='F' || (*val)=='n' || (*val)=='N');
		   //  v->SetPrintOption(&po_docontents, !((*val)=='f' || (*val)=='F' || (*val)=='n' || (*val)=='N'));
		    break;
		case 'C':
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    else if(++i  < argc){
			val = argv[i];
		    }
		    else {
			usage(this);
			break;
		    }
		    environ::Put("ContentsList",val);
		    break;
		case 'N':
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    else if(++i  < argc){
			val = argv[i];
		    }
		    else {
			usage(this);
			break;
		    }
		    environ::Put("InitialChapNumber",val);
		    break;
		case 'T':
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    else if(++i  < argc){
			val = argv[i];
		    }
		    else {
			usage(this);
			break;
		    }
		    environ::Put("TroffArgs",val);
		    break;
		case 'n':
		    if(argv[i][2] != '\0')
			val = &argv[i][2];
		    else if(++i  < argc){
			val = argv[i];
		    }
		    else {
			usage(this);
			break;
		    }
		    {
		    char buff[128];
		    sprintf(buff,"-n%s",val);
		    environ::Put("TroffArgs",buff);
		    }
		    break;
		case 'p':
		    popt = PREVIEW;
		    break;
		case 't':
		    /* equivalent to -M troff -f - */
		    popt  = PRINT;
		    mechanism = 0;
		    outputfile = "-";
		    break;
		case 'h':	/* hard copy (default) */
		    popt = PRINT;
		    break;
		case 'M':
		    if(argv[i][2] != '\0')
			c = &argv[i][2];
		    else if(++i  < argc){
			c = argv[i];
		    }
		    else {
			usage(this);
		    }
		    if (!strcmp(c, "ps") || !strcmp(c, "PS") || !strcmp(c, "psc") || !strcmp(c, "postscript")) {
			mechanism = print_POSTSCRIPTMASK;
		    }
		    else if (!strcmp(c, "tr") || !strcmp(c, "troff")) {
			mechanism = 0;
		    }
		    else {
			fprintf(stderr, "Unknown print mechanism: %s\n", c);
		    }
		    break;
		case 'L':
		    orient_landscape = TRUE;
		    break;
		case '%':
		    if(argv[i][2] != '\0')
			c = &argv[i][2];
		    else if(++i  < argc){
			c = argv[i];
		    }
		    else {
			usage(this);
		    }
		    ix = atoi(c);
		    if (ix <= 0) {
			usage(this);
		    }
		    print_scale = ix;
		    break;
		case 'O':
		case 'o':
		    if(argv[i][2] != '\0')
			DocumentName = &argv[i][2];
		    else if(++i  < argc){
			DocumentName = argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		case 's':
		    /* Read document from stdin */
		    f = stdin;
		    break;
		case 'f':
		    /* send output to a file */
		    if(argv[i][2] != '\0')
			outputfile = &argv[i][2];
		    else if(++i  < argc){
			outputfile = argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		case 'g':
		    /* paper size */
		    if(argv[i][2] != '\0')
			papersize = &argv[i][2];
		    else if(++i  < argc){
			papersize = argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		case 'a':
		    if(argv[i][2] != '\0')
			printargs = &argv[i][2];
		    else if(++i  < argc){
			printargs = argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
#ifdef PageOffsetOpt
		case 'm':	/* Page offset - left margin */
		    if(argv[i][2] != '\0')
			topts[PageOffsetOpt] = &argv[i][2];
		    else if(++i  < argc){
			topts[PageOffsetOpt] =argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		case 'l':	/* Line length */
		    if(argv[i][2] != '\0')
			topts[LineLengthOpt] = &argv[i][2];
		    else if(++i  < argc){
			topts[LineLengthOpt] =argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		case 'n': /* no fill lines */
		    topts[FillLineOpt] = "";
		    break;
#endif /* PageOffsetOpt */
		case 'v':
		    if (argv[i][2]) {
			ScribeVersion = 2+argv[i];
		    } else if (++i < argc) {
			ScribeVersion = argv[i];
		    } else {
			usage(this);
		    }
		    break;
		case 'V':
		    ScribeVersion = NULL;
		    break;
		case 'z':
		    LockAndDelete = 1;
		    break;
		case 'S':
		case 'P':
		    if(argv[i][2] != '\0')
			printer = &argv[i][2];
		    else if(++i  < argc){
			printer =argv[i];
		    }
		    else {
			usage(this);
		    }
		    break;
		default:
		    printf("bad flag %s\n", argv[i]);
		    break;
	    }
	}
	else {
	    /* argument not beginning with dash */
	    struct attributes *attribs;
	    if((f = fopen(argv[i],(LockAndDelete ? osi_F_READLOCK : "r")) ) == NULL){
		fprintf(stderr,"Can't open %s\n",argv[i]);
		continue;
	    }
	    currentfile = argv[i];
printit:  ;
	    attribs= NULL;
	    ffl++;
	    if(!quiet){
		(this)->PrintVersionNumber();
		quiet = TRUE;
	    }
	    if (LockAndDelete) {
		if (osi_ExclusiveLockNoBlock(fileno(f))){
		    fprintf(stderr, "Cannot lock %s (%d)\n", currentfile, errno);
		    fclose(f);
		    continue;
		}
	    }
	    if (ScribeVersion) {
		objectName = "text";
		objectID = 0;
	    } else {
		if(f == stdin)
		    objectName = (char *) filetype::Lookup(f,NULL, &objectID, &attribs);
		else
		    objectName = (char *) filetype::Lookup(f,currentfile, &objectID, &attribs);
	    }
	    if(objectName == NULL) d = (class dataobject *) new text;
	    else {
		d = (class dataobject *) ATK::NewObject(objectName);
		if (attribs) d->SetAttributes(attribs);
	    }
	    (d)->Read(f,objectID);
	    viewName = (d)->ViewName();
	    if(viewName == NULL) v = (class view *) new textview;
	    else v =(class view *) ATK::NewObject(viewName);
	    (v)->SetDataObject(d);
	    (v)->LinkTree(ff);

	    v->SetPrintOption(&po_enumcontents, enumcontents);
	    v->SetPrintOption(&po_endnotes, printendnotes);
	    v->SetPrintOption(&po_docontents, docontents);
	    /* shove in all the print options we have thus far */
	    if (printer) {
		v->SetPrintOption(&po_printer, (long)printer);
	    }
	    if (outputfile) {
		v->SetPrintOption(&po_tofile, TRUE);
		v->SetPrintOption(&po_psfile, (long)outputfile);
	    }
	    else {
		v->SetPrintOption(&po_tofile, FALSE);
	    }
	    if (papersize) {
		v->SetPrintOption(&po_papersize, (long)papersize);
	    }
	    if (orient_landscape) {
		v->SetPrintOption(&po_landscape, TRUE);
	    }
	    if (print_scale) {
		v->SetPrintOption(&po_scale, print_scale);
	    }

	    /* do the nasty */
	    if (ScribeVersion) {
		class text *mytext;

		mytext = (class text *) d;
		if (readscr::Begin(mytext, 0, (mytext)->GetLength(), 1, ScribeVersion, 1) != mytext) {
		    fprintf(stderr, "Can't read scribe format in file %s\n", currentfile);
		    exit(1);
		}
	    }
#if 0
	    if(popt == TROFF){
		/* write out troff file */
		if(DocumentName){
		    if((ofile = fopen(DocumentName,"w")) == NULL){
			fprintf(stderr,"can't open %s for output\n",DocumentName);
			exit(1);
		    }
		}
		if(indexflag){
		    status = print::ProcessView(v, print_INDEXTROFF, 0 ,currentfile,printargs);
		}
		else {
		    (v)->Print(ofile,"troff","PostScript",1);
		    status = 0; /* Bogus -- no error code returned! */
		}
		fflush(ofile);
		if(ofile != stdout){
		    fclose(ofile);
		    ofile = stdout;
		}
	    } else
#endif
	    {
		if(DocumentName){
		    char *d = strdup(DocumentName), *s;
		    if(!d)
			exit(1);
		    for(s = d; *s != '\0'; s++){
			if(*s == '/'  ) *s = '-';
			else if((!isprint(*s)) || isspace(*s)) *s = '_';
		    }
		    status = print::ProcessView(v, popt|mechanism, 0, d, printargs);
		    free(d);
		}
		else {
		    status = print::ProcessView(v, popt|mechanism, 0, currentfile, printargs);
		}
	    }
	    DocumentName = NULL;
	    if (status) {
		fprintf(stderr, "Print request for %s apparently failed.\n", currentfile);
		if (LockAndDelete) {
		    char Fname[1+MAXPATHLEN], DirName[1+MAXPATHLEN], *s;
		    int ctr = 0;

		    strcpy(DirName, currentfile);
		    s = strrchr(DirName, '/');
		    if (s) *++s = '\0';
		    while (++ctr < 1000) {
			sprintf(Fname, "%s/PrintErr.%d", DirName, ctr);
			if (access(Fname, F_OK) != 0) break;
		    }
		    if (ctr >= 1000) {
			fprintf(stderr, "Cannot rename failed print file %s!", currentfile);
		    } else {
			if (rename(currentfile, Fname)) {
			    fprintf(stderr, "Cannot rename %s to %s!", currentfile, Fname);
			}
		    }
		}
	    } else if (LockAndDelete && unlink(currentfile)) {
		fprintf(stderr, "Cannot delete file %s\n", currentfile);
	    }
	    /* Only close AFTER delete when we did the locking */
	    fclose(f);
	    (v)->Destroy();
	    /* 		dataobject_Destroy(d);
	     */	    }
    }
    if(!ffl) {
	if(f == stdin) {
	    LockAndDelete = FALSE;
	    currentfile = "Stdin";
	    goto printit;
	}
	usage(this);
    }
    return TRUE;
}

int ezprintapp::Run()
{   /* we are already done */
    return 0;
}

ezprintapp::ezprintapp()
{
    (this)->SetPrintVersionFlag(FALSE);
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 0);
    THROWONFAILURE( TRUE);
}

