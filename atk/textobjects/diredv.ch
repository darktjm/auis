/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
*Permission to use, copy, modify, and distribute this software and its 
*documentation for any purpose and without fee is hereby granted, provided 
*that the above copyright notice appear in all copies and that both that 
*copyright notice and this permission notice appear in supporting 
*documentation, and that the name of IBM not be used in advertising or 
*publicity pertaining to distribution of the software without specific, 
*written prior permission. 
*                        
*THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
*TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
*HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
*DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
*DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
*OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
*WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $
*/

/*
 * Return status values for StartNewDir
 */

#define diredview_Done         0
#define diredview_NoSuchDir   -1
#define diredview_NotADir     -2

class diredview[diredv]: textview[textv] {
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct diredview *self) returns boolean;
    FinalizeObject(struct diredview *self);
overrides:
    SetDataObject(struct basicobject *object);
    Hit(enum view_MouseAction action, long x, long y,
      long numberOfClicks) returns struct view *;
    PostKeyState(struct keystate *keystate);
    PostMenus(struct menulist *menus);
    ObservedChanged(struct observable *changed, long value);
methods:
data:
    struct keystate *keystate;
    struct menulist *menulist;
};
