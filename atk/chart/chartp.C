/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart View-object Palette

MODULE	chartp.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Chart View-object Palette.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/28/89	Created (TCP)
  08/30/89	Remove redundant matrix.ih include (TCP)
  08/31/89	Change static long to long for VAX compiler (TCP)
  09/01/89	Upgrade Suite attribute spellings (TCP)
  09/14/89	Suppress references to un-released Matrix class (TCP)

END-SPECIFICATION  ************************************************************/

#include  <andrewos.h>
#include  <view.H>
#include  <im.H>
#include  <frame.H>
#include  <menulist.H>
#include  <keymap.H>
#include  <keystate.H>
#include  <message.H>
#include  <apt.H>
#include  <suite.H>
#include  <chart.H>
#include  <chartv.H>

#include "chart.h"

#define  add_code		    1
#define  delete_code		    2
#define  sort_ascend_value_code	    3
#define  sort_descend_value_code    4
#define  sort_ascend_label_code	    5
#define  sort_descend_label_code    6
#define  sort_ascend_position_code  7
#define  sort_descend_position_code 8
#define  histogram_code		   10 /*=== Must be dynamic===*/
#define  dot_code		   11
#define  pie_code		   12
/*#define  map_code		   13*/
#define  stack_code		   14
#define  cartesian_code		   15
#define  line_code		   16
#define  print_code		   20
#define  save_code		   21
#define  top_title_code		   22
#define  bottom_title_code	   23
#define  left_title_code	   24
#define  right_title_code	   25

static long Initialize_Palette( class chartv	  *self );
NO_DLL_EXPORT void Destroy_Palette( class chartv	   *self );
NO_DLL_EXPORT void Expose_Palette( class chartv	   *self );
NO_DLL_EXPORT void Hide_Palette( class chartv	   *self );
static class view * Palette_Hit( class chartv	   *self, class suite		   *suite, struct suite_item	   *item, long			    type, enum view_MouseAction    action, long			    x , long			    y , long			    clicks );
static long Palette_Titles_Handler( class chartv	   *self, class suite		   *suite, struct suite_item	   *item, long			    action );
NO_DLL_EXPORT void Activate_Viewer( class chartv	  *self );
static void Activate( class chartv	  *self, long			   code );

static suite_Specification		add_button[] =
  {
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_ItemCaption("Add"),
  suite_ItemDatum(add_code),
  0
  };

static suite_Specification		delete_button[] =
  {
  suite_ItemCaption("Delete"),
  suite_ItemDatum(delete_code),
  0
  };

static suite_Specification		histogram_button[] =
  {
  suite_ItemCaption("Histogram"), /*===NAMES MUST BE DYNAMIC===*/
  suite_ItemDatum(histogram_code),
  0
  };

static suite_Specification		line_button[] =
  {
  suite_ItemCaption("Line"),
  suite_ItemDatum(line_code),
  0
  };

static suite_Specification		dot_button[] =
  {
  suite_ItemCaption("Dot"),
  suite_ItemDatum(dot_code),
  0
  };

static suite_Specification		pie_button[] =
  {
  suite_ItemCaption("Pie"),
  suite_ItemDatum(pie_code),
  0
  };

static suite_Specification		stack_button[] =
  {
  suite_ItemCaption("Stack"),
  suite_ItemDatum(stack_code),
  0
  };

static suite_Specification		cartesian_button[] =
  {
  suite_ItemCaption("Cartesian"),
  suite_ItemDatum(cartesian_code),
  0
  };

static suite_Specification		sort_ascend_label_button[] =
  {
  suite_ItemCaption(AscendPhrase),
  suite_ItemDatum(sort_ascend_label_code),
  0
  };

static suite_Specification		sort_descend_label_button[] =
  {
  suite_ItemCaption(DescendPhrase),
  suite_ItemDatum(sort_descend_label_code),
  0
  };

static suite_Specification	sort_ascend_value_button[] =
  {
  suite_ItemCaption(AscendPhrase),
  suite_ItemDatum(sort_ascend_value_code),
  0
  };

static suite_Specification		sort_descend_value_button[] =
  {
  suite_ItemCaption(DescendPhrase),
  suite_ItemDatum(sort_descend_value_code),
  0
  };

