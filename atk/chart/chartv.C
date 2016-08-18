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

#include  <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/chart/RCS/chartv.C,v 1.8 1995/11/07 20:17:10 robr Stab74 $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart View-object

MODULE	chartv.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Chart View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/23/89	Created (TCP)
  05/02/89	Accomodate SUN compiler casting (TCP)
  05/04/89	Add Line chart (TCP)
  05/07/89	Change lpair_VSplit to lpair_SetNth in ReChart (TCP)
  05/10/89	Eliminate FullUpdate after lpair_SetNth (unneeded) (TCP)
  05/21/89	Use sub-window ("Form" class) for Buttons (TCP)
  06/02/89	Announce that Delete feature not yet ready (TCP)
		Add Sort-by-Position to Palette
		Fix new chart creation displaying
  06/08/89	Set Desired dimensions to 300,300 (was 200,200) (TCP)
  07/20/89	Accomodate matrix-class interface change (TCP)
  08/24/89	Upgrade to Suite V1.0 interfaces (TCP)
  08/28/89	Create chartp.c to cope with "too many defines" (TCP)
  09/01/89	Upgrade to V1.0 (TCP)
  09/06/89	Suppress Palette til real (TCP)
  09/07/89	Suppress menu of non-supported Chart types til real (TCP)

END-SPECIFICATION  ************************************************************/


ATK_IMPL("chartv.H")
#include  <errno.h>
#include  <sys/stat.h>
#include  <graphic.H>
#include  <view.H>
#include  <im.H>
#include  <frame.H>
#include  <rect.h>
#include  <chartobj.H>
#include  <menulist.H>
#include  <keymap.H>
#include  <keystate.H>
#include  <bind.H>
#include  <lpair.H>
#include  <text.H>
#include  <textview.H>
#include  <proctable.H>
#include  <message.H>
#include  <suite.H>
#include  <chart.H>
#include  <chartv.H>
#include  <apts.H>
#include  <aptv.H>


static   class menulist	 *class_menulist;
static   class keymap		 *class_keymap;

boolean chartv_debug = 0;
#define debug chartv_debug



ATKdefineRegistry(chartv, aptv, chartv::InitializeClass);
static long SetChartAttribute( register class chartv      *self, register long		       attribute , register long		       value );
static long ChangeChartAttribute( register class chartv      *self, register long		       attribute , register long		       value );
static void Initialize( register class chartv      *self );
void chartv_Add_Command( register class chartv      *self );
void chartv_Delete_Command( register class chartv      *self );
void chartv_ReChart( register class chartv      *self, register char		      *moniker );
void chartv_ReChart_Command( register class chartv      *self, register char		      *moniker );
void chartv_Print_Command( register class chartv      *self );
static void chartv_Sort_Command( register class chartv      *self, register long		       datum );
void chartv_Save_Command( register class chartv      *self );
static void DEBUG_Command( register class chartv      *self );
static void chartv_Palette_Command( register class chartv      *self );
static void chartv_Quit_Command( register class chartv      *self );
static boolean Description_Modified( register class chartv	  *self );
static void Preserve_Description( register class chartv	  *self );


class chartv *
chartv::Create( chartv_Specification		  *specification, char	  *anchor )
        {
	ATKinit;

  register class chartv	 *self;

  IN(chartv_Create);
  if ( (self = new chartv) == NULL )
    {
    fprintf( stderr, "ChartV:  ERROR -- Unable to Create self\n" );
    }
    else
    {
    ClientAnchor = (class view *) anchor;
    while ( specification  &&  specification->attribute )
      {
      ::SetChartAttribute( self, specification->attribute, specification->value );
      specification++;
      }
    }
  OUT(chartv_Create);
  return  self;
  }
  
      
  

