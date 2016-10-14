/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "butter.H"
#include "fontdesc.H"
#include "graphic.H"
#include "cursor.H"


ATKdefineRegistryNoInit(butter, dataobject);

butter::butter()
{
    this->text = NULL;
    this->mycursor=NULL;
    this->HitFunction = NULL;
    this->rockptr = NULL;
    this->rockint = 0;
    this->myfontdesc = NULL;
    this->myfontname = NULL;
    this->myfontsize = 12;
    this->myfonttype = fontdesc_Bold;
    THROWONFAILURE((TRUE));
}

void
butter::SetText(const char  *txt)
{
    if (this->text) free(this->text);
    this->text = strdup(txt);
    (this)->NotifyObservers( 0);
}

void
butter::SetButtonFont(class fontdesc  *f)
{
    this->myfontdesc = f;
    (this)->NotifyObservers( 0);
}

void
butter::SetHitFunction(butter_hitfptr f)
{
    this->HitFunction = f;
}

void
butter::SetRocks(char  *r1, int  r2)
{
    this->rockptr = r1;
    this->rockint = r2;
}

