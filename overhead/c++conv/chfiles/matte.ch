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


 

class matte : view {
overrides:
    WantNewSize(struct view *requestor);
    PostMenus(struct menulist *menulist);
    FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
    Update();
    Print(FILE *file, char *processor, char *finalFormat, boolean topLevel);
    Hit (enum view_MouseAction action, long x, long y, long numberOfClicks) returns struct view *;
    DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight) returns enum view_DSattributes;
    GetOrigin(long width, long height, long *originX, long *originY);
    ReceiveInputFocus();
    LoseInputFocus();
    PostMenus(struct menulist *menulist);
    SetDataObject(struct dataobject *dataobject);
    ObservedChanged (struct observable *changed, long value);
    LinkTree(struct view *parent);
    InitChildren();
methods:
    SetResizing(long key);
classprocedures:
    Create(struct viewref *vr,struct view *parent) returns struct matte *;
    InitializeClass() returns boolean;
    InitializeObject(struct matte *self) returns boolean;
    FinalizeObject(struct matte *self);
data:
    int desw,desh;
    struct view *child;
    struct viewref *ref;
    struct cursor *widthcursor, *heightcursor;
    int Moving,resizing,WasMoving, WasResizing;
    struct menulist *menus;
    int drawing, OldMode,sizepending;
};