/* Copyright 1996 Carnegie Mellon University All rights reserved. */

/* awgtfig.C	

	Build widgets atop figure

 *   $Log: awgtfig.C,v $
 * Revision 1.5  1996/05/07  20:36:04  wjh
 * add slot initialImage
 *
 * Revision 1.4  1996/05/02  02:46:58  wjh
 * std hdr; AFWidget --> AWgtFig; remove screenImage slot
 *
 * Revision 1.0  95/09/05  16:23:23  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
 */

#include <andrewos.h>
ATK_IMPL("awgtfig.H")

#include <awgtfig.H>

ATKdefineRegistryNoInit(AWgtFig,AWidget);

AWgtFig::AWgtFig() {
    SLOTINIT(initialImage,NULLFIG);
    SLOTINIT(prepImage,NULLACT);
    SLOTINIT(reviseImage,NULLACT);
    SLOTINIT(passMouse,FALSE);
}

AWgtFig::~AWgtFig() {
}

