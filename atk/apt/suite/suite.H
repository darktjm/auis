#ifndef _suite_H_
#define _suite_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsuite Object suite
 *  A dynamically loadable ATK class file.  Contains suite, suitecv,
 *  suiteev, suitetapp, suitex1app, vector.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suite.ch

VERSION	1.0

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

END-SPECIFICATION  ************************************************************/



#include <vector.H>
#define	suite_VERSION			    1
/*** Suite Object-types ***/
#define	suite_NoObject				    0
#define	suite_ItemObject			    1
#define	suite_TitleObject			    2

/***  Suite Attribute Values  ***/

/*** List Wrapping Styles ***/
#define	 suite_LeftIndent		    (1)
#define	 suite_LeftRight		    (1<<1)

/*** Ordering ***/
#define  suite_ColumnMajor		    (1)
#define  suite_RowMajor			    (1<<1)

/***  Arrangements  ***/
#define  suite_Matrix			    (1)
#define  suite_Column			    (1<<1)
#define  suite_Row			    (1<<2)
#define  suite_Balanced			    (1<<3)
#define  suite_Unbalanced		    (1<<4)
#define  suite_List			    (1<<5)
#define  suite_RowLine			    (1<<6)
#define  suite_ColumnLine		    (1<<7)
#define	 suite_Fixed			    (1<<8)

/***  BorderStyles  ***/
#define  suite_Rectangle		    (1)
#define  suite_Invisible		    (1<<5)
#define	 suite_Line			    (1<<6)

/***  Selection Modes   ***/
#define  suite_Exclusive		    (1)
#define  suite_Inclusive		    (1<<1)
#define	 suite_Toggle			    (1<<2)

/***  TitlePlacements  ***/
#define  suite_Left			    (1)
#define  suite_Right			    (1<<1)
#define  suite_Top			    (1<<2)
#define  suite_Bottom			    (1<<3)
#define  suite_Center			    (1<<4)
#define	 suite_Middle			    (1<<5)

/***  Highlight Styles ***/
#define  suite_Invert			    (1)
#define  suite_Border			    (1<<1)
#define  suite_Bold			    (1<<2)
#define  suite_Italic			    (1<<3)
#define  suite_Pale			    (1<<4)
#define  suite_None			    (1<<9)
#define  suite_Removed			    (1<<6)

/*** Item AccessTypes ***/
#define  suite_ReadOnly			    (1)
#define  suite_ReadWrite		    (1<<1)
#define	 suite_Proceed			    (1<<2)

/*** Sort Modes ***/
#define  suite_Alphabetic		    (1)
#define  suite_Numeric			    (1<<1)
#define  suite_Ascend			    (1<<2)
#define  suite_Descend			    (1<<3)

/*** Sort fields ***/
#define suite_ByCaption			    (1<<4)
#define suite_ByTitle			    (1<<5)
#define suite_ByDatum			    (1<<6)

/*** Reset States ***/
#define	suite_Clear			    (1)
#define	suite_ClearItems		    (1<<1)
#define	suite_ClearTitle		    (1<<2)
#define	suite_Normalize			    (1<<3)
#define	suite_Activate			    (1<<4)
#define	suite_Immediate			    (1<<5)
#define	suite_Defer			    (1<<6)
#define	suite_Expose			    (1<<7)

#define	suite_NoCursor			    (0)

/*** Exception Codes ***/
#define suite_NonExistentItem		    (1)
#define suite_InsufficientSpace		    (1<<1)
#define suite_AttributeConflict		    (1<<2)

typedef	struct suite_specification  suite_Specification;
struct  suite_specification {
    char attribute;
    long value;
};

/*** Attribute Macros ***/

