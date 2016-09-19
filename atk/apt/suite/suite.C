/*********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved     *
 *        For full copyright information see:'andrew/doc/COPYRITE'       *
\* ********************************************************************* */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suite.c

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


HISTORY

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("suite.H")
#include <graphic.H>
#include <dataobject.H>
#include <view.H>
#include <bind.H>
#include <im.H>
#include <environ.H>
#include <scroll.H>
#include <cursor.H>
#include <fontdesc.H>
#include <proctable.H>
#include <text.H>
#include <textview.H>
#include <message.H>
#include <sbuttonv.H>
#include <sbutton.H>
#include <apt.H>
#include <vector.H>
#include <region.H>
#include <aptv.H>
#include <color.H>
#include <suite.H>
#include <suitecv.H>
#include <suiteev.H>

#define	CurrentItem		    (self->current_item)
#define Apt			    (self->aptp)
#define Items			    (self->items)
#define ItemWidth		    (self->itemwidth)
#define ItemHeight		    (self->itemheight)
#define ItemFixedWidth		    (self->itemfixedwidth)
#define ItemFixedHeight		    (self->itemfixedheight)
#define SetView			    (self->setview)
#define Bounds			    (self->bounds)
#define Container		    (self->container)
#define ContainerLeft		    (Container.left)
#define ContainerTop		    (Container.top)
#define ContainerWidth		    (Container.width)
#define ContainerHeight		    (Container.height)
#define Scroll			    (self->scroll)
#define ScrollType		    (self->scrolltype)
#define ScrollTop		    (ScrollType & scroll_TOP)
#define ScrollBottom		    (ScrollType & scroll_BOTTOM)
#define ScrollLeft		    (ScrollType & scroll_LEFT)
#define ScrollRight		    (ScrollType & scroll_RIGHT)
#define HScroll			    (ScrollTop || ScrollBottom)
#define VScroll			    (ScrollLeft || ScrollRight)
#define ScrollRect		    (self->scrollrect)
#define TitleCaption		    (self->title_caption)
#define TitleRect		    (self->title_rect)
#define TitleHighlightStyle	    (self->titlehighlightstyle)
#define TitleBorderStyle	    (self->titleborderstyle)
#define	TitleBorderSize		    (self->titlebordersize)
#define TitleFontName		    (self->titlefontname)
#define TitleFont		    (self->titlefont)
#define TitleFontSize		    (self->titlefontsize)
#define TitleFontType		    (self->titlefonttype)
#define TitleForeground		    (self->titlecolor.fg)
#define TitleBackground		    (self->titlecolor.bg)
#define ItemTitleFontName	    (self->itemtitlefontname)
#define ItemTitleFontSize	    (self->itemtitlefontsize)
#define ItemTitleFontType	    (self->itemtitlefonttype)
#define TitleDataObjectName	    (self->titledataobjectname)
#define TitleDataObject		    (self->titledataobject)
#define TitleDataObjectHandler	    (self->titledataobjecthandler)
#define TitlePlacement		    (self->titleplacement)
#define	TitleCaptionAlignment	    (self->titlecaptionalignment)
#define ItemTitlePlacement	    (self->itemtitleplacement)
#define	ItemTitleCaptionAlignment   (self->itemtitlecaptionalignment)
#define TitleHitHandler		    (self->titlehithandler)
#define TitleViewObjectName	    (self->titleviewobjectname)
#define TitleViewObject		    (self->titleviewobject)
#define TitleViewObjectHandler	    (self->titleviewobjecthandler)
#define	TitleHighlighted	    (self->titlehighlighted)
#define ItemOrder		    (self->itemorder)
#define AccessType		    (self->accesstype)
#define Arrangement		    (self->arrangement)
#define	List			    (Arrangement & suite_List)
#define	SingleColumn		    (Arrangement & suite_Column)
#define	SingleRow		    (Arrangement & suite_Row)
#define SelectionMode		    (self->selection_mode)
#define BorderStyle		    (self->borderstyle)
#define BorderSize		    (self->bordersize)
#define CaptionFontName		    (self->captionfontname)
#define CaptionFont		    (self->captionfont)
#define CaptionFontSize		    (self->captionfontsize)
#define CaptionFontType		    (self->captionfonttype)
#define	CaptionAlignment	    (self->captionalignment)
#define ItemBorderStyle		    (self->itemborderstyle)
#define ItemBorderSize		    (self->itembordersize)
#define ItemHighlightStyle	    (self->itemhighlightstyle)
#define ItemPassiveStyle	    (self->itempassivestyle)
#define ItemDataObjectHandler	    (self->itemdataobjecthandler)
#define ItemViewObjectHandler	    (self->itemviewobjecthandler)
#define ItemDataObjectName	    (self->itemdataobjectname)
#define ItemViewObjectName	    (self->itemviewobjectname)
#define HitHandler		    (self->hithandler)
#define	ClientAnchor		    (self->anchor)
#define Datum			    (self->datum)
#define FirstVisible		    (self->firstvisible)
#define NewFirstVisible		    (self->newfirstvisible)
#define LastVisible		    (self->lastvisible)
#define VisibleRows		    (self->visiblerows)
#define VisibleColumns		    (self->visiblecolumns)
#define Rows			    (self->rows)
#define Columns			    (self->columns)
#define NumVisible		    (self->numvisible)
#define Debug			    (self->debug)
#define ExceptionHandler	    (self->exception)
#define ExceptionStatus		    (self->exception_status)
#define ExceptionItem		    (self->exception_item)
#define SortHandler		    (self->sort_handler)
#define SortOrder		    (self->sortorder)


#define	SuiteBackground		    (self->suiteColor.bg)
#define	SuiteForeground		    (self->suiteColor.fg)
#define	SuiteBackgroundName	    ((char *)(self->suiteColor.bg).Name())
#define	SuiteForegroundName	    ((char *)(self->suiteColor.fg).Name())
#define	ActiveItemBackground 	    (self->activeItemColor.bg)
#define	ActiveItemForeground 	    (self->activeItemColor.fg)
#define	ActiveItemBackgroundName    ((char *)(self->activeItemColor.bg).Name())
#define	ActiveItemForegroundName    ((char *)(self->activeItemColor.fg).Name())
#define	PassiveItemBackground 	    (self->passiveItemColor.bg)
#define	PassiveItemForeground 	    (self->passiveItemColor.fg)
#define	PassiveItemBackgroundName   ((char *)(self->passiveItemColor.bg).Name())
#define	PassiveItemForegroundName   ((char *)(self->passiveItemColor.fg).Name())

#define Cursor			    (self->cursor)
#define CursorFont		    (self->cursorfont)
#define CursorByte		    (self->cursorbyte)
#define CursorFontName		    (self->cursorfontname)    
#define ItemCursor		    (self->itemcursor)
#define ItemCursorFont		    (self->itemcursorfont)
#define ItemCursorByte		    (self->itemcursorbyte)
#define ItemCursorFontName	    (self->itemcursorfontname)    
#define ItemArray		    (self->itemarray)
#define YGutterSize		    (self->y_guttersize)
#define XGutterSize		    (self->x_guttersize)
#define TitleMWidth		    (self->title_m_width)
#define CaptionMWidth		    (self->caption_m_width)
#define HasFocus		    (self->has_focus)
#define ITEM(i)			    (struct suite_item *) (Items)->Item(i)
#define SETTRANSFERMODE(v,m)	    if((m) != (((class view*)(v)))->GetTransferMode())\
					(((class view*)(v)))->SetTransferMode((m));
#define SetFont(v,f)		    if((f) != (((class view*)(v)))->GetFont())\
					(((class view*)(v)))->SetFont((f));
#ifndef MAX
#define MAX(a,b)		    (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b)		    (((a)<(b))?(a):(b))
#endif
#define Active(item)		    (item->mode & item_Active)
#define Highlighted(item)	    (item->mode & item_Highlighted)
#define Normalized(item)	    (item->mode & item_Normalized)
#define Exposed(item)		    (self)->ItemExposed(item)
#define	myInsetRect(r,deltax,deltay)	    rectangle_SetRectSize((r), rectangle_Left((r)) + (deltax), rectangle_Top((r)) + (deltay), rectangle_Width((r)) - 2*(deltax), rectangle_Height((r)) - 2*(deltay));
#define	DecrementRect(r,n)	    myInsetRect(r,n,n)
#define DEFAULT_FONT_RANGE	    4
#define SCROLL_WIDTH		    20
#define	NumberLines(str)	     (SetView)->LineCount(str)
#define	WrapStyle		    (self->wrappingstyle)
#define	MaxItemPosGiven		    (self->max_item_pos_given)
#define	IsLinked		    ((self)->IsAncestor((self)->GetIM()))
#define graphicIsMono			(self->mono)



ATKdefineRegistry(suite, aptv, suite::InitializeClass);

static long Within( long  x , long  y , long  left , long  top , long  width , long  height );
static long WithinRect( long  x , long  y, struct rectangle  *r );
static char * strip( char  *str );
static long BreakSorter( long  *item1 , long  *item2 );
static struct suite_item * GenerateItem( class suite  *self, suite_Specification  *spec, char  *name, long  datum );
static void SetMWidths( class suite  *self );
static void SetViewColors( class view  *v, class color  *fg , class color  *bg );
static void DrawTitle( class suite  *self, struct rectangle  *rect );
static void DrawOutline(class suite  *self, struct rectangle  *rect, short  width, unsigned  style, class color  *fg , class color  *bg);
static long TitleSectionWidth( class suite  *self );
static long TitleSectionHeight( class suite  *self, int  newlineHeight );
static void AssignSetAndTitleSpace( class suite  *self, struct rectangle  *title , struct rectangle  *container );
static void PlaceTitle( class suite  *self, struct rectangle  *title_sect , struct rectangle  *title );
static void SetCaptionList( class suite  *self, char  **captions );
static void ParseFontFullName( class suite  *self, char  *fullname , char  *familyName, long  buffSize, long  *size , long  *type );
static void ChangeItemCaption( class suite  *self, struct suite_item  *item, char  *caption );
static struct suite_item* AllocItem();
static void FinalizeItem( struct suite_item  *item );
static void SetSortRoutine( class suite  *self );
static long SortStub( struct suite_item  **item1, struct suite_item  **item2 );
void CheckForNewFirstVisible( class suite  *self );
static long AlphasortAscend( struct suite_item  **item1 , struct suite_item  **item2 );
static long NumericAscend( struct suite_item  **item1 , struct suite_item  **item2 );
static long AlphasortDescend( struct suite_item  **item1 , struct suite_item  **item2 );
static long NumericDescend( struct suite_item  **item1 , struct suite_item  **item2 );
static void AllocNameSpace( char  **target , const char  *source );
static void SetArrangementAttribute( class suite  *self, unsigned long  value );
static void SetBorderStyleAttribute( class suite	 *self, unsigned int *border_style, long  value );
static void SetSuiteAttribute( class suite  *self, long  attribute , long  value     );
static void ChangeSuiteAttribute( class suite  *self, long  attribute , long  value     );
static void SetItemAttribute( class suite  *self, struct suite_item  *item, long  attribute , long  value     );
static void ChangeItemAttribute( class suite  *self, struct suite_item  *item, long  attribute , long  value     );
static void DefaultExceptionHandler( class suite  *self );
static void HandleException( class suite  *self, struct suite_item  *item, long  code );
static void ValidateItem( class suite  *self, struct suite_item  *item );
static void  DrawRectSize(class suite	 *self, long  x , long  y , long  width , long  height);
static void  DrawRect(class suite  *self, struct rectangle  *Rect, int  border_size, class color  *fg , class color  *bg);
static void SetItems(class suite	 *self, char  *elts);
static void AllocColorFromPref( class suite  *self, class color  *color, const char  *prefName , const char  *defaultColorName );
static void AllocateColors( class suite  *self );


