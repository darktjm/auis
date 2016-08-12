/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/apt/suite/RCS/suiteev.C,v 1.11 1996/03/18 23:10:42 robr Stab74 $";
#endif


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suiteev.c

VERSION	0.0

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


END-SPECIFICATION  ************************************************************/



#include <andrewos.h>
ATK_IMPL("suiteev.H")
#include <ctype.h>
#include <graphic.H>
#include <dataobject.H>
#include <view.H>
#include <cursor.H>
#include <fontdesc.H>
#include <proctable.H>
#include <menulist.H>
#include <scroll.H>
#include <text.H>
#include <textview.H>
#include <im.H>
#include <region.H>
#include <sbuttonv.H>
#include <sbutton.H>
#include <vector.H>
#include <apt.H>
#include <suite.H>
#include <suitecv.H>
#include <suiteev.H>

#define Suite			    (self->parent)
#define	CurrentItem		    (Suite->current_item)
#define Rows			    (Suite->rows)
#define Columns			    (Suite->columns)
#define ItemHeight		    (Suite->itemheight)
#define ItemWidth		    (Suite->itemwidth)
#define ItemFixedHeight		    (Suite->itemfixedheight)
#define ItemFixedWidth		    (Suite->itemfixedwidth)
#define Items			    (Suite->items)
#define ItemArray		    (Suite->itemarray)
#define Scroll			    (Suite->scroll)    
#define ScrollType		    (Suite->scrolltype)
#define ScrollTOP		    (ScrollType & scroll_TOP)
#define ScrollBOTTOM		    (ScrollType & scroll_BOTTOM)
#define ScrollLEFT		    (ScrollType & scroll_LEFT)
#define ScrollRIGHT		    (ScrollType & scroll_RIGHT)
#define HScroll			    (ScrollTOP || ScrollBOTTOM)
#define VScroll			    (ScrollLEFT || ScrollRIGHT)
#define ItemViewObjectName	    (Suite->itemviewobjectname)
#define ItemOrder		    (Suite->itemorder)
#define RowMajor		    (ItemOrder & suite_RowMajor)
#define ColumnMajor		    (ItemOrder & suite_ColumnMajor)
#define Arrangement		    (Suite->arrangement)
#define List			    (Arrangement & suite_List)
#define	WrapStyle		    (Suite->wrappingstyle)
#define Unbalanced		    (Arrangement & suite_Unbalanced)
#define Balanced		    (Arrangement & suite_Balanced)
#define SingleColumn		    (Arrangement & suite_Column)
#define SingleRow		    (Arrangement & suite_Row)
#define Matrix			    (Arrangement & suite_Matrix)
#define RowLine			    (Arrangement & suite_RowLine)
#define ColumnLine		    (Arrangement & suite_ColumnLine)
#define	Fixed			    (Arrangement & suite_Fixed)
#define SelectionMode		    (Suite->selection_mode)
#define CaptionFont		    (Suite->captionfont)
#define	TitleFont		    (Suite->titlefont)
#define	TitlePlacement		    (Suite->titleplacement)
#define CaptionFontSize		    (Suite->captionfontsize)
#define ItemBorderStyle		    (Suite->itemborderstyle)
#define ItemBorderSize		    (Suite->itembordersize)
#define ItemHighlightStyle	    (Suite->itemhighlightstyle)
#define ItemPassiveStyle	    (Suite->itempassivestyle)
#define HitHandler		    (Suite->hithandler)
#define ClientAnchor		    (Suite->anchor)
#define FirstVisible		    (Suite->firstvisible)
#define	FirstVisibleSubString	    (self->firstvisiblesubstring)
#define NewFirstVisible		    (Suite->newfirstvisible)
#define LastVisible		    (Suite->lastvisible)
#define VisibleRows		    (Suite->visiblerows)
#define VisibleColumns		    (Suite->visiblecolumns)
#define NumVisible		    (Suite->numvisible)
#define Container		    (Suite->container)
#define ContainerLeft		    (Container.left)
#define ContainerTop		    (Container.top)
#define ContainerWidth		    (Container.width)
#define ContainerHeight		    (Container.height)
#define	ContainerRight		    (ContainerLeft + ContainerWidth)
#define	ContainerBottom		    (ContainerTop + ContainerHeight)
#define ExceptionHandler	    (Suite->exception)
#define ExceptionStatus		    (Suite->exception_status)    
#define	SuiteBackground		    (&Suite->suiteColor.bg)
#define	SuiteForeground		    (&Suite->suiteColor.fg)
#define	SuiteBackgroundName	    (((char *)Suite->suiteColor.bg).Name())
#define	SuiteForegroundName	    (((char *)Suite->suiteColor.fg).Name())
#define	ActiveItemForeground	    (&Suite->activeItemColor.fg)
#define	ActiveItemBackground 	    (&Suite->activeItemColor.bg)
#define	ActiveItemForegroundName    (((char *)Suite->activeItemColor.fg).Name())
#define	ActiveItemBackgroundName    (((char *)Suite->activeItemColor.bg).Name())
#define	PassiveItemForeground 	    (&Suite->passiveItemColor.fg)
#define	PassiveItemBackground 	    (&Suite->passiveItemColor.bg)
#define	PassiveItemForegroundName   (((char *)Suite->passiveItemColor.fg).Name())
#define	PassiveItemBackgroundName   (((char *)Suite->passiveItemColor.bg).Name())
#define XGutterSize		    (Suite->x_guttersize)
#define YGutterSize		    (Suite->y_guttersize)
#define ItemCursor		    (Suite->itemcursor)
#define ItemCursorFont		    (Suite->itemcursorfont)
#define ItemCursorByte		    (Suite->itemcursorbyte)
#define CVIF			    (self->cvif)
#define Debug			    (self->debug)
#define FirstHit		    (self->firsthit)
#define LastHit			    (self->lasthit)
#define HasFocus		    (Suite->has_focus)
#define TitleMWidth		    (Suite->title_m_width)
#define CaptionMWidth		    (Suite->caption_m_width)
#define ITEM(i)			    (struct suite_item*) (Items)->Item(i)
#define Active(item)		    (item->mode & item_Active)
#define Exposed(item)		    (Suite)->ItemExposed(item)
#define Normalized(item)	    (item->mode & item_Normalized)
#define Highlighted(item)	    (item->mode & item_Highlighted)
#ifndef MAX
#define MAX(a,b)		    (((a)>=(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b)		    (((a)<=(b))?(a):(b))
#endif
#define Bounds			    (item->bounds)
#define Left			    (Bounds.left)
#define Top			    (Bounds.top)
#define Width			    (Bounds.width)
#define Height			    (Bounds.height)
#define	Right			    (Left + Width)
#define	Bottom			    (Top + Height)
#define	FarthestRightItemBorder	    (ContainerRight-XGutterSize)
#define	FarthestBottomItemBorder    (ContainerBottom-YGutterSize)
#define CaptionRect		    (item->caption_rect)
#define TitleRect		    (item->title_rect)
#define InsetRect		    (item->inset_rect)
#define InsetLeft		    (InsetRect.left)
#define InsetTop		    (InsetRect.top)
#define InsetWidth		    (InsetRect.width)
#define InsetHeight		    (InsetRect.height)
#define	myInsetRect(r,deltax,deltay) rectangle_SetRectSize((r), rectangle_Left((r)) + (deltax), rectangle_Top((r)) + (deltay), rectangle_Width((r)) - 2*(deltax), rectangle_Height((r)) - 2*(deltay));
#define	DecrementRect(r,n) myInsetRect(r,n,n)
#define SETTRANSFERMODE(v,m) if((m) != (((class view*)(v)))->GetTransferMode()) (((class view*)(v)))->SetTransferMode((m));
#define SetFont(v,f) if((f) != (((class view*)(v)))->GetFont())\
			(((class view*)(v)))->SetFont((f));
#define graphicIsMono			(Suite->mono)

static int suiteev_debug = 0;

   
 


#define LIT TRUE

ATKdefineRegistry(suiteev, view, suiteev::InitializeClass);
#ifndef NORCSID
#endif
#ifndef MAX
#endif
#ifndef MIN
#endif
static void AllocNameSpace(register char  **target , register char  *source);
static void CheckForNewFirstVisible( class suiteev  *self );
static long Within(register long  x , register long  y , register long  left , register long  top , register long  width , register long  height);
static long WithinRect(register long  x , register long  y, register struct rectangle  *r);
static long CopySelected(register class suiteev  *self, register class suite  *suite, register struct suite_item  *item, register long  datum);
static void Copy(register class suiteev  *self);
static struct suite_item * NthAfter(register class suiteev  *self, register struct suite_item  *start, register long  numToSkip);
static struct suite_item * NthPrior(register class suiteev  *self, register struct suite_item  *start, register long  numToSkip);
static struct suite_item * NPixelsAfter(register class suiteev  *self, register struct suite_item  *start, register long  pix , register long  *numToSkip);
static struct suite_item * NPixelsPrior(register class suiteev  *self, register struct suite_item  *start, register long  pix , register long  *numToSkip);
static void DrawGutterLines(register class suiteev  *self);
static long ResetItemBreaks(register class suite  *self, register class suite  *suite, register struct suite_item  *item, register long  datum);
static void getinfo(register class suiteev  *self, register struct range  *total , register struct range  *seen , register struct range  *dot);
static void endzone(register class suiteev  *self, register int  zone, register enum view_MouseAction  action);
static long ywhatis(register class suiteev  *self, register long  num , register long  denom);
static long xwhatis(register class suiteev  *self, register long  num , register long  denom);
static void ysetframe(register class suiteev  *self, register long  posn , register long  coord , register long  outof);
static void xsetframe(register class suiteev  *self, register long  posn , register long  coord , register long  outof);
static void AttemptSymmetry(register class suiteev  *self, register long  numItems , register long  *rows , register long  *columns);
static void SetBreakPoint(register class suiteev  *self, register struct suite_item  *item, register char  *end);
static boolean DoesItFit(class suiteev  *self, struct suite_item  *item, char  *head, char  *tail, long  width);
static char * WalkBackwardToPunctuation(register char  *head, register char  *tail);
static char * WalkBackwardBlackSpace( register class suiteev	 *self, register struct suite_item	 *item, register char			 *head, register char			 *tail, register long			  width );
static void PlaceItems( register class suiteev	     *self, struct rectangle		     *rect, long				      rows , long				      cols , long				      numleftOvers, long				      itemWidth , long				      itemHeight );
static void DetermineVisibleListItems( class suiteev  *self, long  height );
static void EraseItems( register class suiteev  *self );
void suiteev_HandleExclusiveHit( register class suiteev  *self, register struct suite_item  *item, enum view_MouseAction  action, long  x , long  y, long  numberOfClicks );
void suiteev_HighlightFirstToLast( register class suiteev  *self, register struct suite_item  *first, register struct suite_item  *last );
void suiteev_HandleInclusiveHit( register class suiteev  *self, register struct suite_item  *item, register enum view_MouseAction  action, register long  x , register long  y , register long  numberOfClicks );
void suiteev_HandleToggleHit( register class suiteev  *self, register struct suite_item  *item, register enum view_MouseAction  action, register long  x , register long  y , register long  numberOfClicks );
static void ItemFullUpdate( register class suiteev  *self, register struct suite_item  *item, register enum	view_UpdateType  type, register long  left , register long  top , register long  width , register long  height );
static void ItemPlaceCaption( register class suiteev  *self, register struct suite_item  *item, long  captionwidth, long  captionheight, unsigned  *place );
void ItemPlaceTitle(class suiteev  *self, struct suite_item  *item, long  titlewidth , long  titleheight, int  newlineHeight);
static void ReadWriteHandler( register long  anchor, register class suite  *suite, register struct suite_item  *item );
static void MaxSubStringSize( class suiteev  *self, struct suite_item  *item, char  *str, class fontdesc  *font,  int *w ,  int *h );
static long MaxListSubStringWidth( class suiteev  *self, struct suite_item  *item, char  *str, class fontdesc  *font );
static void  DrawRectSize(class suiteev	 *self, long  x , long  y , long  width , long  height);
static void  DrawRect(class suiteev  *self, struct suite_item  *item, struct rectangle  *Rect, boolean  lit);