#define suite_TitleCaption(x)		    suite_titlecaption,			(long) (x)
#define suite_TitleCaptionAlignment(x)	    suite_titlecaptionalignment,	(long) (x)
#define suite_TitleBorderStyle(x)	    suite_titleborderstyle,		(long) (x)
#define suite_TitleHighlightStyle(x)	    suite_titlehighlightstyle,		(long) (x)
#define suite_TitleBorderSize(x)	    suite_titlebordersize,		(long) (x)
#define suite_ItemOrder(x)		    suite_itemorder,			(long) (x)
#define suite_ItemCaptionList(x)	    suite_itemcaptionlist,		(long) (x)
#define suite_Item(x)			    suite_itemspec,			(long) (x)
#define suite_SelectionMode(x)		    suite_selectionmode,		(long) (x)
#define suite_BorderStyle(x)		    suite_borderstyle,			(long) (x)
#define suite_BorderSize(x)		    suite_bordersize,			(long) (x)
#define suite_HitHandler(x)		    suite_hithandler,			(long) (x)
#define suite_Arrangement(x)		    suite_arrangement,			(long) (x)
#define suite_Scroll(x)			    suite_scroll,			(long) (x)
#define suite_TitlePlacement(x)		    suite_titleplacement,		(long) (x)
#define	suite_FontName(x)		    suite_fontname,			(long) (x)
#define suite_TitleFontName(x)		    suite_titlefontname,		(long) (x)
#define suite_TitleCaptionFontName(x)	    suite_titlefontname,		(long) (x)
#define suite_TitleViewObjectName(x)	    suite_titleviewobjectname,		(long) (x)
#define suite_TitleViewObjectHandler(x)	    suite_titleviewobjecthandler,	(long) (x)
#define suite_TitleDataObjectName(x)	    suite_titledataobjectname,		(long) (x)
#define suite_TitleDataObjectHandler(x)	    suite_titledataobjecthandler,	(long) (x)
#define	suite_TitleDataObject(x)	    suite_titledataobject,		(long) (x)
#define	suite_TitleViewObject(x)	    suite_titleviewobject,		(long) (x)
#define suite_TitleHitHandler(x)	    suite_titlehithandler,		(long) (x)
#define	suite_ItemPosition(x)		    suite_itemposition,			(long) (x)
#define suite_ItemCaptionFontName(x)	    suite_itemcaptionfontname,		(long) (x)
#define suite_ItemBorderStyle(x)	    suite_itemborderstyle,		(long) (x)
#define suite_ItemBorderSize(x)		    suite_itembordersize,		(long) (x)
#define suite_ItemHighlightStyle(x)	    suite_itemhighlightstyle,		(long) (x)
#define suite_ItemPassiveStyle(x)	    suite_itempassivestyle,		(long) (x)
#define suite_ItemCaption(x)		    suite_itemcaption,			(long) (x)
#define suite_ItemCaptionAlignment(x)	    suite_itemcaptionalignment,		(long) (x)
#define suite_ItemTitleCaption(x)	    suite_itemtitlecaption,		(long) (x)
#define suite_ItemTitleFontName(x)	    suite_itemtitlefontname,		(long) (x)
#define suite_ItemTitleCaptionFontName(x)   suite_itemtitlefontname,		(long) (x)
#define suite_ItemTitleCaptionAlignment(x)  suite_itemtitlecaptionalignment,	(long) (x)
#define suite_ItemTitlePlacement(x)	    suite_itemtitleplacement,		(long) (x)
#define suite_ItemAccessMode(x)		    suite_accesstype,			(long) (x)
#define suite_AccessMode(x)		    suite_accesstype,			(long) (x)
#define suite_ItemWidth(x)		    suite_itemwidth,			(long) (x)
#define suite_ItemHeight(x)		    suite_itemheight,			(long) (x)
#define suite_ItemViewObjectName(x)	    suite_itemviewobjectname,		(long) (x)
#define suite_ItemViewObjectHandler(x)	    suite_itemviewobjecthandler,	(long) (x)
#define suite_ItemDataObjectName(x)	    suite_itemdataobjectname,		(long) (x)
#define suite_ItemDataObjectHandler(x)	    suite_itemdataobjecthandler,	(long) (x)
#define suite_ItemHitHandler(x)		    suite_itemhithandler,		(long) (x)
#define suite_Data(x)			    suite_datum,				(long) (x)
#define suite_Datum(x)			    suite_datum,				(long) (x)
#define suite_ItemDatum(x)		    suite_itemdatum,			(long) (x)
#define suite_ItemData(x)		    suite_itemdatum,			(long) (x)
#define suite_HorizontalGuttersize(x)	    suite_horizontalguttersize,		(long) (x)
#define suite_VerticalGuttersize(x)	    suite_verticalguttersize,		(long) (x)
#define	suite_Guttersize(x)		    suite_guttersize,			(long) (x)
#define suite_HorizontalGutterSize(x)	    suite_horizontalguttersize,		(long) (x)
#define suite_VerticalGutterSize(x)	    suite_verticalguttersize,		(long) (x)
#define	suite_GutterSize(x)		    suite_guttersize,			(long) (x)
#define suite_SortMode(x)		    suite_sortmode,			(long) (x)
#define suite_SortHandler(x)		    suite_sorthandler,			(long) (x)
#define suite_CursorFontName(x)		    suite_cursorfontname,		(long) (x)
#define	suite_CursorType(x)		    suite_cursorbyte,			(long) (x)
#define suite_Cursor(x)			    suite_cursorbyte,			(long) (x)
#define suite_ItemCursorFontName(x)	    suite_itemcursorfontname,		(long) (x)
#define suite_ItemCursor(x)		    suite_itemcursorbyte,		(long) (x)
#define suite_ItemCursorType(x)		    suite_itemcursorbyte,		(long) (x)
#define	suite_WrappingStyle(x)		    suite_wrappingstyle,		(long) (x)
#define	suite_ItemName(x)		    suite_itemname,			(long) (x)
#define	suite_Rows(x)			    suite_rows,				(long) (x)
#define	suite_Columns(x)		    suite_columns,			(long) (x)
#define	suite_ForegroundColor(x)	    suite_foregroundcolor,		(long) (x)
#define	suite_BackgroundColor(x)	    suite_backgroundcolor,		(long) (x)
#define	suite_TitleForegroundColor(x)	    suite_titleforegroundcolor,		(long) (x)
#define	suite_TitleBackgroundColor(x)	    suite_titlebackgroundcolor,		(long) (x)
#define	suite_ItemCaptionColor(x)	    suite_itemcaptioncolor,		(long) (x)
#define	suite_ItemForegroundColor(x)	    suite_itemforegroundcolor,		(long) (x)
#define	suite_ItemBackgroundColor(x)	    suite_itembackgroundcolor,		(long) (x)
#define	suite_ActiveItemCaptionColor(x)	    suite_activeitemcaptioncolor,	(long) (x)
#define	suite_ActiveItemForegroundColor(x)  suite_activeitemforegroundcolor,	(long) (x)
#define	suite_ActiveItemBackgroundColor(x)  suite_activeitembackgroundcolor,	(long) (x)
#define	suite_PassiveItemCaptionColor(x)    suite_passiveitemcaptioncolor,	(long) (x)
#define	suite_PassiveItemForegroundColor(x) suite_passiveitemforegroundcolor,	(long) (x)
#define	suite_PassiveItemBackgroundColor(x) suite_passiveitembackgroundcolor,	(long) (x)