static long
Within( long  x , long  y , long  left , long  top , long  width , long  height )
{
  return((x >= left) && (x <= left + width) && (y >= top) && (y <= top + height));
}

static long
WithinRect( long  x , long  y, struct rectangle  *r )
{
  return(Within(x,y,r->left,r->top,r->width,r->height));
}

static char *
strip( char  *str )
{ char *tmp = NULL, *head = NULL;
  if(!str) return(str);
  tmp = head = str;
  while(*str == 040) str++;
  while((*tmp++ = *str++));
  tmp = head + strlen(head) - 1;
  while(*tmp == 040) tmp--;
  *(tmp+1) = '\0';
  return(head);
}

boolean
suite::InitializeClass( )
{
  proctable::DefineProc("suite-set-items", 
			(proctable_fptr)SetItems, &suite_ATKregistry_ ,
			"suite", "Set item list (colon separated list)");
  return(TRUE);
}

static const char suitebutton[] = "suite";

suite::suite( )
{
    class suite *self=this;
	ATKinit;

  IN(suite_InitializeObject);

  graphicIsMono = TRUE;
  if(!(Items = vector::Create(100,3)) || 
     !(SetView = new suiteev) || 
     !(Apt = new apt)) 
    HandleException(this,NULL,suite_InsufficientSpace);
  (this)->SetDataObject(Apt);
  Datum = 0;
  CurrentItem = NULL;
  SetView->parent = this;
  ContainerLeft = ContainerTop = ContainerWidth = ContainerHeight = 0;
  ItemOrder = suite_RowMajor;
  TitleCaption = NULL;
  rectangle_SetRectSize(&TitleRect,0,0,0,0);
  TitleHighlightStyle = suite_Invert;
  TitleBorderStyle = suite_None;
  TitleBorderSize = 4;
  TitleDataObjectName = NULL;
  TitleDataObject = NULL;
  TitleDataObjectHandler = NULL;    
  TitleViewObjectHandler = NULL;    
  TitleHighlighted = FALSE;
  Arrangement = suite_Matrix | suite_Balanced;
  SelectionMode = suite_Exclusive;
  TitlePlacement = suite_Top;
  TitleCaptionAlignment = suite_Middle | suite_Center;
  ItemTitlePlacement = TitlePlacement;
  ItemTitleCaptionAlignment = suite_Middle | suite_Center;
  BorderSize = 4;
  BorderStyle = suite_Rectangle;
  Scroll = NULL;
  rectangle_SetRectSize(&ScrollRect,0,0,0,0);
  ScrollType = 0;
  HitHandler = 0;
  TitleHitHandler = 0;
  TitleViewObjectName = NULL;
  TitleViewObject = NULL;
  TitleFontName = NULL;
  AllocNameSpace(&TitleFontName,"andytype");
  TitleFont = NULL;
  TitleFontSize = 12;
  TitleFontType = fontdesc_Plain;
  ItemTitleFontName = NULL;
  AllocNameSpace(&ItemTitleFontName,"andytype");
  ItemTitleFontSize = 10;
  ItemTitleFontType = fontdesc_Plain;
  ItemViewObjectName = ItemDataObjectName = 0;
  ItemViewObjectHandler = ItemDataObjectHandler = 0;
  CaptionFontName = NULL;
  AllocNameSpace(&CaptionFontName,"andytype");
  CaptionFont = NULL;
  CaptionFontSize = 10;
  CaptionFontType = fontdesc_Plain;
  CaptionAlignment = suite_Middle | suite_Center;
  ItemBorderStyle = suite_Rectangle;
  ItemBorderSize = 4;
  ItemHighlightStyle = suite_Invert;
  ItemPassiveStyle = suite_Pale;
  AccessType = suite_ReadOnly;
  FirstVisible = NewFirstVisible = LastVisible = NULL;
  VisibleRows = Rows = VisibleColumns = Columns = 0; 
  ItemHeight = ItemWidth = ItemFixedWidth = ItemFixedHeight = 0;
  Debug = 1;
  ExceptionHandler = 0;
  ExceptionStatus = 0;
  ExceptionItem = NULL;
  Cursor = NULL;
  CursorFontName = NULL;
  CursorFont = NULL;
  CursorByte = 0;
  ItemCursor = NULL;
  ItemCursorFontName = NULL;
  ItemCursorFont = NULL;
  ItemCursorByte = 0;
  SortHandler = 0;
  SortOrder = 0;
  ClientAnchor = 0;
  XGutterSize = YGutterSize = 1;
  ItemArray = NULL;
  HasFocus = FALSE;
  (this)->SetOptions(aptv_SuppressControl | 
		    aptv_SuppressBorder | 
		    aptv_SuppressEnclosures);
  WrapStyle = suite_LeftIndent;
  MaxItemPosGiven = 0;
  this->buttonprefs = sbutton::GetNewPrefs(suitebutton);
  sbutton::InitPrefs(this->buttonprefs, suitebutton);

  OUT(suite_InitializeObject);
  THROWONFAILURE((TRUE));
}

suite::~suite( )
{
	ATKinit;
	class suite *self=this;
  IN(suite_FinalizeObject);
  (this)->ClearAllItems();
  if(Apt) (Apt)->Destroy();
  if(Scroll) (Scroll)->Destroy();
  if(TitleCaption) free(TitleCaption);
  if(TitleDataObjectName) free(TitleDataObjectName);
  if(TitleViewObjectName) free(TitleViewObjectName);
  if(TitleDataObject) (TitleDataObject)->Destroy();
  if(TitleViewObject) (TitleViewObject)->Destroy();
  if(TitleFontName) free(TitleFontName);
  if(CaptionFontName) free(CaptionFontName);
  if(ItemTitleFontName) free(ItemTitleFontName);
  if(ItemViewObjectName) free(ItemViewObjectName);
  if(ItemDataObjectName) free(ItemDataObjectName);
  OUT(suite_FinalizeObject);
}

void
suite::ReceiveInputFocus( )
{
    class suite *self=this;
  IN(suite_ReceiveInputFocus);
  HasFocus = TRUE;
  (this)->aptv::ReceiveInputFocus();
  OUT(suite_ReceiveInputFocus);
}

void
suite::LoseInputFocus( )
{
    class suite *self=this;
  IN(suite_LoseInputFocus);
  HasFocus = FALSE;
  (this)->aptv::LoseInputFocus();
  OUT(suite_LoseInputFocus);
}

class suite *
suite::Create( const suite_Specification  *suitep, long  anchor )
{
	ATKinit;
 class suite *self = NULL;
  if(!(self = new suite)) 
    HandleException(self,NULL,suite_InsufficientSpace);
  ClientAnchor = anchor;    
  while(suitep && suitep->attribute) {
    ::SetSuiteAttribute(self,suitep->attribute,suitep->value);
    suitep++;
  }
  if(SortHandler)
    if(Items) {
      (Items)->SetSortRoutine((vector_sortfptr)SortStub);
      (Items)->Sort();
    }
  if(TitleDataObject && TitleDataObjectHandler) 
    TitleDataObjectHandler(ClientAnchor,self,NULL,suite_TitleObject);
  if(TitleViewObject && TitleViewObjectHandler)
    TitleViewObjectHandler(ClientAnchor,self,NULL,suite_TitleObject);
  if(Items) {
    FirstVisible = ITEM(0); 
    if(Items->Count()) LastVisible = ITEM((Items)->Count() - 1);
    else LastVisible=ITEM(0);
  }
  OUT(suite_Create);
  return(self);
}

void
suite::DestroyItem( struct suite_item  *item )
{
    class suite *self=this;
  IN(suite_DestroyItem);
  if(Items && item) {
    if(Active(item) && Exposed(item)) {
      item->mode &= ~item_Active;
      if(IsLinked) (this)->HideItem(item);
    }
    (Items)->RemoveItem((long)item);
    FinalizeItem(item);
    item = NULL;
  }
  OUT(suite_DestroyItem);
}

struct suite_item *
suite::CreateItem( char  *name, long  datum )
{
    struct suite_item *item = NULL;
  IN(::CreateItem);
  item = GenerateItem(this,NULL,name,datum);
  OUT(::CreateItem);
  return(item);
}

static long
BreakSorter( long  *item1 , long  *item2 )
{
  if(!item1 || !item2) return(0);
  if(*item1 < *item2) return(-1);
  else if(*item1 == *item2) return(0);
  else return(1);
}


static struct suite_item *
GenerateItem( class suite  *self, suite_Specification  *spec, char  *name, long  datum )
{ struct suite_item *item = NULL;

  IN(GenerateItem);
  if(!(item = AllocItem()))
    HandleException(self,NULL,suite_InsufficientSpace);
  if(!Items) {
    if(!(Items = vector::Create(100,3))) {
      HandleException(self,NULL,suite_InsufficientSpace);
    }
    else {
      if(SortHandler) (Items)->SetSortRoutine((vector_sortfptr)SortStub);
      else if(SortOrder) SetSortRoutine(self);
    }
  }
  if(spec) /* from static declaration */
    while(spec && spec->attribute) {
      ::SetItemAttribute(self,item,spec->attribute,spec->value);
      spec++;
    }
  else {	   /* from CreateItem() */
    if(name) AllocNameSpace(&item->name,name);
    item->datum = datum;
  }
  item->suite = self;
  item->exposed = TRUE;
  item->mode = item_Active | item_Normalized;
  Breaks(item) = vector::Create(5,2);
  (Breaks(item))->SetSortRoutine(BreakSorter);
  (Items)->AddItem((long)item);
  OUT(GenerateItem);
  return(item);
}

static void
SetMWidths( class suite  *self )
{ struct fontdesc_charInfo M_Info;
  IN(SetMWidths);
  if(CaptionFont) {
    (CaptionFont)->CharSummary( (self)->GetDrawable(),'m',&M_Info);
    CaptionMWidth = M_Info.width/2;
  }
  if(TitleFont) {
    (TitleFont)->CharSummary( (self)->GetDrawable(),'m',&M_Info);    
    TitleMWidth = M_Info.width/2;
  }
  OUT(SetMWidths);
}

static void SetViewColors( class view  *v, class color  *fg , class color  *bg )
{
    (v)->SetFGColorCell( fg);
    (v)->SetBGColorCell( bg);
}

void
suite::Update( )
{
    class suite *self=this;
    IN(suite_Update);
    SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
    (SetView)->Clear();
    (this)->WantUpdate( SetView);
    if(Scroll) 
	(this)->WantUpdate( Scroll);
    SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
    OUT(suite_Update);
}