static struct scrollfns horizInterface = {(scroll_getinfofptr) getinfo, (scroll_setframefptr)xsetframe, (scroll_endzonefptr)endzone, (scroll_whatfptr)xwhatis };
static struct scrollfns vertInterface =  {(scroll_getinfofptr)getinfo, (scroll_setframefptr)ysetframe, (scroll_endzonefptr)endzone, (scroll_whatfptr)ywhatis };

 


static class menulist *smenulist = NULL;




static void
AllocNameSpace(register char  **target , register char  *source)
  {
  if(target && *target) {
    free(*target);
    *target = NULL;
  }
  if(source && *source) {
    *target = (char*) malloc(strlen(source)+1);
    if(*target) strcpy(*target,source);
  }
  else *target = NULL;
}

static void
CheckForNewFirstVisible( class suiteev  *self )
    {
    if(NewFirstVisible) {
	FirstVisible = NewFirstVisible;
	NewFirstVisible = NULL;
    }
    else if( !FirstVisible )
	FirstVisible = ITEM(0);
}

static long
Within(register long  x , register long  y , register long  left , register long  top , register long  width , register long  height)
  {
  return((x >= left) && (x <= left + width) && (y >= top) && (y <= top + height));
}

static long
WithinRect(register long  x , register long  y, register struct rectangle  *r)
    {
  return(Within(x, y, r->left, r->top, r->width, r->height));
}

static long
CopySelected(register class suiteev  *self, register class suite  *suite, register struct suite_item  *item, register long  datum)
        {
  register long int status = 0;
  static char buffer[1025];

  IN(CopySelected);
  if(suite && item)
    if(Exposed(item) && Active(item) && Highlighted(item)) {
      sprintf(buffer,"%s ",(char*)(suite)->ItemAttribute(item, suite_itemcaption));
      fputs(buffer,(FILE*)datum);
    }
  OUT(CopySelected);
  return(status);
}

static void
Copy(register class suiteev  *self)
  {
  register FILE *CutFile = NULL;
  register class im *im = NULL;

  IN(Copy);
  if(im = (self)->GetIM()) {
    CutFile = (im)->ToCutBuffer();
    (Suite)->Apply((suite_applyfptr)CopySelected,(long)Suite,(long)CutFile);
    (im)->CloseToCutBuffer(CutFile);
  }
  OUT(Copy);
}

boolean
suiteev::InitializeClass()
  {
  struct proctable_Entry *tempProc = NULL;

  IN(suiteev_InitializeClass);
  ::smenulist = new menulist;
  tempProc = proctable::DefineProc( "suiteev-copy", (proctable_fptr)Copy, 
	&suiteev_ATKregistry_ , NULL, "copy selected region to cut buffer" );
  (::smenulist)->AddToML(  "Copy~10", tempProc, 0, 0 );
  OUT(suiteev_InitializeClass);
  return(TRUE);
}

static char suiteItemPrefs[] = "suiteItemPrefs";

suiteev::suiteev()
{
    class suiteev *self=this;
	ATKinit;

  this->parent = NULL;
  this->cvif = NULL;
  this->firsthit = this->lasthit = NULL;
  this->menulistp = (::smenulist)->DuplicateML(this);
  FirstVisibleSubString = 0;
  suiteev_debug = Debug = 1;
  this->itemPrefs = sbutton::GetNewPrefs(suiteItemPrefs);
  sbutton::InitPrefs(this->itemPrefs, suiteItemPrefs);
  THROWONFAILURE((TRUE));
}

suiteev::~suiteev()
{
    class suiteev *self=this;
	ATKinit;

  if(this->menulistp) 
      delete this->menulistp;
}

void
suiteev::PostMenus(class menulist  *menulistp)
{
    class suiteev *self=this;
  IN(suiteev_PostMenus);
  if(List) {
    (this->menulistp)->ClearChain();
    if(menulistp) 
      (this->menulistp)->ChainBeforeML(menulistp,0);
    (this)->view::PostMenus(this->menulistp);
  }
  else (this)->view::PostMenus(menulistp);
  OUT(suiteev_PostMenus);
}

static struct suite_item *
NthAfter(register class suiteev  *self, register struct suite_item  *start, register long  numToSkip)
      {
  register int i = 0;
  register struct suite_item *item = start;
  register long count = 0, size = 0;

  IN(NthAfter);
  if(Items && ITEM(0) && (i = (Items)->Subscript((long)start)) != -1) {
    size = (Items)->Count();
    while((count < numToSkip) && (i < size) && (item = ITEM(++i)))
      if((Suite)->ItemExposed(item)) count++;
  }
  OUT(NthAfter);
  return(item);
}

static struct suite_item *
NthPrior(register class suiteev  *self, register struct suite_item  *start, register long  numToSkip)
      {
  register int i = 0;
  register struct suite_item *item = start;
  register long count = 0;

  IN(NthAfter);
  if(Items && ITEM(0) && (i = (Items)->Subscript((long)start)) != -1)
    while((count < numToSkip) && (i > 0) && (item = ITEM(--i)))
      if((Suite)->ItemExposed(item)) count++;
  OUT(NthPrior);
  return(item);
}

static struct suite_item *
NPixelsAfter(register class suiteev  *self, register struct suite_item  *start, register long  pix , register long  *numToSkip)
      {
  register int i = 0;
  register struct suite_item *item = start;
  register long count = 0, size = 0, tmp = 0;

  IN(NthAfter);
  if(Items && ITEM(0) && (i = (Items)->Subscript((long)start)) != -1) {
    size = (Items)->Count();
    if(ColumnMajor) tmp = YGutterSize;
    else tmp = XGutterSize;
    while((tmp < pix) && (i < size) && (item = ITEM(++i)))
      if((Suite)->ItemExposed(item)) {
	if(ColumnMajor) tmp += (Height + YGutterSize);
	else tmp += (Width + XGutterSize);
	count++;
      }
  }
  *numToSkip = count;
  OUT(NPixelsAfter);
  return(item);
}

static struct suite_item *
NPixelsPrior(register class suiteev  *self, register struct suite_item  *start, register long  pix , register long  *numToSkip)
      {
  register int i = 0;
  register struct suite_item *item = start;
  register long count = 0, tmp = 0;

  IN(NthAfter);
  if(Items && ITEM(0) && (i = (Items)->Subscript((long)start)) != -1) {
    if(ColumnMajor) tmp = YGutterSize;
    else tmp = XGutterSize;
    while((tmp < pix) && (i > 0) && (item = ITEM(--i)))
	if((Suite)->ItemExposed(item)) {
	    if(ColumnMajor || List) {
		if (Height == 0 && LastVisible)
		    tmp += (LastVisible->bounds.height + YGutterSize);
		else
		    tmp += (Height + YGutterSize);
	    } else {
		if (Width == 0 && LastVisible)
		    tmp += (LastVisible->bounds.width + XGutterSize);
		else
		    tmp += (Width + XGutterSize);
	    }
	    count++;
	}
  }
  *numToSkip = count;
  OUT(NPixelsPrior);
  return(item);
}

static void
DrawGutterLines(register class suiteev  *self)
  {
  register struct suite_item *item = FirstVisible;
  register long i;
  long numToDo, numToSkip, last, first;
  long Offset, clearLeft, clearTop;

  IN(DrawGutterLines);
  if(Items && ITEM(0) && FirstVisible ) {
      if(RowLine || ColumnLine) {
	  last =  (Items)->Subscript((long) LastVisible);
	  first = (Items)->Subscript((long) FirstVisible);
	  if(RowLine) {
	      if(RowMajor)
		  numToSkip = VisibleColumns;
	      else if(ColumnMajor)
		  numToSkip = 1;
	      numToDo = (last - first)/VisibleColumns;
	      Offset = YGutterSize/2;
	      for(i = 0 ;i < numToDo && item ; i++) {
		  (self)->MoveTo( ContainerLeft, Top + Height + Offset);
		  (self)->DrawLineTo( ContainerLeft + ContainerWidth, Top + Height + Offset);
		  item = NthAfter(self, item, numToSkip);
	      }
	      clearTop = LastVisible->bounds.top + LastVisible->bounds.height;
	  }
	  if(ColumnLine) {
	      if(RowMajor)
		  numToSkip = 1;
	      else if(ColumnMajor)
		  numToSkip = VisibleRows;
	      item = FirstVisible;
	      numToDo = (last - first)/VisibleRows;
	      Offset = XGutterSize/2;
	      for(i = 0 ; i < numToDo && item ; i++) {
		  (self)->MoveTo( Left + Width + Offset, Top);
		  (self)->DrawLineTo( Left + Width + Offset, Top + ContainerHeight);
		  item = NthAfter(self, item, numToSkip);
	      }
	      clearLeft = LastVisible->bounds.left + LastVisible->bounds.width; 
	  }
#if 0
	  (self)->SetBGColorCell( SuiteBackground);
	  if(ColumnMajor) {
	      (self)->FillRectSize( 
				   clearLeft = LastVisible->bounds.left, 
				   clearTop = (LastVisible->bounds.top + LastVisible->bounds.height), 
				   ContainerWidth - clearLeft, 
				   ContainerHeight - clearTop, NULL);
	      (self)->FillRectSize( 
				   clearLeft = LastVisible->bounds.left + LastVisible->bounds.width, 
				   0, 
				   ContainerWidth - clearLeft, 
				   ContainerHeight, NULL);
	  }
	  else {	/* RowMajor */
	      (self)->FillRectSize( 
				   clearLeft = LastVisible->bounds.left + LastVisible->bounds.width, 
				   clearTop = LastVisible->bounds.top, 
				   ContainerWidth - clearLeft, 
				   ContainerHeight - clearTop, NULL);
	      (self)->FillRectSize( 
				   0, 
				   clearTop = (LastVisible->bounds.top + LastVisible->bounds.height), 
				   ContainerWidth, 
				   ContainerHeight - clearTop, NULL);
	  }
#endif
      }
  }
  SETTRANSFERMODE(self, graphic_COPY);
  OUT(DrawGutterLines);
}

