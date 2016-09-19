/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("arbiter.H")
#include <cel.H>
#include <arbiter.H>
static class arbiter *master;


ATKdefineRegistry(arbiter, cel, arbiter::InitializeClass);

class arbiter *arbiter::GetMaster()
{
	ATKinit;

    return master;
}
class arbiter *arbiter::SetMaster(class arbiter  *newmaster)
{
	ATKinit;

    class arbiter *oldmaster;
    oldmaster = master;
    master = newmaster;
    return oldmaster;
}
boolean arbiter::InitializeClass()
{
    master = NULL;
    return TRUE;
}

long arbiter::Read(FILE  *file, long  id)
{
    long result; /* set self as master so that as child cells are read, they will
		   call declare read on self, thus the arbiter will find all
		   of its children */
    class arbiter *lastmaster;
    lastmaster = arbiter::SetMaster(this);
    (this)->SetArbiter(lastmaster);
    this->first = (class cel *)this;
    result = (this)->cel::Read( file, id);
    arbiter::SetMaster(lastmaster);
    (this)->SetArbiter(lastmaster);
    (this)->ReadObjects();
    return result;
}
long arbiter::ReadFile(FILE  *thisFile)
{  
    long result; /* set self as master so that as child cells are read, they will
		   call declare read on self, thus the arbiter will find all
		   of it's children */
    class arbiter *lastmaster;
    lastmaster = arbiter::SetMaster(this);
    (this)->SetArbiter(lastmaster);
    this->first = (class cel *)this;
    result = (this)->cel::ReadFile( thisFile);
    arbiter::SetMaster(lastmaster);
    (this)->SetArbiter(lastmaster);
    (this)->ReadObjects();
    return result;
}
FILE *arbiter::DeclareRead(class cel  *cel)
{
#ifdef DEBUG
    fprintf(stdout,"declaring %d(%s) to %d(%s)\n",cel,(cel)->GetRefName(),this,(this)->GetRefName());fflush(stdout);
#endif /* DEBUG */
    if(cel != (class cel *) this){
	cel->chain = this->first;
	this->first = cel;
    }
    else if((this)->GetArbiter() && (this)->GetArbiter() != this)
	    ((this)->GetArbiter())->DeclareRead((class cel *)this);

    return NULL;
}
class cel *arbiter::FindChildCelByName(char  *name)
{
    class cel *cel,*ncel;
    const char *tname;
    cel = (this)->GetFirst();
    while(cel != (class cel *) this){
	if((tname = (cel)->GetRefName()) != NULL &&
	   strcmp(tname,name) == 0)
	    return cel;
	ncel = (cel)->GetNextChain();
	if(ncel == cel) return NULL; /* shouldn't happen */
	cel = ncel;
    }
    return NULL;
}
class dataobject *arbiter::FindChildObjectByName(char  *name)
{
    class cel *cel;
    cel = (this)->FindChildCelByName(name);
    if(cel != NULL) return (cel)->GetObject();
    return NULL;
}
void arbiter::ReadObjects()
{
/* for subclass to use */
}
arbiter::arbiter()
{
	ATKinit;

this->first = (class cel *)this;
THROWONFAILURE( TRUE);
}