void
suite::FullUpdate( enum view_UpdateType  type, long  left , long  top , long  width , long  height )
{
    class suite *self=this;struct rectangle *curse_rect = NULL, title;

  IN(suite_FullUpdate);
  graphicIsMono = ((this)->GetDrawable())->DisplayClass() & graphic_Monochrome;
  if(type != view_FullRedraw && type != view_LastPartialRedraw) 
      return;
  (this)->GetVisualBounds( &Bounds);
  (this)->GetVisualBounds( &Container);
  curse_rect = rectangle_Duplicate(&Container);
  if(!Cursor) {
    Cursor = cursor::Create(this);
    if(Cursor && CursorFont && (CursorByte != suite_NoCursor)) 
      (Cursor)->SetGlyph( CursorFont, CursorByte);
    else if(CursorByte != suite_NoCursor)
      (Cursor)->SetStandard( CursorByte);
    else (Cursor)->SetStandard( Cursor_Octagon);
  }

  CaptionFont = fontdesc::Create( CaptionFontName,
				CaptionFontType,
				CaptionFontSize );
  TitleFont = fontdesc::Create(TitleFontName,
			      TitleFontType,
			      TitleFontSize );
  SetMWidths(this);
  if(Scroll) (Scroll)->LinkTree( this);
  else (SetView)->LinkTree( this);

  SetMWidths(this);
  DrawOutline(this, &Container, BorderSize, BorderStyle, &SuiteForeground, &SuiteBackground);
  if(TitleCaption || TitleViewObjectName || TitleDataObjectName) {
    AssignSetAndTitleSpace(this, &title, &Container);
    DrawTitle(this, &title);
    if(TitleHighlighted) 
	(this)->HighlightTitle();
  }
  SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
  if(Scroll) {
    ScrollRect.left = ContainerLeft; ScrollRect.top = ContainerTop;
    ScrollRect.width = ContainerWidth; ScrollRect.height = ContainerHeight;
    if(ScrollLeft) {
      curse_rect->left += SCROLL_WIDTH; curse_rect->width -= SCROLL_WIDTH;
      ContainerLeft += SCROLL_WIDTH; ContainerWidth -= SCROLL_WIDTH;
    }
    if(ScrollRight) {
      curse_rect->width -= SCROLL_WIDTH;
      ContainerWidth -= SCROLL_WIDTH;
    }
    if(ScrollTop) {
      curse_rect->top += SCROLL_WIDTH; curse_rect->height -= SCROLL_WIDTH;
      ContainerTop += SCROLL_WIDTH; ContainerHeight -= SCROLL_WIDTH;
    }
    if(ScrollBottom) {
      curse_rect->width -= SCROLL_WIDTH;
      ContainerHeight -= SCROLL_WIDTH;
    }
    SETTRANSFERMODE(this, graphic_COPY);
    (this)->FillRect( &ScrollRect, (this)->WhitePattern());

    (Scroll)->InsertView( this, &ScrollRect);
    SetViewColors((class view *) Scroll, &SuiteForeground, &SuiteBackground);
    (Scroll)->FullUpdate( view_FullRedraw, 0, 0, ScrollRect.width, ScrollRect.height);
  }
  else {
      (SetView)->InsertView( this, &Container);
      SetViewColors((class view *) SetView, &SuiteForeground, &SuiteForeground);
      (SetView)->FullUpdate( view_FullRedraw, 0, 0, ContainerWidth, ContainerHeight);
  }
  if(Cursor) 
      (this)->PostCursor( curse_rect, Cursor);
  if(curse_rect) 
      free(curse_rect);
  SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
  OUT(suite_FullUpdate);
}


static void
DrawTitle( class suite  *self, struct rectangle  *rect )
{ long title_lines = NumberLines(TitleCaption);
  long vert_point = 0, horiz_point = 0, i;
  struct rectangle title;
  char *tmp_title = NULL;
  char *str = NULL, *next_str = NULL, *newline = NULL;
  long align = 0;
  int newlineHeight;

  IN(DrawTitle);
  SetViewColors((class view *) self, &TitleForeground, &TitleBackground);
  SETTRANSFERMODE(self, graphic_COPY);
  (self)->FillRect( rect, (self)->WhitePattern());
  if(TitleCaption && *TitleCaption != (char)0) {
    AllocNameSpace(&tmp_title, TitleCaption);
    str = next_str = tmp_title;
    (self)->SetClippingRect( rect);
    DrawOutline(self, rect, TitleBorderSize, TitleBorderStyle, &TitleForeground, &TitleBackground);
    TitleFont = fontdesc::Create( TitleFontName,
				TitleFontType,
				TitleFontSize );
    PlaceTitle(self, rect, &title);
    newlineHeight = (TitleFont)->FontSummary( (self)->GetDrawable())->newlineHeight;
    horiz_point = title.left + title.width/2;
    vert_point = title.top + newlineHeight/2;
    align = graphic_BETWEENTOPANDBOTTOM;
    if(TitleCaptionAlignment & suite_Left) {
      horiz_point = title.left + 1;
      align |= graphic_ATLEFT;
    }
    else if(TitleCaptionAlignment & suite_Right) {
      horiz_point = title.left + title.width - 1;
      align |= graphic_ATRIGHT;
    }
    else align |= graphic_BETWEENLEFTANDRIGHT;
    if(TitleFont) SetFont(self, TitleFont);
    for( i = 0 ; i < title_lines ; i++ ) {
      if((newline = (char*)strrchr(next_str,'\n'))) {
	next_str = newline + 1;
	*newline = '\0';
      }
      (self)->MoveTo( horiz_point, vert_point);
      (self)->DrawString( str, align);
      vert_point += newlineHeight;
      str = next_str;
    }
    if(tmp_title) free(tmp_title);
 }
  else if(TitleViewObjectName || TitleDataObjectName) {
    if(TitleDataObjectName) {
      if(!TitleDataObject)
	TitleDataObject = 
	    (class dataobject*)ATK::NewObject(TitleDataObjectName);
      if(TitleDataObject && TitleDataObjectHandler) 
        TitleDataObjectHandler(ClientAnchor, self, NULL, suite_TitleObject);
    }
    if(!TitleViewObject) {
      if(!TitleViewObjectName)
	AllocNameSpace(&TitleViewObjectName,
		       (TitleDataObject)->ViewName());
      TitleViewObject = (class view*)ATK::NewObject(TitleViewObjectName);
      if(TitleViewObject && TitleViewObjectHandler)
        TitleViewObjectHandler(ClientAnchor, self, NULL, suite_TitleObject);
    }
    (TitleViewObject)->SetDataObject( TitleDataObject);
    DrawOutline(self, rect, TitleBorderSize, TitleBorderStyle, &TitleForeground, &TitleBackground);

    (TitleViewObject)->LinkTree( self);
    (TitleViewObject)->InsertView( self, rect);
    SetViewColors((class view *) TitleViewObject, &TitleForeground, &TitleBackground);
    (TitleViewObject)->FullUpdate( view_FullRedraw, 0, 0, rect->width, rect->height);
  }
  else {
    (self)->FullUpdate( view_FullRedraw, 0, 0, Bounds.width, Bounds.height);
  }
  (self)->ClearClippingRect();
  OUT(DrawTitle);
}

static void
DrawOutline(class suite  *self, struct rectangle  *rect, short  width, unsigned  style, class color  *fg , class color  *bg)
{ long X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;

  IN(DrawOutline);
  SETTRANSFERMODE(self,graphic_COPY);
  if(style & (suite_Invisible | suite_None)) return;
  else if(style & suite_Line) {
    if(TitlePlacement & suite_Top) {
      X1 = rect->left; Y1 = rect->top + rect->height;
      X2 = X1 + rect->width; Y2 = Y1;
    }
    else if(TitlePlacement & suite_Bottom) {
      X1 = rect->left; Y1 = rect->top;
      X2 = X1 + rect->width; Y2 = Y1;
    }
    else if(TitlePlacement & suite_Left) {
      X1 = rect->left + rect->width; Y1 = rect->top;
      X2 = X1; Y2 = Y1 + rect->height;
    }
    else if(TitlePlacement & suite_Right) {
      X1 = rect->left; Y1 = rect->top;
      X2 = X1; Y2 = Y1 + rect->height;
    }
    (self)->MoveTo(X1,Y1);
    (self)->DrawLineTo(X2,Y2);
  }
  else if(style & suite_Rectangle) { 
      DrawRect(self, rect, width, fg, bg);
      DecrementRect(rect, width);
  }
  OUT(DrawOutline);
}

class view *
suite::Hit( enum view_MouseAction  action, long  x , long  y , long  numberOfClicks )
{
    class suite *self=this;class view *retval = (class view *) this;

  IN(suite_Hit);
  if(Scroll) {
    if(WithinRect(x, y, &ScrollRect))
      retval = (Scroll)->Hit( action, x - ScrollRect.left, y - ScrollRect.top, numberOfClicks);
  }
  else {
    if(WithinRect(x, y, &Container))
      retval = (SetView)->Hit( action, x - ContainerLeft, y - ContainerTop, numberOfClicks);
  }
  if(WithinRect(x, y, &TitleRect)) {
    if(TitleHitHandler) 
      TitleHitHandler(ClientAnchor, this, NULL, suite_TitleObject, action, x, y, numberOfClicks);
    else if(HitHandler)
      HitHandler(ClientAnchor, this, NULL, suite_TitleObject, action, x, y, numberOfClicks);
    else if(TitleViewObject) 
      retval = (TitleViewObject)->Hit( action, x, y, numberOfClicks);
  }
  return(retval);
}

long
suite::Reset( long  state )
{
    class suite *self=this;
    long i = 0, status = 0;
  struct suite_item *item = NULL;
  boolean onScreen = FALSE, doFullRedraw = FALSE;
  boolean doContainerRedraw = FALSE;

  if(!(state & suite_Defer)) {
      state |= suite_Immediate;
  }
  if(state & suite_Clear) {
    (this)->ClearAllItems();
    (this)->SetSuiteAttribute(suite_TitleCaption(NULL));
    doFullRedraw = TRUE;
  }
  if(state & suite_ClearItems) {
    (this)->ClearAllItems();
    doContainerRedraw = TRUE;
  }
  if(state & suite_ClearTitle) {
    (this)->SetSuiteAttribute(suite_TitleCaption(NULL));
    doFullRedraw = TRUE;
  }
  if(state & suite_Activate) {
    if(Items && ITEM(0)) {
      while((item = ITEM(i++))) {
	if(FirstVisible && (FirstVisible == item)) onScreen = TRUE;
  	if(Exposed(item) && !Active(item)) {
	  (this)->ActivateItem(item);
	  if((state & suite_Immediate) && onScreen)
	    (SetView)->ItemNormalize(item);
	}
	else if(Exposed(item) && Highlighted(item)) {
	  if((state & suite_Immediate) && onScreen)
	    (SetView)->ItemNormalize(item);
	  else if((state & suite_Expose) && !Exposed(item)) {
	    item->exposed = TRUE;
	    doContainerRedraw = TRUE;
	  }
	}
	item->mode = ((item_Active | item_Normalized) & ~item_Highlighted);
	if(LastVisible && (LastVisible == item)) onScreen = FALSE;
      }
    }
    else status = -1;
  }
  if(state & suite_Normalize) {
    if(Items && ITEM(0)) {
      while((item = ITEM(i++))) {
	if(FirstVisible && (FirstVisible == item)) onScreen = TRUE;
	if(Exposed(item) && Active(item) && Highlighted(item)) {
	  if((state & suite_Immediate) && onScreen)
	    (SetView)->ItemNormalize(item);
	  else item->mode = ((item_Active | item_Normalized) & 
			     ~item_Highlighted);
	}
	else if((state & suite_Expose) && !Exposed(item)) {
	  item->exposed = TRUE;
	  doContainerRedraw = TRUE;
	}
	if(LastVisible && (LastVisible == item)) onScreen = FALSE;
      }
    }
  }
  if(doFullRedraw && IsLinked) {
      SetViewColors((class view *) SetView, &SuiteForeground, &SuiteBackground);
      (SetView)->Clear();
      (this)->FullUpdate(view_FullRedraw,0,0,0,0);
  }
  else if(doContainerRedraw && IsLinked) {
      SetViewColors((class view *) SetView, &SuiteForeground, &SuiteBackground);
      (SetView)->Clear();
      (SetView)->FullUpdate( view_FullRedraw,
			0, 0, ContainerWidth, ContainerHeight);
      if(Scroll)
	  (Scroll)->Update();
  }
  SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
  return(status);
}

