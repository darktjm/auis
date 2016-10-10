#ifndef _suiteev_H_
#define _suiteev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsuite
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suiteev.ch

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
  05/04/89	Changed to lower-case naming convention (GW Keim)
END-SPECIFICATION  
**********************************************************/

#include <view.H>
class suiteev : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	
	static boolean InitializeClass();

	
	suiteev();
	
	virtual ~suiteev();

    
	 void FullUpdate(enum view::UpdateType type,long left,long top,long width,long height);
	 const void* GetInterface(const char *type)  ;
	 class view * Hit(enum view::MouseAction action,long x,long y,long numClicks)  ;
	 void Update();
	 void PostMenus(class menulist *menulistp);
	 void LinkTree( class view *parent );

    
        virtual void ShrinkWrap(long width, long height);
        virtual long LineCount(char *str)  ;
	virtual void Arrange(struct rectangle *rect);
	virtual void DrawItems(struct rectangle *rect);
	virtual long NumberItems()  ;
	virtual long NumberVisible()  ;
	virtual long NumberExposed()  ;
	virtual void Clear()  ;
	virtual struct suite_item * WhichItem(long x,long y)  ;
	virtual void ItemUpdate(struct suite_item *item);
	virtual class view * ItemHit(struct suite_item *item,enum view::MouseAction action,long x,long y,long numClicks)  ;
	virtual void ItemClear(struct suite_item *item);
	virtual void ItemBlackOut(struct suite_item *item);
	virtual void ItemToggle(struct suite_item *item);
	virtual void ItemHighlightReverseVideo(struct suite_item *item,boolean border);
	virtual void ItemHighlightBorder(struct suite_item *item );
	virtual void ItemHighlightCaptionBoldItalic(struct suite_item *item);
	virtual void ItemHighlightCaptionBold(struct suite_item *item);
	virtual void ItemHighlightCaptionItalic(struct suite_item *item);
	virtual void ItemNormalize(struct suite_item *item);
	virtual void ItemDrawCaption(struct suite_item *item);
	virtual void ItemDrawTitle(struct suite_item *item);
	virtual void ItemHighlight(struct suite_item *item);
	virtual void ItemClearCaption(struct suite_item *item);
	virtual void ItemShade(struct suite_item *item);
	virtual long Locate(long x,long y)  ;
	virtual void DrawItemBorder(struct suite_item *item);
	virtual void AllocItemArray(long count);
	virtual void SetItemToReadWrite( struct suite_item *item)  ;
	virtual void MaxStringSize(long *width,long *height);

    
	class suite	    *parent;
	class view	    *cvif; /* Current View In Focus */
	struct suite_item   *firsthit, *lasthit;
	class menulist	    *menulistp;
	int		     firstvisiblesubstring;
	long int	     debug;
	struct sbutton_prefs *itemPrefs;
};



/*
    $Log: suiteev.H,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1994/08/12  17:25:56  rr2b
// new fiasco
//
// Revision 1.3  1993/06/10  20:21:20  rr2b
// Great atk_inited removal.
//
# Revision 1.2  1993/05/18  15:44:59  rr2b
# No changes.
#
# Revision 1.1  1993/05/05  21:32:07  rr2b
# Initial revision
#
*Revision 1.15  1993/03/11  23:40:51  gk5g
*changes to deal with new (proposed) color scheme
*suite now grabs hold of color handles
*
*Revision 1.14  1993/01/11  21:50:58  gk5g
*added LinkTree override
*
*Revision 1.13  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.11  1992/07/23  18:02:51  gk5g
Many changes:
1) item borders are now drawn via sbutton
2) several attributes have been removed and are not supported (font scaling attributes mainly -- CaptionFontHigh, CaptionFontLow, etc.)
3) New attributes have been added to support color: suite_ForegroundColor, suite_BackgroundColor, suite_ActiveItemForegroundColor, .., suite_PassiveItemForegoundColor)
.

Revision 1.10  1991/09/12  19:20:34  bobg
Update copyright notice

Revision 1.9  1990/05/08  15:50:21  gk5g
Just cleaned up a bit.

Revision 1.8  89/11/02  18:58:27  gk5g
Added instance variable firstvisiblesubstring.

Revision 1.7  89/09/29  15:54:27  gk5g
Added extra argument to ShrinkWrap(width, height).

Revision 1.6  89/08/25  17:47:00  gk5g
More changes for V1.0 of the documentation.

Revision 1.5  89/08/24  19:47:54  gk5g
Changes in support of V1.0 of the SuiteProgGuide.doc.

Revision 1.4  89/07/28  19:05:02  gk5g
Moved methods suite_MaxStringSize, MaxSubStringSize, suite_SetItemToReadWrite from suite.c to suiteev.c.

Revision 1.3  89/07/13  16:14:18  gk5g
Added method suiteev_ShrinkWrap() to implement wrapping of long List items.

Revision 1.2  89/05/11  14:37:03  gk5g
Changed suiteev.c to not talke the input focus.
Made suite_Update() call suiteev_Update() and then scroll_Update() if there is a scroll.
Added endzone handlers.

Revision 1.1  89/05/04  12:35:28  gk5g
Initial revision

Revision 1.1  89/04/28  20:26:21  tom
Initial revision

*/
/** @} */
#endif /* _suiteev_H_ */
