/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * eqv.c
 * This module handles the view for eq.
 */

#include <andrewos.h>
ATK_IMPL("eqview.H")
#include <eqview.H>

#include <eq.H>
#include <rectlist.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <graphic.H>
#include <view.H>
#include <mark.H>
#include <print.H>
#include <im.H>

#include "eqview.h"


static int fudge = 5;		/* extra space: for visual balance and for cursor */
static class keymap *eqviewKeymap;
static class menulist *eqviewMenus, *eqviewCutMenus;
static class graphic *pat;

static int debug_flag = 0;
#define debug(f) if (debug_flag) { printf f; fflush(stdout); }


ATKdefineRegistry(eqview, view, eqview::InitializeClass);

eqview::eqview()
{
	ATKinit;

    debug(("Init here\n"));

    this->off_x = this->off_y = 0;
    this->changed = EQVIEW_nothing;
    this->caret_x = this->caret_y = 0;
    this->selection_width = this->selection_height = -1;
    this->hasinputfocus = FALSE;
    this->embedded = TRUE;
    this->keystate = keystate::Create(this, eqviewKeymap);
    this->normalMenus = (eqviewMenus)->DuplicateML( this);
    this->cutMenus = (eqviewCutMenus)->DuplicateML( this);
    this->dot = NULL;
    this->filename = NULL;

    THROWONFAILURE( TRUE);
}

eqview::~eqview()
{
	ATKinit;

    if (this->keystate)
	delete this->keystate;
    if (this->normalMenus)
	delete this->normalMenus;
    if (this->cutMenus)
	delete this->cutMenus;
    if (this->filename)
	free(this->filename);
    if(GetDataObject()) {
	eq *e=(eq *)GetDataObject();
	e->RemoveMark(this->dot);
    }
    delete this->dot;
}

void eqview::SetDataObject(class dataobject  *dataObject)
{
    if (!ATK::IsTypeByName((dataObject)->GetTypeName(), "eq"))  {
	fprintf(stderr, "Incompatible dataobject associated with eqview\n");
	return;
    }
    (this)->view::SetDataObject( dataObject);
    this->dot = ((class eq *) dataObject)->CreateMark( 0, 0);
    (this->dot)->SetStyle( FALSE, FALSE);
    (this)->SetDotPosition( 4);
}

void eqview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    debug(("FullUpdate here\n"));

    if(type == view_MoveNoRedraw){
	this->changed = EQVIEW_caret;
    } 
    else {
	this->changed = EQVIEW_everything;
    }
    (this)->Update();
}