void
suite::ClearAllItems( )
{
    class suite *self=this;
int i = 0, count = 0;
  struct suite_item *item = NULL;

  IN(suite_ClearAllItems);
  if(Items && ITEM(0)) {
    count = (Items)->Count();
    for( i = 0 ; i < count ; i++ ){ 
      item = ITEM(i);
      FinalizeItem(item);
      item = NULL;
    }
    delete Items;
    Items = NULL;
  }
  NewFirstVisible = FirstVisible = LastVisible = NULL;
  OUT(suite_ClearAllItems);
}

static long
TitleSectionWidth( class suite  *self )
  { char *title = NULL, *newline = NULL, *tmp = NULL;
  long numLines = 0;
  long i = 0;
  int XWidth=0, YWidth=0, maxWidth=0;

  IN(TitleSectionWidth);
  if(TitleCaption) {
    tmp = title = strdup(TitleCaption);
    numLines = NumberLines(TitleCaption);
    for (i = 0 ; i < numLines ; i++) {
      if((newline = (char*) strrchr(tmp,'\n'))) {
        *newline = '\0';
	(TitleFont)->StringBoundingBox( (self)->GetDrawable(), tmp, &XWidth, &YWidth);
	tmp = newline + 1;
      }
      else 
	  (TitleFont)->StringBoundingBox( (self)->GetDrawable(), tmp, &XWidth, &YWidth);
      if(XWidth > maxWidth) maxWidth = XWidth;
    }
    free(title);
  }
  else if(TitleViewObjectName || TitleDataObjectName) {
    if(TitlePlacement & (suite_Top | suite_Bottom))
      maxWidth = ContainerWidth;
    else maxWidth = ContainerWidth/3;
  }
  OUT(TitleSectionWidth);
  return(maxWidth);
}

static long
TitleSectionHeight( class suite  *self, int  newlineHeight )
    {
  IN(TitleSectionHeight);
  if(TitleCaption)
      return((NumberLines(TitleCaption) * newlineHeight) + 1);
  else if(TitleViewObjectName || TitleDataObjectName) {
    if(TitlePlacement & (suite_Top | suite_Bottom))
      return(ContainerHeight/3);
    else return(ContainerHeight);
  }
  OUT(TitleSectionHeight);
  return 0;
}

static void
AssignSetAndTitleSpace( class suite  *self, struct rectangle  *title , struct rectangle  *container )
    { int newlineHeight;
  long TitleHeight, TitleWidth;
  
  newlineHeight = (TitleFont)->FontSummary( (self)->GetDrawable())->newlineHeight;
  TitleHeight = TitleSectionHeight(self, newlineHeight);
  TitleWidth = TitleSectionWidth(self);
  switch(TitlePlacement) {
    case suite_Top:
      title->left = container->left;
      title->width = container->width;
      title->top = container->top;
      title->height = TitleHeight + (TitleBorderSize * 2) + TitleMWidth;
      container->top = title->top + title->height;
      container->height -= title->height;
      break;
    case suite_Bottom:
      title->left = container->left;
      title->width = container->width;
      title->height = TitleHeight + (TitleBorderSize * 2) + TitleMWidth;
      title->top = container->top + container->height - title->height;
      container->height -= title->height;
      break;
    case suite_Left:
      title->left = container->left;
      title->top = container->top;
      title->width = TitleWidth + (TitleBorderSize * 2) + TitleMWidth;
      title->height = container->height;
      container->left = title->left + title->width;
      container->width -= title->width;
      break;
    case suite_Right:
      title->top = container->top;
      title->height = container->height;
      title->width = TitleWidth + (TitleBorderSize * 2) + TitleMWidth;
      title->left = container->left + container->width - title->width;
      container->width -= title->width;
      break;
  }
  rectangle_SetRectSize(&TitleRect,title->left,title->top,
	title->width,title->height);
}

static void
PlaceTitle( class suite  *self, struct rectangle  *title_sect , struct rectangle  *title )
    { long Width, Height;
  int newlineHeight;
  unsigned alignment = TitleCaptionAlignment;

  newlineHeight = (TitleFont)->FontSummary((self)->GetDrawable())->newlineHeight;
  Width = TitleSectionWidth(self);
  Height = TitleSectionHeight(self, newlineHeight);
  title->left = title_sect->left + (title_sect->width - Width)/2;
  title->top = title_sect->top + (title_sect->height - Height)/2;
  title->width = Width; title->height = Height;
  if(alignment & suite_Left) { 
      if((title_sect->width - Width) < 0)
	  title->left = title_sect->left + 2;
  }
  else if(alignment & suite_Right) {
    title->left = title_sect->left + (title_sect->width - Width) - 2;
  }
  if(alignment & suite_Top) {
      if((title_sect->height - Height) < 0)
	  title->top = title_sect->top + 2;
  }
  else if(alignment & suite_Bottom) {
    title->top = title_sect->top + (title_sect->height - Height) - 2;
  }
}

static void
SetCaptionList( class suite  *self, char  **captions )
    { char **ptr = captions;

    (self)->ClearAllItems();
    if(ptr && *ptr && **ptr) {
	if(!(Items = vector::Create(100, 3))) 
	    HandleException(self, NULL, suite_InsufficientSpace);
	else {
	    if(SortHandler) 
		(Items)->SetSortRoutine((vector_sortfptr)SortStub);
	    else if(SortOrder) 
		SetSortRoutine(self);
	}
	while(*ptr)
	    if(!((self)->CreateItem( *ptr++, 0)))
		HandleException(self, NULL, suite_InsufficientSpace);
	if(Items && ITEM(0)) {
	    FirstVisible = ITEM(0);
	    LastVisible = ITEM((Items)->Count() - 1);
	}
    }
}

static void
ParseFontFullName( class suite  *self, char  *fullname , char  *familyName, long  buffSize, long  *size , long  *type )
        {
  if(fullname && *fullname)
      fontdesc::ExplodeFontName(fullname, familyName, buffSize, type, size);
}

static void
ChangeItemCaption( class suite  *self, struct suite_item  *item, char  *caption )
      { class text *txt = NULL;
  class suitecv *CV = NULL, *t;

    ValidateItem(self,item);
    AllocNameSpace(&item->caption, caption);
    if(item_AccessType & suite_ReadWrite) {
	(txt = (class text*) item->dataobject)->Clear();
	(txt)->InsertCharacters( 0, item->caption, strlen(item->caption));
	t = (class suitecv *)item->viewobject;
	t->WantUpdate( CV);
	CV = t; /* tjm - FIXME: verify this is what was intended */
    }
    (SetView)->ItemUpdate(item);
}

void
suite::PassivateItem( struct suite_item  *item )
{
    class suite *self=this;
long mode = 0;

  if(item) {
      if(!Active(item))
	  return;
      if(IsLinked && (ItemPassiveStyle & suite_Removed)) 
	  (this)->HideItem( item);
      else {
	  mode = (item->mode & ~item_Active & ~item_Highlighted) | 
	    item_Normalized;
	  if(ItemPassiveStyle & suite_Invisible) 
	      mode |= suite_Invisible;
	  item->mode = mode;
	  if(IsLinked)
	      (SetView)->ItemUpdate( item);
      }
  }
}

void
suite::ActivateItem( struct suite_item  *item )
{
    class suite *self=this;
long mode = 0;

  if(item) {
      if(Active(item)) return;
      if(IsLinked && (ItemPassiveStyle & suite_Removed))
	  (this)->ExposeItem( item);
      else {
	  mode = (item->mode | item_Active | item_Normalized) & 
	    ~item_Highlighted;
	  if(ItemPassiveStyle & suite_Invisible) 
	      mode |= ~suite_Invisible;
	  item->mode = mode;
	  if(IsLinked) {
	      (SetView)->ItemUpdate( item);
	  }
      }
  }
}

static struct suite_item*
AllocItem()
{
  return((struct suite_item*) calloc(1,sizeof(struct suite_item)));
}

static void
FinalizeItem( struct suite_item  *item )
  {
  IN(FinalizeItem);
  if(Breaks(item)) delete Breaks(item);
  if(item->caption) free(item->caption);
  if(item->title) free(item->title);
  if(item->titlefontname) free(item->titlefontname);
  if(item->captionfontname) free(item->captionfontname);
  if(item->viewobjectname) free(item->viewobjectname);
  if(item->dataobjectname) free(item->dataobjectname);
  if(item->viewobject) {
    (item->viewobject)->UnlinkTree();
    (item->viewobject)->Destroy();
    item->viewobject = NULL;
  }
/*===
    if(item->dataobject)	dataobject_Destroy(item->dataobject);
gk5g 5/1/89
===*/
  free(item);
  OUT(FinalizeItem);
}

static void
SetSortRoutine( class suite  *self )
  {
    if(suite_Ascend & SortOrder) {
	if(suite_Alphabetic & SortOrder) 
	    (Items)->SetSortRoutine((vector_sortfptr)AlphasortAscend);
	else if(suite_Numeric & SortOrder)
	    (Items)->SetSortRoutine((vector_sortfptr)NumericAscend);
    }
    else if(suite_Descend & SortOrder) {
	if(suite_Alphabetic & SortOrder)
	    (Items)->SetSortRoutine((vector_sortfptr)AlphasortDescend);
	else if(suite_Numeric & SortOrder)
	    (Items)->SetSortRoutine((vector_sortfptr)NumericDescend);
    }
}

static long
SortStub( struct suite_item  **item1, struct suite_item  **item2 )
    { class suite *self = NULL;
  long status = 0;

  if(item1 && *item1 && item2 && *item2) {
    self = (*item1)->suite;
    if(self)
	status = SortHandler(ClientAnchor, self, *item1, *item2);
  }
  return status;
}

void
CheckForNewFirstVisible( class suite  *self )
    {
    if(NewFirstVisible) {
	FirstVisible = NewFirstVisible;
	NewFirstVisible = NULL;
    }
    else if( !FirstVisible )
	FirstVisible = ITEM(0);
}