static struct bind_Description	    view_menu[] =
  {
  { "chartv-Save", "",	    0,	"Save~50",	0, menu_default,
    (proctable_fptr) chartv_Save_Command,	"Save Database" },
  { "chartv-Add", "",	    0,	"Add~60",	0, menu_default,
    (proctable_fptr) chartv_Add_Command,	"Add Item" },
  { "chartv-Delete", "",   0,	"Delete~61",	0, menu_default,
    (proctable_fptr) chartv_Delete_Command,	"Delete Item" },
  { "chartv-Print", "",    0,	"Print~63",	0, menu_default,
    (proctable_fptr) chartv_Print_Command,	"Print Database" },
  { "chartv-Palette",  "", 0,	"Expose Palette~70",0, menu_palette_hidden,
    (proctable_fptr) chartv_Palette_Command,	"Palette-toggle" },
  { "chartv-Palette",  "", 0,	"Hide Palette~70", 0, menu_palette_exposed,
    (proctable_fptr) chartv_Palette_Command,	"Palette-toggle" },
  { "chartv-Quit",  "",    0,	"Quit~99",	0, menu_default,
    (proctable_fptr) chartv_Quit_Command,	"Quit" },
/*===MUST BE DYNAMIC===*/
  { "chartv-ReChart", "",   0,	"ReChart~10,Histogram~1",
    (long)"Histogram",	menu_default,	(proctable_fptr) chartv_ReChart_Command,"ReChart To Histogram" },
  { "chartv-ReChart", "",   0,	"ReChart~10,Pie~2",
    (long)"Pie",	menu_default, (proctable_fptr) chartv_ReChart_Command,	"ReChart To Pie" },
  { "chartv-ReChart", "",   0,	"ReChart~10,Dot~3",
    (long)"Dot",	menu_default, (proctable_fptr) chartv_ReChart_Command,	"ReChart To Dot" },
  { "chartv-ReChart", "",   0,	"ReChart~10,Line~4",
    (long)"Line",	menu_default, (proctable_fptr) chartv_ReChart_Command,	"ReChart To Line" },
/*===
  { "chartv-ReChart", "",   0,	"ReChart~10,Cartesian~5",
    (long)"Cartesian",	menu_default,	ReChart_Command,"ReChart To Cartesian" },
  { "chartv-ReChart", "",   0,	"ReChart~10,Map~10",
    (long)"Map",	menu_default, ReChart_Command,	"ReChart To Map" },
  { "chartv-ReChart", "",   0,	"ReChart~10,Stack~11",
    (long)"Stack",	menu_default,	ReChart_Command,"ReChart To Stack" },
===*/
  { "chartv-Sort", "",    0,	"Sort~20,Label Ascend~10",
	chart_ByLabel | chart_Ascend, menu_default,    (proctable_fptr) chartv_Sort_Command,	"Sort " },
  { "chartv-Sort", "",    0,	"Sort~20,Label Descend~11",
	chart_ByLabel | chart_Descend, menu_default,   (proctable_fptr) chartv_Sort_Command,	"Sort " },
  { "chartv-Sort", "",    0,	"Sort~20,Value Ascend~20",
	chart_ByValue | chart_Ascend, menu_default,    (proctable_fptr) chartv_Sort_Command,	"Sort " },
  { "chartv-Sort", "",    0,	"Sort~20,Value Descend~21",
	chart_ByValue | chart_Descend, menu_default,   (proctable_fptr) chartv_Sort_Command,	"Sort " },
  { "chartv-Sort", "",    0,	"Sort~20,Position Ascend~30",
	chart_ByPosition | chart_Ascend, menu_default,    (proctable_fptr) chartv_Sort_Command,	"Sort " },
  { "chartv-Sort", "",    0,	"Sort~20,Position Descend~31",
	chart_ByPosition | chart_Descend, menu_default,   (proctable_fptr) chartv_Sort_Command,	"Sort " },
  NULL
  };

boolean
chartv::InitializeClass( )
    {
  IN(chartv_InitializeClass );
  class_menulist = new menulist;
  class_keymap = new keymap;
  bind::BindList( view_menu, class_keymap, class_menulist, &chartv_ATKregistry_  );
  proctable::DefineProc( "chartv-DEBUG", (proctable_fptr) DEBUG_Command, &chartv_ATKregistry_ ,  NULL, "Toggle debug flag.");
  OUT(chartv_InitializeClass );
  return TRUE;
  }

