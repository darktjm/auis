/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <sbutton.ih>
#define dialog_DS_VERSION 5

class dialog : dataobject[dataobj] {
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct sbutton *self) returns boolean;
    FinalizeObject(struct sbutton *self);
    
overrides:
    ViewName() returns char *;
    Read (FILE *fp, long id) returns long;
    Write (FILE *fp, long id, int level) returns long;

macromethods:
    GetStyle() (sbutton_GetStyle(self->prefs))
    GetText() ((self)->text)
    GetButtons() ((self)->buttons)
    GetHitFunc() ((self)->buttons->hitfunc)
    GetPrefs() ((self)->prefs)

methods:
    SetButtons(struct sbutton *buttons);
    GetForeground() returns char *;
    GetBackground() returns char *;
    SetText(struct text *text);
    
data:
    struct text *text;
    struct sbutton *buttons;
    struct sbutton_prefs *prefs;
};