/***  Suite Attribute Codes  ***/

#define	suite_titlecaption			    1
#define	suite_titleborderstyle			    2
#define	suite_titlehighlightstyle		    3
#define	suite_titlebordersize			    4
#define	suite_titledataobjectname		    5
#define	suite_titledataobjecthandler		    6
#define	suite_selectionmode			    7
#define	suite_titleplacement			    8
#define	suite_borderstyle			    9
#define	suite_hithandler			    10
#define	suite_arrangement			    11
#define	suite_scroll				    12
#define	suite_titlefontname			    13
#define	suite_titleviewobjectname		    14
#define	suite_titleviewobjecthandler		    15
#define	suite_titlehithandler			    18
#define	suite_itemcaptionfontname		    19

#define	suite_itemborderstyle			    23
#define	suite_itembordersize			    24
#define	suite_itemhighlightstyle		    25
#define	suite_itempassivestyle			    26
#define	suite_itemorder				    27
#define	suite_itemcaptionlist			    28
#define	suite_itemspec				    29
#define	suite_bordersize			    30
#define	suite_datum				    31
#define	suite_itemcaption			    32
#define	suite_itemtitlecaption			    33
#define	suite_itemtitlefontname			    34
#define	suite_itemtitleplacement		    35
#define	suite_accesstype			    36
#define	suite_itemdataobjectname		    37
#define	suite_itemdataobjecthandler		    38
#define	suite_itemviewobjectname		    39
#define	suite_itemviewobjecthandler		    40
#define	suite_itemhithandler			    41
#define	suite_itemwidth				    42
#define	suite_itemheight			    43
#define	suite_horizontalguttersize		    46
#define	suite_verticalguttersize		    47
#define	suite_guttersize			    48
#define	suite_sortmode				    49
#define	suite_sorthandler			    50
#define	suite_cursorfontname			    52
#define	suite_cursorbyte			    53
#define	suite_itemcursorbyte			    55
#define	suite_itemcursorfontname		    56
#define	suite_itemdatum				    57
#define	suite_wrappingstyle			    58
#define	suite_itemposition			    59
#define	suite_fontname				    60
#define	suite_itemname				    61
#define	suite_itemcaptionalignment		    62
#define	suite_itemtitlecaptionalignment		    63
#define	suite_titlecaptionalignment		    64
#define	suite_titleviewobject			    65
#define	suite_titledataobject			    66
#define	suite_rows				    67
#define	suite_columns				    68

