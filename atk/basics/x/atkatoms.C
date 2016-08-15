/* Copyright 1992 by the Andrew Consortium and Carnegie Mellon University. All rights reserved. */

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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/x/RCS/atkatoms.C,v 3.2 1993/08/02 17:44:57 rr2b Stab74 $";
#endif

#include <X11/Xlib.h>
#include <atoms.h>

int main(int  argc, char  **argv)
{
    char *dpyname;
    Display *dpy;
    if(argc>2) {
	fprintf(stderr, "atkatoms usage: atkatoms [display]\n");
	exit(-1);
    }
    if(argc>1) dpyname=argv[1];
    else dpyname=NULL;
    dpy=XOpenDisplay(dpyname);
    if(dpy==NULL) {
	fprintf(stderr, "atkatoms: couldn't open display '%s'\n", dpyname?dpyname:"(NULL)");
	exit(-1);
    }
    (void) xim_SetupAtoms(dpy, TRUE);
    XFlush(dpy);
    XCloseDisplay(dpy);
}
    
