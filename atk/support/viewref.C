/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
