/* ********************************************************************** *\
 *	Copyright 1994, Carnegie Mellon University
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

#include <andrewos.h>
#include <stdio.h>

#include <im.H>
#include <view.H>
#include <frame.H>
#include <print.H>

#include <gofig.H>
#include <gofigview.H>


extern void doStaticLoads();
static void printdata(gofig  *dobj);
static void fail(char *msg);

static void fail(char *msg) {
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
	exit(9);
}

	int
main(int argc, char **argv) {
	gofig *dobj, *dobj2;
	gofigview *dview, *dview2;
	boolean debug = TRUE;

	frame *frm;
	FILE *f;
	char *fnm;
	im *im;
	int i;
    
	printf("Start\n"); fflush(stdout);

	doStaticLoads();

	printf("Init done\n"); fflush(stdout);

	im::SetProgramName("gofigview test");

	fnm = NULL;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
				case 'f': /* Debugging. */
					debug = FALSE;
					break;
				default: /* Unknown switch. */
					fprintf(stderr, "'%s'", argv[i]);
					fail( " - unknown switch" );
					break;
			}
		else { /* Its a filename */
			fnm = argv[i];
		}
	}

	dobj = new gofig;
	printdata( dobj );

	if (fnm) {
		f=fopen( fnm, "r" );
		if ( ! f)  {
			fprintf( stderr, "'%s'", fnm );
			fail( " - cannot open named file" );
		}
		(dobj)->Read( f, 0 );
		fclose( f );
		printf( "\nAfter reading from %s/n", fnm );
		printdata( dobj );
	}
	else
		(dobj->addstone( 1, 2 ))->color = 'B';

	f = fopen( "gout.gofig", "w" );
	if ( ! f) fail( "cannot open gout.gofig for writing" );
	dobj->Write( f, 13, 0 );
	fclose( f );
	f = fopen( "gout.gofig", "r" );
	if ( ! f) fail( "cannot reopen gout.gofig for reading" );
	fscanf( f, "\\begindata{gofig,%d}/n", &i );
	dobj2 = new gofig;
	printf("rereading gives code %d\n", dobj2->Read( f, i ));
	fclose( f );
	printdata( dobj2 );

	struct stone *s1, *s2, *s3, *s4;
	s1 = dobj->addstone( 2, 4 );
	s1->note = 1;
	s2 = dobj->addstone( 3, 0 );
	s2->note = 2;
	s3 = dobj->addstone( 3, 2 );
	s3->note = 3;
	s4 = dobj->addstone( 3, 3 );
	s4->note = 4;
	printdata( dobj );		/* B 1 2 3 4 gap */
	dobj->deletestone( s2 );	/* move gap left */
	printdata( dobj );		/* B 1 gap 3 4 */
	s1 = dobj->addstone( 4, 3 );	/* move gap right */
	s1->note = 5;
	printdata( dobj );		/* B 1 3 4 5 gap */
	dobj->deletestone( s1 );	/* don't really need to move gap */
	printdata( dobj );		/* B 1 3 4 gap */

	dview = new gofigview;
	(dview)->SetDataObject( dobj );

	/* generate postscript for the image */
	struct view_printopt po;
	po.label = NULL;
	po.name = atom::Intern("psfile");
	po.type = atom::Intern("file");
	dview->SetPrintOption(&po, (long)"gout.ps");
	po.name = atom::Intern("tofile");
	po.type = atom::Intern("boolean");
	dview->SetPrintOption(&po, TRUE);
	print::ProcessView(dview, print_PRINTPOSTSCRIPT, FALSE, "gofigtest", NULL);

	im = im::Create(NULL);
	if (im == NULL) {	
	    fprintf(stderr,"Could not create new window.\nexiting.\n");
	    exit(1);
	}
	frm = new frame;
	if (frm == NULL) {
	    fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
	    exit(2);
	}
	(im)->SetView( frm);
	(frm)->PostDefaultHandler( "message", 
			(frm)->WantHandler( "message"));
	(frm)->SetView( dview );

	if (! debug) {
		int pid;
		if ((pid = fork()) < 0)   /* fork failed */
			exit(3);
      		else if (pid != 0)  /* exit from parent: release typescript */
			exit(0);
	}

	/* tell im to make the view be the input focus */
	(dview)->WantInputFocus( dview);

	printf("Blastoff !!\n"); fflush(stdout);

	im::KeyboardProcessor();		/* Do it */
}

	static void 
printdata(gofig  *dobj)  {
	printf("gofig data object at 0x%lx\n", dobj);
	printf( "board is %dx%d  (printcolwidth: %d)  edges lrtb:%c%c%c%c\n",
			dobj->width, dobj->height, dobj->printcolwidth, 
			dobj->edges&LEFTedge?'|':'.',
			dobj->edges&RIGHTedge?'|':'.', 
			dobj->edges & TOPedge?'-':'.', 
			dobj->edges & BOTTOMedge?'_':'.' );
	int i;
	long *flx = (long *)dobj;

	printf("flex starts 0x%lx 0x%lx 0x%lx 0x%lx\n", 
		flx[1], flx[2], flx[3], flx[4]);
	struct stone *s0 = &(*dobj)[0];
	for (i = 0; i < dobj->nstones(); i++) {
		struct stone *s = &(*dobj)[i];
		printf("	[%d]  %c @ (%d,%d) ", s-s0,
				s->color, s->row, s->col);
		if (s->note < 0) printf("%c\n", -s->note); 
		else if (s->note > 0) printf("%d\n", s->note);
		else printf("\n");
	}
	fflush(stdout);
}