static long
ResetItemBreaks(register class suite  *self, register class suite  *suite, register struct suite_item  *item, register long  datum)
        {
  register int status = 0, i = 0;

  if(item) {
    for(i = 0; i < BreakCount(item); i++)
      (Breaks(item))->Item( i) = 0;
    BreakCount(item) = 0;
  }
  return(status);
}

void
suiteev::FullUpdate(register enum view_UpdateType  type, register long  left , register long  top , register long  width , register long  height)
{
    class suiteev *self=this;
  struct rectangle r;
  
    IN(suiteev_FullUpdate);
    if((type == view_FullRedraw) || (type == view_LastPartialRedraw)) {
	(this)->GetVisualBounds( &r);
	if(List) 
	    (Suite)->Apply((suite_applyfptr)ResetItemBreaks, (long)Suite, 0);
	(this)->Arrange( &r);
	(this)->DrawItems( &r);
    }
    DrawGutterLines(this);
    OUT(suiteev_FullUpdate);
}

char *
suiteev::GetInterface(register char  *type)
{
    class suiteev *self=this;
    IN(suiteev_GetInterface);
    if(!strcmp(type, "scroll,vertical"))
	return((char *) &vertInterface);
    else if(!strcmp(type, "scroll,horizontal"))
	return((char *) &horizInterface);
    else
	return(NULL);
}

static void
getinfo(register class suiteev  *self, register struct range  *total , register struct range  *seen , register struct range  *dot)
    {
    IN(getinfo);
    total->beg = 0;
    total->end = (self)->NumberExposed() - 1;
    if(Items && ITEM(0) && FirstVisible) 
	seen->beg = (Items)->Subscript((long)FirstVisible);
    else
	seen->beg = 0;
    if(Items && ITEM(0) && LastVisible) 
	seen->end = (Items)->Subscript((long)LastVisible);
    else
	seen->end = total->end;
    dot->beg = dot->end = seen->beg;
}

static void
endzone(register class suiteev  *self, register int  zone, register enum view_MouseAction  action)
      {
  register int numVisible = 0, EndOffset = 0;
  register struct suite_item *LastItem = NULL;

    IN(endzone);
    if(Items && ITEM(0)) {
	if(action != view_LeftDown && action != view_RightDown)
	    return;
	if(action == view_LeftDown) {
	    if(zone == scroll_BOTTOMENDZONE) {
		numVisible = (self)->NumberExposed();
		if((LastItem = ITEM((Items)->Count() - 1)) != LastVisible) {
		    if(Matrix)
			if(numVisible > (VisibleRows * VisibleColumns)) {
			    EndOffset = (VisibleRows * VisibleColumns) - 1;
			    NewFirstVisible = NthPrior(self, LastItem, EndOffset);
			}
			else NewFirstVisible = ITEM(0);
		    else if(SingleRow)
			if(numVisible > VisibleColumns) {
			    EndOffset = VisibleColumns - 1;
			    NewFirstVisible = NthPrior(self, LastItem, EndOffset);
			}
			else NewFirstVisible = ITEM(0);
		    else if(SingleColumn)
			if(numVisible > VisibleRows) {
			    EndOffset = VisibleRows - 1;
			    NewFirstVisible = NthPrior(self, LastItem, EndOffset);
			}
			else NewFirstVisible = ITEM(0);
		}
	    }
	    else if(zone == scroll_TOPENDZONE) /* obviously */
		if(Items && ITEM(0))
		    NewFirstVisible = ITEM(0);
	}
	else if(action == view_RightDown) {
	    if((numVisible = (self)->NumberExposed()) > 1) {
		if(zone == scroll_BOTTOMENDZONE)
		    NewFirstVisible = NthAfter(self, FirstVisible, 1);
		else
		    NewFirstVisible = NthPrior(self, FirstVisible, 1);
	    }
	}
	if(NewFirstVisible && (NewFirstVisible != FirstVisible))
	    (self)->WantUpdate( self);
    }
    OUT(endzone);
}

static long
ywhatis(register class suiteev  *self, register long  num , register long  denom)
    {
  return((self)->Locate(  0, (num * (self)->GetLogicalHeight()) / denom) );
}

static long
xwhatis(register class suiteev  *self, register long  num , register long  denom)
    {
    return((self)->Locate( (num * (self)->GetLogicalWidth()) / denom, 0));
}

static void
ysetframe(register class suiteev  *self, register long  posn , register long  coord , register long  outof)
    {
  register long vertOffset = 0, height = ItemHeight;
  long numToSkip = 0;

    IN(ysetframe);
    if(coord) { /* Right Click */
	if(RowLine)
	    height = ItemHeight + 3;
	vertOffset = (self)->GetLogicalHeight() * coord / outof;
	if(List) {
	    NPixelsPrior(self, FirstVisible, vertOffset, &numToSkip);
	    if(RowMajor)
		numToSkip *= VisibleColumns;
	}
	else {
	    if(ColumnMajor)
		numToSkip = vertOffset/height;
	    else if(RowMajor)
		numToSkip = ((vertOffset/height) * VisibleColumns);
	}
	NewFirstVisible = NthPrior(self, FirstVisible, numToSkip);
	if(!NewFirstVisible)
	    NewFirstVisible = ITEM(0);
    }
    else /* Left Click */ 
	NewFirstVisible = NthAfter(self, ITEM(0), posn);
    if(NewFirstVisible != FirstVisible)
	(self)->WantUpdate( self);
}

static void
xsetframe(register class suiteev  *self, register long  posn , register long  coord , register long  outof)
    {
  register long width = ItemWidth, horizOffset = 0, numToSkip = 0;

  IN(xsetframe);
  if(coord) { /* Right Click */
    if(ColumnLine)
	width += 3;
      horizOffset = (self)->GetLogicalWidth() * coord / outof;
      if(ColumnMajor)
	  numToSkip = ((horizOffset/width) * VisibleRows);
      else if(RowMajor)
	  numToSkip = horizOffset/width;
      NewFirstVisible = NthPrior(self, FirstVisible, numToSkip);
      if(!NewFirstVisible)
	  NewFirstVisible = ITEM(0);
  }
  else /* Left Click */ 
      NewFirstVisible = NthAfter(self, ITEM(0), posn);
  if(NewFirstVisible != FirstVisible)
      (self)->WantUpdate( self);
}

#define TwiceBorderSize (2 * ItemBorderSize)

static void
AttemptSymmetry(register class suiteev  *self, register long  numItems , register long  *rows , register long  *columns)
    {
  if(numItems >= ((*rows) * (*columns))) {
      return;
  }
  else {
      if(RowMajor && FALSE) {
	  *columns = (numItems/(*rows));
	  *columns += ((numItems % (*rows)) ? 1 : 0);
      }
      else {
	  *rows = (numItems/(*columns));
	  *rows += ((numItems % (*columns)) ? 1 : 0);
      }
  }
}

static void
SetBreakPoint(register class suiteev  *self, register struct suite_item  *item, register char  *end)
      {
  (Breaks(item))->AddItem((long)(end - item_Caption));
}

static boolean
DoesItFit(class suiteev  *self, struct suite_item  *item, char  *head, char  *tail, long  width)
          {
  char save = *tail;
  int XWidth = 0, YWidth = 0;

  *tail = '\0';
  (item_CaptionFont)->StringBoundingBox((self)->GetDrawable(),
		       head,&XWidth,&YWidth);
  *tail = save; /*replace potential break*/
  return((XWidth < width) ? TRUE : FALSE);
}

static char *
WalkBackwardToPunctuation(register char  *head, register char  *tail)
    {
  if(tail && (*tail != ' ') && (*(tail - 1) == ' ')) tail--;
  while((tail > head) && (*tail == ' ')) tail--;
  while(tail > head) {
    if(*tail == '.' || *tail == '-' || *tail == ';' || *tail == ':')
      break;
    else tail--;	
  }
  if(*tail == ' ') tail++;
  return(tail);
}

static char *
WalkBackwardBlackSpace( register class suiteev	 *self, register struct suite_item	 *item, register char			 *head, register char			 *tail, register long			  width )
          {
  char				*saved_tail = tail;

  if((*tail != ' ') && (*(tail - 1) == ' ')) (tail)--;
  while((tail > head) && (*tail == ' ')) (tail)--;	
  while((tail > head) && (*tail != ' ')) (tail)--;
  if(*tail == ' ') (tail)++;
  if(tail == head) {
    tail = saved_tail;
    while((tail > head) && (tail = WalkBackwardToPunctuation(head,tail)))
      if(DoesItFit(self,item,head,tail,width)) break;
      else tail--;
  }
  if(tail == head) {
    tail = saved_tail-1;
    while(tail > head) {
      if(DoesItFit(self,item,head,tail,width)) break;
      else tail--;
    }
    if(tail == head) tail = head + 1;
  }
  return(tail);
}

void
suiteev::ShrinkWrap( long				      width , long				      height )
{
    class suiteev *self=this;
  boolean			     end = FALSE, ResetWidthForOffset = TRUE;
  register int			     indx = 0, i = 0, numLines = 0;
  register struct suite_item	    *item = NULL;
  long				     saved_width = 0;
  register char			    *head = NULL, *nl = NULL, *t = NULL;
  long				     carriedHeight = YGutterSize + ItemBorderSize;

  IN(suiteev_ShrinkWrap);
  saved_width = width;
  if(Items && ITEM(0)) {
    indx = (Items)->Subscript((long)FirstVisible);
    while((indx < (Items)->Count()) && (item = ITEM(indx)) && (carriedHeight < height)) {
      if(width > (3 * CaptionMWidth)) 
	  ResetWidthForOffset = FALSE;
      else ResetWidthForOffset = TRUE;
      width = saved_width;
      if(Exposed(item) && item_Caption) {
	BreakCount(item) = 0;
	numLines = (this)->LineCount( item_Caption);
	head = item_Caption;
	for(i = 0 ; i < numLines ; i++) {
	  end = FALSE;
	  while(!end) {
	    if(nl = (char*)strchr(head,'\n')) 
	      *nl = '\0';
	    else {
	      nl = head + strlen(head);
	      end = TRUE;
	    }
	    for(t = nl;t > head;t = WalkBackwardBlackSpace(this, item, head, t, width))
	      if(DoesItFit(this, item, head, t, width)) break;
	        if(!end) *nl = '\n';
		if(t == head) t = head+2;
		if(t > head && *t != '\0') {
		  if(*t == '\n') head = t + 1;
		  else {
		    SetBreakPoint(this,item,t-1);
		    end = FALSE;
		    head = t;
		  }
		  if(!ResetWidthForOffset) {
		    width -= (2 * CaptionMWidth);
		    ResetWidthForOffset = TRUE;
		  }
		}
	  }
	}
	carriedHeight += (((item_CaptionFontSize+1) * (numLines+BreakCount(item))) + item_BorderSize + YGutterSize);
      }
      indx++;
    }
  }
  OUT(suiteev_ShrinkWrap);
}

