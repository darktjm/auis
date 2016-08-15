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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/datacat/RCS/datacata.C,v 1.7 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 
ATK_IMPL("datacata.H")
#include <sys/param.h> /* For MAXPATHLEN. */
#include <errno.h>

/*
*	datacat - A program for concatening ez documents
*
*
*
*
*/

#include <datacata.H>
#include <text.H>
#include <dataobject.H>
#include <environ.H>
#include <keymap.H>
#include <filetype.H>
#include <im.H> /* for im_GetDir and im_ChangeDir */
/* output options */


static const char *progname;
#define checkandprint(A) if(A) {(this)->PrintVersionNumber();A = FALSE;};

#define MAXIMUM_DEPTH (20)


ATKdefineRegistry(datacata, application, NULL);
#ifndef NORCSID
#endif
static void clean_insert(class datacata  *self, FILE  *fp);
static void doinsert(class datacata  *self, long  size , long  endskip, long  depth);
static void usage();


void datacata::ReadInitFile()
{
}

/* assumes that we want to insert before the last character */
static void clean_insert(class datacata  *self, FILE  *fp)
{
#define BUFSIZE (85)

    static char buf[BUFSIZE];
    static char namebuf[256];
    static char obuf[512];
    long tid;
    long ix, len, tpos;
    long pos;
    boolean done = FALSE;
    class dataobject *o;
    long startpos;

    pos = (self->tx)->GetLength()-1;
    startpos = pos;

    if (fgets(buf, BUFSIZE, fp) == NULL)
	done = TRUE; /* end of file */

    while (!done) {

	ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	if (ix!=2) {
	    /* not an object inset */
	    (self->tx)->InsertCharacters( pos, buf, strlen(buf));
	    if (self->ipros) doinsert(self, pos, 0, 0);
	}
	else {

	    o = (class dataobject *)ATK::NewObject(namebuf);
	    if (!o) {
		/* error message, read to enddata */
		sprintf(obuf, "[Unable to create '%s']\n", namebuf);
		(self->tx)->InsertCharacters( pos, obuf, strlen(obuf));
	    }
	    else {
		ix = (o)->Read( fp, tid);
		switch (ix) {
		    case dataobject_NOREADERROR:
			if ((o)->IsType( self->tx)) {
			    if (!self->insertedtemplate) {
				self->insertedtemplate = TRUE;
				(self->tx)->ReadTemplate( "default", FALSE);
			    }
			    len = ((class text *)o)->GetLength();
			    if (self->verbose)
				fprintf(stderr, "Copying text object\n", len);
			    (self->tx)->AlwaysCopyText( pos, (class text *)o, 0, len);
			    if (self->ipros) doinsert(self, pos, 0, 0);
			    tpos = (self->tx)->GetLength();
			    (self->tx)->InsertCharacters( tpos, "\n", 1);
			}
			else {
			    if (self->verbose)
				fprintf(stderr, "Inserting object '%s', view '%s'\n", namebuf, (o)->ViewName());
			    (self->tx)->AddView( pos, (o)->ViewName(), o);
			    tpos = (self->tx)->GetLength();
			    (self->tx)->InsertCharacters( tpos, "\n", 1);
			}
			break;
		    default:
			sprintf(obuf, "[Unable to read '%s']\n", namebuf);
			(self->tx)->InsertCharacters( pos, obuf, strlen(obuf));
			break;
		}
	    }
	}

	pos = (self->tx)->GetLength()-1;
	if (fgets(buf, BUFSIZE, fp) == NULL)
	    done = TRUE; /* end of file */
    }

}

