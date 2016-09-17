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

#include <andrewos.h>
ATK_IMPL("viewref.H")
#include <dataobject.H>
#include <viewref.H>

static long viewID = 0;


ATKdefineRegistryNoInit(viewref, observable);


viewref::viewref()
{
    env=NULL;
    this->viewType = NULL;
    this->viewID = ::viewID++;
    this->dataObject = NULL;
    this->desw = this->desh = 0;
    THROWONFAILURE( TRUE);
}

viewref::~viewref()
{
    if (this->viewType != NULL) {
	free(this->viewType);
	this->viewType=NULL;
    }
    
    if(this->dataObject) {
	(this->dataObject)->Destroy();
	this->dataObject=NULL;
    }
}

class viewref *viewref::Create(const char  *viewType, class dataobject  *dataObject)
{
    class viewref *newvr;
    
    if ((newvr = new viewref))  {
	if ((newvr->viewType = strdup(viewType))) {
	    newvr->dataObject = dataObject;
	    (dataObject)->Reference();
	    return newvr;
	}
    }
    fprintf(stderr, "Could not allocate viewref structure.\n");
    return NULL;
}
