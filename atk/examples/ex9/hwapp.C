/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/config/COPYRITE

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/examples/ex9/RCS/hwapp.C,v 1.5 1996/12/19 20:21:41 fred Exp $";
#endif



#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "im.H"
#include "frame.H"
#include "hwview.H"


ATKdefineRegistry(helloworldapp, application, NULL);
#ifndef NORCSID
#endif


boolean helloworldapp::Start()
{
    class helloworldview *hwv;
    class view *applayer;
    class frame *theFrame;
    class im *im;

    if(!(this)->application::Start())
	return FALSE;

    hwv=new helloworldview;
    if(hwv==NULL)
	return FALSE;

    applayer=(hwv)->GetApplicationLayer();
    if(applayer==NULL) {
	(hwv)->Destroy();
	return FALSE;
    }

    theFrame=new frame;
    if(theFrame==NULL) {
	(hwv)->DeleteApplicationLayer(applayer);
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL) {
	(theFrame)->Destroy();
	return FALSE;
    }

    (theFrame)->SetView(applayer);
    (im)->SetView(theFrame);

    (hwv)->WantInputFocus(hwv);

    return TRUE;

}