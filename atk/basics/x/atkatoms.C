/* Copyright 1992 by the Andrew Consortium and Carnegie Mellon University. All rights reserved. */

#include <andrewos.h>
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
    