chartv::chartv( )
      {
	ATKinit;
  class chartv *self=this;
  register long		       status = true;

  IN(chartv_InitializeObject);
  DEBUGst(RCSID,rcsidchartv);
  this->instance = (struct chartv_instance *) calloc( 1, sizeof(struct chartv_instance) );
  ReadOnly = true;
/*===*/  ReadOnly = false;
/*===*/  DescriptionExposed = true;
  Menu = (class_menulist)->DuplicateML(  this );
  (Menu)->SetView(  this );
  Keystate = keystate::Create( this, class_keymap );
  (this)->SetShrinkIcon(  'e', "icon12", "Chart", ItemCaptionFontNamePhrase );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  (this)->SetDimensions(  300, 300 );
  (this)->SetOptions(  aptv_SuppressControl |
			   aptv_SuppressBorder |
			   aptv_SuppressEnclosures );
  if ( status == true )
    {
    Description = new text;
    DescriptionView = new textview;
    (DescriptionView)->SetDataObject(  Description );
    DescriptionViewScroll = (DescriptionView )->GetApplicationLayer( );
    (Description)->AddObserver(  this );
    }
  if ( status == true )
    { DEBUG(Created Description);
    PairView = new lpair;
    }
  OUT(chartv_InitializeObject);
  THROWONFAILURE(  status);
  }

extern void Destroy_Palette(class chartv *);

chartv::~chartv( )
      {
	ATKinit;
  class chartv *self=this;

  IN(chartv_FinalizeObject);
  if ( this->instance )
    {
    if ( ChartViewer )	(ChartViewer )->Destroy();
    if ( Menu )		delete Menu ;
    if ( Keystate )	delete Keystate ;
    Destroy_Palette( this );
    free( this->instance );
    }
  OUT(chartv_FinalizeObject);
  }

class view *
chartv::GetApplicationLayer( )
    {
  class chartv *self=this;
  IN(chartv_GetApplicationLayer);
  ApplicationLayer = true;
  if ( Menu )
    {
    (Menu)->SetMask(  menu_default | menu_applicationlayer );
    (this)->PostMenus(  Menu );
    }
  OUT(chartv_GetApplicationLayer);
  return  (class view *) this;
  }

void
chartv::DeleteApplicationLayer( register class view	      *view )
      {
  class chartv *self=this;

  IN(chartv_DeleteApplicationLayer);
  ApplicationLayer = false;
  if ( Menu )
    {
    (Menu)->SetMask(  menu_default );
    (this)->PostMenus(  Menu );
    }
  OUT(chartv_DeleteApplicationLayer);
  }

void
chartv::SetDataObject( register class dataobject	       *data_object )
      {
  class chartv *self=this;
  IN(chartv_SetDataObject);
  Chart = (class chart *) data_object;
  (this)->aptv::SetDataObject(  Chart );
  if ( ChartViewer )
    (ChartViewer)->SetDataObject(  Chart );
  DEBUGst(ChartType,chart_Type( Chart ));
  OUT(chartv_SetDataObject);
  }

void
chartv::ReceiveInputFocus( )
    {
  class chartv *self=this;
  IN(chartv_ReceiveInputFocus);
  
  InputFocus = true;
  if ( Keystate )
    {
    Keystate->next = NULL;
    (this)->PostKeyState(  Keystate );
    }
  if ( Menu )
    {
    (Menu)->SetMask(  menu_default | 
	((PaletteExposed) ? menu_palette_exposed : menu_palette_hidden) |
	((Application) ? menu_application : 0) );
    if(PaletteExposed) 
	((class view *)Palette)->PostMenus(  Menu );
    (this)->PostMenus(  Menu );
    }
  (this)->SetTransferMode(  graphic_BLACK );
  (this)->DrawRectSize(  Left, Top, Width-1, Height-1 );
  OUT(chartv_ReceiveInputFocus);
  }

void
chartv::LoseInputFocus( )
    {
  class chartv *self=this;
  IN(chartv_LoseInputFocus);
  InputFocus = false; 
  if ( ! IgnoreLoseInputFocus )
    { DEBUG(Do Not Ignore);
    (this)->SetTransferMode(  graphic_WHITE );
    (this)->DrawRectSize(  Left, Top, Width-1, Height-1 );
    }
  OUT(chartv_LoseInputFocus);
  }

long
chartv::SetChartAttribute( long attribute, long value )
  {  return  ::SetChartAttribute( this, attribute, value );  }

