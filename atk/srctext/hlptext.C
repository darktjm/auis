/* File hlptext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   hlptext, a text subclass with dogtags, for creating help files. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include "dogtags.h"
#include "hlptext.H"

ATKdefineRegistry(hlptext, text, NULL);

hlptext::hlptext()
{
    SetCopyAsText(TRUE);
    THROWONFAILURE(TRUE);
}

long hlptext::Read(FILE *file, long id)
{
    long tmpRetValue;

    tmpRetValue = (this)->text::Read(file, id);
    if (tmpRetValue == dataobject_NOREADERROR) {
	dogtags_substitute(this);
    }
    return tmpRetValue;
}
