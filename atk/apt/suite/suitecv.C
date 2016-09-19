/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suitecv.c

VERSION: 0.0

AUTHOR	TC Peters & GW Keim
 	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Suite-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  11/01/88	Created (GW Keim)
  05/04/89	Changed to lower-case naming convention (GW Keim)
  05/23/89	Removed the clearing of the suitecv textview on 
		ReceiveInputFocus (GW Keim)
  05/26/89	Worked on getting the correct caption from the text object
		after a caption string has already been entered once; (GW Keim)
   05/30/89	Finally fixed getting the correct caption from a RW item! (GW Keim)
END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
ATK_IMPL("suitecv.H")
#include <rect.h>
#include <keystate.H>
#include <keymap.H>
#include <bind.H>
#include <proctable.H>
#include <text.H>
#include <suite.H>
#include <suiteev.H>
#include <suitecv.H>

#define EV			    ((self)->parent_EV)
#define ParentItem		    ((self)->parent_item)
#define KeyState		    ((self)->kstate)
#define Debug			    ((self)->debug)
#define	Suite			    ((EV)->parent)
#define	ClientAnchor		    ((Suite)->anchor)


ATKdefineRegistry(suitecv, textview, suitecv::InitializeClass);
void suitecv_InsertNLCmd( class suitecv  *self, long  key );


static class keymap *KeyMap;
static const struct bind_Description Bindings[] = {
    {"suitecv-insert-newline","\015",0,NULL,0,0,(proctable_fptr)suitecv_InsertNLCmd,
	"Insert a newline character","suitecv"},NULL};


boolean 
suitecv::InitializeClass( )
    {
    if(!(KeyMap =  new keymap)) {
	printf("suitecv:Could not create a keymap\n");
	exit(1);
    }
    bind::BindList(Bindings,KeyMap,NULL,&suitecv_ATKregistry_ );
    return(TRUE);
}

suitecv::suitecv( )
        {
	ATKinit;

    this->kstate = keystate::Create(this,KeyMap);
    this->parent_item = NULL;
    this->parent_EV = NULL;
    THROWONFAILURE((TRUE));
}

suitecv::~suitecv( )
    {
	ATKinit;
}

void
suitecv_InsertNLCmd( class suitecv  *self, long  key )
        {
#if 0
    if(ClientAnchor)
	(self)->WantInputFocus(ClientAnchor);
    else 
	(self)->WantInputFocus(Suite);
#else
    (self)->WantInputFocus(Suite);
#endif
}

void 
suitecv::PostKeyState(class keystate  *kstate)
{
    class suitecv *self=this;
    if (kstate == this->keystate) {
	(KeyState)->AddBefore(kstate); 
	(this)->textview::PostKeyState(KeyState);
    }
    else (this)->textview::PostKeyState(kstate);
}

static void
Update_Caption( class suitecv *self )
{
    class text *txt = (class text *) ParentItem->dataobject;
    long len = (txt)->GetLength(), returnedLen;
    char *buf = (txt)->GetBuf( 0, len, &returnedLen );
    if (buf)
	*(buf + returnedLen) = (char) 0;
    if (ParentItem->caption && (buf == NULL || strcmp(buf, ParentItem->caption)) ) {
	(txt)->Clear();
	(txt)->InsertCharacters( 0, ParentItem->caption, strlen(ParentItem->caption) );
    }
}

void
suitecv::ReceiveInputFocus( )
{
    Update_Caption( this );
   (this)->CollapseDot();
}

void
suitecv::FullUpdate( enum view_UpdateType type, long left, long top, long width, long height )
{
  Update_Caption( this );
  textview::FullUpdate(type, left, top, width, height);
}

void
suitecv::LoseInputFocus( )
{
    class suitecv *self=this;
    class text *RWtext = (class text*) (this)->GetDataObject();
    long len = 0;
    struct suite_item *item = ParentItem;
    class suite *suite = EV->parent;
    long i = 0;

    (this)->textview::LoseInputFocus();
    (EV)->ItemNormalize(item);
    if(item->caption) free(item->caption);
    len = (RWtext)->GetLength();
    item->caption = (char*)malloc(len+1);
    while(i < len) {
	item->caption[i] = (RWtext)->GetChar(i);
	i++;
    }
    item->caption[len] = '\0';
    if(item->hithandler) 
	item->hithandler(suite->anchor,suite,item,suite_ItemObject,view_NoMouseEvent, 0,0,0);
    else if(suite->hithandler) 
	suite->hithandler(suite->anchor,suite,item,suite_ItemObject,view_NoMouseEvent, 0,0,0);
}