static
long SetChartAttribute( register class chartv      *self, register long		       attribute , register long		       value )
      {
  register long		      status = ok;

  IN(::SetChartAttribute);
  switch ( attribute )
    {
    case  chartv_arrangement:
      Arrangement = value;			break;
    case  chartv_backgroundshade:
      BackgroundShade = value;			break;
    case  chartv_borderstyle:
      BorderStyle = value;			break;
    case  chartv_bordersize:
      BorderSize = value;			break;
    case  chartv_cursor:
      ChartCursorByte = value;			break;
    case  chartv_cursorfontname:
      apts::CaptureString( (char *) value, &ChartCursorFontName ); break;
    case  chartv_datum:
      ClientDatum = value;			break;
    case  chartv_hithandler:
      HitHandler = (class view *(*)()) value;	break;
    case  chartv_itemborderstyle:
      ItemBorderStyle = value;			break;
    case  chartv_itembordersize:
      ItemBorderSize = value;			break;
    case  chartv_itemhighlightstyle:
      ItemHighlightStyle = value;		break;
    case  chartv_labelfontname:
      apts::CaptureString( (char *) value, &LabelFontName ); break;
    case  chartv_scalefontname:
      apts::CaptureString( (char *) value, &ScaleFontName ); break;
    case  chartv_titleborderstyle:
      TitleBorderStyle = value;			break;
    case  chartv_titlebordersize:
      TitleBorderSize = value;			break;
    case  chartv_titlecaptionfontname:
      apts::CaptureString( (char *) value, &TitleFontName ); break;
    case  chartv_titledataobjecthandler:
      TitleDataObjectHandler = (class view (*)()) value;break;
    case  chartv_titlehighlightstyle:
      TitleHighlightStyle = value;		break;
    case  chartv_titlehithandler:
      TitleHitHandler = (class view (*)()) value;break;
    case  chartv_titleplacement:
      TitlePlacement = value;			break;
    case  chartv_titleviewobjecthandler:
      TitleViewObjectHandler = (class view (*)()) value;break;

    default:
      fprintf( stderr, "ChartV: Unrecognized ChartAttribute (%ld) -- Ignored\n", attribute );
    }

  OUT(::SetChartAttribute);
  return  status;
  }

long
chartv::ChangeChartAttribute( long attribute, long value )
  {  return  ::ChangeChartAttribute( this, attribute, value );  }

static
long ChangeChartAttribute( register class chartv      *self, register long		       attribute , register long		       value )
      {
  register long		      status = ok;

  IN(::ChangeChartAttribute);
  if ( (status = ::SetChartAttribute( self, attribute, value )) == ok )
    {
/*===*/
    }
  OUT(::ChangeChartAttribute);
  return  status;
  }

long
chartv::ChartAttribute( register long		       attribute )
      {
  register long		      value = 0;

/*===*/
  return  value;
  }

struct chart_item *
chartv::CurrentItem( )
    {
  class chartv *self=this;
  register struct chart_item *item = NULL;

  if ( ChartViewer )
    item = (ChartViewer )->CurrentItem( );
  return  item;
  }

void
chartv::SetDebug( boolean		        state )
      {
  class chartv *self=this;
  IN(chartv_SetDebug);
  chartv_debug = state;
  if ( Chart )		(Chart)->SetDebug(  chartv_debug );
  if ( ChartViewer )	(ChartViewer)->SetDebug(  chartv_debug );
  OUT(chartv_SetDebug);
  }

void 
chartv::FullUpdate( register enum view_UpdateType	   type, register long			   left , register long			   top , register long			   width , register long			   height )
        {
  class chartv *self=this;
  IN(chartv_FullUpdate);
  if ( (!IgnoreFullUpdate)  &&  Chart  &&
       (type == view_FullRedraw || type == view_LastPartialRedraw) )
    {
    (this)->aptv::FullUpdate(  type, left, top, width, height );
    (this )->ClearClippingRect( );
    (this)->GetLogicalBounds(  BOUNDS );
    if ( InputFocus )
      (this)->DrawRectSize(  Left, Top, Width-1, Height-1 );
    if ( ! (this)->BypassUpdate() )
      { DEBUG(Not Bypassed);
      if ( ! Initialized )  Initialize( this );
      (this )->ClearBody( );
      if ( ChartViewer )
	{ DEBUG(ChartViewer Exists);
	(PairView)->InsertViewSize(  this,
	    (this)->BodyLeft()+4,  (this)->BodyTop()+4,
	    (this)->BodyWidth()-8, ((this)->BodyHeight() - 8) );
        (PairView)->FullUpdate(  type, 0,0,
	    (this)->BodyWidth()-8, ((this)->BodyHeight() - 8) );
	(PairView)->GetEnclosedBounds(  PairBounds );
	}
      }
    }
  IgnoreFullUpdate = false;
  OUT(chartv_FullUpdate);
  }