static void
PlaceItems( register class suiteev	     *self, struct rectangle		     *rect, long				      rows , long				      cols , long				      numleftOvers, long				      itemWidth , long				      itemHeight )
        {
  register int			     i = 0;
  register struct suite_item	    *item = NULL;
  long				     itemIndex = 0;
  long int			     width = 0, height = 0, top = 0, left = 0; 
  long int			     OrigWidth = 0, OrigHeight = 0;
  long int			     OrigTop = 0, OrigLeft = 0; 
  long				     delta = 0;
  register long			     XIndex = 0, XMax = cols;
  register long			     YIndex = 0, YMax = rows;
  register long			     leftOverIndex = 0;
  long				     Ax = 0, Ay = 0;
  long				     X_LeftOvers = 0, Y_LeftOvers = 0;
  register long			     AggrigateLeft = 0, AggrigateTop = 0;
  long				     X_epsilon = 0, Y_epsilon = 0;
  unsigned			     newlineHeight;

  IN(PlaceItems);
  rectangle_GetRectSize(rect,&left,&top,&width,&height);
  rectangle_GetRectSize(rect,&OrigLeft,&OrigTop,&OrigWidth,&OrigHeight );
  if(Items && ITEM(0)) LastVisible = ITEM((Items)->Count() - 1);
  else return;
  if(RowMajor && (numleftOvers > 0)) rows++;
  else if(ColumnMajor && (numleftOvers > 0)) cols++;
  width -= ((cols+1) * XGutterSize);
  height -= ((rows+1) * YGutterSize);
  Y_LeftOvers = height - (rows * itemHeight);
  X_LeftOvers = width - (cols * itemWidth);
  if(Y_LeftOvers < 0) Y_LeftOvers = 0;
  if(X_LeftOvers < 0) X_LeftOvers = 0;
  if(!List) {
    if(ItemFixedWidth && (delta = (width - (cols * itemWidth))) > 0) {
      left += (delta / 2);
      width -= delta;
    }
    if(ItemFixedHeight && (delta = (height - (rows * itemHeight))) > 0) {
      top += (delta / 2);
      height -= delta;
    }
  }
  AggrigateLeft = Ax = left + XGutterSize;
  AggrigateTop = Ay = top + YGutterSize;
  i = (Items)->Subscript((long)FirstVisible);
  while(item = ITEM(i++))
    if(Exposed(item)) {
      newlineHeight = (item_CaptionFont)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
      itemIndex++;
      if(X_LeftOvers> XIndex) X_epsilon = 1;
      else X_epsilon = 0;
      if(Y_LeftOvers> YIndex) Y_epsilon = 1;
      else Y_epsilon = 0;
      if(List && VisibleColumns == 1) {
	itemHeight = ((newlineHeight+2) * ((self)->LineCount(item_Caption) + BreakCount(item))) + TwiceBorderSize;
	item_SetUpperLeft(item,AggrigateLeft,AggrigateTop);
	item_SetDimensions(item,itemWidth,itemHeight);
	rectangle_SetRectSize(&InsetRect,AggrigateLeft,AggrigateTop,
			      itemWidth,itemHeight);
      }
      else {
	item_SetUpperLeft(item,AggrigateLeft,AggrigateTop);
	item_SetDimensions(item,itemWidth + X_epsilon,
			   itemHeight + Y_epsilon);
	rectangle_SetRectSize(&InsetRect,AggrigateLeft,AggrigateTop,
			      itemWidth + X_epsilon,
			      itemHeight + Y_epsilon);
      }
      if(RowMajor) {
	AggrigateLeft += (itemWidth + X_epsilon + XGutterSize);
	if(++XIndex == XMax) {
	    AggrigateTop += (itemHeight + Y_epsilon + YGutterSize);
	    XIndex = 0;
	    AggrigateLeft = Ax;
	    YIndex++;
	}
      }
      else {
	AggrigateTop += (itemHeight + Y_epsilon + YGutterSize);
	if(++YIndex == YMax) {
	    AggrigateLeft += (itemWidth + X_epsilon + XGutterSize);
	    YIndex = 0;
	    AggrigateTop = Ay;
	    XIndex++;
	}
      }
      if((itemIndex == NumVisible) && (LastVisible = item)) break;
	if( numleftOvers && 
	   ((RowMajor && (YIndex == (YMax - 1))) || 
	    (ColumnMajor && (XIndex == (XMax - 1))))) break;
    }
  if(numleftOvers) {
    leftOverIndex = 0;
    X_LeftOvers = Y_LeftOvers = 0;
    left = OrigLeft;top = OrigTop;
    width = OrigWidth;height = OrigHeight; 
    if(RowMajor) {
	if(Balanced) {
	    width -= ((numleftOvers+1) * XGutterSize);
	    AggrigateLeft = (abs(width-(numleftOvers*itemWidth))/2);
	    Ax = left + XGutterSize;
	}
	X_LeftOvers = width - (numleftOvers * itemWidth);
	if(X_LeftOvers < 0) X_LeftOvers = 0;
    }
    else if(ColumnMajor) {
	if(Balanced) {
	    height -= ((numleftOvers+1) * YGutterSize);
	    AggrigateTop = (abs(height-(numleftOvers*itemHeight))/2);
	    Ay = top + YGutterSize;
	}
	Y_LeftOvers = height - (numleftOvers * itemHeight);
	if(Y_LeftOvers < 0) Y_LeftOvers = 0;
    }
    while(item = ITEM(i++))
      if(Exposed(item)) {
	  if(X_LeftOvers > leftOverIndex) X_epsilon = 1;
	  else X_epsilon = 0;
	  if(Y_LeftOvers > leftOverIndex) Y_epsilon = 1;
	  else Y_epsilon = 0;
	  leftOverIndex++;
	  itemIndex++;
	  item_SetUpperLeft(item, AggrigateLeft, AggrigateTop);
	  item_SetDimensions(item,itemWidth + X_epsilon, itemHeight + Y_epsilon);
	  rectangle_SetRectSize(&InsetRect, AggrigateLeft, AggrigateTop, itemWidth + X_epsilon, itemHeight + Y_epsilon);
	  if(RowMajor)
	      AggrigateLeft += (itemWidth + X_epsilon + XGutterSize);
	  else
	      AggrigateTop += (itemHeight + Y_epsilon + YGutterSize);
	  if((itemIndex == NumVisible) && (LastVisible = item)) break;
      }
  }
  NumVisible = itemIndex;
  OUT(PlaceItems);
}

static void
DetermineVisibleListItems( class suiteev  *self, long  height )
    {
  register int i = 0, count = 0;
  struct suite_item *item = NULL;
  int sum = YGutterSize, newlineHeight;

    IN(DetermineVisibleListItems);
    i = (Items)->Subscript((long) FirstVisible);
    while(item = ITEM(i++)) {
	newlineHeight = (item_CaptionFont)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
	count++;
	sum += (TwiceBorderSize + YGutterSize + (((self)->LineCount( item_Caption) + BreakCount(item)) *  (newlineHeight + 2)));
	if(sum > height)
	    break;
    }
    NumVisible = count;
    OUT(DetermineVisibleListItems);
}

void
suiteev::Arrange( register struct rectangle	 *rect )
{
    class suiteev *self=this;
  long				 itemWidth = 0, itemHeight = 0, 
                                 minHeight = 0, minWidth = 0, 
			         width = 0, height = 0, top = 0, left = 0; 
  long				 numleftOvers = 0, maxCols = 0;
  long				 maxRows = 0, numItems = 0;
  int				 newlineHeight;

  IN(suiteev_Arrange);
  newlineHeight = (CaptionFont)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
  if(!Items || ((numItems = (this)->NumberExposed()) <= 0) ||
      (List && (rect->width < (3 * CaptionMWidth))) ||
      (List && (rect->height < (newlineHeight + TwiceBorderSize)))) {
    FirstVisible = LastVisible = NULL;
    NumVisible = 0;
    return;
  }
  else
      CheckForNewFirstVisible(this);
  (this)->MaxStringSize( &itemWidth, &itemHeight);
  rectangle_GetRectSize(rect, &left, &top, &width, &height);
  if(ItemFixedHeight) minHeight = ItemFixedHeight + YGutterSize;
  else minHeight = itemHeight + TwiceBorderSize + 2 + YGutterSize;
  if(ItemFixedWidth) minWidth = ItemFixedWidth + XGutterSize;
  else minWidth = itemWidth + TwiceBorderSize + (2 * CaptionMWidth) + XGutterSize;
  if(List && (minWidth > width)) {
      if(width > (TwiceBorderSize + (2 * CaptionMWidth) + XGutterSize)) {
	  (this)->ShrinkWrap( width - TwiceBorderSize - 
			     (2 * CaptionMWidth) - (2 * XGutterSize), height);
	  (this)->MaxStringSize( &itemWidth, &itemHeight);
	  minHeight = itemHeight + TwiceBorderSize + 2;
	  minWidth = width - TwiceBorderSize - (2*XGutterSize);
      }
  }
  if(!Scroll) {
    NumVisible = numItems;
    FirstVisible = ITEM(0);
  }
  if(Matrix) {
    if( ! Fixed ) {
        maxCols = ((width/minWidth > 1) ? (width/minWidth) : 1);
        maxRows = ((height/minHeight > 1) ? (height/minHeight) : 1);
    }
    else {
	maxCols = Columns;
	maxRows = Rows;
    }
    if(RowMajor && (maxCols > numItems)) {
      maxCols = numItems;
      maxRows = 1;
    }
    else if(ColumnMajor && (maxRows > numItems)) {
      maxRows = numItems;
      maxCols = 1;
    }
    if( ! Fixed && ! List) AttemptSymmetry(this,numItems,&maxRows,&maxCols);
    width -= ((maxCols+1) * XGutterSize);
    height -= ((maxRows+1) * YGutterSize);
    if(List) {
      ItemWidth = itemWidth = minWidth;
      ItemHeight = itemHeight = minHeight;
    }
    else {
      if(ItemFixedWidth) ItemWidth = itemWidth = ItemFixedWidth;
      else ItemWidth = itemWidth = width/maxCols;
      if(ItemFixedHeight) ItemHeight = itemHeight = ItemFixedHeight;
      else ItemHeight = itemHeight = height/maxRows;
    }
    VisibleColumns = Columns = maxCols;
    VisibleRows = Rows = maxRows;
    if( numItems <= (VisibleRows * VisibleColumns) ) 
      NumVisible = numItems;
    else NumVisible = VisibleRows * VisibleColumns;
    if(RowMajor) numleftOvers = NumVisible % maxCols;
    else if(ColumnMajor) numleftOvers = NumVisible % maxRows;
    if(List && (maxCols == 1)) {
      DetermineVisibleListItems(this,rect->height);
      VisibleRows = Rows = maxRows = NumVisible;
      numleftOvers = 0;
    }
  }
  else if(SingleColumn) { 
    if(ItemFixedHeight) { 
      itemHeight = ItemFixedHeight;
      if(Scroll) NumVisible = height/minHeight;
      else NumVisible = numItems;
    }
    else {
      if(Scroll) {
        if((NumVisible = height/minHeight) == 0)
	  NumVisible = 1;
      }
      else NumVisible = numItems;
      height -= ((NumVisible+1) * YGutterSize );
      itemHeight = height/NumVisible;
    }
    if(ItemFixedWidth)
      itemWidth = ItemFixedWidth;
    else 
      itemWidth = width - (2 * XGutterSize);
    ItemHeight = itemHeight;
    ItemWidth = itemWidth;
    if(Scroll) {
      if(VScroll) {
        Rows = numItems;
	maxRows = VisibleRows = NumVisible;
      }
    }
    else NumVisible = Rows = VisibleRows = maxRows = numItems;
    Columns = VisibleColumns = maxCols = 1;
    if(List) {
      DetermineVisibleListItems(this,rect->height);
      VisibleRows = Rows = maxRows = NumVisible;
      numleftOvers = 0;
    }
  }
  else {
    if(ItemFixedWidth) {
      minWidth = itemWidth = ItemFixedWidth;
      if(Scroll) NumVisible = width/minWidth;
      else NumVisible = numItems;
    }
    else {
      if(Scroll) 
	if((NumVisible = width/minWidth) == 0)
          NumVisible = 1;
	else NumVisible = numItems;
      width -= ((NumVisible+1) * XGutterSize);
      itemWidth = width/NumVisible;
    }
    if(ItemFixedHeight)
      itemHeight = ItemFixedHeight;
    else 
      itemHeight = height - (2 * YGutterSize);
    ItemHeight = itemHeight;
    ItemWidth = itemWidth;
    if(Scroll) {
      if(HScroll) {
	Columns = numItems;
	maxCols = VisibleColumns = NumVisible;
      }
    }
    else NumVisible = Columns = VisibleColumns = maxCols = numItems;
    Rows = VisibleRows = maxRows = 1;
  }
  PlaceItems(this,rect,maxRows,maxCols,numleftOvers,itemWidth,itemHeight);
  OUT(suiteev_Arrange);
}

