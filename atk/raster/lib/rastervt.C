/*LIBS: -lframe -ltext -lsupviews -lsupport -lbasics -lclass -lmalloc -lerrors -lutil
*/

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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/* testv.c

	test rasterview
*/

#include <andrewos.h>
#include <andyenv.h>
#include <stdio.h>


#include <raster.H>
#include <rasterview.H>

#include <im.H>
#include <view.H>
#include <frame.H>
#include <lpair.H>

/* include the ones utilized, but not by this .c file itself */
#define class_StaticEntriesOnly

/* required classes */
#include <environ.H>
#include <graphic.H>
#include <fontdesc.H>
#include <cursor.H>
#ifdef WM_ENV
#include <wws.ih>
#include <wgraphic.ih>
#include <wim.ih>
#include <wcursor.ih>
#include <wfontd.ih>
#include <mrl.ih>
#endif /* WM_ENV */

/* classes that are anyway dynamically loaded by the
	required classes */
#include <dataobject.H>
#include <event.H>
#include <filetype.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <proctable.H>
#include <message.H>
#include <msghandler.H>
#include <observable.H>
#include <updatelist.H>

#undef class_StaticEntriesOnly


int printdata(class raster  *dobj);


main(int	   argc, char   **argv)
		{
	class raster *dobj;
	class rasterview *dview;

	class frame *frame;
	FILE *f;
	char *fnm;
	class im *im;
	short i;

	printf("This test program has not been completely ported to C++...\nIt needs to statically load everything.\n");
	
	printf("Start\n"); fflush(stdout);
	printf("Init done\n"); fflush(stdout);


	printf("About to New\n"); fflush(stdout);

	fnm = NULL;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
				case 'd': /* Debugging. */
					/* debug = TRUE; */
					break;
				default: /* Unknown switch. Treat it as a file... */
					fnm = argv[i]+1;
					break;
			}
		else { /* Its a file right? */
			fnm = argv[i];
		}
	}

	dobj = new raster;
	printf("Newed\n"); fflush(stdout);

	if (fnm && (f=fopen(fnm, "r")))
		(dobj)->Read( f, 0);
	else 
		(dobj)->Read( fopen(environ::AndrewDir("/lib/rasters/people/doug.ras"), "r"), 0);
	printdata(dobj);
	dview = new rasterview;
	printf("View Newed\n"); fflush(stdout);
	(dview)->SetDataObject( dobj);
	printf("View SDOed\n"); fflush(stdout);

	if((im = im::Create(NULL)) == NULL) {	/* (argument can be hostname) */
	    fprintf(stderr,"rastervt: Could not create new window; exiting.\n");
	    exit(-1);
	}
	if((frame = new frame) == NULL) {	/* use a frame to get message line */
	    fprintf(stderr,"rastervt: Could not allocate enough memory; exiting.\n");
	    exit(-1);
	}
	(im)->SetView( frame);
	(frame)->PostDefaultHandler( "message", 
			(frame)->WantHandler( "message"));

	printf("SetView\n"); fflush(stdout);
	(frame)->SetView( dview);

	printf("Blastoff !!\n"); fflush(stdout);

	im::KeyboardProcessor();		/* Do it */
}

int printdata(class raster  *dobj)
	{
	printf ("%d x %d\n", (dobj)->GetWidth(), 
			(dobj)->GetHeight());
	fflush(stdout);
	return 0;
}

