/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <text.H>
#include <sys/types.h>
#include <pwd.h>
#include <note.H>


/****************************************************************/
/*		private functions				*/
/****************************************************************/



/****************************************************************/
/*		class procedures				*/
/****************************************************************/

ATKdefineRegistry(note, icon, note::InitializeClass);

boolean
note::InitializeClass()
    {
    return TRUE;
}

note::note()
{
	ATKinit;

    class text * to;
    struct passwd *userentry;

    userentry = getpwuid(getuid());
    (this)->SetTitle(userentry->pw_name);

    to = new text;
    (to)->SetReadOnly(0);
    (this)->SetChild(to);
    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
note::SetChild(class dataobject  * child)
        {
    (this)->icon::SetChild(child);
    if (child != (class dataobject *)0)
	((class text *)child)->SetReadOnly( 0);
}
