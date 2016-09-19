/* Copyright 1992 by the Andrew Consortium and Carnegie Mellon University. All rights reserved. */

#include <andrewos.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <atoms.h>
/* Note the order and number of the entries below should be reflected in the atom #defines in atoms.h */
static const char * const atomnames[]={
    "ATK",
    "INCOMING",
    "TARGETS",
    "TIMESTAMP",
    "MULTIPLE",
    "TEXT",
    "INCR",
    "CLIPBOARD",
    "LENGTH",
    "WM_CHANGE_STATE",
    DROP_PROTOCOL,
    HOST_FILE_NAME,
    "ATK_SHADES",
    "WM_PROTOCOLS",
    "WM_DELETE_WINDOW",
    "WM_SAVE_YOURSELF",
    "ATK_COLORS",
    "MANAGER",
    "ATK_ALLOCATE",
    "ATK_DEALLOCATE",
    "ATK_EVALUATE",
    "PIXEL"
};
#define NUMATOMS (sizeof(atomnames)/sizeof(char *))

static Atom xim_ATOMS;


static Atom *SetAtoms(Display  *dpy, char  *buf)
{
    Atom *result;
    char *p;
    unsigned int i;
    result=(Atom *)malloc(sizeof(Atom)*NUMATOMS);
    if(result==NULL) {
	fprintf(stderr, "xim: couldn't allocate atom cache for display '%s'\n", XDisplayName(DisplayString(dpy)));
	return NULL;
    }
    p=buf;
    sprintf(p, "%ld ", NUMATOMS);
    p+=strlen(p);
    for(i=0;i<NUMATOMS;i++) {
	result[i]=XInternAtom(dpy, atomnames[i], FALSE);
	sprintf(p, "%s %ld ", atomnames[i], result[i]);
	p+=strlen(p);
    }
    XChangeProperty(dpy, RootWindow(dpy, DefaultScreen(dpy)), xim_ATOMS, XA_STRING, 8, PropModeReplace, (unsigned char*)buf, strlen(buf));
    return result;
}

Atom *xim_SetupAtoms(Display  *dpy, boolean  force)
{
    unsigned int i;
    Atom RetAtom;
    int RetFormat;
    unsigned long RetNumItems;
    unsigned long RetBytesAfter;
    unsigned char *RetData = NULL;
    char buf[1024], *p;

    buf[0]='\0';
    xim_ATOMS=XInternAtom(dpy, "ATK_ATOMS", FALSE);
    if(force || XGetWindowProperty(dpy, RootWindow(dpy, DefaultScreen(dpy)), xim_ATOMS, 0L, 1024L, False, XA_STRING, &RetAtom, &RetFormat, &RetNumItems, &RetBytesAfter,  &RetData)!=Success || RetAtom==None) {
	return SetAtoms(dpy, buf);
    } else {
	Atom *result;
	p=(char *)RetData;
	if(RetData==NULL) return SetAtoms(dpy, buf);
	if(atol(p)!=NUMATOMS) {
	    XFree((char *)RetData);
	    return SetAtoms(dpy, buf);
	}
	result=(Atom *)malloc(sizeof(Atom)*NUMATOMS);
	if(result==NULL) {
	    fprintf(stderr, "xim: couldn't allocate atom cache for display '%s'\n", XDisplayName(DisplayString(dpy)));
	    return NULL;
	}
	p=strchr(p, ' ');
	if(p==NULL) return NULL;
	p++;
	while (p && *p) {
	    char *a=strchr(p, ' ');
	    if(a==NULL) return result;
	    *a='\0';
	    for(i=0;i<NUMATOMS;i++) {
		if(strcmp(p, atomnames[i])==0) {
		    result[i]=(Atom)atol(a+1);
		    break;
		 }
	     }
	     p=strchr(a+1, ' ');
	     if(p) p++;
	}
	XFree((char *)RetData);
	return result;
    }
}