#define	suite_foregroundcolor			    69
#define	suite_backgroundcolor			    70
#define	suite_itemcaptioncolor			    71
#define	suite_itemforegroundcolor		    72
#define	suite_itembackgroundcolor		    73
#define	suite_activeitemcaptioncolor		    74
#define	suite_activeitemforegroundcolor		    75
#define	suite_activeitembackgroundcolor		    76
#define	suite_passiveitemcaptioncolor		    77
#define	suite_passiveitemforegroundcolor	    78
#define	suite_passiveitembackgroundcolor 	    79
#define suite_titleforegroundcolor		    80
#define suite_titlebackgroundcolor		    81
#include <color.H>

struct color_state {
    color fg, bg;
};


typedef struct suite_item *suite_type_item;
#include <aptv.H>
class suite;
typedef void (*suite_dataobjectfptr)(long ClientAnchor,class suite *self,struct suite_item *item,int otype);
typedef void (*suite_viewobjectfptr)(long ClientAnchor,class suite *self,struct suite_item *item,int otype);
typedef void (*suite_hitfptr)(long ClientAnchor, class suite *self, struct suite_item *item, int otype, view::MouseAction action, long x, long y, long numberOfClicks);
typedef long (*suite_sortfptr)(long clientanchor, class suite *self, struct suite_item *i1, struct suite_item *i2);
typedef long (*suite_applyfptr)(long ca, class suite *self, struct suite_item *i, long datum);
typedef void (*suite_exceptfptr)(long ca, class suite *self, struct suite_item *i, long es);
struct suite_item {
	char				    *name;
	class suite			    *suite;
	char				    *caption;
	class fontdesc			    *captionfont;
	char				    *captionfontname;
	char				    *title;
	char				    *titlefontname;
	class fontdesc			    *titlefont;
	char				    *dataobjectname;
	class dataobject		    *dataobject;
	suite_dataobjectfptr dataobjecthandler;
	char				    *viewobjectname;
	class view			    *viewobject;
	struct rectangle		     inset_rect;
	struct rectangle		     title_rect;
	struct rectangle		     caption_rect;
	struct rectangle		     bounds;
	suite_viewobjectfptr viewobjecthandler;
	suite_hitfptr hithandler;
	long				     datum;
	class fontdesc			    *cursorfont;
	char				    *cursorfontname;
	class cursor			    *cursor;
	short				     captionfontsize;
	short				     titlefontsize;
	unsigned char			     cursorbyte;
	unsigned char			     bordersize;
	unsigned char			     captionalignment;
	unsigned char			     titleplacement;
	unsigned char			     titlecaptionalignment;
	unsigned char			     highlightstyle;
	unsigned char			     passivestyle;
	unsigned char			     titlefonttype;
	unsigned char			     captionfonttype;
	unsigned char			     borderstyle;
	unsigned char			     mode;
	unsigned char			     accesstype;
	boolean				     exposed;
	class vector			    *breaks;
	boolean				     debug;
	long				     position;
	struct color_state		    color;
};

