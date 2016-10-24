#ifndef _arbcon_H_
#define _arbcon_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter
 * @{ */
/* user code begins here for HeaderInfo */
/* user code ends here for HeaderInfo */
#include <observable.H>
/** A controller which is the primary ADEW interface.
 *  Upon creation, a new window will be popped up with the Arb document
 *  ($ANDREWDIR/lib/arbiters/Arb).  This controller is the "ADEW" GUI for
 *  manipulating \ref arbiter views.  An arbiter view is a view which
 *  wraps an ordinary inset that may have child insets.  The child insets
 *  may be wrapped in \ref cel views.  A cel view holds an inset, and
 *  assigns that inset a name which can be referred to in Ness scripts.
 *  The name is managed by the arbiter to ensure all cels are uniquely named.
 *
 *  The GUI provides functions for creating new arbiters with a single inset,
 *  and inserting and deleting cel-controlled insets, with or without
 *  the application layer (scroll bars).  It may also be used to create
 *  cel-encapsulated views on other insets in the document, although these
 *  share a name (the name is associated with the data object, not the view).
 *  It also provides the ability to view and edit \ref cel names, as well as
 *  parameter values for objects with resource parameters.
 *
 *  The list of insets which can be created comes from the preferences
 *  ObViewList, which is a comma-separated (and newline-separated) list
 *  of names, and ObViewFile, which is a file containing the same.
 *  In addition, a default list is appended, unless the IgnoreDefaultViewList
 *  preference is true.  This default list comes from the file
 *  $ANDREWDIR/lib/arbiters/viewlist, although the file can be changed
 *  using the prefernece AdewObViewList.  If the file is not present,
 *  a compiled-in default list will be used.  In any case, new entires
 *  can be added manually while the GUI is running, although these added
 *  names will not be saved.
 *
 *  In addition to the regular list of insets, a set of insets which are
 *  meant to be GUI elements can be selected from; this is done by
 *  popping up the document $ANDREDIR/lib/arbiters/vallist.  This file
 *  name can be overridden with the ValueFile preference.
 */
class arbcon : public  observable  {
public:
	TRACED_CLASS(arbcon);
 

        
	static boolean InitializeClass();

	
	
	arbcon();    
/* user code begins here for classprocedures */
static void AddCel(class arbiterview *arb,class cel *cl,boolean notify);
static class celview * currentcelview()  ;
static void SetCurrentCelview(class celview *cv);
static void DeleteCelview(class arbiterview *arb,class celview *cv);
static void EditCurrentCelview();
static void SaveCurrentCelview();
static void DestroyCurrentCelview();
static void SetCurrentArbiterview(class arbiterview *ab);
static void DeleteArbiter(class arbiterview *arb);
static void AddArbiter(class arbiterview *arb);
static boolean InitCel(class celview *cv,class arbiterview *abv)  ;
static void InitArbiters();

/* user code ends here for classprocedures */

	 void ObservedChanged( class observable * observed, long status );
/* user code begins here for overrides */
/* user code ends here for overrides */

	class value *ArbLinkCel;
	class onoffV *ArbLinkCelView;
	class value *ArbCutCel;
	class buttonV *ArbCutCelView;
	class value *ArbApplicationChoice;
	class onoffV *ArbApplicationChoiceView;
	class value *Arbobviewlist;
	class clicklistV *ArbobviewlistView;
	class value *ArbTextEdit;
	class menterstrV *ArbTextEditView;
/* user code begins here for classdata */
	class celview *currentcelviewval;
	class arbiterview *CurrentArbiterview;
	int obcount,vwcount;
	int obsize,vwsize;
	char **obnamelist, **vwlist,buf[256];
	class cel *cl;
	char *arr; 
	long arrlen;
	class view *overlay;
 	class text *ArbCelList;
 	class cltextview *ArbCelListView;
 	class text *ArbArbList;
 	const char *ArbiterName;
 	const char *ObjectName;
 	const char *ViewName;

/* user code ends here for classdata */
	class view *v;
	class arbiterview *arbv;
	class arbcon *next;
};
/** @} */
#endif /* _arbcon_H_ */