static
void Initialize( register class chartv      *self )
    {
  register char		     *moniker = NULL;

  IN(Initialize);
  if ( Chart )
    {
    moniker = (char *) (Chart)->ChartAttribute(  chart_type );
    DEBUGst(Moniker,moniker);
    DEBUGst(Chart-module-name,(Chart)->ModuleName(  moniker ));
    }
  if ( moniker )
    {
    if ( ChartViewer = (class chartobj *)
	ATK::NewObject( (Chart)->ModuleName(  moniker ) ) )
      { DEBUG(Created);
      Initialized = true;
      (ChartViewer)->SetDebug(  chartv_debug );
      (ChartViewer)->SetDataObject(  Chart );
      (PairView)->VSplit(  ChartViewer, DescriptionViewScroll, 0, 100 );
      }
    }
  OUT(Initialize);
  }

class view *
chartv::Hit( register enum view_MouseAction    action, register long			    x , register long			    y , register long			    clicks )
        {
  class chartv *self=this;
  register class view		  *hit;

  IN(chartv_Hit );
  (this)->Announce(  "" );
  if ( (hit = (this)->aptv::Hit(  action, x, y, clicks )) == NULL )
    { DEBUG(Accept ::Hit);
    if ( ChartViewer  &&  (this)->Within(  x, y, PairBounds ) )
      { DEBUG(Pair ::Hit);
      if ( !InputFocus  &&  action == view_LeftDown )
        (this)->WantInputFocus(  this );
      hit = (class view *) (PairView)->Hit(  action,
	    (PairView)->EnclosedXToLocalX(  x ),
	    (PairView)->EnclosedYToLocalY(  y ), clicks );
      }
    }
  OUT(chartv_Hit );
  return  hit;
  }

void
chartv::Print( register FILE		      *file, register char		      *processor, register char		      *format, register boolean	       level )
            {
  class chartv *self=this;
  IN(chartv_Print);
  if ( ChartViewer )
    (ChartViewer)->Print(  file, processor, format, level );
  OUT(chartv_Print);
  }

void
chartv::AddItemCmd()
{
    chartv_Add_Command(this);
}

void
chartv::SaveCmd()
{
    chartv_Save_Command(this);
}

void
chartv::DeleteItemCmd()
{
    chartv_Delete_Command(this);
}

void
chartv::TogglePaletteCmd()
{
    chartv_Palette_Command(this);
}

void
chartv::QuitCmd()
{
    chartv_Quit_Command(this);
}

void
chartv::ReChartCmd(char *moniker)
{
    chartv_ReChart_Command(this, moniker);
}

void
chartv::SortCmd(long datum)
{
    chartv_Sort_Command(this, datum);
}

void
chartv::PrintCmd()
{
    chartv_Print_Command(this);
}

void
chartv_Add_Command( register class chartv      *self )
    {
  char			     *reply;
  register struct chart_item *item;

  IN(Add_Command);
/*===*/
  while ( true )
    {
    (self)->Query(  "Enter Name: ", "", &reply );
    if ( reply == NULL  ||  *reply == 0 )
      break;
    if ( item = (Chart)->CreateItem(  reply, 0 ) )
      {
      (self)->Query(  "Enter Value: ", "", &reply );
      (self)->Announce(  "" );
      if ( reply == NULL  ||  *reply == 0 )
        break;
      (Chart)->SetItemAttribute(  item, chart_ItemValue(atoi( reply )) );
      (Chart)->NotifyObservers(  chart_ItemsSorted/*===*/ );
      }
      else
      {
/*===*/
      }
    }
/*===*/
  (Chart )->SetModified( );
  if( PaletteInitialized )
      (ControlSuite)->Reset(  suite_Normalize );
  OUT(Add_Command);
  }

void
chartv_Delete_Command( register class chartv      *self )
    {
  IN(Delete_Command);
  if ( (ChartViewer )->CurrentItem( ) )
    { DEBUG(::CurrentItem Exists);
    (Chart)->DestroyItem(  (ChartViewer )->CurrentItem( ) );
    (ChartViewer )->SetCurrentItem((struct chart_item *) 0 );
    (Chart)->NotifyObservers(  chart_ItemDestroyed );
    (Chart )->SetModified( );
    }
  if( PaletteInitialized )
      (ControlSuite)->Reset(  suite_Normalize );
  OUT(Delete_Command);
  }


extern void Activate_Viewer(class chartv *);