class suite : public  aptv {
public:
	virtual ATKregistryEntry *ATKregistry();


    

	
	static boolean InitializeClass();

	
	suite();
	
	virtual ~suite();
	static class suite * Create( const suite_Specification *suite_Spec,long anchor)  ;

    

	 void FullUpdate(enum view::UpdateType type,long left,long top,long width,long height);
	 void Update();
	 class view * Hit(enum view::MouseAction action,long x,long y,long numberOfClicks)  ;
	 void ReceiveInputFocus();
	 void LoseInputFocus();
	 void LinkTree( class view *parent );

    

	virtual struct suite_item * CreateItem( char *name, long datum )  ;
	virtual void DestroyItem(struct suite_item *item)  ;
	virtual void ClearAllItems();
	virtual void ExposeItem(struct suite_item *item);
	virtual void HideItem(struct suite_item *item);
	virtual long Reset(long state)  ;
	virtual void PassivateItem(struct suite_item *item);
	virtual void ActivateItem(struct suite_item *item);
	virtual boolean ItemActivated( struct suite_item *item )  ;
	virtual long HighlightItem( struct suite_item *item )  ;
	virtual boolean ItemHighlighted( struct suite_item *item )  ;
	virtual long NormalizeItem( struct suite_item *item )  ;
	virtual boolean ItemNormalized( struct suite_item *item )  ;
	virtual void Apply(suite_applyfptr proc,long anchor,long datum);
	virtual void Sort(unsigned mode,suite_sortfptr handler);
	virtual struct suite_item ** SelectedItems(long *number)  ;
	virtual long SuiteAttribute(long attribute)  ;
	virtual long SetSuiteAttribute(long attribute, long value)  ;
	virtual long ChangeSuiteAttribute(long attribute, long value)  ;
	virtual long ItemAttribute(struct suite_item *item,long attribute)  ;
	virtual long SetItemAttribute(struct suite_item *item,long attribute, long value)  ;
	virtual long ChangeItemAttribute(struct suite_item *item,long attribute, long value)  ;
	virtual struct suite_item * ItemOfDatum(long datum)  ;
	virtual struct suite_item ** ItemsOfDatum(long datum)  ;
	virtual struct suite_item * ItemOfName(char *name)  ;
	virtual struct suite_item ** ItemsOfName(char *name)  ;
	virtual struct suite_item * ItemAtPosition(long position)  ;
	virtual void SetDebug(boolean value);
	virtual void HighlightTitle();
	virtual void NormalizeTitle();

    

        inline struct suite_item * CurrentItem()		    { return (this->current_item); }
	inline int ItemCount()		    { return (this->items ? (this->items)->Count():0); }
#if 0
	inline long ItemAnchor(suite_type_item item)	    { return (item->anchor); }
	inline procedure SetExceptionHandler(procedure handler){ return (this->exception_handler); }

#endif
	inline void SetItemDataObject(suite_type_item item, class dataobject *d) { (item)->dataobject = (d); }
	inline class dataobject * ItemDataObject(suite_type_item item)	    { return (item->dataobject); }
	inline void SetItemViewObject(suite_type_item item, class view *v) { (item)->viewobject = (v); }
	inline class view * ItemViewObject(suite_type_item item)	    { return (item->viewobject); }
	inline boolean ItemExposed(suite_type_item item)	    { return (item->exposed); }
	inline long ExceptionCode()		    { return (this->exception_status); }
	inline suite_type_item ExceptionItem()		    { return (this->exception_item); }
	inline suite_type_item FirstVisible()		    { return (this->firstvisible); }
	inline suite_type_item LastVisible()		    { return (this->lastvisible); }
	inline void SetFirstVisible(suite_type_item item)	    { ((this)->newfirstvisible = item); }
	inline void SetLastVisible(suite_type_item item)	    { ((this)->lastvisible = item); }

    