void
suiteev::DrawItems( register struct rectangle  *rect )
{
    class suiteev *self=this;
  register int i = 0;
  register struct suite_item *item = NULL;

    IN(suiteev_DrawItems);
    if(!Items || !ITEM(0))
	return;
    CheckForNewFirstVisible(this);
    i = (Items)->Subscript((long) FirstVisible);
    while(item = ITEM(i++))
	if(Exposed(item)) {
	    ItemFullUpdate(this, item, view_FullRedraw, 0, 0, 0, 0);
	    if(item == LastVisible) 
		break;
	}
    OUT(suiteev_DrawItems);
}

static void
EraseItems( register class suiteev  *self )
  {
  register int i = 0;
  register struct suite_item *item = NULL;

    IN(EraseItems);
    if(!Items || !ITEM(0) || !FirstVisible) 
	return;
    i = (Items)->Subscript((long)FirstVisible);
    while(item = ITEM(i++))
	if(Exposed(item)) {
	    (self)->ItemClear(item);	    
	    if(item == LastVisible) 
		break;
	}
    SETTRANSFERMODE(self,graphic_COPY);
    OUT(EraseItems);
}

void
suiteev::Update( )
{
    class suiteev *self=this;
  struct rectangle r;

  IN(suiteev_Update);    
  (this)->GetVisualBounds( &r);
  EraseItems(this);
  if(List) 
      (Suite)->Apply((suite_applyfptr)ResetItemBreaks, (long)Suite, 0);
  (this)->Arrange( &r);
  (this)->DrawItems( &r);
  DrawGutterLines(this);
  OUT(suiteev_Update);    
}

long
suiteev::NumberItems( )
{
    class suiteev *self=this;
  if(Items && ITEM(0)) 
      return((Items)->Count());
  else return(-1);
}

long
suiteev::NumberVisible( )
{
    class suiteev *self=this;
  register long i = 0;
  register struct suite_item *item = FirstVisible;

  while(item && ++i && (item != LastVisible));
  return(i);
}

long
suiteev::NumberExposed( )
{
    class suiteev *self=this;
  register long i = 0, index = 0;
  register struct suite_item *item = NULL;

  if(Items && ITEM(0))
    while(item = ITEM(index++))
      if(Exposed(item)) i++;
  return(i);
}

void
suiteev::Clear( )
{
    class suiteev *self=this;struct rectangle r;

  (this)->GetVisualBounds( &r);
  (this)->SetBGColorCell(SuiteBackground);
  SETTRANSFERMODE(this, graphic_COPY);
  (this)->FillRect( &r, (this)->WhitePattern());
}

struct suite_item *
suiteev::WhichItem( register long  x , register long  y )
{
    class suiteev *self=this;
  register int i = 0;
  register struct suite_item *item = NULL;

    IN(suiteev_WhichItem);
    if(Items && ITEM(0)) {
	CheckForNewFirstVisible(this);
	i = (Items)->Subscript((long) FirstVisible);
	while(item = ITEM(i++))
	    if(WithinRect(x, y, &Bounds)) {
		if(Exposed(item) && Active(item)) 
		    return(item);
		else
		    return(NULL);
	    }
	    else if(item == LastVisible)
		return(NULL);
    }
    return(NULL);
}    

#define SetViewColors( v, fg, bg )		\
  ((class view *) (v))->SetFGColorCell( (fg));	\
  ((class view *) (v))->SetBGColorCell( (bg))

#define SetItemColor(self, item)	\
SetViewColors(				\
  self,			\
  item_ForegroundColor,			\
  item_BackgroundColor )

void
suiteev_HandleExclusiveHit( register class suiteev  *self, register struct suite_item  *item, enum view_MouseAction  action, long  x , long  y, long  numberOfClicks )
          {
    IN(suiteev_HandleExclusiveHit);
    switch(action) {
	case view_LeftDown:
	    if(Highlighted(item))
		break;
	    else {
		(Suite)->Reset( suite_Normalize);
		(self)->ItemHighlight( item);
	    }
	    break;
	case view_LeftUp:
	case view_LeftMovement:
	    if(LastHit != item) {
		(Suite)->Reset( suite_Normalize);
		(self)->ItemHighlight( item);
	    }
	    break;
	case view_RightUp:  break;
	case view_RightMovement:
	    if(LastHit != item) {
		if(Highlighted(item)) {
		    (self)->ItemNormalize( item);
		}
		else {
		    (Suite)->Reset( suite_Normalize);
		    (self)->ItemHighlight( item);
		}
	    }
	    break;
	case view_RightDown:
	    if(Highlighted(item)) {
		(self)->ItemNormalize( item);
	    }
	    else {
		(Suite)->Reset( suite_Normalize);
		(self)->ItemHighlight( item);
	    }
	    break;
    }
    LastHit = item;
    OUT(suiteev_HandleExclusiveHit);
}

void
suiteev_HighlightFirstToLast( register class suiteev  *self, register struct suite_item  *first, register struct suite_item  *last )
      {
    IN(suiteev_HighlightFirstToLast);
    if(Items && ITEM(0)) {
	register struct suite_item *item = NULL;
	boolean lastFound = FALSE;
	register int f, l, i = 0;

	while(item = ITEM(i++)) {
	    if(last == item) {
		lastFound = TRUE;
		break;
	    }
	    else if(first == item)
		break;
	}
	f = (Items)->Subscript((long) first);
	l = (Items)->Subscript((long) last);
	if(lastFound) {
	    for( i = 0; i < l && (item = ITEM(i)); i++ )
		if(Exposed(item) && Active(item) && Highlighted(item)) {
		    (self)->ItemNormalize( item);
		}
	    for( i = l; i <= f && (item = ITEM(i)); i++ )
		if(Exposed(item) && Active(item) && Normalized(item)) {
		    (self)->ItemHighlight( item);
		}
	}
	else {
	    for( i = f; i <= l && (item = ITEM(i)); i++ )
		if(Active(item) && Exposed(item) && Normalized(item)) {
		    (self)->ItemHighlight( item);
		}
	}
	if(item)
	    while(item = ITEM(i++))
		if(Exposed(item) && Active(item) && Highlighted(item)) {
		    (self)->ItemNormalize( item);
		}
    }
    OUT(suiteev_HighlightFirstToLast);
}

void
suiteev_HandleInclusiveHit( register class suiteev  *self, register struct suite_item  *item, register enum view_MouseAction  action, register long  x , register long  y , register long  numberOfClicks )
        {
    IN(suiteev_HandleInclusiveHit);
    switch(action) {
	case view_LeftMovement:
	    if(LastHit != item)
		suiteev_HighlightFirstToLast(self, FirstHit, item);
	    break;
	case view_LeftDown:
	    if(FirstHit != LastHit || FirstHit != item)
		(Suite)->Reset( suite_Normalize);
	    if(Normalized(item)) {
		(self)->ItemHighlight( item);
	    }
	    FirstHit = item;
	    break;
	case view_RightUp:
	case view_LeftUp:
	    break;
	case view_RightMovement:
	    if(LastHit != item) 
		(self)->ItemToggle( item);
	    break;
	case view_RightDown:
	    FirstHit = item;
	    (self)->ItemToggle( item);
	    break;
    }
    LastHit = item;
    OUT(suiteev_HandleInclusiveHit);
}

void
suiteev_HandleToggleHit( register class suiteev  *self, register struct suite_item  *item, register enum view_MouseAction  action, register long  x , register long  y , register long  numberOfClicks )
        {
    IN(suiteev_HandleToggleHit);
    if((action == view_LeftDown) || (action == view_RightDown))
	(self)->ItemToggle( item);
    OUT(suiteev_HandleToggleHit);
}

class view *
suiteev::Hit( register enum view_MouseAction  action, register long  x , register long  y , register long  numClicks )
{
    class suiteev *self=this;
    register struct suite_item *item = NULL;
    struct rectangle r;
    register class view *ret = (class view *) this;

    IN(suiteev_Hit);
    (this)->GetVisualBounds( &r);
    if(WithinRect(x, y, &r)) {
	if((item = (this)->WhichItem( x, y)) && Active(item)) {
	    CurrentItem = item;
	    switch(SelectionMode) {
		case suite_Toggle:
		    suiteev_HandleToggleHit(this, item, action, x, y, numClicks);
		    break;
		case suite_Exclusive:
		    suiteev_HandleExclusiveHit(this, item, action, x, y, numClicks);
		    break;
		case suite_Inclusive:
		    suiteev_HandleInclusiveHit(this, item, action, x, y, numClicks);
		    break;
	    }
	    if(HitHandler && !item->hithandler)
		HitHandler(ClientAnchor, Suite, item, suite_ItemObject, action, x, y, numClicks);
	    if(action != view_LeftUp && action != view_RightUp) {
		if(CVIF && (CVIF != item->viewobject))
		    CVIF = NULL;
		if(item->viewobject)
		    CVIF = item->viewobject;
	    }
	    ret = (this)->ItemHit( item, action, x, y, numClicks);
	}
	else if(HitHandler && (!item || Active(item)))
	    HitHandler(ClientAnchor, Suite, item, suite_NoObject, action, x, y, numClicks);
    }
    OUT(suiteev_Hit);
    return(ret);
}