void
suite::Sort( unsigned  mode , suite_sortfptr handler)
{
    class suite *self=this;
  IN(suite_Sort);
  SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
  (SetView)->Clear();
  if(Items && ITEM(0)) { 
    if(mode) SortOrder = mode;
    if(handler) {
      SortHandler = (suite_sortfptr)handler;
      (Items)->SetSortRoutine((vector_sortfptr)SortStub);
    }
    else
	SetSortRoutine(this);
    (Items)->Sort();
    CheckForNewFirstVisible(this);
    LastVisible = ITEM((Items)->Count() - 1);
    SetViewColors((class view *) SetView, &SuiteForeground, &SuiteBackground);
    (SetView)->FullUpdate( view_FullRedraw, 0, 0, ContainerWidth, ContainerHeight);
    if(Scroll)
	(Scroll)->Update();
  }
  OUT(suite_Sort);
}

void
suite::Apply( suite_applyfptr proc, long anchor, long  datum )
{
    class suite *self=this;
  int i = 0;
  struct suite_item *item = NULL;
  long status = 0;

  if(Items && ITEM(0))
    while((item = ITEM(i++)) && (status >= 0)) 
      status = proc(anchor, this, item, datum);
}

static long
AlphasortAscend( struct suite_item  **item1 , struct suite_item  **item2 )
  { char *str1 = NULL, *str2 = NULL;
  if(!item1 || !*item1 || !item2 || !*item2) return(0);
  if((*item1)->caption) str1 = (*item1)->caption;
  else str1 = (*item1)->name;
  if((*item2)->caption) str1 = (*item2)->caption;
  else str2 = (*item2)->name;
  return(strcmp(str1, str2));
}

static long
NumericAscend( struct suite_item  **item1 , struct suite_item  **item2 )
  {
  if(!item1 || !*item1 || !item2 || !*item2) return(0);
  if((*item1)->datum > (*item2)->datum) return(1);
  else if((*item1)->datum < (*item2)->datum) return(-1);
  else return(0);
}

static long
AlphasortDescend( struct suite_item  **item1 , struct suite_item  **item2 )
  { char *str1 = NULL, *str2 = NULL;
  if(!item1 || !*item1 || !item2 || !*item2) return(0);
  if((*item1)->caption) str1 = (*item1)->caption;
  else str1 = (*item1)->name;
  if((*item2)->caption) str1 = (*item2)->caption;
  else str2 = (*item2)->name;
  return(-1 * strcmp(str1, str2));
}

static long
NumericDescend( struct suite_item  **item1 , struct suite_item  **item2 )
  {
  if(!item1 || !*item1 || !item2 || !*item2) return(0);
  if((*item1)->datum < (*item2)->datum) return(-1);
  else if((*item1)->datum > (*item2)->datum) return(1);
  else return(0);
}


static void
AllocNameSpace( char  **target , const char  *source )
  {
  if(target && *target) {
    free(*target);
    *target = NULL;
  }
  if(source && *source)
    *target = strdup(source);
  else *target = NULL;
}
    
struct suite_item **
suite::SelectedItems( long  *number )
{
    class suite *self=this;
  struct suite_item *item = NULL;
  int i = 0;
  long count = 0, index = 0;

  if(Items && ITEM(0)) {
      while((item = ITEM(i++)))
	  if(Exposed(item) && Active(item) && Highlighted(item)) 
	      count++;
      (SetView)->AllocItemArray(count);
      if(ItemArray) {
	  i = 0;
	  while((item = ITEM(i++)))
	      if(Exposed(item) && Active(item) && Highlighted(item)) 
		  ItemArray[index++] = item;
      }
  }
  if(number) 
      *number = count;
  return(ItemArray);
}

void
suite::SetDebug( boolean  value )
{
    class suite *self=this;
  this->debug = SetView->debug = value;
}

static void
SetArrangementAttribute( class suite  *self, unsigned long  value )
    {
  if(value & suite_List) 
    Arrangement |= suite_List;
  if(value & suite_Matrix) {
    Arrangement &= (~suite_Row & ~suite_Column);
    Arrangement |= suite_Matrix;
    if(value & suite_Fixed)
	Arrangement |= suite_Fixed;
  }
  if(value & suite_Row) {
    Arrangement &= (~suite_Matrix & ~suite_Column);
    Arrangement |= suite_Row;
  }
  if(value & suite_Column) {
    Arrangement &= (~suite_Matrix & ~suite_Row);
    Arrangement |= suite_Column;
  }
  if(value & suite_Balanced) {
    Arrangement &= ~suite_Unbalanced;
    Arrangement |= suite_Balanced;
  }
  if(value & suite_Unbalanced) {
    Arrangement &= ~suite_Balanced;
    Arrangement |= suite_Unbalanced;
  }
  if(value & suite_RowLine) Arrangement |= suite_RowLine;
  if(value & suite_ColumnLine) Arrangement |= suite_ColumnLine;
}

static void
SetBorderStyleAttribute( class suite	 *self, unsigned int *border_style, long  value )
      {
  if(value & suite_Invisible) {
    *border_style &= ~suite_None;
    *border_style |= suite_Invisible;
  }
  if(value & suite_Rectangle) {
    *border_style &= ~suite_None;
    *border_style |= suite_Rectangle;
  }
  if(value & suite_Line) {
    *border_style &= (~suite_Rectangle & ~suite_None);
    *border_style |= suite_Line;
  }
  if(value & suite_None) {
    *border_style &= ~suite_Rectangle;
    *border_style |= suite_None;
  }
}


static void SetColorDesc(class suite  *self, color *dest, char  *name, unsigned short  red , unsigned short  green , unsigned short  blue)
{
    if(name) *dest=name;
    else *dest=color(red, green, blue);
}

#define suite_SetSuiteFGColor(self, name, r, g, b) \
SetColorDesc(self, &SuiteForeground, name, r, g, b)

#define suite_SetSuiteBGColor(self, name, r, g, b) \
SetColorDesc(self, &SuiteBackground, name, r, g, b)

#define suite_SetTitleFGColor(self, name, r, g, b) \
SetColorDesc(self, &TitleForeground, name, r, g, b)

#define suite_SetTitleBGColor(self, name, r, g, b) \
SetColorDesc(self, &TitleBackground, name, r, g, b)

#define suite_SetActiveItemFGColor(self, name, r, g, b) \
SetColorDesc(self, &ActiveItemForeground, name, r, g, b)

#define suite_SetActiveItemBGColor(self, name, r, g, b) \
SetColorDesc(self, &ActiveItemBackground, name, r, g, b)

#define suite_SetItemBGColor(self, item, name, r, g, b) \
SetColorDesc(self, &item->color.bg, name, r, g, b)

#define suite_SetItemFGColor(self, item, name, r, g, b) \
SetColorDesc(self, &item->color.fg, name, r, g, b)

#define suite_SetPassiveItemFGColor(self, name, r, g, b) \
SetColorDesc(self, &PassiveItemForeground, name, r, g, b)

#define suite_SetPassiveItemBGColor(self, name, r, g, b) \
SetColorDesc(self, &PassiveItemBackground, name, r, g, b)


static void
SetSuiteAttribute( class suite  *self, long  attribute , long  value     )
    { char Name[100], *tmp = NULL;
  long Size, Type;
  IN(SetSuiteAttribute);
  switch(attribute) {
      case suite_rows:
	  Rows = value;
	  break;
      case suite_columns:
	  Columns = value;
	  break;
      case suite_titlecaption:
	  AllocNameSpace(&tmp,(char*)value);
	  AllocNameSpace(&TitleCaption,strip(tmp));
	  if(tmp) free(tmp);
	  break;
      case suite_titlehighlightstyle:
	  TitleHighlightStyle = value;
	  break;
      case suite_titleborderstyle:
	  SetBorderStyleAttribute(self,&TitleBorderStyle,value);
	  break;
      case suite_titlebordersize:
	  TitleBorderSize = value;
	  break;
      case suite_titledataobjectname:
	  AllocNameSpace(&tmp,(char*)value);
	  AllocNameSpace(&TitleDataObjectName,strip(tmp));
	  if(tmp) free(tmp);
	  break;
      case suite_itemorder:
	  ItemOrder = value;
	  break;
      case suite_itemcaptionlist:
	  SetCaptionList(self,(char**)value);
	  break;
      case suite_itemwidth:
	  ItemFixedWidth = value;
	  break;
      case suite_itemheight:
	  ItemFixedHeight = value;
	  break;
      case suite_itemspec:
	  GenerateItem(self,(suite_Specification*)value, 0, 0);
	  break;
      case suite_selectionmode:
	  SelectionMode = (unsigned)value;
	  break;
      case suite_borderstyle:
	  SetBorderStyleAttribute(self,&BorderStyle,value);
	  break;
      case suite_bordersize:
	  BorderSize = value;
	  break;
      case suite_hithandler:
	  self->hithandler = (suite_hitfptr)value;
	  break;
      case suite_arrangement:
	  SetArrangementAttribute(self,value);
	  break;
      case suite_scroll:
	  ScrollType = value;
	  Scroll = scroll::Create(SetView,ScrollType);
	  break;
      case suite_titleplacement:
	  TitlePlacement = value;
	  break;
      case suite_titlecaptionalignment:
	  TitleCaptionAlignment = value;
	  break;
      case suite_titlefontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  AllocNameSpace(&TitleFontName,Name);
	  TitleFontSize = Size;
	  TitleFontType = Type;
	  TitleFont = (self)->BuildFont(tmp,&Size);
	  if(tmp) free(tmp);
	  break;
      case suite_titleviewobjectname:
	  AllocNameSpace(&tmp,(char*)value);
	  AllocNameSpace(&TitleViewObjectName,strip(tmp));
	  if(tmp) free(tmp);
	  break;
      case suite_titleviewobjecthandler:
	  TitleViewObjectHandler = (suite_viewobjectfptr)value;
	  break;
      case suite_titledataobjecthandler:
	  TitleDataObjectHandler = (suite_dataobjectfptr)value;
	  break;
      case suite_itemcaptionalignment:
	  CaptionAlignment = value;
	  break;
      case suite_itemtitlecaptionalignment:
	  ItemTitleCaptionAlignment = value;
	  break;
      case suite_itemtitleplacement:
	  ItemTitlePlacement = value;
	  break;
      case suite_titlehithandler:
	  TitleHitHandler = (suite_hitfptr)value;
	  break;
      case suite_itemcaptionfontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  AllocNameSpace(&CaptionFontName,Name);
	  CaptionFontSize = Size;
	  CaptionFontType = Type;
	  CaptionFont = (self)->BuildFont(tmp,&Size);
	  if(tmp) free(tmp);
	  break;
      case suite_itemtitlefontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  if(tmp) free(tmp);
	  AllocNameSpace(&ItemTitleFontName,Name);
	  ItemTitleFontSize = Size;
	  ItemTitleFontType = Type;
	  break;
      case suite_fontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  AllocNameSpace(&TitleFontName,Name);
	  TitleFontSize = Size;
	  TitleFontType = Type;
	  TitleFont = (self)->BuildFont(tmp,&Size);
	  AllocNameSpace(&ItemTitleFontName,Name);
	  ItemTitleFontSize = Size;
	  ItemTitleFontType = Type;
	  TitleFont = (self)->BuildFont(tmp,&Size);
	  AllocNameSpace(&CaptionFontName,Name);
	  CaptionFontSize = Size;
	  CaptionFontType = Type;
	  CaptionFont = (self)->BuildFont(tmp,&Size);
	  if(tmp) free(tmp);
	  break;
      case suite_itemborderstyle:
	  SetBorderStyleAttribute(self,&ItemBorderStyle,value);
	  break;
      case suite_itembordersize:
	  self->itembordersize = value;
	  break;
      case suite_itemhighlightstyle:
	  ItemHighlightStyle = value;
	  break;
      case suite_itempassivestyle:
	  ItemPassiveStyle = value;
	  break;
      case suite_datum:
	  Datum = value;
	  break;
      case suite_accesstype:
	  AccessType = (unsigned)value;
	  break;
      case suite_itemdataobjectname:
	  AllocNameSpace(&tmp,(char*)value);
	  AllocNameSpace(&ItemDataObjectName,strip(tmp));
	  if(tmp) free(tmp);
	  break;
      case suite_itemdataobjecthandler:
	  ItemDataObjectHandler =(suite_dataobjectfptr)value;
	  break;
      case suite_itemviewobjecthandler:
	  ItemViewObjectHandler = (suite_viewobjectfptr)value;
	  break;
      case suite_itemviewobjectname:
	  AllocNameSpace(&tmp,(char*)value);
	  AllocNameSpace(&ItemViewObjectName,strip(tmp));
	  if(tmp) free(tmp);
	  break;
      case suite_guttersize:
	  YGutterSize = XGutterSize = value;
	  break;
      case suite_verticalguttersize:
	  YGutterSize = value;
	  break;
      case suite_horizontalguttersize:
	  XGutterSize = value;
	  break;
      case suite_sortmode:
	  SortOrder = value;
	  if(Items) SetSortRoutine(self);
	  break;
      case suite_sorthandler:
	  SortHandler = (suite_sortfptr)value;
	  if(Items) (Items)->SetSortRoutine((vector_sortfptr)SortStub);
	  break;
      case suite_cursorfontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  AllocNameSpace(&CursorFontName,Name);
	  CursorFont = (self)->BuildFont(tmp,&Size);
	  if(tmp) free(tmp);
	  if(Cursor) {
	      delete Cursor;
	      Cursor = NULL;
	  }
	  break;
      case suite_cursorbyte:
	  CursorByte = (char) value;
	  if(Cursor) {
	      delete Cursor;
	      Cursor = NULL;
	  }
	  break;
      case suite_itemcursorfontname:
	  AllocNameSpace(&tmp,(char*)value);
	  ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
	  AllocNameSpace(&ItemCursorFontName,Name);
	  ItemCursorFont = (self)->BuildFont(tmp,&Size);
	  if(tmp) free(tmp);
	  if(ItemCursor) {
	      delete ItemCursor;
	      ItemCursor = NULL;
	  }
	  break;
      case suite_itemcursorbyte:
	  ItemCursorByte = (char) value;
	  if(ItemCursor) {
	      delete ItemCursor;
	      ItemCursor = NULL;
	  }
	  break;
      case suite_wrappingstyle:
	  WrapStyle = (short) value;
	  break;
      case suite_foregroundcolor:
	  suite_SetSuiteFGColor(self, (char*)value, 0, 0, 0);
  	  break;
      case suite_backgroundcolor:
	  suite_SetSuiteBGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_titleforegroundcolor:
	  suite_SetTitleFGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_titlebackgroundcolor:
	  suite_SetTitleBGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_activeitemforegroundcolor:
	  suite_SetActiveItemFGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_activeitembackgroundcolor:
	  suite_SetActiveItemBGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_passiveitemforegroundcolor:
	  suite_SetPassiveItemFGColor(self, (char*)value, 0, 0, 0);
	  break;
      case suite_passiveitembackgroundcolor:
	  suite_SetPassiveItemBGColor(self, (char*)value, 0, 0, 0);
	  break;
      default: fprintf( stderr, "Suite: Unknown Suite Attribute (%ld)\n",attribute);
  }
  OUT(SetSuiteAttribute);
}