	struct suite_item	    *current_item;
	class apt		    *aptp;
	class vector		    *items;
	class suiteev		    *setview;
	struct rectangle	     container;
	struct rectangle	     bounds;
	class scroll		    *scroll;
	struct rectangle	     scrollrect;

	char			    *title_caption;
	struct rectangle	     title_rect;
	unsigned		     titlehighlightstyle;
	unsigned		     titleborderstyle;
	short			     titlebordersize;
	char			    *titledataobjectname;
	class dataobject	    *titledataobject;
	class view		    *titleviewobject;
	suite_dataobjectfptr titledataobjecthandler;
	suite_viewobjectfptr titleviewobjecthandler;
	unsigned		     titleplacement;
	unsigned		     titlecaptionalignment;
	suite_hitfptr titlehithandler;
	char			    *titleviewobjectname;
	char			    *titlefontname;
	class fontdesc		    *titlefont;
	long			     titlefontsize;
	struct color_state	titlecolor;
	unsigned		     titlefonttype;
	unsigned		     titlehighlighted;

	unsigned		     itemorder;
	unsigned		     arrangement;
	unsigned		     selection_mode;
	unsigned		     borderstyle;
	short			     bordersize;
	unsigned		     scrolltype;
	char			    *captionfontname;
	class fontdesc		    *captionfont;
	short			     captionfontsize;
	unsigned		     captionfonttype;
	char			    *itemtitlefontname;
	short			     itemtitlefontsize;
	unsigned		     itemtitlefonttype;
	unsigned		     captionalignment;
	unsigned		     itemborderstyle;
	unsigned		     itembordersize;
	unsigned		     itemhighlightstyle;
	unsigned		     itempassivestyle;
	unsigned		     itemtitleplacement;
	unsigned		     itemtitlecaptionalignment;
	unsigned		     accesstype;
	char			    *itemviewobjectname, 
				    *itemdataobjectname;
	suite_viewobjectfptr			   itemviewobjecthandler;
	suite_dataobjectfptr itemdataobjecthandler;
	suite_hitfptr hithandler;
	long			     anchor, datum;
	struct suite_item	    *firstvisible, *lastvisible, *newfirstvisible;
	long			     visiblerows, visiblecolumns, 
				     rows, columns, numvisible;
	short			     itemwidth, itemheight;
	short			     itemfixedwidth, itemfixedheight;
	class cursor		    *cursor;
	char			    *cursorfontname;
	class fontdesc		    *cursorfont;
	char			     cursorbyte;
	class cursor		    *itemcursor;
	char			    *itemcursorfontname;
	class fontdesc		    *itemcursorfont;
	char			     itemcursorbyte;
	suite_sortfptr sort_handler;
	unsigned		     sortorder;
	short			     x_guttersize, y_guttersize;
	struct suite_item	   **itemarray;
	suite_exceptfptr exception;
	long			     exception_status;
	struct suite_item	    *exception_item;
	long			     title_m_width, caption_m_width;
	boolean			     has_focus;
	boolean			     debug;
	short			     wrappingstyle;
	long			     max_item_pos_given;
	struct sbutton_prefs	    *buttonprefs;
	struct color_state	    suiteColor;
	struct color_state	    activeItemColor;
	struct color_state		passiveItemColor;
	double			     suite_bg_shade[3];
	boolean			     mono;
};


/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suite.h

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
  11/01/88	Created (GW Keim)

END-SPECIFICATION  ************************************************************/

#define item_Normalized			    (1)
#define item_Highlighted		    (1<<1)
#define item_Active			    (1<<2)

#define	item_SetUpperLeft(item,X,Y) (item->bounds).left = (X);(item->bounds).top = (Y)
#define	item_SetDimensions(item,WIDTH,HEIGHT)\
    (item->bounds).width = (WIDTH);(item->bounds).height = (HEIGHT)

#define FONTHEIGHT(f) ((f)->max_bounds.ascent + (f)->max_bounds.descent)