void chartv_ReChart( register class chartv      *self, register char		      *moniker )
      {
  struct rectangle	      bounds;
  register class chartobj   *prior_viewer = ChartViewer;

  IN(ReChart);
  DEBUGst(Moniker,moniker);
  if ( moniker  &&  *moniker )
    {
    (Chart)->SetChartAttribute(  chart_Type(moniker) );
    bounds.left = 0;
    bounds.top = 0;
    bounds.width = Width;
    bounds.height = Height;
    if ( prior_viewer )
      (ChartViewer)->GetEnclosedBounds(  &bounds );
    if ( ChartViewer = (class chartobj *)
	ATK::NewObject( (Chart)->ModuleName(  moniker ) ) )
      { DEBUGst(Created,moniker);
      (ChartViewer)->SetDebug(  chartv_debug );
      (ChartViewer)->SetDataObject(  Chart );
      if ( prior_viewer )
	{ DEBUG(Prior Viewer);
	(self)->SetTransferMode(  graphic_WHITE );
	(self)->FillRect(  &bounds, graphic_WHITE );
	(self)->SetTransferMode(  graphic_BLACK );
	}
	else
	{ DEBUG(No Prior Viewer);
        Initialized = true;
        (PairView)->VSplit(  ChartViewer, DescriptionViewScroll, 0, 100 );
	(PairView)->InsertViewSize(  self,
	    (self)->BodyLeft()+4,  (self)->BodyTop()+4,
	    (self)->BodyWidth()-8, ((self)->BodyHeight() - 8) );
        (PairView)->FullUpdate(  view_FullRedraw, 0,0,
	    (self)->BodyWidth()-8, ((self)->BodyHeight() - 8) );
	(PairView)->GetEnclosedBounds(  PairBounds );
	Activate_Viewer( self );
	}
      (PairView)->SetNth(  0, ChartViewer );
      if ( prior_viewer )
	{ DEBUG(Destroy Prior Viewer);
	(Chart)->RemoveObserver(  prior_viewer );
	(prior_viewer )->Destroy();
	}
      }
    }
  OUT(ReChart);
  }
void
chartv_ReChart_Command( register class chartv      *self, register char		      *moniker )
      {
  IN(ReChart_Command);
  DEBUGst(moniker,moniker);
  chartv_ReChart( self, moniker );
  OUT(ReChart_Command);
  }

void
chartv_Print_Command( register class chartv      *self )
    {
  register FILE		     *file;
  char			      msg[512], *chart_file_name;
  static char		      file_name[] = "/tmp/chart_print.PS";

  IN(Print_Command);
  (self )->UseWaitCursor( );
  chart_file_name = (char *) (Chart)->ChartAttribute(  chart_filename );
  sprintf( msg, "Printing '%s' ...", chart_file_name );
  (self)->Announce(  "Printing ..." );
  if ( file = fopen( file_name, "w" ) )
    {
    (ChartViewer)->Print(  file, "PostScript", "PostScript", 1 );
    fclose( file );
    sprintf( msg, "print -Tnative %s", file_name );
    system( msg );
    sprintf( msg, "Printed '%s'", chart_file_name );
    (self)->Announce(  msg );
    }
    else (self)->Announce(  "Error Printing" );
  (self )->UseNormalCursor( );
  if( PaletteInitialized )
      (ControlSuite)->Reset(  suite_Normalize );
  OUT(Print_Command);
  }

static void
chartv_Sort_Command( register class chartv      *self, register long		       datum )
      {
  IN(chartv_Sort_Command);
  (Chart)->Sort(  datum, NULL );
  (Chart)->NotifyObservers(  chart_ItemsSorted );
  OUT(chartv_Sort_Command);
  }