long
suite::SetSuiteAttribute( long  attribute , long  value )
{
long status = 0;
  ::SetSuiteAttribute(this,attribute,value);
  return(status);
}

static void
ChangeSuiteAttribute( class suite  *self, long  attribute , long  value     )
    {
  struct rectangle *title_rect = rectangle_Duplicate(&TitleRect);

  ::SetSuiteAttribute(self,attribute,value);
  switch(attribute) {
	case suite_titleviewobjectname:
	    if(TitleViewObject) {
		(TitleViewObject)->UnlinkTree();
		(TitleViewObject)->Destroy();
		TitleViewObject = NULL;
	    }
	    DrawTitle(self,title_rect);
	    if(TitleHighlighted)
		(self)->HighlightTitle();
	    break;
	case suite_titledataobjectname:
	    (TitleViewObject)->UnlinkTree();
	    (TitleViewObject)->Destroy();
	    free(TitleViewObjectName);
	    TitleViewObjectName = NULL;
	    (TitleDataObject)->Destroy();
	    TitleDataObject = NULL;
	    TitleViewObject = NULL;
	    DrawTitle(self,title_rect);
	    if(TitleHighlighted)
		(self)->HighlightTitle();
	    break;
	case suite_titleplacement:
	case suite_titlecaptionalignment:
	case suite_titlehighlightstyle:
	case suite_titleborderstyle:
	case suite_titlebordersize:
	case suite_titlecaption:
	    DrawTitle(self,title_rect);
	    if(TitleHighlighted)
		(self)->HighlightTitle();
	    break;
	case suite_itemorder:
	case suite_rows:
	case suite_columns:
	case suite_arrangement:
	case suite_itemcaptionalignment:
	case suite_itemtitleplacement:
	case suite_itemtitlecaptionalignment:
	case suite_itemborderstyle:
	case suite_itempassivestyle:
	case suite_itemcursorfontname:
	case suite_itemcursorbyte:
	case suite_wrappingstyle:
	    SetViewColors((class view *) self, &SuiteForeground, &SuiteBackground);
	    (SetView)->Clear();
	    (SetView)->FullUpdate(view_FullRedraw,0,0,
		ContainerWidth,ContainerHeight);
	    if(Scroll) (Scroll)->Update();
	    break;
	case suite_itemwidth:
	case suite_itemheight:
	case suite_itemcaptionlist:
	case suite_scroll:
	case suite_borderstyle:
	case suite_bordersize:
	case suite_fontname:
	case suite_itemcaptionfontname:
	case suite_itemtitlefontname:
	case suite_itembordersize:
	case suite_itemhighlightstyle:
	case suite_accesstype:
	case suite_itemdataobjectname:
	case suite_itemviewobjectname:
	case suite_guttersize:
        case suite_verticalguttersize:
	case suite_horizontalguttersize:
	case suite_cursorfontname:
	case suite_cursorbyte:
	case suite_titlefontname:
	    SetViewColors((class view *) self, &SuiteForeground, &SuiteBackground);
	    (SetView)->Clear();
	    (self)->FullUpdate(view_FullRedraw,0,0,Bounds.width,Bounds.height);
	    break;
	case suite_sortmode:
	case suite_sorthandler:
	    (self)->Sort(SortOrder,SortHandler);
	    break;
	case suite_titledataobjecthandler:
	case suite_selectionmode:
	case suite_hithandler:
	case suite_titleviewobjecthandler:
	case suite_titlehithandler:
	case suite_datum:
	    break;
	default: fprintf(stderr,"Suite: Unknown Suite Attribute (%ld)\n",attribute);
  }
  
  free(title_rect);
  SetViewColors((class view *) self, &SuiteForeground, &SuiteBackground);
}

long
suite::ChangeSuiteAttribute( long  attribute , long  value )
{
    long status = 0;
    ::ChangeSuiteAttribute(this,attribute,value);
    return(status);
}

long
suite::SuiteAttribute( long  attribute )
{
    class suite *self=this;
    long value = 0;

  switch(attribute) {
      case suite_titlecaption:	    value = (long)TitleCaption;		    break;
      case suite_titlehighlightstyle:	    value = (long)TitleHighlightStyle;	    break;
      case suite_titleborderstyle:	    value = (long)TitleBorderStyle;	    break;
      case suite_titlebordersize:	    value = (long)TitleBorderSize;	    break;
      case suite_titledataobjectname:	    value = (long)TitleDataObjectName;	    break;
      case suite_itemorder:		    value = (long)ItemOrder;		    break;
      case suite_itemwidth:		    value = (long)ItemFixedWidth;	    break;
      case suite_itemheight:		    value = (long)ItemFixedHeight;	    break;
      case suite_selectionmode:	    value = (long)SelectionMode;		    break;
      case suite_borderstyle:		    value = (long)BorderStyle;		    break;
      case suite_bordersize:		    value = (long)BorderSize;		    break;
      case suite_hithandler:		    value = (long)this->hithandler;	    break;
      case suite_arrangement:		    value = (long)Arrangement;		    break;
      case suite_scroll:		    value = (long)Scroll;		    break;
      case suite_titleplacement:	    value = (long)TitlePlacement;	    break;
      case suite_titlecaptionalignment:   value = (long)TitleCaptionAlignment;
    break;
      case suite_fontname:
      case suite_titlefontname:	    value = (long)TitleFontName;	    break;
      case suite_titleviewobject:	    value = (long)TitleViewObject;	    break;
      case suite_titleviewobjectname:	    value = (long)TitleViewObjectName;	    break;
      case suite_titleviewobjecthandler:  value = (long)TitleViewObjectHandler;   break;
      case suite_titledataobjecthandler:  value = (long)TitleDataObjectHandler;   break;
      case suite_titledataobject:	    value = (long)TitleDataObject;	    break;
      case suite_itemcaptionalignment:    value = (long)CaptionAlignment;	    break;
      case suite_itemtitleplacement:	    value = (long)ItemTitlePlacement;	    break;
      case suite_itemtitlecaptionalignment:    value = (long)ItemTitleCaptionAlignment;	  break;
      case suite_titlehithandler:	    value = (long)TitleHitHandler;	    break;
      case suite_itemcaptionfontname:	    value = (long)CaptionFontName;	    break;
      case suite_itemtitlefontname:	    value = (long)ItemTitleFontName;	    break;
      case suite_itemborderstyle:	    value = (long)ItemBorderStyle;	    break;
      case suite_itembordersize:	    value = (long)this->itembordersize;    break;
      case suite_itemhighlightstyle:	    value = (long)ItemHighlightStyle;	    break;
      case suite_itempassivestyle:	    value = (long)ItemPassiveStyle;	    break;
      case suite_datum:		    value = (long)Datum;			    break;
      case suite_accesstype:		    value = (long)AccessType;		    break;
      case suite_itemdataobjectname:	    value = (long)ItemDataObjectName;	    break;
      case suite_itemdataobjecthandler:   value = (long)ItemDataObjectHandler;    break;
      case suite_itemviewobjecthandler:   value = (long)ItemViewObjectHandler;    break;
      case suite_itemviewobjectname:	    value = (long)ItemViewObjectName;	    break;
      case suite_guttersize:
      case suite_verticalguttersize:	    value = (long)YGutterSize;		    break;
      case suite_horizontalguttersize:    value = (long)XGutterSize;		    break;
      case suite_sortmode:		    value = (long)SortOrder;		    break;
      case suite_cursorbyte:		    value = (long)CursorByte;		    break;
      case suite_cursorfontname:	    value = (long)CursorFontName;	    break;
      case suite_wrappingstyle:	    value = (long)WrapStyle;		    break;
      case suite_rows:		    value = (long)Rows;			    break;    
      case suite_columns:		    value = (long)Columns;		    break;    
      default:
	  fprintf(stderr,"Suite: Unknown Suite Attribute (%ld)\n",attribute);
	  break;
  }
  return(value);
}

