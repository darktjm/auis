/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

#include <text.H>
#include <sys/types.h>
#include <pwd.h>
#include <stroffet.H>
#define WIDTH 438
#define HEIGHT 100

/****************************************************************/
/*		private functions				*/
/****************************************************************/



/****************************************************************/
/*		class procedures				*/
/****************************************************************/

ATKdefineRegistry(stroffet, icon, stroffet::InitializeClass);

boolean
stroffet::InitializeClass()
    {
    return TRUE;
}

stroffet::stroffet()
{
	ATKinit;

    class text * to;

    to = new text;
    (to)->SetReadOnly(0);
    (this)->SetChild(to);
    (this)->SetTitle("FomatNote: Troff Commands");
    (this)->SetSize( WIDTH, HEIGHT);
    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
stroffet::SetChild(class dataobject  * child)
        {
    (this)->icon::SetChild(child);
    if (child != (class dataobject *)0)
	((class text *)child)->SetReadOnly( 0);
}