#define item_Caption (item->caption?item->caption:item->name)
#define	item_Name (item->name?item->name:item->caption)
#define item_CaptionPlacement \
 ((item->captionplacement)?(item->captionplacement):(item->suite->captionplacement))
#define item_CaptionAlignment \
 ((item->captionalignment)?(item->captionalignment):(item->suite->captionalignment))
#define item_CaptionFontName \
 ((item->captionfontname)?(item->captionfontname):(item->suite->captionfontname))
#define item_CaptionFont \
 ((item->captionfont)?(item->captionfont):(item->suite->captionfont))

#if 1
#define item_CaptionFontSize \
 ((item->captionfontsize)?(item->captionfontsize):(item->suite->captionfontsize))
#else
#define item_CaptionFontSize (fontdesc_FontSummary(item_CaptionFont,suite_GetDrawable(item->suite))->newlineHeight)
#endif

#define item_CaptionFontType \
 ((item->captionfonttype)?(item->captionfonttype):(item->suite->captionfonttype))
#define item_Title				(item->title)
#define item_TitlePlacement \
 ((item->titleplacement)?(item->titleplacement):(item->suite->titleplacement))
#define item_TitleCaptionAlignment \
 ((item->titlecaptionalignment)?(item->titlecaptionalignment):\
 (item->suite->itemtitlecaptionalignment))
#define item_TitleFontName \
 ((item->titlefontname)?(item->titlefontname):(item->suite->titlefontname))
