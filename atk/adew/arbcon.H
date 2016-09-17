ATK_INTER
#ifndef _arbcon_H_
#define _arbcon_H_
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

/* user code begins here for HeaderInfo */
/* user code ends here for HeaderInfo */
#include <observable.H>
class arbcon : public  observable  {
public:
	virtual ATKregistryEntry *ATKregistry();
 

        
	static boolean InitializeClass();

	
	virtual ~arbcon();
	
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


#endif /* _arbcon_H_ */