void
suite::ExposeItem( struct suite_item  *item )
{
    class suite *self=this;
    item->exposed = TRUE;
    if( IsLinked ) {
	SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
	(SetView)->Clear();
	(SetView)->FullUpdate(view_FullRedraw,0,0, ContainerWidth, ContainerHeight);
	if(Scroll) (Scroll)->Update();
    }
}

void
suite::HideItem( struct suite_item  *item )
{
    class suite *self=this;
    item->exposed = FALSE;
    if( IsLinked ) {
	SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
	(SetView)->Clear();
	(SetView)->FullUpdate(view_FullRedraw,0,0, ContainerWidth, ContainerHeight);
	if(Scroll) (Scroll)->Update();
    }
}


boolean
suite::ItemHighlighted( struct suite_item  *item )
{
  if(!item) return(FALSE);
  return(Highlighted(item));
}

long
suite::HighlightItem( struct suite_item  *item )
{
    class suite *self=this;
  long status = 0, i = 0;
  boolean onScreen = FALSE;
  struct suite_item *this_one = NULL;

  if(!Items || !ITEM(0) || !item) return status;
  if(IsLinked) {
    i = (Items)->Subscript((long)FirstVisible);
    if(SelectionMode & suite_Exclusive) 
      (this)->Reset(suite_Normalize);
    while((this_one = ITEM(i++)))
      if(item == this_one) {
	onScreen = TRUE;
	break;
      }
      else if(this_one == LastVisible) 
	break;
    if(Exposed(item) && !Active(item))
      (this)->ActivateItem(item);
  }
  if(onScreen) 
    (SetView)->ItemHighlight(item);
  else 
    item->mode = ((item_Active | item_Highlighted) & ~item_Normalized);
  CurrentItem = item;
  return(status);
}

boolean
suite::ItemNormalized( struct suite_item  *item )
{
  if(!item) return(FALSE);
  return(Normalized(item));
}

long
suite::NormalizeItem( struct suite_item  *item )
{
    class suite *self=this;
    long status = 0, i = 0;
  boolean onScreen = FALSE;
  struct suite_item *this_one = NULL;

  if(!Items || !ITEM(0) || !item) return status;
  if(IsLinked) {
    i = (Items)->Subscript((long)FirstVisible);
    if(SelectionMode & suite_Exclusive) 
      (this)->Reset(suite_Normalize);
    while((this_one = ITEM(i++)))
      if(item == this_one) {
	onScreen = TRUE;
	break;
      }
      else if(this_one == LastVisible) 
	break;
    if(Exposed(item) && !Active(item))
      (this)->ActivateItem(item);
  }
  if(onScreen) 
    (SetView)->ItemNormalize(item);
  else item->mode = ((item_Active | item_Normalized) & ~item_Highlighted);
  CurrentItem = item;
  return(status);
}

boolean
suite::ItemActivated( struct suite_item  *item )
{
  if(!item) return(FALSE);
  return(Active(item));
}

static void
SetItemAttribute( class suite  *self, struct suite_item  *item, long  attribute , long  value     )
      { char Name[101], *tmp = NULL;
      long Type, Size;
  switch(attribute) {
	case suite_itemname:
                AllocNameSpace(&tmp,(char*)value);
	        AllocNameSpace(&item->name,strip(tmp));
	        if(tmp) free(tmp);
	        break;
	case suite_itemposition:
	        MaxItemPosGiven = MAX(MaxItemPosGiven,value);
	        item->position = value;
		break;
	case suite_itemcaption:
                AllocNameSpace(&tmp,(char*)value);
		AllocNameSpace(&item->caption,tmp);
		if(tmp) free(tmp);
		break;
	case suite_itemcaptionfontname:
                AllocNameSpace(&tmp,(char*)value);
		ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
		if(tmp) free(tmp);
		AllocNameSpace(&item->captionfontname,Name);
		item->captionfontsize = Size;
		item->captionfonttype = Type;
		item->captionfont = NULL;
		break;
	case suite_itemtitlecaption:
                AllocNameSpace(&tmp,(char*)value);
		AllocNameSpace(&item->title,strip(tmp));
		if(tmp) free(tmp);
		break;
	case suite_itemtitlefontname:
                AllocNameSpace(&tmp,(char*)value);
		ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
		if(tmp) free(tmp);
		AllocNameSpace(&item->titlefontname,Name);
		item->titlefontsize = Size;
		item->titlefonttype = Type;
		item->titlefont = NULL;
		break;
	case suite_itemtitleplacement:
		item->titleplacement = value;
		break;
	case suite_itemtitlecaptionalignment:
	        item->titlecaptionalignment = value;
	        break;
	case suite_itemcaptionalignment:
	        item->captionalignment = value;
	        break;
	case  suite_accesstype:
		if((item->accesstype = (unsigned)value) == suite_ReadWrite)
		    (SetView)->SetItemToReadWrite(item);
		break;
	case suite_itemviewobjectname:
                AllocNameSpace(&tmp,(char*)value);
		AllocNameSpace(&item->viewobjectname,strip(tmp));
		if(tmp) free(tmp);
	        if(item->viewobject) {
		    (item->viewobject)->UnlinkTree();
		    (item->viewobject)->Destroy();
	        }
		item->viewobject = 
		    (class view*)ATK::NewObject(item_ViewObjectName);
		break;
	case suite_itemdataobjectname:
                AllocNameSpace(&tmp,(char*)value);
		AllocNameSpace(&item->dataobjectname,strip(tmp));
		if(tmp) free(tmp);
	        if(item->dataobject) (item->dataobject)->Destroy();
		item->dataobject = 
		    (class dataobject*)ATK::NewObject(item_DataObjectName);
		break;
	case suite_itemdataobjecthandler:
		item->dataobjecthandler = (suite_dataobjectfptr)value;
		break;
	case suite_itemviewobjecthandler:
		item->viewobjecthandler = (suite_viewobjectfptr)value;
		break;
	case suite_itemhithandler:
		item->hithandler = (suite_hitfptr)value;
		break;
	case suite_itemdatum:
		item->datum = value;
		break;
	case suite_bordersize:
		item->bordersize = (short)value;
		break;
	case suite_itemhighlightstyle:
		item->highlightstyle = value;
		break;
	case suite_itemcursorfontname:
                AllocNameSpace(&tmp,(char*)value);
		ParseFontFullName(self,strip(tmp),Name,100,&Size,&Type);
		AllocNameSpace(&item->cursorfontname,Name);
		item->cursorfont = (self)->BuildFont(tmp,&Size);
		if(tmp) free(tmp);
		if(item->cursor) {
		    delete item->cursor;
		    item->cursor = NULL;
		}
		break;
	case suite_cursorbyte:
		item->cursorbyte = (char) value;
		if(item->cursor) {
		    delete item->cursor;
		    item->cursor = NULL;
		}
		break;
	case suite_itemcursorbyte:
		item->cursorbyte = (char) value;
		if(item->cursor) {
		    delete item->cursor;
		    item->cursor = NULL;
		}
		break;
	    case suite_itemforegroundcolor:
		suite_SetItemFGColor(self, item, (char*)value, 0, 0, 0);
		break;
	    case suite_itembackgroundcolor:
		suite_SetItemBGColor(self, item, (char*)value, 0, 0, 0);
		break;
	default:
		fprintf(stderr, "Suite: Unknown Item Attribute (%ld)\n", attribute);
		break;
  }
}

long
suite::SetItemAttribute( struct suite_item  *item, long  attribute , long  value )
{
long status = 0;
  ::SetItemAttribute(this,item,attribute,value);
  return(status);
}

static void
ChangeItemAttribute( class suite  *self, struct suite_item  *item, long  attribute , long  value     )
      { char *tmp = NULL;
      ::SetItemAttribute(self,item,attribute,value);
  switch(attribute) {
	case suite_itemcaption:
	    AllocNameSpace(&tmp,(char*)value);
	    ChangeItemCaption(self,item,tmp);
	    if(tmp) free(tmp);
	    break;
	case suite_itemdataobjectname:
	    if(item->viewobject) {
		(item->viewobject)->UnlinkTree();
		(item->viewobject)->Destroy();
	    }
	    AllocNameSpace(&item->viewobjectname, (item->dataobject)->ViewName());
	    item->viewobject = 
	      (class view*) ATK::NewObject(item->viewobjectname);
	    (item->viewobject)->SetDataObject( item->dataobject);
	    break;
	case suite_itemviewobjectname:
	    if(item->dataobject)
		(item->viewobject)->SetDataObject( item->dataobject);
	    (SetView)->ItemUpdate( item);
	    break;
	case suite_itemdataobjecthandler: 
	case suite_itemhithandler: 
	case suite_itemdatum:
	case suite_itemviewobjecthandler:    
	    break;
	case suite_itemtitlecaption:
	case suite_itemtitlefontname:
	case suite_itemtitleplacement:
	case suite_itemtitlecaptionalignment:
	    if( item->title ) {
		SetViewColors((class view *) SetView, &SuiteForeground, &SuiteBackground);
		(SetView)->FillRect( &item->title_rect, (SetView)->WhitePattern());
		(SetView)->ItemDrawTitle( item);
	    }
	    break;
	case suite_itemcaptionfontname:
	case suite_itemcaptionalignment:
	case suite_accesstype:
	case suite_itembordersize:
	case suite_itemborderstyle:
	case suite_itemhighlightstyle:
	case suite_itempassivestyle:
	case suite_itemcursorfontname:
	case suite_itemcursorbyte:
	case suite_itemheight:
	case suite_itemwidth:
	case suite_itemposition:
	    (SetView)->ItemUpdate(item);
	    break;
	default:
	    fprintf(stderr,"Suite: Unknown Item Attribute (%ld)\n",attribute);
	    break;
  }
  SetViewColors((class view *) self, &SuiteForeground, &SuiteBackground);
}

long
suite::ChangeItemAttribute( struct suite_item  *item, long  attribute , long  value )
{
  long status = 0;
  ::ChangeItemAttribute(this,item,attribute,value);
  return(status);
}

