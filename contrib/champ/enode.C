/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "enode.H"


ATKdefineRegistry(enode, dataobject, NULL);


enode::enode()
{
    this->event = NULL;
    this->mychimp = NULL;
    THROWONFAILURE((TRUE));
}

void enode::SetEvent(struct eventnode  *event)
{
    this->event = event;
    (this)->NotifyObservers( 0);
}

void enode::SetChimp(class chimp  *chimp)
{
    this->mychimp = chimp;
}