static void doinsert(class datacata  *self, long  size , long  endskip, long  depth)
{
    long i,j,len;
    char name[MAXPATHLEN+1],cname[MAXPATHLEN+1],wdname[MAXPATHLEN+1];
    char *c,*cn;
    FILE *f;
    boolean foundopen ;
    int charflag ;

    if (depth >= MAXIMUM_DEPTH) {
	fprintf(stderr, "halting -- there appears to be a recursion of @include lines (depth %d reached)\n", depth);
	return;
    }

    /*getwd(wdname);*/
    im::GetDirectory(wdname);

    while((i = (self->tx)->Index(size,'@',(len = (self->tx)->GetLength()) - (size+endskip))) >= 0){
	if((i == size || ((self->tx)->GetChar(i - 1) == '\n')) &&  (self->tx)->Strncmp(i,"@include",8) == 0 && i + 10 < len){
	    foundopen = FALSE;
	    charflag = 0;
	    cn = cname;
	    j = i + 8;
	    if(len > j + 1024) len = j + 1024;
	    for(c = name; j < len; j++){
		*c = (self->tx)->GetChar(j);
		*cn++ = *c;
		if(*c == ')' || *c == '\n') break;
		if(*c == '(' ){
		    foundopen = TRUE;
		    continue;
		}
		if(*c == ' ' || *c == '\t'){
		    if(charflag == 1 ) charflag = 2;
		    continue;
		}
		if(charflag > 1){
		    /* found space in name */
		    charflag = 3;
		    break;
		}
		else charflag = 1;
		c++;
	    }
	    if(charflag > 0 && charflag < 3 && foundopen && *c == ')'){
		char *slash;

		*c = '\0';
		slash = (char*) strchr(name, '/');
		if(slash && *name != '/') { /* relative filename */
		    char tmp_name[MAXPATHLEN+1];
		    *slash = (char)0;
		    sprintf(tmp_name, "%s/%s", wdname, name);
		    /*chdir(tmp_name);*/
		    im::ChangeDirectory(tmp_name);
		    filetype::CanonicalizeFilename(cname, slash+1, 1024);
		}
		else
		    filetype::CanonicalizeFilename(cname, name, 1024);
		if(self->verbose) {
		    fprintf(stderr,"Including  %s\n",cname);
		    fflush(stderr);
		}
		if((f = fopen(cname,"r") ) == NULL){
		    fprintf(stderr,"Can't open include file %s\n",cname);
		}
		else {
		    (self->tx)->DeleteCharacters(i,j + 1 - i);
		    len = (self->tx)->InsertFile(f,name, i);
		    fclose(f);
		    /*if(self->followlinks) size = i;
		    else size = i + len;*/
		    if (self->followlinks) {
			doinsert(self, i, (self->tx)->GetLength()-(i+len), depth+1);
		    };
		    size = i + len;
		    continue;
		}
	    }
	    else {
		*cn-- = '\0';
		if(*cn == '\n'){
		    *cn = '\0';
		    fprintf(stderr,"badly formed include line '@include%s'\n",cname);
		}
		else 		    
		    fprintf(stderr,"badly formed include line '@include%s...'\n",cname);
	    }

	}
	size = i + 1;
    }
    im::ChangeDirectory(wdname);
}

boolean datacata::ParseArgs (int  argc, const char  **argv)
/*
 Since we are non-interactive, everything is done in this function so we
      can handle the args as they come.
 The procedure is to call BeginRun(), then AddFile() for each file, then FinishRun().
 */
{
    register int i;

    /* initialize as if ez */
    ((class application * )this)->name = "ez";
    (this)->application::ReadInitFile();

    (this)->BeginRun(); /* sets stuff up, sets application name to "datacat" */

    progname = argv[0];
    for (i=1;i<argc;i++){
	if (argv[i][0] == '-'){
	    switch(argv[i][1]){
		case '\0':
		    /* argument was just - */
		    (this)->AddFile( NULL);
		    break;
		case 'i':
		    (this)->SetIncludeLevel( datacata_IncludeYes);
		    break;
		case 'I':
		    (this)->SetIncludeLevel( datacata_IncludeRecursive);
		    break;
		case 'q': 
		    (this)->SetVerboseLevel( datacata_TrackQuiet);
		    break;
		case 'v':
		    (this)->SetVerboseLevel( datacata_TrackVerbose);
		    break;
		case 'c': 
		    (this)->SetCleanMode( TRUE);
		    break;
		case 'O':
		case 'o':
		    if(argv[i][2] != '\0') {
			(this)->SetOutputFile( &argv[i][2]);
		    }
		    else if(++i  < argc) {
			(this)->SetOutputFile( argv[i]);
		    }
		    else {
			checkandprint(this->pv);
			usage();
		    }
		    break;

#ifdef LOCKANDDELETE
		case 'z':
		    (this)->SetLockAndDelete( TRUE);
		    break;
#endif /* LOCKANDDELETE */

		default:
		    checkandprint(this->pv);
		    if((this->tx)->GetLength() == 0){
			fprintf(stderr,"bad flag %s\n", argv[i]);
			usage();
		    }
		    else 
			fprintf(stderr,"bad flag %s - ignoring\n", argv[i]);
		    break;
	    }
	}
	else {
	    (this)->AddFile( argv[i]);
	}
    }
    (this)->FinishRun();
    return TRUE;
}

int datacata::Run()
{   /* we are already done */
    return 0;
}
datacata::~datacata()
{
}
datacata::datacata()
{
    (this)->SetMajorVersion(1);
    (this)->SetMinorVersion(2);
    (this)->SetPrintVersionFlag(FALSE);

    this->ipros = FALSE;
    this->followlinks = FALSE;
    this->LockAndDelete = 0;
    this->pv = TRUE;
    this->verbose = FALSE;
    this->cleanmode = FALSE;

    this->ofilename = NULL;

    THROWONFAILURE( TRUE);
}

/* prepare self for an execution cycle. */
void datacata::BeginRun()
{
    ((class application * )this)->name = "datacat";

    this->insertedtemplate = FALSE;
    this->ofilename = NULL;

    this->tx = new text;
}