static
void ItemFullUpdate( register class suiteev  *self, register struct suite_item  *item, register enum	view_UpdateType  type, register long  left , register long  top , register long  width , register long  height )
        {
  struct rectangle *r = NULL;

  IN(ItemFullUpdate);
  if(!ItemCursor && !item->cursor && item->cursorbyte) {
    item->cursor = cursor::Create(self);
    if(item->cursor && item_CursorFont && item_CursorByte) 
      (item->cursor)->SetGlyph( item_CursorFont, item_CursorByte);
    else if(item_CursorByte) (item->cursor)->SetStandard( item_CursorByte);
    else (item->cursor)->SetStandard( Cursor_Octagon);
  }
  if(Active(item)) { 
    if(item_AccessType & suite_ReadWrite) {
	if(!item->dataobject) (self)->SetItemToReadWrite( item);
	if(!(item_BorderStyle & suite_Invisible)) 	
	    (self)->DrawItemBorder( item);
	if( item->title ) (self)->ItemDrawTitle( item);
	r = rectangle_Duplicate(&InsetRect);
	DrawRect(self, item, r, TRUE);
	DecrementRect(r, item_BorderSize);
	((class suitecv *) item->viewobject)->LinkTree( self);
	SetViewColors((class view *) item->viewobject, item_ForegroundColor, item_BackgroundColor);
	((class suitecv *) item->viewobject)->InsertView( self, r);
	((class suitecv *) item->viewobject)->FullUpdate( type, 0, 0, r->width, r->height);
    }
    else if(item->viewobject) {
	(item->viewobject)->LinkTree( self);
	SetViewColors((class view *) item->viewobject, item_ForegroundColor, item_BackgroundColor);
	if(!(item_BorderStyle & suite_Invisible))
	    (self)->DrawItemBorder( item);
	if(item->title)
	    (self)->ItemDrawTitle( item);
	(item->viewobject)->InsertView( self, &InsetRect);
	(item->viewobject)->FullUpdate( type, 0, 0, InsetWidth, InsetHeight);
    }
    else if(item_DataObjectName) {
      if(!item->dataobject) { 
	item->dataobject = (class dataobject*)ATK::NewObject(item_DataObjectName);
	if(item_DataObjectHandler)
	  item_DataObjectHandler(ClientAnchor,Suite,item, suite_ItemObject);
      }
      if(!item_ViewObjectName) {
	if(!item->viewobjectname) 
	  AllocNameSpace(&ItemViewObjectName, (item->dataobject)->ViewName());
      }
      if(!item->viewobject) { 
	item->viewobject = (class view*)ATK::NewObject(item_ViewObjectName);
	if(item_ViewObjectHandler)
	  item_ViewObjectHandler(ClientAnchor,Suite,item, suite_ItemObject);
      }

      (item->viewobject)->LinkTree(self);
      SetViewColors((class view *) item->viewobject, item_ForegroundColor, item_BackgroundColor);
      if(!(item_BorderStyle & suite_Invisible))
	  (self)->DrawItemBorder(item);
      if(item->title)
	  (self)->ItemDrawTitle( item);
      (item->viewobject)->InsertView(self,&InsetRect);
      (item->viewobject)->FullUpdate( type, 0, 0, InsetWidth, InsetHeight);
    }
    else {
	SetItemColor(self, item);
	if(Normalized(item))
	    (self)->ItemNormalize(item);
	else if(Highlighted(item))
	    (self)->ItemHighlight(item);
    }
  }
  else if(ItemPassiveStyle & suite_Pale)
      (self)->ItemShade( item);
  if(item->cursor || ItemCursorByte)
    (self)->PostCursor( &Bounds, item_Cursor);
  OUT(ItemFullUpdate);
}

void
suiteev::ItemUpdate( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemUpdate);
  if(item->viewobject && item->dataobject) {
      (item->viewobject)->Update();
  }
  else {
      (this)->ItemClear(item);
      SetItemColor(this, item);
      ItemFullUpdate(this,item,view_FullRedraw,0,0,0,0);
  }
  OUT(suiteev_ItemUpdate);
}

class view *
suiteev::ItemHit( register struct suite_item  *item, register enum view_MouseAction  action, register long  x , register long  y , register long  numClicks )
{
    class suiteev *self=this;
  class view *retval = (class view*)this;

  IN(suiteev_ItemHit);
  if(item && Active(item)) {
    if(!CVIF || (item->viewobject && (item->viewobject == CVIF))) {
      if(item_AccessType & suite_ReadWrite) {
	if(WithinRect(x,y,&InsetRect)) {
	    return((class view*)((class suitecv*)item->viewobject)->Hit(
				action,x - InsetLeft,y - InsetTop,numClicks));
	}
	else 
	  return((class view*)((class suitecv*)item->viewobject)->Hit(
		    action,0,0,numClicks));
      }
      else if(item->viewobject) 
	  retval = (item->viewobject)->Hit(action,x - InsetLeft,
		    y - InsetTop,numClicks);
      if(item->hithandler) 
	  item->hithandler(ClientAnchor,Suite,item,suite_ItemObject,
			   action,x,y,numClicks);
    }
  }
  OUT(suiteev_ItemHit);
  return((class view*)retval);
}

void
suiteev::ItemClear( register struct suite_item  *item )
{
    class suiteev *self=this;struct rectangle r;
  IN(suiteev_ItemClear);
  rectangle_SetRectSize(&r, Left, Top, Width, Height);
  (this)->SetBGColorCell( SuiteBackground);
  SETTRANSFERMODE(this, graphic_COPY);
  (this)->FillRect( &r, (this)->WhitePattern());
  OUT(suiteev_ItemClear);
}

void
suiteev::ItemBlackOut( register struct suite_item  *item )
{
    class suiteev *self=this;
  struct rectangle *r;

  IN(suiteev_ItemBlackOut);
  r = rectangle_Duplicate(&Bounds);
  ::DrawRect(this, item, r, Highlighted(item));
  rectangle_SetRectSize(r, Left + 1, Top + 1, Width - 2, Height - 2);
  DecrementRect(r, item_BorderSize);
  SETTRANSFERMODE(this, graphic_COPY);
  // huh... presumably r was going to be used for something,
  // but what? how?  -robr 3/15/96
  free(r);
  OUT(suiteev_ItemBlackOut);
}

void
suiteev::ItemHighlightReverseVideo( register struct suite_item  *item, boolean  border )
{
    class suiteev *self=this;

  IN(suiteev_ItemHighlightReverseVideo);
  if(item_BorderStyle & suite_Rectangle)
    (this)->ItemBlackOut( item);
  OUT(suiteev_ItemHighlightReverseVideo);
}

void
suiteev::ItemHighlightBorder( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemHighlightBorder);
  (this)->DrawItemBorder( item);
  OUT(suiteev_ItemHighlightBorder);
}

void
suiteev::ItemHighlightCaptionBoldItalic( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemHighlightCaptionBoldItalic);
  item->captionfonttype = fontdesc_Bold | fontdesc_Italic;
  item->captionfont = NULL;
  OUT(suiteev_ItemHighlightCaptionBoldItalic);
}

void
suiteev::ItemHighlightCaptionBold( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemHighlightCaptionBold);
  item->captionfonttype = fontdesc_Bold;
  item->captionfont = NULL;
  OUT(suiteev_ItemHighlightCaptionBold);
}

void
suiteev::ItemHighlightCaptionItalic( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemHighlightCaptionItalic);
  item->captionfonttype = fontdesc_Italic;
  item->captionfont = NULL;
  OUT(suiteev_ItemHighlightCaptionItalic);
}

void
suiteev::ItemNormalize( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemNormalize);
  if(item_AccessType & suite_ReadWrite) return;
  (this)->ItemClear( item);
  SetItemColor(this, item);
  item->mode = ((item_Active | item_Normalized) & ~item_Highlighted);
  if(!(item_BorderStyle & (suite_Invisible | suite_None)))
    (this)->DrawItemBorder( item);
  if(item_HighlightStyle & (suite_Italic | suite_Bold)) {
    item->captionfonttype = fontdesc_Plain;
    item->captionfont = NULL;
  }
  if(item->title) 
      (this)->ItemDrawTitle( item);
  if(item->viewobject) 
    (item->viewobject)->FullUpdate( view_FullRedraw, 0, 0, Width, Height);
  else if(item_Caption)
      (this)->ItemDrawCaption( item);
  OUT(suiteev_ItemNormalize);
}

#define BorderPlusOne (ItemBorderSize+1)
#define TwiceBorderPlusOne (BorderPlusOne*2)

static void
ItemPlaceCaption( register class suiteev  *self, register struct suite_item  *item, long  captionwidth, long  captionheight, unsigned  *place )
          {
  unsigned char alignment = 0;
  long l = Left + ItemBorderSize, t = Top + ItemBorderSize;
  long w = Width - (2*ItemBorderSize);
  long h = Height - (2*ItemBorderSize);

  IN(ItemPlaceCaption);
  if(item) {
    alignment = item_CaptionAlignment;
    if(captionwidth > w) captionwidth = w;
    if(captionheight > h) captionheight = h;
    rectangle_SetRectSize(&CaptionRect,
			   l + ((w - captionwidth)/2),
			   t + ((h - captionheight)/2),
			   captionwidth,captionheight);
    if(alignment & suite_Left) CaptionRect.left = l;
    if(alignment & suite_Right) CaptionRect.left = l + w - captionwidth;
    if(alignment & suite_Top) CaptionRect.top = t;
    if(alignment & suite_Bottom) CaptionRect.top = t + h - captionheight;
  }
  *place = graphic_BETWEENTOPANDBOTTOM;
  if(alignment & suite_Middle) *place |= graphic_BETWEENTOPANDBOTTOM;
  if(alignment & suite_Center) *place |= graphic_BETWEENLEFTANDRIGHT;
  if(alignment & suite_Left) *place |= graphic_ATLEFT;
  if(alignment & suite_Right) *place |= graphic_ATRIGHT;
  OUT(ItemPlaceCaption);
}