static suite_Specification		sort_ascend_position_button[] =
  {
  suite_ItemCaption(AscendPhrase),
  suite_ItemDatum(sort_ascend_position_code),
  0
  };

static suite_Specification		sort_descend_position_button[] =
  {
  suite_ItemCaption(DescendPhrase),
  suite_ItemDatum(sort_descend_position_code),
  0
  };

static suite_Specification		print_button[] =
  {
  suite_ItemCaption(PrintPhrase),
  suite_ItemDatum(print_code),
  0
  };

static suite_Specification		save_button[] =
  {
  suite_ItemCaption(SavePhrase),
  suite_ItemDatum(save_code),
  0
  };



static suite_Specification		top_title_button[] =
  {
  suite_ItemTitleCaption( TopPhrase ),
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemHitHandler( Palette_Titles_Handler ),
  suite_ItemDatum(top_title_code),
  0
  };

static suite_Specification		bottom_title_button[] =
  {
  suite_ItemTitleCaption( BottomPhrase ),
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemHitHandler( Palette_Titles_Handler ),
  suite_ItemDatum(bottom_title_code),
  0
  };

static suite_Specification		left_title_button[] =
  {
  suite_ItemTitleCaption( LeftPhrase ),
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemHitHandler( Palette_Titles_Handler ),
  suite_ItemDatum(left_title_code),
  0
  };

static suite_Specification		right_title_button[] =
  {
  suite_ItemTitleCaption( RightPhrase ),
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemHitHandler( Palette_Titles_Handler ),
  suite_ItemDatum(right_title_code),
  0
  };



