/* ********************************************************************** *\
 *         Copyright Carnegie Mellon University, 1992 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

/*
 * app for gofig
 *
 *	Program to execute the gofig inset as an application
 *
 *	usage:  gofig   [<saved gofig inset file>]
 *
 */

/*
 * $Log: gofigapp.C,v $
 * Revision 1.2  1994/11/10  21:58:26  wjh
 *  fixed compile errors on Pmax
 *
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/17  18:52:51  wjh
 * Copied from /usr/andrew.C++/lib/null
 */


#include <andrewos.h>

#include <filetype.H>
#include <view.H>
#include <im.H>
#include <frame.H>
#include <buffer.H>

#include <gofig.H>
#include <gofigview.H>
#include <gofigapp.H>


ATKdefineRegistry(gofigapp, application, gofigapp::InitializeClass);

static void show_usage(gofigapp  *self);

	boolean
gofigapp::InitializeClass() {
	return TRUE;
}

gofigapp::gofigapp()
		{
	this->inputfile = NULL;
	this->dobj = new gofig;
	(this)->SetMajorVersion( MAJORVERSION);
	(this)->SetMinorVersion( MINORVERSION);
	(this)->SetFork( TRUE);

	THROWONFAILURE(  TRUE);
}

	gofigapp::~gofigapp()
		{
	(this->dobj)->Destroy();
	/* do not free self->inputfile because it is in argv */
}


/*
 * usage statement
 */
	static void
show_usage(gofigapp  *self)
	{
	fprintf(stderr,  "Usage: %s [file]\n",  (self)->GetName());
}


	boolean 
gofigapp::ParseArgs(int  argc, const char  **argv)
			{
	char *name;

	/* application::ParseArgs() passes across the "runapp" and its switches,
		leaving "gofiga" as the first arg. 
		The following switches are also processed and removed:  
			-fg -bg -display -geometry -host  -iconic -ni -profile
	 */

	if( ! (this)->application::ParseArgs( argc, argv))
		return FALSE;

	while(*++argv != NULL && **argv == '-') {
		switch((*argv)[1]){
			case 'd':		
				(this)->SetFork( FALSE);
				break;
			default:
				fprintf(stderr,"%s - unrecognized switch: %s\n",
					(this)->GetName(), *argv);
				show_usage(this);
				return FALSE;
		}
	}

	/* get the name of the gofig inset file, if any */
	this->inputfile = *argv++;

	/* testing gofigapp_GetFork(self) determines if the -d switch 
		was set to start debugging */

	if ( ! (this)->GetFork())
		printf("Args parsed.  dobj @ 0x%p\n", this->dobj);

	return TRUE;
}

	boolean
gofigapp::Start()
	{
	FILE *f;
	long objectID;
	long val;
	char *objectType;
	struct attributes *attributes;
	int c;

	/* we will use the frame/buffer system, 
			though it is not necessary */

	buffer *newbuf;
	char tempName[100];
	frame *newfrm;
	im *im;
	view *v;

	if ( ! (this)->GetFork())
		printf("Start.   file: %s\n", this->inputfile);

	(this)->application::Start();	/* get colors correct */

	if (this->inputfile != NULL) {
		f = fopen(this->inputfile, "r");
		objectType = filetype::Lookup(f, this->inputfile, 
				&objectID, &attributes);
		c = EOF;
		if (feof(f) == 0) c = getc(f);
		if (c != EOF) ungetc(c, f);
		if (c != EOF && objectType != NULL 
				&&  ! ATK::IsTypeByName(objectType,
					 "gofig")) {
			fprintf(stderr, "File is not a saved gofig object, it's a %s\n",
					objectType);  
			return FALSE;
		}

		if (attributes != NULL)
			(this->dobj)->SetAttributes( attributes);

		if (c != EOF)
			val = (this->dobj)->Read( f, objectID);
		else val = dataobject_NOREADERROR;

		if (val != dataobject_NOREADERROR) {
			fprintf(stderr, "Input file is corrupted (%ld): %s\n",
					 val, this->inputfile);
			return FALSE;
		}
	}

	newbuf = buffer::Create("gofigdata", NULL, NULL, this->dobj);
	(newbuf)->SetFilename( this->inputfile ? this->inputfile : "/tmp/gofigdata");


/*	if ( ! gofigapp_GetFork(self))
 *		gofigv_SetDebug(TRUE);
 */

	if((im = im::Create(NULL)) == NULL) {
		/* no window manager found.  
				Could uses curses version here */
		exit(0);
	}

	newfrm = new frame;
	if(newbuf == NULL || newfrm == NULL) {
		fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
		exit(8);
	}
	(newfrm)->SetCommandEnable( TRUE);  /* allow cmds */

	/* connect frame to window */
	(im)->SetView( newfrm);	
	(newfrm)->PostDefaultHandler( "message", 
			(newfrm)->WantHandler( "message"));
	message::DisplayString(newfrm, 0, "");

	/* connect the frame (container for view) 
			to the buffer (container for data object)
			create the gofigview as a side effect !  */
	(newfrm)->SetBuffer( newbuf, TRUE);

	/* give view the input focus */
	v = (newfrm)->GetView();
	(v)->WantInputFocus( v);

	if ( ! (this)->GetFork())
		printf("Focussed.  gofigv @ 0x%p  im @ 0x%p   frame @ 0x%p\n", 
				v, im, newfrm);

	return TRUE;
}

/* super_Run forks and then calls im_KeyboardProcessor.
	It can be overridden if other processing is required here.
	However, that other processing should be at the end of gofigapp_Start 
*/