/*
    $Log: suitecv.C,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1994/08/11  02:49:59  rr2b
// The great gcc-2.6.0 new fiasco, new class foo -> new foo
//
// Revision 1.3  1993/06/03  20:27:00  gk5g
// overrode FullUpdate so that we can update the Read/Write text and only change text if the caption has changed.
//
// Revision 1.2  1993/05/18  15:37:22  rr2b
// Added cast on the functions in the bind list.
// Added class suitecv *self = this in methods which use macros relying on
// self.
// Fixed call on the hit handler to have the right # of arguments.
//
// Revision 1.1  1993/05/05  21:32:07  rr2b
// Initial revision
//
 * Revision 1.19  1992/12/15  21:26:22  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.18  1992/12/14  23:20:33  rr2b
 * add $Logs back after disclaimerization took them out
 *
 * Revision 1.16  1992/07/23  18:02:51  gk5g
 * Many changes:
 * 1) item borders are now drawn via sbutton
 * 2) several attributes have been removed and are not supported (font scaling attributes mainly -- CaptionFontHigh, CaptionFontLow, etc.)
 * 3) New attributes have been added to support color: suite_ForegroundColor, suite_BackgroundColor, suite_ActiveItemForegroundColor, .., suite_PassiveItemForegoundColor)
 * .
 *
 * Revision 1.15  1991/09/12  15:56:43  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.14  1990/06/13  19:02:18  gk5g
 * Added use of fontdesc_StringBoundingBox method contributed by Bill Janssen.
 * Changed the way ItemFullUpdate handles a suite_item that simply has a handle on a child-view. Now just insert it and FullUpdate it.
 *
 * Revision 1.13  89/12/14  15:14:28  cfe
 * Sync with MIT tape (add additional log stuff).
 * 
 * Revision 1.12  89/12/12  14:58:15  ghoti
 * sync with MIT tape
 * 
 * Revision 1.2  89/11/28  15:50:12  xguest
 * Added Gary Keim's diffs.
 * 
 * Revision 1.1  89/11/28  15:42:32  xguest
 * Initial revision
 * 
 * Revision 1.11  89/09/08  09:20:00  ghoti
 * removed unused variable declaration
 * 
 * Revision 1.10  89/08/24  19:47:02  gk5g
 * Changes in support of V1.0 of the SuiteProgGuide.doc.
 * 
 * Revision 1.9  89/08/04  16:19:14  gk5g
 * Fixed a stupid typo.
 * 
 * Revision 1.8  89/08/04  15:41:17  gk5g
 * Fixed the synthesis of a carriage-return when a ReadWrite item has the focus and then it looses it due to a hit outside of its space.
 * 
 * Revision 1.7  89/07/13  16:09:43  gk5g
 * Simply changed all occurances of #include "foo.h" to #include <foo.h>.
 * 
 * Revision 1.6  89/06/09  17:25:23  gk5g
 * Removed suite_Reset() from suitecv_ReceiveInputFocus().
 * Added text_Clear() and text_InsertCharacters() to suitecv_ReceiveInputFocus().
 * 
 * Revision 1.5  89/05/30  18:43:02  gk5g
 * Finally fixed getting the correct caption from a RW item!
 * 
 * Revision 1.4  89/05/26  20:02:42  gk5g
 * Worked on improving the ReadWrite items behavior with
 * 		respect to hits in other items when it has the inputFocus
 * 
 * Revision 1.3  89/05/23  20:28:42  gk5g
 * Removed clearing of the suitecv upon ReceiveInputFocus().
 * 
 * Revision 1.2  89/05/08  16:42:47  gk5g
 * changed references from suiteEV to suiteev
 * 
 * Revision 1.1  89/05/04  12:36:57  gk5g
 * Initial revision
 * 
 * Revision 1.1  89/04/28  20:26:37  tom
 * Initial revision
 * 
*/