void
suiteev::ItemDrawCaption( register struct suite_item  *item )
{
    class suiteev *self=this;
  long captionwidth = 0, captionheight = 0, totalWidth = 0;
  long X = 0, Y = 0, SubStringIndex = 0;
  unsigned tMode = graphic_COPY, placement = 0;
  unsigned alignment = item_CaptionAlignment;
  register long pos = 0, i = 0, j = 0, numLines = 0;
  char *tmp = NULL, *head = NULL, save;
  boolean WasBreakPos = FALSE, WasNewLine = FALSE;
  boolean dontDraw = FALSE;
  unsigned newlineHeight;

  IN(suiteev_ItemDrawCaption);
  numLines = (this)->LineCount( item_Caption) + BreakCount(item);
  item->captionfontsize = CaptionFontSize;
  item->captionfont = fontdesc::Create(item_CaptionFontName, item_CaptionFontType, item->captionfontsize);
  SetFont(this, item->captionfont);
  (this)->SetClippingRect( &Bounds);
  if(List) {
      totalWidth = MaxListSubStringWidth(this, item, item_Caption, item_CaptionFont);
      if(item_Title) totalWidth += TitleRect.width;
      if((totalWidth > Width) && (Width > (4 * CaptionMWidth))) {
	  (this)->Update();
	  return;
      }
  }
  newlineHeight = (item_CaptionFont)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
  captionheight = numLines * newlineHeight;
  ItemPlaceCaption(this, item, captionwidth, captionheight, &placement);
  SETTRANSFERMODE(this, tMode);
  tmp = head = item_Caption;
  Y = CaptionRect.top + newlineHeight/2;
  X = CaptionRect.left + CaptionRect.width/2;
  if(alignment & suite_Left) X = CaptionRect.left;
  if(alignment & suite_Right) X = CaptionRect.left + CaptionRect.width;
  for( j = 0, i = 0 ; i < numLines ; i++ ) {
    WasNewLine = WasBreakPos = FALSE;
    save = '\0';
    while(tmp && (*tmp != '\0')) {
      if(*tmp == '\n') {
        WasNewLine = TRUE;
	break;
      }
      else if((j < BreakCount(item)) && (pos == BreakPos(item,j))) {
	WasBreakPos = TRUE;
	break;
      }
      else {
	pos++;
	tmp++;
      }
    }
    if(WasNewLine) *tmp = '\0';
    else if(WasBreakPos) {
      save = *(tmp + 1);
      *(tmp+1) = (char)0;
    }
    if(List) {
      if(i > 0) {
	if(WrapStyle & suite_LeftIndent) {
          X = Left + CaptionMWidth + (2 * CaptionMWidth);
	  placement = graphic_ATLEFT | graphic_BETWEENTOPANDBOTTOM;
	}
	else if(WrapStyle & suite_LeftRight) {
	  X = Left + Width - (2 * CaptionMWidth);
	  placement = graphic_ATRIGHT | graphic_BETWEENTOPANDBOTTOM;
	}
      }
      else {
	X = Left + CaptionMWidth;
	placement = graphic_ATLEFT | graphic_BETWEENTOPANDBOTTOM;
      }
      if(item == FirstVisible && i < FirstVisibleSubString) 
	dontDraw = TRUE;
      else dontDraw = FALSE;
    }
    if(i > 0) 
	Y += (newlineHeight + 2);
    if(!List ||
	(Y + (newlineHeight/2) + 1) < (this)->GetVisualBottom()) {
      if(!dontDraw) {
	(this)->MoveTo( X, Y);
	(this)->DrawString( head, placement);
      }
    }
    if(WasNewLine) *tmp = '\n'; 
    else if(WasBreakPos) {
      *(tmp + 1) = save;
      j++;
    }
    tmp++;
    pos++;
    head = tmp;
  }
  SETTRANSFERMODE(this, graphic_COPY);
  (this)->ClearClippingRect();
  OUT(suiteev_ItemDrawCaption);
}


void
ItemPlaceTitle(class suiteev  *self, struct suite_item  *item, long  titlewidth , long  titleheight, int  newlineHeight)
        {
  unsigned char titleplacement = item_TitlePlacement;
  boolean left = FALSE, right = FALSE, top = FALSE, bottom = FALSE;

  IN(suiteev_ItemPlaceTitle);
  if(titleplacement & suite_Left) {
    left = TRUE;
    rectangle_SetRectSize(&TitleRect,
			   Left + BorderPlusOne,
			   Top + BorderPlusOne,
			   titlewidth + 2,
			   Height - TwiceBorderPlusOne);
    rectangle_SetRectSize(&InsetRect,
			   TitleRect.left + TitleRect.width + 1,
			   Top + BorderPlusOne,
			   Width - TitleRect.width - TwiceBorderPlusOne - 1, 
			   Height - TwiceBorderPlusOne);
  }
  else if(titleplacement & suite_Right) {
    right = TRUE;
    rectangle_SetRectSize(&TitleRect,
			   Left + Width - (titlewidth + 2),
			   Top + BorderPlusOne,
			   titlewidth + 2,
			   Height - TwiceBorderPlusOne);
    rectangle_SetRectSize(&InsetRect,
			   Left + BorderPlusOne,
			   Top + BorderPlusOne,
			   Width - TitleRect.width - TwiceBorderPlusOne - 1, 
			   Height - TwiceBorderPlusOne);
  }
  else if(titleplacement & suite_Top) {
    top = TRUE;
    rectangle_SetRectSize(&TitleRect,
			   Left + BorderPlusOne,
			   Top + BorderPlusOne,
			   Width - TwiceBorderPlusOne,
			   titleheight + 2);
    rectangle_SetRectSize(&InsetRect,
			   Left + BorderPlusOne,
			   TitleRect.top + TitleRect.height + 1,
			   Width - TwiceBorderPlusOne,
			   Height - TitleRect.height - TwiceBorderPlusOne);
  }
  else if(titleplacement & suite_Bottom) {
    bottom = TRUE;
    rectangle_SetRectSize(&TitleRect,
			   Left + BorderPlusOne,
			   Top + Height - titleheight - BorderPlusOne,
			   Width - TwiceBorderPlusOne,
			   titleheight + 2);
    rectangle_SetRectSize(&InsetRect,
			   Left + BorderPlusOne,
			   Top + BorderPlusOne,
			   Width - TwiceBorderPlusOne,
			   Height - TitleRect.height - BorderPlusOne);
  }
  if(item_AccessType & suite_ReadWrite) {
    InsetHeight = (2 * newlineHeight);
    if(Height < InsetHeight) 
      InsetHeight = Height - 4;
    if(left || right) 
      InsetTop = TitleRect.top + (TitleRect.height - InsetHeight)/2;
    else if(top) 
      InsetTop = TitleRect.top + TitleRect.height + 2;
    else if(bottom) 
      InsetTop = TitleRect.top + 2 + InsetHeight;
  }
  OUT(suiteev_ItemPlaceTitle);
}

void
suiteev::ItemDrawTitle( register struct suite_item  *item )
{
    class suiteev *self=this;
  long x = 0, y = 0, count = 0;
  char *tmp = NULL, *head = NULL;
  unsigned tMode = graphic_COPY, alignment = 0;
  unsigned placement = graphic_BETWEENTOPANDBOTTOM;
  int titlewidth = 0, titleheight = 0;
  long titleLines = 0;
  int newlineHeight;

  IN(suiteev_ItemDrawTitle);
  item->titlefont = fontdesc::Create(item_TitleFontName, item_TitleFontType, item_TitleFontSize);
  SetFont(this, item->titlefont);
  (this)->SetClippingRect( &Bounds);
  if(item_Title) {
    (item->titlefont)->StringBoundingBox( (this)->GetDrawable(), item_Title, &titlewidth, &titleheight);
    titleLines = (this)->LineCount( item_Title);
    newlineHeight = (item->titlefont)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
    titleheight = titleLines * newlineHeight;
    alignment = item_TitleCaptionAlignment;
    if(alignment & suite_Left) placement |= graphic_ATLEFT;
    else if(alignment & suite_Right) placement |= graphic_ATRIGHT;
    else placement |= graphic_BETWEENLEFTANDRIGHT;
    ItemPlaceTitle(this, item, titlewidth, titleheight, newlineHeight);
  }
#if 0
    else if(item->titleviewobject) {
	titlewidth = Width/3;
	titleheight = Height/3;
    }
#endif
  SETTRANSFERMODE(this, tMode);
  if(item_Title) {
    head = item_Title;
    if(alignment & suite_Left) x = TitleRect.left;
    else if(alignment & suite_Right) x = TitleRect.left + TitleRect.width;
    else x = TitleRect.left + TitleRect.width/2;
    if(alignment & suite_Top) y = TitleRect.top + newlineHeight/2;
    else if(alignment & suite_Bottom) 
      y = TitleRect.top + TitleRect.height - titleheight + newlineHeight/2;
    else y = TitleRect.top + (TitleRect.height - titleheight)/2 + newlineHeight/2;
    for( count = 0; (count < titleLines) && (head != '\0'); count++) {
      if(tmp = (char*) strchr(head,'\n')) *tmp = (char)0;
      (this)->MoveTo( x, y + (newlineHeight * count));
      (this)->DrawString( head, placement);
      if(tmp) {
	*tmp = '\n';
	head = tmp + 1;
      }
    }
  }
#if 0
    else if(item->titleviewobject)
	(item->titleviewobject)->FullUpdate(view_FullRedraw,0,0,
			  TitleRect.width,TitleRect.height);
#endif
  SETTRANSFERMODE(this, graphic_COPY);
  (this)->ClearClippingRect();
  OUT(suiteev_ItemDrawTitle);
}

void
suiteev::ItemHighlight( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemHighlight);
  if( (item_HighlightStyle & suite_None) || 
      (item_AccessType & suite_ReadWrite)) 
      return;
  SetItemColor(this, item);
  if(item_HighlightStyle & suite_Invert)
    (this)->ItemHighlightReverseVideo( item, item_HighlightStyle & suite_Border);
  else if(!item->viewobject) 
      (this)->ItemClear( item);
  if((item_HighlightStyle & suite_Bold) && (item_HighlightStyle & suite_Italic))
    (this)->ItemHighlightCaptionBoldItalic( item);
  else {
    if(item_HighlightStyle & suite_Bold)
      (this)->ItemHighlightCaptionBold( item);
    if(item_HighlightStyle & suite_Italic) 
      (this)->ItemHighlightCaptionItalic( item);
  }
  item->mode = ((item_Active | item_Highlighted) & ~item_Normalized);
  if(!(item_BorderStyle & suite_None)) 
      (this)->DrawItemBorder( item);
  if(item->title) 
      (this)->ItemDrawTitle( item);
  if(item_Caption && !(item_AccessType & suite_ReadWrite)) 
    (this)->ItemDrawCaption( item);
  OUT(suiteev_ItemHighlight);
}

void
suiteev::ItemClearCaption( struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemClearCaption);
  if(item_Caption) 
      (this)->ItemDrawCaption( item);
  OUT(suiteev_ItemClearCaption);
}

void
suiteev::ItemShade( register struct suite_item  *item )
{
    class suiteev *self=this;
  short int shade = 0;
  IN(suiteev_ItemShade);
  item->mode &= ~item_Active;
  item->mode &= ~item_Normalized;
  if(!(item_BorderStyle & suite_None))
      (this)->DrawItemBorder( item);
  if(item->title)
    (this)->ItemDrawTitle( item);
  if(item_Caption)
    (this)->ItemDrawCaption( item);
  OUT(suiteev_ItemShade);
}

long
suiteev::Locate( register long  x , register long  y )
{
    class suiteev *self=this;
  register struct suite_item *item = NULL;
  register long i = 0;
  long realLeft = 0, realTop = 0, realWidth = 0, realHeight = 0;
  long YGutterOffset = YGutterSize + 1;
  long XGutterOffset = XGutterSize + 1;

  IN(suiteev_Locate);
  if(Items && ITEM(0) && (i = (Items)->Subscript((long) FirstVisible)) != -1) {
    if(!x) 
      while(item = ITEM(i)) {
	realTop = Top - (YGutterOffset/2);
	realHeight = Height + YGutterOffset;
	if((Suite)->ItemExposed( item) &&
	   (realTop <= y) && (y <= (realTop + realHeight))) {
	  if((item == FirstVisible) && ((i+1) < (Items)->Count())) 
	    return(i+1);
	  else return(i);
	}
	else i++;
      }
    else if(!y)
      while(item = ITEM(i)) {
	realLeft = Left - (XGutterOffset/2);
	realWidth = Width + XGutterOffset;
	if((Suite)->ItemExposed( item) &&
	   (realLeft <= x) && (x <= (realLeft + realWidth))) return(i);
	else i++;
      }
  }
  else return(0);
  OUT(suiteev_Locate);
  return((Items)->Subscript((long)FirstVisible)); /* === */
}

