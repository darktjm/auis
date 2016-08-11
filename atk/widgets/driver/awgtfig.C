/* Copyright 1996 Carnegie Mellon University All rights reserved.
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/widgets/driver/RCS/awgtfig.C,v 1.5 1996/05/07 20:36:04 wjh Exp $";
#endif

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

