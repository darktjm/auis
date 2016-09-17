/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* testv.c

	test the lookzview object
*/

#include <andrewos.h>
#include <stdio.h>

#include <andyenv.h>

#include <im.H>
#include <view.H>
#include <frame.H>
#include <lpair.H>
#include <stylesheet.H>
#include <style.H>

#include <lookz.H>
#include <lookzview.H>

/* include the ones utilized, but not by this .c file itself */
#define class_StaticEntriesOnly

/* required classes */
#include <environ.H>
#include <graphic.H>
#include <fontdesc.H>
#include <cursor.H>

/* classes that are anyway dynamically loaded by the
	required classes */
#include <dataobject.H>
#include <event.H>
#include <filetype.H>
#include <keymap.H>
#include <keystate.H>
#include <proctable.H>
#include <menulist.H>
#include <message.H>
#include <msghandler.H>
#include <observable.H>
#include <updatelist.H>
#include <bind.H>

#undef class_StaticEntriesOnly

printdata(class lookz  *dobj);
static boolean PrintAStyle(class style  *s, class lookzview  *v);
boolean findDefine(FILE  *f);
main( int	   argc, char   **argv );


printdata(class lookz  *dobj)
	{
	printf("Image is %s\n", ((dobj)->GetVisibility() ? "visible" : "hidden"));
	fflush(stdout);
}

static boolean
PrintAStyle(class style  *s, class lookzview  *v)
		{
	printf("%s\n", (s)->GetMenuName()); fflush(stdout);
	return FALSE;
}

/* findDefine - parse file looking for "\define{" */
boolean
findDefine(FILE  *f)
	{
	c;
	char *s;
	while (TRUE) {
		while ((c=getc(f)) != EOF  && c != '\\') 
			{}
		if (c == EOF) return FALSE;
		s = "define{";
		while (*s && (c=getc(f)) != EOF  && c == *s++) 
			{}
		if (c == EOF) return FALSE;
		if (*s == '\0') return TRUE;
		ungetc(c, f);		
	}
}

main( int	   argc, char   **argv )
		{
	class lookz *dobj;
	class lookzview *dview;
	class stylesheet *ss;

	class frame *frame;
	FILE *f;
	class im *im;

	printf("Start\n"); fflush(stdout);
	ATK_Init(".");		/* set up classpath */
	printf("Init done\n"); fflush(stdout);

	im_StaticEntry;
	view_StaticEntry;
	frame_StaticEntry;
	lpair_StaticEntry;
	stylesheet_StaticEntry;
	style_StaticEntry;
/* try making it dynamic
	lookz_StaticEntry;
	lookzview_StaticEntry;
*/
	environ_StaticEntry;
	graphic_StaticEntry;
	fontdesc_StaticEntry;
	cursor_StaticEntry;
	dataobject_StaticEntry;
	event_StaticEntry;
	filetype_StaticEntry;
	keymap_StaticEntry;
	keystate_StaticEntry;
	proctable_StaticEntry;
	menulist_StaticEntry;
	message_StaticEntry;
	msghandler_StaticEntry;
	observable_StaticEntry;
	updatelist_StaticEntry;
	bind_StaticEntry;


	im::SetProgramName("lookzView");

	printf("About to New\n"); fflush(stdout);

	dobj = new lookz;
	printdata(dobj);
	dview = new lookzview;
	(dview)->SetDataObject( dobj);

	printf("Get style\n"); fflush(stdout);

	/* get the style sheet from default.tpl */
	ss = new stylesheet;
	f = fopen ("/usr/andy/lib/tpls/default.tpl", "r");
	if (f) while (findDefine(f)) 
			(ss)->Read( f, TRUE);
/*	stylesheet_EnumerateStyles(ss, PrintAStyle, NULL); */
	(dview)->SetStyleSheet( ss);


	if((im = im::Create(NULL)) == NULL) {	/* (argument can be hostname) */
	    fprintf(stderr,"Could not create new window; exiting.\n");
	    exit(-1);
	}
	if((frame = new frame) == NULL) {	/* use a frame to get message line */
	    fprintf(stderr,"Could not allocate enough memory; exiting.\n");
	    exit(-1);
	}
	(im)->SetView( frame);
	(frame)->PostDefaultHandler(  "message", 
			(frame)->WantHandler(  "message" ) );

		/* in general, the application layer may have a surround (eg. scrollbar) 
			for lookzview, GetApplicationLayer returns its argument */

	printf("Frame built\n"); fflush(stdout);

	(frame)->SetView( (dview)->GetApplicationLayer());

	printf("Blastoff !!\n"); fflush(stdout);

	im::KeyboardProcessor();		/* Do it */
}
