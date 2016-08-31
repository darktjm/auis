/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h>
#include "butter.H"
#include "fontdesc.H"
#include "graphic.H"
#include "cursor.H"


ATKdefineRegistry(butter, dataobject, NULL);

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
    this->text = (char *)malloc(1+strlen(txt));
    strcpy(this->text, txt);
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