static suite_Specification		sort_label_buttons[] =
  {
  suite_TitleCaption( "By Label" ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_Arrangement( suite_Matrix ),
  suite_HitHandler( Palette_Hit ),
  suite_Item( sort_ascend_label_button ),
  suite_Item( sort_descend_label_button ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  0
  };

static suite_Specification		sort_value_buttons[] =
  {
  suite_TitleCaption( "By Value" ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_Arrangement( suite_Matrix ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  suite_HitHandler( Palette_Hit ),
  suite_Item( sort_ascend_value_button ),
  suite_Item( sort_descend_value_button ),
  0
  };

static suite_Specification		sort_position_buttons[] =
  {
  suite_TitleCaption( "By Position" ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  suite_Arrangement( suite_Column ),
  suite_HitHandler( Palette_Hit ),
  suite_Item( sort_ascend_position_button ),
  suite_Item( sort_descend_position_button ),
  0
  };

static suite_Specification		type_buttons[] =
  {
  suite_TitleCaption( "Chart Types" ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_Arrangement( suite_Matrix ),
  suite_HitHandler( Palette_Hit ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  suite_Item( histogram_button ),
  suite_Item( dot_button ),
  suite_Item( line_button ),
  suite_Item( pie_button ),
/*  suite_Item( map_button ),*/
  suite_Item( stack_button ),
  suite_Item( cartesian_button ),
  0
  };

static suite_Specification		title_buttons[] =
  {
  suite_TitleCaption( TitlesPhrase ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_Arrangement( suite_Column ),
  suite_Item( top_title_button ),
  suite_Item( bottom_title_button ),
  suite_Item( left_title_button ),
  suite_Item( right_title_button ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  0
  };

static suite_Specification		control_buttons[] =
  {
  suite_TitleCaption( "Controls" ),
  suite_TitleCaptionFontName( TitleFontNamePhrase ),
  suite_ItemCaptionFontName( ItemCaptionFontNamePhrase ),
  suite_Arrangement( suite_Matrix | suite_Fixed ),
  suite_Columns( 2 ), suite_Rows( 2 ),
  suite_HitHandler( Palette_Hit ),
  suite_Item( add_button ),
  suite_Item( delete_button ),
  suite_Item( print_button ),
  suite_Item( save_button ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  0
  };

#define				  palette_item1_code	    1
#define				  palette_item2_code	    2
#define				  palette_item3_code	    3
#define				  palette_item4_code	    4

#define				  sort_item1_code	    1
#define				  sort_item2_code	    2
#define				  sort_item3_code	    3

static suite_Specification		  palette_item1[] =
  {
  suite_ItemDatum( palette_item1_code ),
  0
  };

static suite_Specification		  palette_item2[] =
  {
  suite_ItemDatum( palette_item2_code ),
  0
  };

static suite_Specification		  palette_item3[] =
  {
  suite_ItemDatum( palette_item3_code ),
  0
  };

static suite_Specification		  palette_item4[] =
  {
  suite_ItemDatum( palette_item4_code ),
  0
  };

static suite_Specification		  palette_suite[] =
  {
  suite_Arrangement( suite_Matrix ),
  suite_ItemHighlightStyle( suite_None ),
  suite_Item( palette_item1 ),
  suite_Item( palette_item2 ),
  suite_Item( palette_item3 ),
  suite_Item( palette_item4 ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  0
  };

static suite_Specification		  sort_item1[] =
  {
  suite_ItemDatum( sort_item1_code ),
  0
  };

static suite_Specification		  sort_item2[] =
  {
  suite_ItemDatum( sort_item2_code ),
  0
  };

static suite_Specification		  sort_item3[] =
  {
  suite_ItemDatum( sort_item3_code ),
  0
  };

static suite_Specification		  sort_suite[] =
  {
  suite_Arrangement( suite_Column ),
  suite_ItemHighlightStyle( suite_None ),
  suite_Item( sort_item1 ),
  suite_Item( sort_item2 ),
  suite_Item( sort_item3 ),
  suite_TitleCaption( "Sort" ),
  suite_TitleCaptionFontName( "Andysans12b" ),
  suite_BorderSize( 3 ),
  suite_ItemBorderSize( 3 ),
  0
  };

#define SetItemView(suite, item, v) (suite)->SetItemViewObject(item, v)


static long
Initialize_Palette( class chartv	  *self ) /*=== CONVERT TO REAL FORM ===*/
    {
  long			  status = ok;

  IN(Initialize_Palette);
  if ( ! PaletteInitialized )
    {
    PaletteInitialized = true;
    if ( ((ControlSuite = suite::Create( control_buttons, (long) self )) == NULL)  ||
       ((SortLabelSuite = suite::Create( sort_label_buttons, (long) self )) == NULL )  ||
       ((SortValueSuite = suite::Create( sort_value_buttons, (long) self )) == NULL )  ||
       ((SortPositionSuite = suite::Create( sort_position_buttons, (long) self )) == NULL )  ||
       ((TypeSuite = suite::Create( type_buttons, (long) self )) == NULL )  ||
       ((TitleSuite = suite::Create( title_buttons, (long) self )) == NULL )  ||
       ((Palette = suite::Create( palette_suite, (long) self )) == NULL )  ||
       ((SortForm = suite::Create( sort_suite, (long) self )) == NULL )  )
      { DEBUG(Object Creation Failed);
      status = failure;
      printf( "ChartV: Failed to Create Palette Object\n" );
      }
      else
      { DEBUG(Objects Created);
      DEBUGst(LeftAreaTitle,(Chart)->AreaTitle(  apt_LeftArea ));
      DEBUGst(TopAreaTitle,(Chart)->AreaTitle(  apt_TopArea ));
      DEBUGst(RightAreaTitle,(Chart)->AreaTitle(  apt_RightArea ));
      DEBUGst(BottomAreaTitle,(Chart)->AreaTitle(  apt_BottomArea ));
      (TitleSuite)->SetItemAttribute(  (TitleSuite)->ItemOfDatum(  left_title_code ),
	suite_ItemCaption( (Chart)->AreaTitle(  apt_LeftArea ) ) );
      (TitleSuite)->SetItemAttribute(  (TitleSuite)->ItemOfDatum(  top_title_code ),
	suite_ItemCaption( (Chart)->AreaTitle(  apt_TopArea ) ) );
      (TitleSuite)->SetItemAttribute(  (TitleSuite)->ItemOfDatum(  right_title_code ),
	suite_ItemCaption( (Chart)->AreaTitle(  apt_RightArea ) ) );
      (TitleSuite)->SetItemAttribute(  (TitleSuite)->ItemOfDatum(  bottom_title_code ),
	suite_ItemCaption( (Chart)->AreaTitle(  apt_BottomArea ) ) );
      (Palette)->SetItemViewObject((Palette)->ItemOfDatum(palette_item1_code), ControlSuite );
      (Palette)->SetItemViewObject((Palette)->ItemOfDatum(palette_item2_code), TitleSuite );
      (Palette)->SetItemViewObject((Palette)->ItemOfDatum(palette_item3_code), SortForm );
      (Palette)->SetItemViewObject((Palette)->ItemOfDatum(palette_item4_code), TypeSuite );
      (SortForm)->SetItemViewObject((SortForm)->ItemOfDatum(sort_item1_code), SortLabelSuite );
      (SortForm)->SetItemViewObject((SortForm)->ItemOfDatum(sort_item2_code), SortValueSuite );
      (SortForm)->SetItemViewObject((SortForm)->ItemOfDatum(sort_item3_code), SortPositionSuite );
      PaletteFrame = new frame;
      (PaletteFrame)->SetView(  Palette );
      (PaletteFrame)->PostDefaultHandler(  "message",
	    (PaletteFrame)->WantHandler(  "message" ) );
      }
    }
  OUT(Initialize_Palette);
  return  status;
  }

NO_DLL_EXPORT void Destroy_Palette( class chartv	   *self )
    {
  if ( ControlSuite )	        (ControlSuite )->Destroy();
  if ( TitleSuite )		(TitleSuite )->Destroy();
  if ( TypeSuite )		(TypeSuite )->Destroy();
  if ( SortValueSuite )		(SortValueSuite )->Destroy();
  if ( SortLabelSuite )		(SortLabelSuite )->Destroy();
  if ( SortPositionSuite )	(SortPositionSuite )->Destroy();
  if ( Palette )		(Palette )->Destroy();
  if ( SortForm )		(SortForm )->Destroy();
  }

NO_DLL_EXPORT void
Expose_Palette( class chartv	   *self )
    {
  IN(Expose_Palette);
  if ( ! PaletteExposed  &&  Initialize_Palette( self ) == ok )
    { DEBUG(Expose);
    if((PaletteIm = im::Create( NULL )) == NULL) {
	fprintf(stderr,"Could not create new window.\nexiting.\n");
	exit(-1);
    }
    (PaletteIm)->SetView(  PaletteFrame );
    (PaletteIm)->SetTitle(  "Auxiliary Chart Palette" );
    (Menu)->SetMask(  ((Menu )->GetMask( ) &
		      ~menu_palette_hidden) | menu_palette_exposed );
    (self)->PostMenus(  Menu );
    PaletteExposed = true;
    }
  OUT(Expose_Palette);
  }

NO_DLL_EXPORT void
Hide_Palette( class chartv	   *self )
    {
  IN(Hide_Palette);
  if ( PaletteExposed  &&  PaletteIm )
    { DEBUG(Hide);
    (PaletteIm )->Destroy();
    PaletteIm = NULL;
    (Menu)->SetMask(  ((Menu )->GetMask( ) &
		      ~menu_palette_exposed) | menu_palette_hidden );
    (self)->PostMenus(  Menu );
    PaletteExposed = false;
    }
  OUT(Hide_Palette);
  }

static class view *
Palette_Hit( class chartv	   *self, class suite		   *suite, struct suite_item	   *item, long			    type, enum view_MouseAction    action, long			    x , long			    y , long			    clicks )
              {
  char				   msg[512];

  IN(Palette_Hit);
  DEBUGdt(Action,action);
  if ( type == suite_ItemObject  &&  action == view_LeftUp )
    {
    switch ( (suite)->ItemAttribute(  item, suite_itemdatum ) )
      {
      case  add_code:			DEBUG(Add);
	  (self)->AddItemCmd(  );
	break;
      case  delete_code:		DEBUG(Delete);
	(self)->DeleteItemCmd(  );
	break;
      case  histogram_code:
      case  dot_code:
      case  line_code:
      case  pie_code:
/*      case  map_code:*/
      case  stack_code:
      case  cartesian_code:		DEBUG(Types);
	(self)->ReChartCmd( (char *) (suite)->ItemAttribute(  item, suite_itemcaption ) );
	break;
      case  print_code:			DEBUG(Print);
	(self)->PrintCmd(  );
	break;
      case  sort_ascend_value_code:	DEBUG(SortAscendValue);
	(Chart)->Sort(  chart_ByValue | chart_Ascend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortLabelSuite)->Reset(suite_Normalize);
	(SortPositionSuite)->Reset(suite_Normalize);
	break;
      case  sort_descend_value_code:	DEBUG(SortDescendValue);
	(Chart)->Sort(  chart_ByValue | chart_Descend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortLabelSuite)->Reset(suite_Normalize);
	(SortPositionSuite)->Reset(suite_Normalize);
	break;
      case  sort_ascend_label_code:	DEBUG(SortAscendLabel);
	(Chart)->Sort(  chart_ByLabel | chart_Ascend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortValueSuite)->Reset(suite_Normalize);
	(SortPositionSuite)->Reset(suite_Normalize);
	break;
      case  sort_descend_label_code:    DEBUG(SortDescendLabel);
	(Chart)->Sort(  chart_ByLabel | chart_Descend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortValueSuite)->Reset(suite_Normalize);
	(SortPositionSuite)->Reset(suite_Normalize);
	break;
      case  sort_ascend_position_code:	DEBUG(SortAscendPosition);
	(Chart)->Sort(  chart_ByPosition | chart_Ascend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortValueSuite)->Reset(suite_Normalize);
	(SortLabelSuite)->Reset(suite_Normalize);
	break;
      case  sort_descend_position_code: DEBUG(SortDescendPosition);
	(Chart)->Sort(  chart_ByPosition | chart_Descend, NULL );
        (Chart)->NotifyObservers(  chart_ItemsSorted );
	(SortValueSuite)->Reset(suite_Normalize);
	(SortLabelSuite)->Reset(suite_Normalize);
	break;
      case  save_code:			DEBUG(Save);
	(self)->SaveCmd(  );
	break;
      default:
	sprintf( msg, "ChartV: ERROR -- Unknown control-code (%ld)",
		    (suite)->ItemAttribute(  item, suite_itemdatum ) );
	(self)->Announce(  msg );
      } 
    if ( !InputFocus )  (self)->WantInputFocus(  self );
    }
  OUT(Palette_Hit);
  return ((class view*)NULL);
  }

static long
Palette_Titles_Handler( class chartv	   *self, class suite		   *suite, struct suite_item	   *item, long			    action )
          {
  char			  *title;
  long			   area = 0;

  IN(Palette_Titles_Handler);
  DEBUGxt(suite==TitleSuite,suite==TitleSuite);
  title = (char *)(suite)->ItemAttribute(  item, suite_itemcaption );
  DEBUGst(Title,title);
  DEBUGdt(Code,(suite)->ItemAttribute(  item, suite_ItemDatum(0) ) );
  if ( /*===action == */ 1 /*===s/b/suite_EndEntry,Also suite_BeginEntry is 0===*/ )
    {
    switch ( (suite)->ItemAttribute(  item, suite_itemdatum ) )
      {
      case  top_title_code:      area = apt_TopArea;    break;
      case  bottom_title_code:   area = apt_BottomArea; break;
      case  left_title_code:     area = apt_LeftArea;   break;
      case  right_title_code:    area = apt_RightArea;  break;
      default: DEBUG(Error: Bad Code); break;
      }
    DEBUGdt(Area,area);
    (Chart)->SetAreaTitle(  title, area );
    (Chart)->NotifyObservers(  chart_EnclosureModified );
    (Chart )->SetModified( );
    }
  OUT(Palette_Titles_Handler);
  return 0;
  }

NO_DLL_EXPORT void Activate_Viewer( class chartv	  *self )
    {
  Activate( self, delete_code );
  Activate( self, print_code );
  Activate( self, save_code );
  }

static void Activate( class chartv	  *self, long			   code )
      {
  (ControlSuite)->ActivateItem(  (ControlSuite)->ItemOfDatum(  code ) );
  }