void
chartv_Save_Command( register class chartv      *self )
    {
  char			      msg[512],
			      original_name[512], backup_name[512];
  char			     *file_name;
  register FILE		     *file;
  register long		      serial = 1, status = ok;
  struct stat		      st;

  IN(Save_Command);
  if ( (Chart)->ChartAttribute(  chart_filename ) ==  0 )
    { DEBUG(Need FileName);
    (self)->QueryFileName(  "Enter FileName: ", &file_name );
    (self)->Announce(  "" );
    (Chart)->SetChartAttribute(  chart_FileName( file_name ) );
    }
  if ( Description_Modified( self ) )
    Preserve_Description( self );
  file_name = (char *) (Chart)->ChartAttribute(  chart_filename );
  if ( file_name )
    {
    (self )->UseWaitCursor( );
    sprintf( original_name, "%s", file_name );
    DEBUGst(Original-name,original_name);
    (self)->Announce(  "Saving ..." );
    sprintf( backup_name, "%s.BACKUP", file_name );
    if ( stat( original_name, &st ) == 0 )
      { DEBUG(Existent File);
      while ( ! stat( backup_name, &st ) )
        sprintf( backup_name, "%s.BACKUP.%ld", file_name, serial++ );
      DEBUGst(Backup-name,backup_name);
      if ( rename( original_name, backup_name ) )
        { DEBUG(ReName Failure);
        sprintf( msg, "Unable to Create Backup for '%s'", file_name );
        (self)->Announce(  msg );
	status = failure;
        }
      }
    if ( status == ok )
      {
      if ( file = fopen( file_name, "w" ) )
        { DEBUG(File Opened);
        (Chart)->Write(  file, im::GetWriteID()+1/*===*/, 0 );
        fclose( file );
        sprintf( msg, "Wrote '%s'", file_name );
        (self)->Announce(  msg );
        ((self )->GetIM( ))->SetTitle(  file_name );
        LastModified = (Chart )->GetModified( );
        }
        else
        { DEBUG(File Open Failed);
        sprintf( msg, "Unable to Open '%s' (%s)", file_name, strerror(errno) );
        (self)->Announce(  msg );
        (Chart)->SetChartAttribute(  chart_FileName( NULL ) );
        }
      }
    (self )->UseNormalCursor( );
    }
  if( PaletteInitialized )
      (ControlSuite)->Reset(  suite_Normalize );
  OUT(Save_Command);
  }

static void
DEBUG_Command( register class chartv      *self )
    {
  IN(DEBUG_Command);
  (self)->SetDebug(  !chartv_debug );
  (Chart)->SetDebug(  chartv_debug );
  OUT(DEBUG_Command);
  }

extern void Hide_Palette(class chartv *);
extern void Expose_Palette(class chartv *);

static void
chartv_Palette_Command( register class chartv      *self )
    {
  IN(Palette_Command);
  if ( PaletteExposed )
    Hide_Palette( self );
    else {
	Expose_Palette( self );
	((class view *) Palette)->PostMenus( Menu);
    }
  OUT(Palette_Command);
  }

static void
chartv_Quit_Command( register class chartv      *self )
    {
  static char		     *choices[] =
		{"Cancel", "Save", "Save & Quit", "Quit Anyway", 0};
  long			      response = 0;

  IN(chartv_Quit_Command);
  Description_Modified( self );
  if ( ((Chart )->GetModified( ) > LastModified) )
    {
    message::MultipleChoiceQuestion(
	    self, 0, "Outstanding Modifications:", 0, &response, choices, NULL );
    DEBUGdt(Response,response);
    switch ( response )
      {
      case 0:	  break;
      case 1:	  chartv_Save_Command( self );  break;
      case 2:	  chartv_Save_Command( self );
      case 3:	  exit(0);
      default:    break;
      }
    }
    else  exit(0);
  OUT(chartv_Quit_Command);
  }

static
boolean Description_Modified( register class chartv	  *self )
    {
  register boolean		  status = false;

  IN(Description_Modified);
  if ( (Description )->GetModified( ) != DescriptionLastModified )
    {
    DescriptionLastModified = (Description )->GetModified( );
    status = true;
    (Chart )->SetModified( );
    }
  OUT(Description_Modified);
  return  status;
  }

static
void Preserve_Description( register class chartv	  *self )
    {
  register FILE			 *file;
  struct stat			  st;
  register char			 *buffer = NULL;

  IN(Preserve_Description);
  file = fopen( temp_name, "w" );
  (Description)->Write(  file, im::GetWriteID(), 1 );
  fclose( file );
  stat( temp_name, &st );
  buffer = (char *) malloc( st.st_size + 2 );
  file = fopen( temp_name, "r" );
  fread( buffer, st.st_size + 1, st.st_size, file );
  fclose( file );
/*===*/
  OUT(Preserve_Description);
  }


void
chartv::LinkTree(class view *parent )
{
    class chartv *self=this;
    this->aptv::LinkTree(parent);
    if(this->GetIM()) {
	PairView->LinkTree(this);
    }
}