long
suite::ItemAttribute( struct suite_item  *item, long  attribute )
{
    class suite *self=this;
  long value = 0;

  switch(attribute) {
	case suite_itemposition:
	        if(Items)
		    value = (Items)->Subscript((long)item);
		if(value != -1)	value += 1;		break;
	case suite_itemcaption:
		value = (long) item_Caption;		break;
        case suite_itemname:		    
		value =	(long) item_Name;		break;
	case suite_itemcaptionfontname:
		value = (long) item->captionfontname;	break;
	case suite_itemtitlecaption:
		value = (long) item->title;		break;
	case suite_itemtitlefontname:
		value = (long) item->titlefontname;	break;
	case suite_itemtitleplacement:
		value = (long) item->titleplacement;	break;
	case suite_itemtitlecaptionalignment:
		value =	(long) item_TitleCaptionAlignment;  break;
	case  suite_itemcaptionalignment:
		value = (long) item_CaptionAlignment;	break;
	case  suite_accesstype:
		value = (long) item_AccessType;		break;
	case suite_itemdataobjectname:
		value = (long) item->dataobjectname;	break;
	case suite_itemdataobjecthandler:
		value = (long) item->dataobjecthandler;	break;
	case suite_itemviewobjecthandler:
		value = (long) item->viewobjecthandler;	break;
	case suite_itemviewobjectname:
		value = (long) item->viewobjectname;	break;
	case suite_itemhithandler:
		value = (long) item->hithandler;	break;
	case suite_itemdatum:
		value = (long) item->datum;		break;
	case suite_bordersize:
		value = (long) item->bordersize;	break;
	case suite_itemhighlightstyle:
		value = (long) item->highlightstyle;	break;
	case suite_itemcursorfontname:
		value = (long) item->cursorfontname;	break;
	case suite_itemcursorbyte:
		value = (long) item->cursorbyte;	break;
	default:
		fprintf(stderr,"Suite: Unknown Item Attribute (%ld)\n",attribute);
		break;
  }
  return(value);
}

struct suite_item *
suite::ItemOfDatum( long  datum )
{
    class suite *self=this;
  struct suite_item *item = NULL;
  int i = 0;

  if(Items && ITEM(0))
    while((item = ITEM(i++))) if(datum == (long)item->datum) 
      return(item);
  return(NULL);
}

struct suite_item **
suite::ItemsOfDatum( long  datum )
{
    class suite *self=this;
  int i = 0;
  struct suite_item *item = NULL;
  int count = 0;

  if(Items && ITEM(0)) {
    while((item = ITEM(i++))) if(datum == (long)item->datum) count++;
    (SetView)->AllocItemArray(count);
    i = count = 0;
    while((item = ITEM(i++)))
      if(datum == (long)item->datum) ItemArray[count++] = item;
  }
  else return(NULL);
  return(ItemArray);
}

struct suite_item *
suite::ItemOfName( char  *name )
{
    class suite *self=this;
  struct suite_item *item = NULL;
  int i = 0;
  char *item_name;

  if(Items && ITEM(0))
      while((item = ITEM(i++))) {
	  item_name = (char*) (this)->ItemAttribute( item, suite_itemname);
	  if((!name || !(*name)) && item_name == name)
	      return(item);
	  else if(!strcmp(name, item_name))
	      return(item);
      }
  return(NULL);
}

struct suite_item **
suite::ItemsOfName( char  *name )
{
    class suite *self=this;
  int i = 0, count = 0;
  struct suite_item *item = NULL;
  char *item_name;

  if(Items && ITEM(0)) {
      while((item = ITEM(i++))) {
	  item_name = (char*) (this)->ItemAttribute( item, suite_itemname);
	  if((!name || !(*name)) && item_name == name)
	      count++;
	  else if(!strcmp(name, item_name)) 
	      count++;
      }
    (SetView)->AllocItemArray(count);
    i = count = 0;
    while((item = ITEM(i++))) {
	item_name = (char*) (this)->ItemAttribute( item, suite_itemname);
	if((!name || !(*name)) && item_name == name)
	    ItemArray[count++] = item;
	else if(!strcmp(name, item_name))
	    ItemArray[count++] = item;
    }
  }
  else return(NULL);
  return(ItemArray);
}

struct suite_item *
suite::ItemAtPosition( long  position )
{
    class suite *self=this;
  struct suite_item *item = NULL;
  IN(suite_ItemAtPosition);
  if((position > 0) && (position <= (this)->ItemCount()))
    item = ITEM(position-1);
  OUT(suite_ItemAtPosition);
  return(item);
}

static void
DefaultExceptionHandler( class suite  *self )
  {
  char msg[1000];
  long result;
  static const char * const continue_choice[2] = {"continue", 0};

  sprintf(msg, "Suite: DefaultExceptionHandler:: exception code '%ld' detected.",
	(self)->ExceptionCode() );
  message::MultipleChoiceQuestion(self, 100, msg, 0, &result, continue_choice, NULL);
  if(ExceptionItem) {
    sprintf(msg, "Suite: DefaultExceptionHandler:: exception item caption '%s'.", (char *)(self)->ItemAttribute( ExceptionItem, suite_itemcaption));
    message::MultipleChoiceQuestion(self, 100, msg, 0, &result, continue_choice, NULL);
  }
}

static void
HandleException( class suite  *self, struct suite_item  *item, long  code )
      {
  ExceptionStatus = code;
  ExceptionItem = item;
  if(ExceptionHandler) 
    ExceptionHandler(ClientAnchor,self,item,ExceptionStatus);
  else DefaultExceptionHandler(self);
}

static void
ValidateItem( class suite  *self, struct suite_item  *item )
    {
  if(!item || !Items || 
      (Items && ((Items)->Subscript((unsigned int)(unsigned long)item)) == -1))
    HandleException(self,item,suite_NonExistentItem);
}

void
suite::HighlightTitle( )
{
    class suite *self=this;
  unsigned type = 0;
  struct rectangle *rect = rectangle_Duplicate(&TitleRect);

  if(TitleHighlightStyle & (suite_Bold | suite_Italic)) {
    if(TitleHighlightStyle & suite_Bold)
	type = fontdesc_Bold;
    if(TitleHighlightStyle & suite_Italic)
	type |= fontdesc_Italic;
    TitleFont = fontdesc::Create( TitleFontName, 
				type, TitleFontSize );
    DrawTitle(this,rect);
  }
  else if(TitleHighlightStyle == suite_Invert) {
    DecrementRect(rect,1);
    DrawOutline(this, rect, TitleBorderSize, TitleBorderStyle, &TitleForeground, &TitleBackground);
    SETTRANSFERMODE(this,graphic_INVERT);
    (this)->FillRect( rect, NULL);
  }
  else if(TitleHighlightStyle == suite_Border) {
    DecrementRect(rect,1);
    DrawOutline(this, rect, TitleBorderSize, TitleBorderStyle, &TitleForeground, &TitleBackground);
  }
  TitleHighlighted = TRUE;
}

void
suite::NormalizeTitle( )
{
    class suite *self=this;
  struct rectangle *rect = rectangle_Duplicate(&TitleRect);

  if(TitleHighlightStyle & (suite_Bold | suite_Italic)) {
    TitleFont = fontdesc::Create( TitleFontName,
				TitleFontType,
				TitleFontSize );
    DrawTitle(this, rect);
  }
#if 0
  else if(TitleHighlightStyle == suite_Invert) {
    SETTRANSFERMODE(this, graphic_INVERT);
    (this)->FillRect( rect, NULL);
  }
#endif
  else if(TitleHighlightStyle == suite_Border) {
    DrawTitle(this, rect);
  }
  TitleHighlighted = FALSE;
}

static void 
DrawRectSize(class suite	 *self, long  x , long  y , long  width , long  height)
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
DrawRect(class suite  *self, struct rectangle  *Rect, int  border_size, class color  *fg , class color  *bg)
        {
  struct rectangle *childrect = rectangle_Duplicate(Rect);
  class region *re1 = region::CreateEmptyRegion(), *re2 = region::CreateEmptyRegion();

  if(1 || !graphicIsMono) {
      if(!re1 || !re2) {
	  if(re1) delete re1;
	  if(re2) delete re2;
	  free(childrect);
	  return;
      }
      (self)->SetTransferMode( graphic_COPY);
      DecrementRect(childrect, border_size);
      thebutton.prefs = self->buttonprefs;
      self->buttonprefs->bdepth = border_size;
      self->buttonprefs->colors[sbutton_BACKGROUND] = (char *)(bg)->Name();
      self->buttonprefs->colors[sbutton_FOREGROUND] = (char *)(fg)->Name();
      SetViewColors((class view *) self, fg, bg);
      sbuttonv::DrawButton(self, &thebutton, Rect);
      (re2)->RectRegion( childrect);
      delete re1;
      delete re2;
  }
  else {
      int i = border_size;
      while(i > 0) {
	  DrawRectSize(self, rectangle_Left(Rect), rectangle_Top(Rect), rectangle_Width(Rect), rectangle_Height(Rect));
	  DecrementRect(Rect, 1);
	  i--;
      }
  }
  free(childrect);
}

static void
SetItems(class suite	 *self, char  *elts)
    {
  char *tmp = NULL, *ret = NULL, **captions = NULL;
  char *copy = NULL;
  int count = 1, i = 0;

  IN(SetItems);
  if(elts && *elts) {
    tmp = elts;
    AllocNameSpace(&copy,elts);
    if(!copy) return;
    while((ret = (char*)strchr(tmp,':'))) {
      count++;
      tmp = ++ret;
    }
    if(count>0) {
      tmp = copy;
      if(!(captions = (char**) calloc(count+1,sizeof(char*)))) {
	free(copy);
	return;
      }
      for(i = 0;i<count;i++) {
	captions[i] = tmp;
	if(!(ret = (char*)strchr(tmp,':'))) break;
	*ret = '\0';
	tmp = ++ret;
      } 
      SetCaptionList(self,captions);
    }
    if(copy) free(copy);
  }
  OUT(SetItems);
}

static void
AllocColorFromPref( class suite  *self, class color  *color, const char  *prefName , const char  *defaultColorName )
                {
    const char *prefVal;

    if((prefVal = environ::GetProfile(prefName)) == NULL)
	prefVal = defaultColorName;
    *color = prefVal;
}

#define DEFAULT_FOREGROUND "black"
#define DEFAULT_BACKGROUND "white"

static void
AllocateColors( class suite  *self )
    {
    const char *defaultFG = NULL, *defaultBG = NULL;

    graphic::GetDefaultColors(&defaultFG, &defaultBG);

    AllocColorFromPref(self, &SuiteForeground, "suiteforeground", defaultFG ? defaultFG : DEFAULT_FOREGROUND);
    AllocColorFromPref(self, &SuiteBackground, "suitebackground", defaultBG ? defaultBG : DEFAULT_BACKGROUND);

    AllocColorFromPref(self, &TitleForeground, "suitetitleforeground", SuiteForegroundName /*defaultFG*/);
    AllocColorFromPref(self, &TitleBackground, "suitetitlebackground", SuiteBackgroundName /*defaultBG*/);

    AllocColorFromPref(self, &ActiveItemForeground, "suitebuttonactiveforeground", SuiteForegroundName /*defaultFG*/);
    AllocColorFromPref(self, &ActiveItemBackground, "suitebuttonactivebackground", SuiteBackgroundName /*defaultBG*/);

    AllocColorFromPref(self, &PassiveItemForeground, "suitebuttonpassiveforeground", SuiteForegroundName /*defaultFG*/);
    AllocColorFromPref(self, &PassiveItemBackground, "suitebuttonpassivebackground", SuiteBackgroundName /*defaultBG*/);
}

void
suite::LinkTree(class view  *parent)
{
    class suite *self=this;
    (this)->aptv::LinkTree( parent);
    if((this)->GetIM()) {
	AllocateColors(this);
	SetViewColors((class view *) this, &SuiteForeground, &SuiteBackground);
	if(Scroll)
	    (Scroll)->LinkTree( this);
	else
	    (SetView)->LinkTree( this);
    }
}