#define item_TitleFont \
 ((item->titlefont)?(item->titlefont):(item->suite->titlefont)
#define item_TitleFontSize \
 ((item->titlefontsize)?(item->titlefontsize):(item->suite->titlefontsize))
#define item_TitleFontType \
 ((item->titlefonttype)?(item->titlefonttype):(item->suite->titlefonttype))
#define item_BorderStyle \
 ((item->borderstyle)?(item->borderstyle):(item->suite->itemborderstyle))
#define item_BorderSize \
 ((item->bordersize)?(item->bordersize):(item->suite->itembordersize))
#define item_DataObjectName \
 ((item->dataobjectname)?(item->dataobjectname):(item->suite->itemdataobjectname))
#define item_DataObject				(item->dataobject)
#define item_DataObjectHandler \
 ((item->dataobjecthandler)?(item->dataobjecthandler):(item->suite->itemdataobjecthandler))
#define item_ViewObjectName \
 ((item->viewobjectname)?(item->viewobjectname):(item->suite->itemviewobjectname))
#define item_ViewObject				(item->viewobject)
#define item_ViewObjectHandler \
 ((item->viewobjecthandler)?(item->viewobjecthandler):(item->suite->itemviewobjecthandler))
#define item_HitHandler \
 ((item->hithandler)?(item->hithandler):(item->suite->hithandler))
#define item_Data \
 ((item->data)?(item->data):(item->suite->data))
#define item_Mode \
 ((item->mode)?(item->mode):(item->suite->mode))
#define item_AccessType \
 ((item->accesstype)?(item->accesstype):(item->suite->accesstype))
#define item_PassiveStyle \
 ((item->passivestyle)?(item->passivestyle):(item->suite->itempassivestyle))
#define item_HighlightStyle \
 ((item->highlightstyle)?(item->highlightstyle):(item->suite->itemhighlightstyle))
#define item_Debug				(item->debug)
#define item_Cursor \
    (item->cursor?item->cursor:item->suite->itemcursor)
#define item_CursorByte \
    (item->cursorbyte?item->cursorbyte:item->suite->itemcursorbyte)
#define item_CursorType \
    (item->cursortype?item->cursortype:item->suite->itemcursortype)
#define item_CursorFontName \
    (item->cursorfontname?item->cursorfontname:(item->suite->itemcursorfontname?item->suite->itemcursorfontname:item->suite->cursorfontname))
#define item_CursorFont \
    (item->cursorfont?item->cursorfont:item->suite->itemcursorfont)

#define NOFORCEDMODE				    (-1)

#define	Breaks(item)				    (item->breaks)
#define	BreakCount(item)			    ((item->breaks)->Count())
#define	BreakPos(item,i)			    (item->breaks->Item(i))

#define	item_ActiveForegroundColor ( (item) ? &item->color.fg : &item->suite->activeItemColor.fg )
#define	item_ActiveBackgroundColor ( (item) ? &item->color.bg : &item->suite->activeItemColor.bg )
#define	item_ActiveForegroundColorName ( (item) ? (char *)item->color.fg.Name() : (char *)item->suite->activeItemColor.fg.Name() )
#define	item_ActiveBackgroundColorName ( (item) ? (char *)item->color.bg.Name() : (char *)item->suite->activeItemColor.bg.Name() )

#define	item_PassiveForegroundColor ( (item) ? &item->color.fg : &item->suite->passiveItemColor.fg )
#define	item_PassiveBackgroundColor ( (item) ? &item->color.bg : &item->suite->passiveItemColor.bg )
#define	item_PassiveForegroundColorName ( (item) ? (char *)item->color.fg.Name() : (char *)item->suite->passiveItemColor.fg.Name() )
#define	item_PassiveBackgroundColorName ( (item) ? (char *)item->color.bg.Name() : (char *)item->suite->passiveItemColor.bg.Name() )

#define	item_ForegroundColor ( (item->mode & item_Active) ? item_ActiveForegroundColor : item_PassiveForegroundColor )
#define	item_BackgroundColor ( (item->mode & item_Active) ? item_ActiveBackgroundColor : item_PassiveBackgroundColor )
#define	item_ForegroundColorName ( (item->mode & item_Active) ? item_ActiveForegroundColorName : item_PassiveForegroundColorName )
#define	item_BackgroundColorName ( (item->mode & item_Active) ? item_ActiveBackgroundColorName : item_PassiveBackgroundColorName )


/*
    $Log: suite.H,v $
// Revision 1.8  1995/11/07  20:17:10  robr
// OS/2 port
//
 * Revision 1.3  1995/09/28  19:24:40  robr
 * Initial Checkin of the New Color API
 *
 * Revision 1.2  1993/05/18  15:36:54  rr2b
 * Manual conversion of Class-C constructs to C++.
 *
 * Revision 1.1  1993/05/05  21:32:07  rr2b
 * Initial revision
 *
 * Revision 1.12  1993/03/11  23:40:51  gk5g
 * changes to deal with new (proposed) color scheme
 * suite now grabs hold of color handles
 *
 * Revision 1.11  1992/12/14  23:20:33  rr2b
 * add $Logs back after disclaimerization took them out
 *
 * Revision 1.9  1992/09/04  17:14:20  gk5g
 * Added caption color attributes
 * .
 *
 * Revision 1.8  1992/07/23  18:02:51  gk5g
 * Many changes:
 * 1) item borders are now drawn via sbutton
 * 2) several attributes have been removed and are not supported (font scaling attributes mainly -- CaptionFontHigh, CaptionFontLow, etc.)
 * 3) New attributes have been added to support color: suite_ForegroundColor, suite_BackgroundColor, suite_ActiveItemForegroundColor, .., suite_PassiveItemForegoundColor)
 * .
 *
 * Revision 1.7  1991/09/12  19:20:24  bobg
 * Update copyright notice
 *
 * Revision 1.6  1990/04/27  15:36:59  gk5g
 * Added item_Name macro.
 *
 * Revision 1.5  89/09/29  15:55:06  gk5g
 * Added macro item_RealCaptionFontSize.
 * 
 * Revision 1.4  89/09/08  17:00:31  gk5g
 * Added item_CaptionAlignment macro.
 * 
 * Revision 1.3  89/08/24  19:48:00  gk5g
 * Changes in support of V1.0 of the SuiteProgGuide.doc.
 * 
 * Revision 1.2  89/07/13  16:10:42  gk5g
 * Added macros for List item break points.
 * 
 * Revision 1.1  89/04/28  20:27:08  tom
 * Initial revision
 * 
*/
/** @} */
#endif /* _suite_H_ */
