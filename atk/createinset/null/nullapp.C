/* ********************************************************************** *\
 * Copyright Carnegie Mellon University, 1992, 1995 - All Rights Reserved *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * app for null
 *
 *	Program to execute the null inset as an application
 *
 *	usage:  null   [<saved null inset file>]
 *
 */

#include <andrewos.h>
ATK_IMPL("nullapp.H")

#include <filetype.H>
#include <view.H>
#include <im.H>
#include <frame.H>
#include <buffer.H>

#include <null.H>
#include <nullview.H>
#include <nullapp.H>


ATKdefineRegistry(nullapp, application, nullapp::InitializeClass);

static void show_usage(class nullapp  *self);

	boolean
nullapp::InitializeClass() {
	return TRUE;
}

nullapp::nullapp()
		{
	this->inputfile = NULL;
	this->dobj = new null;
	(this)->SetMajorVersion( MAJORVERSION);
	(this)->SetMinorVersion( MINORVERSION);
	(this)->SetFork( TRUE);

	THROWONFAILURE(  TRUE);
}

	nullapp::~nullapp()
		{
	(this->dobj)->Destroy();
	/* do not free self->inputfile because it is in argv */
}


/*
 * usage statement
 */
	static void
show_usage(class nullapp  *self)
	{
	fprintf(stderr,  "Usage: %s [file]\n",  (self)->GetName());
}


	boolean 
nullapp::ParseArgs(int  argc, const char  **argv)
			{
	/* application::ParseArgs() passes across the "runapp" and its switches,
		leaving "nulla" as the first arg. 
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

	/* get the name of the null inset file, if any */
	this->inputfile = *argv++;

	/* testing nullapp_GetFork(self) determines if the -d switch 
		was set to start debugging */

	if ( ! (this)->GetFork())
		printf("Args parsed.  dobj @ 0x%p\n", this->dobj);

	return TRUE;
}

	boolean
nullapp::Start()
	{
	FILE *f;
	long objectID;
	long val;
	char *objectType;
	struct attributes *attributes;
	int c;

	/* we will use the frame/buffer system, 
			though it is not necessary */

	class buffer *buffer;
	class frame *frm;
	class im *im;
	class view *v;

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
					 "null")) {
			fprintf(stderr, "File is not a saved null object, it's a %s\n",
					objectType);  
			return FALSE;
		}

		if (attributes != NULL)
			(this->dobj)->SetAttributes( attributes);

		if (c != EOF)
			val = (this->dobj)->Read( f, objectID);
		else val = dataobject::NOREADERROR;

		if (val != dataobject::NOREADERROR) {
			fprintf(stderr, "Input file is corrupted (%ld): %s\n",
					 val, this->inputfile);
			return FALSE;
		}
	}

	buffer = buffer::Create("nulldata", NULL, NULL, this->dobj);
	(buffer)->SetFilename( this->inputfile ? this->inputfile : "/tmp/nulldata");


/*	if ( ! nullapp_GetFork(self))
 *		nullv_SetDebug(TRUE);
 */

	if((im = im::Create(NULL)) == NULL) {
		/* no window manager found.  
				Could uses curses version here */
		exit(0);
	}

	frm = new frame;
	if(buffer == NULL || frm == NULL) {
		fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
		exit(8);
	}
	(frm)->SetCommandEnable( TRUE);  /* allow cmds */

	/* connect frame to window */
	(im)->SetView( frm);	
	(frm)->PostDefaultHandler( "message", 
			(frm)->WantHandler( "message"));
	message::DisplayString(frm, 0, "");

	/* connect the frame (container for view) 
			to the buffer (container for data object)
			create the nullview as a side effect !  */
	(frm)->SetBuffer( buffer, TRUE);

	/* give view the input focus */
	v = (frm)->GetView();
	(v)->WantInputFocus( v);

	if ( ! (this)->GetFork())
		printf("Focussed.  nullv @ 0x%p  im @ 0x%p   frame @ 0x%p\n", 
				v, im, frm);

	return TRUE;
}

/* super_Run forks and then calls im_KeyboardProcessor.
	It can be overridden if other processing is required here.
	However, that other processing should be at the end of nullapp_Start 
*/
