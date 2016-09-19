/* scpanner.c - scrollbar / panner box view */

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("scpanner.H")
#include <scpanner.H>
#include <panner.H>


ATKdefineRegistry(scrollandpanner, scroll, NULL);

scrollandpanner::scrollandpanner()
{
    this->pannerp = new panner;
    THROWONFAILURE( TRUE);
}

scrollandpanner::~scrollandpanner()
{
    (this->pannerp)->Destroy();
}

class scrollandpanner *scrollandpanner::Create(class view  *scrollee, int  location)
{
    class scrollandpanner *retval=NULL;

    retval = new scrollandpanner;
    if (retval==NULL) return NULL;

    (retval)->SetView( scrollee);
    (retval)->SetLocation( location);

    return retval;
}

void scrollandpanner::SetScrollee(class view  *scrollee)
{
    (this)->scroll::SetScrollee( scrollee);
    (this->pannerp)->SetScrollee( scrollee);
}

void scrollandpanner::SetChild(class view  *child)
{
    (this->pannerp)->SetChild( child);
    (this)->scroll::SetChild( this->pannerp);
}

class view *scrollandpanner::GetChild()
{
    return (this->pannerp)->GetChild();
}