/* end execution cycle; write out result and clean up storage. */
void datacata::FinishRun()
{
    FILE *ofile; 

    if (!this->ofilename)
	ofile = stdout;
    else {
	if ((ofile = (fopen(this->ofilename, "w"))) == NULL) {
	    checkandprint(this->pv);
	    fprintf(stderr,"Can't open %s for output\n", this->ofilename);
	}
    }

    (this->tx)->Write( ofile, 1, 0);
    (this->tx)->Destroy();
    if (this->ofilename)
	free(this->ofilename);
    this->ofilename = NULL;
}

/* returns flag of whether the file was read successfully */
boolean datacata::AddFile(const char  *filename /* NULL for stdin */)
{
    FILE *f;
    long clen;
    long oldlength;

    if (!filename) {
	if (!this->cleanmode) {
	    checkandprint(this->pv);
	    fprintf(stderr, "You must include the -c switch to read stdin.\n");
	    return FALSE;
	}
	else {
	    checkandprint(this->pv);
	    if(this->verbose){
		fprintf(stderr,"concatenating stdin\n");
		fflush(stderr);
	    }
	    oldlength = (this->tx)->GetLength();
	    (this->tx)->InsertCharacters( oldlength,"\n",1);
	    clean_insert(this, stdin);
	    clen = (this->tx)->GetLength();
	    if((this->tx)->GetChar(clen - 2) == '\n')
		(this->tx)->DeleteCharacters(clen - 2,1);
	    if(this->ipros) doinsert(this,oldlength,0,0);
	}
	return TRUE;
    }
    else {
	/* we got a real filename */
#ifndef LOCKANDDELETE
	if((f = fopen(filename,"r") ) == NULL){
	    checkandprint(this->pv);
	    fprintf(stderr,"Can't open %s\n",filename);
	    return FALSE;
	}
#else /* LOCKANDDELETE */

	if ((f = fopen(filename,(this->LockAndDelete ? osi_F_READLOCK : "r")) ) == NULL){
	    checkandprint(this->pv);
	    fprintf(stderr,"Can't open %s\n",filename);
	    return FALSE;
	}
	if (this->LockAndDelete) {
	    if (osi_ExclusiveLockNoBlock(fileno(f))){
		checkandprint(this->pv);
		fprintf(stderr, "Cannot lock %s (%d)\n", filename, errno);
		fclose(f);
		return FALSE;
	    }
	}
#endif /* LOCKANDDELETE */
	checkandprint(this->pv);
	if(this->verbose){
	    fprintf(stderr,"concatenating  %s\n",filename);
	    fflush(stderr);
	}
	oldlength = (this->tx)->GetLength();
	(this->tx)->InsertCharacters(oldlength,"\n",1);
	if (this->cleanmode) {
	    clean_insert(this, f);
	}
	else {
	    (this->tx)->InsertFile(f,filename, oldlength);
	}
	fclose(f);
	clen = (this->tx)->GetLength();
	if ((this->tx)->GetChar(clen - 2) == '\n')
	    (this->tx)->DeleteCharacters(clen - 2,1);
	if (this->ipros && !this->cleanmode) {
	    /* if self->cleanmode is on, doinsert has been called already */
	    doinsert(this,oldlength,0,0);
	}
#ifdef LOCKANDDELETE
	if (this->LockAndDelete && unlink(filename)) {
	    fprintf(stderr, "Cannot delete file %s\n", filename);
	}
#endif /* LOCKANDDELETE */
	return TRUE;
    }
}
void datacata::SetIncludeLevel(int  val)
{
    switch (val) {
	case datacata_IncludeNo:
	    this->ipros = FALSE; 
	    this->followlinks = FALSE;
	    break;
	case datacata_IncludeYes:
	    this->ipros = TRUE; 
	    this->followlinks = FALSE;
	    break;
	case datacata_IncludeRecursive:
	    this->ipros = TRUE; 
	    this->followlinks = TRUE;
	    break;
	default:
	    break;
    }
}

void datacata::SetVerboseLevel(int  val)
{
    switch (val) {
	case datacata_TrackQuiet:
	    this->pv = FALSE;
	    this->verbose = FALSE;
	    break;
	case datacata_TrackNormal:
	    this->pv = TRUE;
	    this->verbose = FALSE;
	    break;
	case datacata_TrackVerbose:
	    this->pv = TRUE;
	    this->verbose = TRUE;
	    break;
	default:
	    break;
    }
}

void datacata::SetCleanMode(boolean  val)
{
    this->cleanmode = val;
}

void datacata::SetLockAndDelete(boolean  val)
{
    this->LockAndDelete = val;
}

void datacata::SetOutputFile(const char  *filename /* NULL for stdout */)
{
    if (this->ofilename)
	free(this->ofilename);

    if (!filename)
	this->ofilename = NULL;
    else {
	this->ofilename = (char *)malloc(strlen(filename)+1);
	strcpy(this->ofilename, filename);
    }
}


static void usage()
{
    fprintf(stderr,"usage: %s  <-quiet> <-verbose> <-cleanup> < -o OutputFileName> [ - ] [files] \n",progname);
    fflush(stderr);
    exit(2);
}
