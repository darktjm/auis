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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/rofftext/RCS/rofftextapp.C,v 1.6 1995/07/17 21:08:55 wjh Stab74 $";
#endif


 

/*
 * app for rofftext
 * 
 */



#include <andrewos.h>
ATK_IMPL("rofftextapp.H")
#include <rofftextapp.H>
#include <text.H>

#include <application.H>
#include <rofftext.H>


ATKdefineRegistry(rofftextapp, application, NULL);
#ifndef NORCSID
#endif
static void show_usage(class rofftextapp  *self);


rofftextapp::rofftextapp()
{
    this->macrofile = NULL;
    this->RoffType = FALSE;
    this->outputfile = NULL;
    this->inputfile = NULL;
    this->argv = NULL;
    this->argc = 0;
    this->inputfiles = NULL;
    this->HelpMode = FALSE;
    this->BeCompletelyBogus = FALSE;
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion(1);
    (this)->SetFork(FALSE);
    THROWONFAILURE(  TRUE);
}


/*
 * usage statement
 */
static void show_usage(class rofftextapp  *self)
{
    fprintf(stderr,
	"Usage: %s [-xhntbw] [-m macro file] [-o outputfile] [-] [file...]\n",
	    (self)->GetName());
    fprintf(stderr,
"	-x: show this usage statement\n\
	-h: format for use with the ATK Help system.  Crush initial blank space\n\
	-w: print message about badly formed numbers\n\
	-n: pretend to be nroff (default)\n\
	-t: pretend to be troff\n\
	-m file: read in %sfile as a macro file\n\
	-o file: set output file to 'file'.  Default is standard output\n\
	- : use standard input as the file input\n\
	file: read in these files\n", TMACPREFIX);
}
    

boolean rofftextapp::ParseArgs(int  argc,char  **argv)
{
    char temp2[128], *andrewdir;
    boolean slash = FALSE;

    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;

#define GETARGSTR(var)\
{\
    if((*argv)[2]!='\0')\
        var= ((*argv)[2]=='=' ? &(*argv)[3] : &(*argv)[2]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",(this)->GetName(),*argv);\
        return FALSE;\
    }else {\
    	var= *++argv;\
        argc--;\
    }\
}

    while(*++argv!=NULL && **argv=='-') {
        boolean stop = FALSE;
        switch((*argv)[1]){
                char *temp;
	    case 'x':
		show_usage(this);
		exit(0);
            case 'n':
                this->RoffType = FALSE;
                break;
            case 't':
                this->RoffType = TRUE;
                break;
            case 'm':
		GETARGSTR(temp);
		switch(*temp) {
		    case 'm':
			if((andrewdir = getenv("ANDREWDIR")) == NULL)
			    andrewdir = "/usr/andrew";
			else if(andrewdir[strlen(andrewdir) - 1] == '/')
			    slash = TRUE;
			sprintf(temp2,"%s%slib/tmac/tmac.%s", andrewdir, 
				(slash ? "" : "/"), temp);
			break;
		    default:
			sprintf(temp2,"%s%s", TMACPREFIX, temp);
			break;
		}
		this->macrofile = StrDup(temp2);
                break;
            case 'o':
                GETARGSTR(this->outputfile);
                break;
            case 'h':
                this->HelpMode = TRUE;
                break;
            case 'b':
                this->BeCompletelyBogus = TRUE;
                break;
            case 'w':
                this->PrintWarnings = TRUE;
                break;
            case '\0':
                stop = TRUE;
                break; /* for stdin, use '-' */
            default:
                fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		show_usage(this);
                return FALSE;
        }
        if (stop)
            break;
        argc--;
    }

    /* are there input filenames? */

    if (*argv != NULL)
        this->argv = argv;
    this->argc = argc-1;

    return TRUE;
}

boolean rofftextapp::Start()
{
    return TRUE;
}

int rofftextapp::Run()
{
    class rofftext *r;
    class text *t;
    FILE *in,*out;
    char **ptr1,**ptr2;
    int size = sizeof(char *);

    if(!(this)->application::Start())
	return FALSE;

    r = new rofftext;
    if(r==NULL)
	return FALSE;

    /* be bogus and copy argv into new array, */
    /* saving last string in self->inputfile for __Read. */

    this->inputfiles = (char **)malloc(this->argc * sizeof(char *));
    for(ptr1 = this->argv,ptr2 = this->inputfiles;(ptr1 != NULL) && (*ptr1 != NULL);ptr1++) {
        if (*(ptr1+1)==NULL) {
            *ptr2 = NULL;
            this->inputfile = *ptr1;
        }
        else {
            *ptr2++ = *ptr1;
            size += sizeof(char *);
        }
    }

    if (this->inputfile == NULL)
        in = stdin;
    else {
        in = fopen(this->inputfile,"r");
        r->filename = this->inputfile;
    }

    if (this->outputfile)
        out = fopen(this->outputfile,"w+");
    else
        out = stdout;

    if (this->BeCompletelyBogus) {
        t = new text;
        fprintf(stderr,"Reading roff into text...");
        fflush(stderr);
        rofftext::ReadRoffIntoText(t,in,0,this->inputfiles);
        fprintf(stderr,"done.\n");
        fflush(stderr);
        (t)->Write(out,(long)t,0);
    }
    else {
        r->inputfiles = this->inputfiles;
        r->macrofile = this->macrofile;
        r->RoffType = this->RoffType;
        r->HelpMode = this->HelpMode;
        r->PrintWarnings = this->PrintWarnings;

        (r)->Read(in,(long)r);
        (r)->Write(out,(long)r,0);
    }
    fflush(stderr);
    fflush(stdout);
    return 0;
}