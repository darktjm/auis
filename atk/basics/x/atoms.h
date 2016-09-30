/* Copyright 1992 by the Andrew Consortium and Carnegie Mellon University. All rights reserved. */

/** \addtogroup libbasics
 * @{ */
#include <andrewos.h>
#include <X11/X.h>
#include <X11/Xlib.h>
extern Atom *xim_SetupAtoms(Display *dpy, boolean force);


/* this should match the atomname table in atoms.c */
#define	atoms_ATK		    0
#define	atoms_INCOMING		    1
#define	atoms_TARGETS		    2
#define	atoms_TIMESTAMP		    3
#define	atoms_MULTIPLE		    4
#define	atoms_TEXT		    5
#define	atoms_INCR		    6
#define	atoms_CLIPBOARD		    7
#define	atoms_LENGTH		    8
#define	atoms_WM_CHANGE_STATE	    9
#define	atoms_DROP_PROTOCOL	    10
#define	atoms_HOST_FILE_NAME	    11
#define	atoms_ATK_SHADES	    12
#define atoms_WM_PROTOCOLS	    13
#define atoms_WM_DELETE_WINDOW	    14
#define atoms_WM_SAVE_YOURSELF	    15
#define atoms_ATK_COLORS 16
#define atoms_MANAGER 17
#define atoms_ATK_ALLOCATE 18
#define atoms_ATK_DEALLOCATE 19
#define atoms_ATK_EVALUATE 20
#define atoms_PIXEL 21

/* Support for IXI's X.Desktop drag and drop protocol. */
#define DROP_PROTOCOL "IXI_DROP_PROTOCOL"
#define HOST_FILE_NAME "HOST_AND_FILE_NAME"
/* static Atom drop_protocol = None;
  static Atom host_filelist = None; */
#define ATOMREF(self, atom) (self->AtomCache[atom])
#define ATOMCREF(atom) (AtomCache[atom])

#define	xim_ATK ATOMREF(self, atoms_ATK)  
#define xim_INCOMING ATOMREF(self, atoms_INCOMING)
#define xim_TARGETS ATOMREF(self, atoms_TARGETS)
#define xim_TIMESTAMP ATOMREF(self, atoms_TIMESTAMP)
#define xim_MULTIPLE ATOMREF(self, atoms_MULTIPLE)
#define xim_TEXT ATOMREF(self, atoms_TEXT)
#define xim_INCR ATOMREF(self, atoms_INCR)
#define xim_CLIPBOARD ATOMREF(self, atoms_CLIPBOARD)
#define xim_LENGTH ATOMREF(self, atoms_LENGTH)
#define wmchangestate ATOMREF(self, atoms_WM_CHANGE_STATE)
#define drop_protocol ATOMREF(self, atoms_DROP_PROTOCOL)
#define host_filelist ATOMREF(self, atoms_HOST_FILE_NAME)
#define xim_ATK_SHADES ATOMREF(self, atoms_ATK_SHADES)
#define xim_WM_PROTOCOLS ATOMREF(self, atoms_WM_PROTOCOLS)
#define xim_WM_DELETE_WINDOW ATOMREF(self, atoms_WM_DELETE_WINDOW)
#define xim_WM_SAVE_YOURSELF ATOMREF(self, atoms_WM_SAVE_YOURSELF)
#define xim_ATK_COLORS ATOMREF(self, atoms_ATK_COLORS)
#define xim_MANAGER ATOMREF(self, atoms_MANAGER)
#define xim_ATK_ALLOCATE ATOMREF(self, atoms_ATK_ALLOCATE)
#define xim_ATK_DEALLOCATE ATOMREF(self, atoms_ATK_DEALLOCATE)
#define xim_ATK_EVALUATE ATOMREF(self, atoms_ATK_EVALUATE)
#define xim_PIXEL ATOMREF(self, atoms_PIXEL)

#define	ac_ATK ATOMCREF(atoms_ATK)  
#define ac_INCOMING ATOMCREF(atoms_INCOMING)
#define ac_TARGETS ATOMCREF(atoms_TARGETS)
#define ac_TIMESTAMP ATOMCREF(atoms_TIMESTAMP)
#define ac_MULTIPLE ATOMCREF(atoms_MULTIPLE)
#define ac_TEXT ATOMCREF(atoms_TEXT)
#define ac_INCR ATOMCREF(atoms_INCR)
#define ac_CLIPBOARD ATOMCREF(atoms_CLIPBOARD)
#define ac_LENGTH ATOMCREF(atoms_LENGTH)
#define ac_wmchangestate ATOMCREF(atoms_WM_CHANGE_STATE)
#define ac_drop_protocol ATOMCREF(atoms_DROP_PROTOCOL)
#define ac_host_filelist ATOMCREF(atoms_HOST_FILE_NAME)
#define ac_ATK_SHADES ATOMCREF(atoms_ATK_SHADES)
#define ac_WM_PROTOCOLS ATOMCREF(atoms_WM_PROTOCOLS)
#define ac_WM_DELETE_WINDOW ATOMCREF(atoms_WM_DELETE_WINDOW)
#define ac_WM_SAVE_YOURSELF ATOMCREF(atoms_WM_SAVE_YOURSELF)
#define ac_ATK_COLORS ATOMCREF(atoms_ATK_COLORS)
#define ac_MANAGER ATOMCREF(atoms_MANAGER)
#define ac_ATK_ALLOCATE ATOMCREF(atoms_ATK_ALLOCATE)
#define ac_ATK_DEALLOCATE ATOMCREF(atoms_ATK_DEALLOCATE)
#define ac_ATK_EVALUATE ATOMCREF(atoms_ATK_EVALUATE)
#define ac_PIXEL ATOMCREF(atoms_PIXEL)
/** @} */