void eqview::Update()
{
    class eq *eqptr = Eq(this);
    struct formula *first = (eqptr)->Access( 0);
    struct rectangle rect;

    debug(("Update here\n"));

    /* undraw */
    switch(this->changed) {
    case EQVIEW_everything:
	(this)->SetTransferMode( graphic_COPY);
	rectangle_SetRectSize(&rect, (this)->GetLogicalLeft(), (this)->GetLogicalTop(), (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
	pat = (this)->WhitePattern();
	(this)->FillRect( &rect, pat);
	break;
    case EQVIEW_eq:
	(this)->DrawCaret();	/* sigh - caret extends outside following box */
	(this)->SetTransferMode( graphic_COPY);
	rectangle_SetRectSize(&rect, this->off_x+first->posp.x+first->min.x-3,
	    this->off_y+first->posp.y+first->min.y-1,
	    first->max.x-first->min.x+4,  first->max.y-first->min.y+2);
	pat = (this)->WhitePattern();
	(this)->FillRect( &rect, pat);
	break;
    case EQVIEW_caret:
	(this)->DrawCaret();
	break;
    case EQVIEW_nothing:
	break;
    }
	
    /* draw */
    switch (this->changed) {
    case EQVIEW_everything:
    case EQVIEW_eq:
	eqview_Format(this, eqptr, NIL, (eqptr)->Access( 0), NIL, T_EQSTYLE);
	if (this->changed == EQVIEW_eq && first->max.y-first->min.y+2*fudge+1 > (this)->GetLogicalHeight()) {
	    (this)->WantNewSize( this);
	    this->changed = EQVIEW_everything;
	    return;
	}
	this->off_x = (this)->GetLogicalLeft() + fudge;
	this->off_y = (this)->GetLogicalTop() + fudge+(first->max.y-first->min.y)/2;
	first->hot.x =  this->off_x;
	first->hot.y = this->off_y;

	(this)->SetTransferMode( graphic_INVERT);
	(this)->Draw( eqptr, first, this->off_x, this->off_y);
	(this)->CalculateCaret();
	(this)->DrawCaret();
	break;
    case EQVIEW_caret:
	(this)->CalculateCaret();
	(this)->DrawCaret();
	break;
    case EQVIEW_nothing:
	break;
    }
    if (this->hasinputfocus) {
	if ((this->dot)->GetLength() > 0) {
	    (this->cutMenus)->ClearChain();
	    (this)->PostMenus( this->cutMenus);
	}
	else if ((this->dot)->GetLength() == 0) {
	    (this->normalMenus)->ClearChain();
	    (this)->PostMenus( this->normalMenus);
	}
    }
    this->changed = EQVIEW_nothing;
}

void eqview::SetDotPosition(long  newp)
{
    long len = (Eq(this))->Size();

    if (newp < 0)
	newp = 0;
    else {
	if (newp > len)
	    newp = len;
    }
    (this->dot)->SetPos( newp);
    (this)->Changed( EQVIEW_caret);
    (this)->WantUpdate( this);
}

void eqview::SetDotLength(long  newl)
{
    if (newl < 0)
	newl = 0;
    (this->dot)->SetLength( newl);
    (this)->Changed( EQVIEW_caret);
    (this)->WantUpdate( this);
}

long eqview::GetDotPosition()
{
    return (this->dot)->GetPos();
}

long eqview::GetDotLength()
{
    return (this->dot)->GetLength();
}

class view *eqview::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    int i, pos, len;
    class eq *eqptr = Eq(this);

    debug(("Hit here\n"));

    if (action == view_LeftDown) {
	i = (this)->Find( eqptr, x, y, 0);
	(this)->SetDotPosition( i);
	(this)->SetDotLength( 0);
    }
    else if (action == view_LeftUp || action == view_RightDown) {
	pos = (this)->GetDotPosition();
	len = (this)->GetDotLength();
	i = (this)->Find( eqptr, x, y, pos);
	if (i < pos+len/2)
	    len = pos + len - i,  pos = i;
	else
	    len = i - pos;
	(this)->SetDotPosition( pos);
	(this)->SetDotLength( len);
    }

    (this)->Changed( EQVIEW_caret);
    (this)->WantUpdate( this);
    (this)->WantInputFocus( this);

    return this;
}

void eqview::ReceiveInputFocus()
{
    debug(("ReceiveInputFocus here\n"));

    this->hasinputfocus = TRUE;
    (this)->Changed( EQVIEW_caret);
    this->keystate->next = NULL;
    (this)->PostKeyState( this->keystate);
    (this)->WantUpdate( this);
}


void eqview::LoseInputFocus()
{
    debug(("LoseInputFocus here\n"));

    this->hasinputfocus = FALSE;
    (this)->Changed( EQVIEW_caret);
    (this)->WantUpdate( this);
}


view_DSattributes eqview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *widthp , long  *heightp)
{
    struct formula *first;
    class eq *eqptr = Eq(this);

    if (eqptr == NULL)
	eqptr = new eq;

    first = (eqptr)->Access( 0);
    eqview_Format(this, eqptr, NIL, first, NIL, T_EQSTYLE);

    debug(("DesiredSize here\n"));

    *widthp = width;
    *heightp = first->max.y - first->min.y + 1 + 2*fudge;

    return(view_Fixed);
}

boolean eqview::InitializeClass()
{
    eqviewKeymap = eqview_InitKeyMap(&eqview_ATKregistry_ , &eqviewMenus, &eqviewCutMenus);

    return TRUE;
}

void eqview::Print(FILE  *file, const char  *process, const char  *final, int  toplevel)
{
    class eq *eqptr = Eq(this);

    if (strcmp(process, "troff") == 0) {
	(eqptr)->Parse( file, 'e');
	fprintf(file,".EQ\ndelim off\n.EN\n");
    }    
}

void *eqview::GetPSPrintInterface(const char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;
    return NULL;
}

void eqview::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    class eq *eqptr = Eq(this);
    (eqptr)->Parse(NULL, 'p', desiredwidth, desiredheight);
}

void eqview::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    class eq *eqptr = Eq(this);
    int val;
    char buf[64];

    val = print::PSRegisterFont("Times-Roman");
    sprintf(buf, "{%s%d %d scalefont setfont} bind", print_PSFontNameID, val, 12); 
    print::PSRegisterDef("eqFr", buf);
    val = print::PSRegisterFont("Times-Italic");
    sprintf(buf, "{%s%d %d scalefont setfont} bind", print_PSFontNameID, val, 12); 
    print::PSRegisterDef("eqFi", buf);
    val = print::PSRegisterFont("Symbol");
    sprintf(buf, "{%s%d %d scalefont setfont} bind", print_PSFontNameID, val, 12); 
    print::PSRegisterDef("eqFs", buf);

    eqptr->Parse(outfile, 'p');
}

#if 0
/* this wasn used before... won't be now either -rr2b */
class view *eqview::GetApplicationLayer()
{
    this->embedded = FALSE;
    (this)->WantInputFocus( this);

    return (class view *) this;
}
#endif