void
suiteev::DrawItemBorder( register struct suite_item  *item )
{
    class suiteev *self=this;
  register long i = 0;
  struct rectangle *rect = NULL;

  IN(suiteev_DrawItemBorder);
  SETTRANSFERMODE(this, graphic_COPY);
  rect = rectangle_Duplicate(&Bounds);
  if(item_BorderStyle & suite_Rectangle) {
      ::DrawRect(this, item, rect, Highlighted(item));
      DecrementRect(rect, item_BorderSize);
  }
  rectangle_SetRectSize(&InsetRect, rect->left, rect->top, rect->width, rect->height);
  free(rect);
  OUT(suiteev_DrawItemBorder);
}

void
suiteev::ItemToggle( register struct suite_item  *item )
{
    class suiteev *self=this;
  IN(suiteev_ItemToggle);

  if(Highlighted(item)) {
      (this)->ItemNormalize( item);
  }
  else if(Normalized(item)) {
      (this)->ItemHighlight( item);
  }
  SetViewColors((class view *) this, SuiteForeground, SuiteBackground);
  OUT(suiteev_ItemToggle);
}

void
suiteev::AllocItemArray( long  count )
{
    class suiteev *self=this;
  IN(suiteev_AllocItemArray);
  if(count > 0) 
    ItemArray = (struct suite_item **)
	calloc(count + 1, sizeof(struct suite_item *));
  OUT(suiteev_AllocItemArray);
}

static void
ReadWriteHandler( register long  anchor, register class suite  *suite, register struct suite_item  *item )
      {
  register class text *txt = NULL;
  register class suitecv *txtv = NULL;

  if((txt = (class text*) (suite)->ItemDataObject( item)) && 
     (txtv = (class suitecv*) (suite)->ItemViewObject( item))) {
    (txtv)->SetBorder( 1, 1);
    (txtv)->SetDataObject( txt);
  }
}

void
suiteev::SetItemToReadWrite( register struct suite_item  *item )
{
    class suiteev *self=this;
  AllocNameSpace(&item->dataobjectname,"text");
  item->dataobject = (class dataobject*) ATK::NewObject(item_DataObjectName);
  AllocNameSpace(&item->viewobjectname,"suitecv");
  item->viewobject = (class view*) ATK::NewObject(item_ViewObjectName);
  ((class suitecv*)item->viewobject)->parent_EV = this;
  ((class suitecv*)item->viewobject)->parent_item = item;
  item->viewobjecthandler = (suite_viewobjectfptr)ReadWriteHandler;
  ReadWriteHandler(ClientAnchor, Suite, item);
  item->highlightstyle = suite_Border;
  if(item_Caption) 
    ((class text*)item->dataobject)->AlwaysInsertCharacters( 0, item_Caption, strlen(item_Caption));
}

static void
MaxSubStringSize( class suiteev  *self, struct suite_item  *item, char  *str, class fontdesc  *font,  int *w ,  int *h )
          {
  long int HMax = 0, WMax = 0;
  char *tmp = NULL, *head = NULL, save;
  register int j = 0, pos = 0, i = 0;

  *w = *h = 0;
  tmp = head = str;
  while(tmp && *tmp != (char)0) {
    if(*tmp == '\n') {
      *tmp = '\0';
      (font)->StringBoundingBox( (self)->GetDrawable(), head, w, h);
      if(*w > WMax) {
	WMax = *w;
      }
      if(*h > HMax) {
	HMax = *h;
      }
      *tmp++ = '\n';
      pos++;
      head = tmp;
      i++;
    }
    else if((j < BreakCount(item)) && (pos == BreakPos(item, j))) {
      save = *(item_Caption + pos + 1);
      *(item_Caption + pos + 1) = '\0';
      (font)->StringBoundingBox( (self)->GetDrawable(), head, w, h);
      if(*w > WMax) {
	WMax = *w;
      }
      if(*h > HMax) {
	HMax = *h;
      }
      *(item_Caption + pos + 1) = save;
      tmp++;pos++;
      head = tmp;
      j++;
      i++;
    }
    else { 
      tmp++;
      pos++; 
    }
  }
  if(*tmp == '\0' && *head != '\0' ) {
      (font)->StringBoundingBox( (self)->GetDrawable(), head, w, h);
      if(*w > WMax)
	  WMax = *w;
      if(*h > HMax) 
	  HMax = *h;
  }
  *w = WMax;
  *h = (font)->FontSummary( (Suite)->GetDrawable())->newlineHeight;
}

static long
MaxListSubStringWidth( class suiteev  *self, struct suite_item  *item, char  *str, class fontdesc  *font )
        {
    long int WMax = 0;
    int w=0, h=0;
  char *tmp = NULL, *head = NULL, save;
  register int j = 0, pos = 0, i = 0;

  w = 0;
  tmp = head = str;
  while(tmp && *tmp != '\0') {
    if(*tmp == '\n') {
      *tmp = '\0';
      (font)->StringBoundingBox( (self)->GetDrawable(), head, &w, &h);
      if(i > 0) w += (2 * CaptionMWidth);
      if(w > WMax) WMax = w;
      *tmp++ = '\n';
      pos++;
      head = tmp;
      i++;
    }
    else if((j < BreakCount(item)) && (pos == BreakPos(item,j))) {
      save = *(item_Caption + pos + 1);
      *(item_Caption + pos + 1) = '\0';
      (font)->StringBoundingBox( (self)->GetDrawable(), head, &w, &h);
      if(i > 0) w += (2 * CaptionMWidth);
      if(w > WMax) WMax = w;
      *(item_Caption + pos + 1) = save;
      tmp++;
      pos++;
      head = tmp;
      j++;
      i++;
    }
    else { 
      tmp++;
      pos++; 
    }
  }
  if(*tmp == '\0' && *head != '\0' ) {
    (font)->StringBoundingBox( (self)->GetDrawable(), head, &w, &h);
    if(i > 0) w += (2 * CaptionMWidth);
    if(w > WMax) WMax = w;
  }
  return(WMax);
}

void
suiteev::MaxStringSize( long  *width , long  *height )
{
    class suiteev *self=this;
  register int i = 0;
  register struct suite_item *item = NULL;
  register long maxWidth = 0, maxHeight = 0;
  int XWidth = 0, YWidth = 0;
  register int numLines = 0;

  IN(::MaxStringSize);
  while(item = ITEM(i++))
    if(Exposed(item)) {
      if(item_Caption) {
	numLines = (this)->LineCount( item_Caption) + BreakCount(item);
	MaxSubStringSize(this, item, item_Caption, item_CaptionFont, &XWidth, &YWidth);
	YWidth *= numLines;
      }
      if(item->title) {
	int titleYWidth = 0, titleXWidth = 0;
	numLines = (this)->LineCount( item->title);
	MaxSubStringSize(this, item, item->title, item->titlefont ? item->titlefont : item_CaptionFont, &titleXWidth, &titleYWidth);
	if(TitlePlacement & (suite_Left | suite_Right))
	    XWidth += titleXWidth;
	if(TitlePlacement & (suite_Top | suite_Bottom))
	  YWidth += (numLines * titleYWidth);
      }
      if(XWidth > maxWidth) 
	  maxWidth = XWidth;
      if(YWidth > maxHeight) 
	  maxHeight = YWidth;
    }
  *width = maxWidth;
  *height = maxHeight;
  OUT(::MaxStringSize);
}

long
suiteev::LineCount( register char  *str )
{
    class suiteev *self=this;
  register long number = 1;
  register char *tmp = str;

  if(tmp)
    while(*tmp != (char)0) 
      if(*tmp++ == '\n') 
        number++;
  return(number);
}

static void 
DrawRectSize(class suiteev	 *self, long  x , long  y , long  width , long  height)
    {
  long left = x;
  long right = x+width-1;
  long top = y;
  long bottom = y+height-1;

  if (left > right) left = right;
  if(top > bottom) top = bottom;
  (self)->MoveTo( left, top);
  (self)->DrawLineTo( right, top);
  (self)->DrawLineTo( right, bottom);
  (self)->DrawLineTo( left, bottom);
  (self)->DrawLineTo( left, top);
}

static struct sbutton_info thebutton = {
    NULL,   /* the prefs struct will be filled later */
    "",	    /* the label is empty */
    0,	    /* the rock isn't needed */
    NULL,   /* ditto for the trigger atom */
    FALSE,  /* initially not lit, will be set appropriately */
    TRUE, /* should be drawn as if sensitive */
};

static void 
DrawRect(class suiteev  *self, struct suite_item  *item, struct rectangle  *Rect, boolean  lit)
        {
  struct rectangle *childrect = rectangle_Duplicate(Rect);
  class region *re1 = region::CreateEmptyRegion(), *re2 = region::CreateEmptyRegion();

  if(1 || !graphicIsMono) {
      if(!re1 || !re2) {
	  if(re1) delete re1;
	  if(re2) delete re2;
	  return;
      }
      SETTRANSFERMODE(self, graphic_COPY);
      DecrementRect(childrect, item_BorderSize);
      self->itemPrefs->bdepth = item_BorderSize;
      self->itemPrefs->colors[sbutton_BACKGROUND] = item_BackgroundColorName;
      self->itemPrefs->colors[sbutton_FOREGROUND] = item_ForegroundColorName;
      thebutton.prefs = self->itemPrefs;
      if(Active(item))
	  thebutton.lit = lit;
      else
	  thebutton.lit = FALSE;
      sbuttonv::DrawButton(self, &thebutton, Rect);
      (re2)->RectRegion( childrect);
      delete re1;
      delete re2;
      free(childrect);
  }
  else {
      register int i = item_BorderSize;
      SetViewColors((class view *) self, item_ForegroundColor, item_BackgroundColor);
      while(i > 0) {
	  DrawRectSize(self, rectangle_Left(Rect), rectangle_Top(Rect), rectangle_Width(Rect), rectangle_Height(Rect));
	  DecrementRect(Rect, 1);
	  i--;
      }
  }
}


void
suiteev::LinkTree(class view  *parent)
{
    class suiteev *self=this;
    register int i = 0;
    register struct suite_item *item = NULL;

    (this)->view::LinkTree( parent);
    if((this)->GetIM()) {
	SetViewColors((class view *) this, SuiteForeground, SuiteBackground);
	if(Items && ITEM(0)) {
	    CheckForNewFirstVisible(this);
	    i = (Items)->Subscript((long) FirstVisible);
	    while(item = ITEM(i++))
		if(Exposed(item)) {
		    if(Active(item)) { 
			if(item_AccessType & suite_ReadWrite)
			    ((class suitecv *) item->viewobject)->LinkTree( this);
			else if(item->viewobject)
			    (item->viewobject)->LinkTree( this);
		    }
		    if(item == LastVisible) 
			break;
		}
	}
    }	
}
