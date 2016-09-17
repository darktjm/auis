ATK_INTER
#ifndef _boxview_H_
#define _boxview_H_
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

/* $ACIS$ */

 

/* boxview.ch - box view definition and interface */
#define boxview_VERSION 1


/* Interface definition */

#include <view.H>
class boxview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();



   void FullUpdate(enum view_UpdateType how, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
   view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
   void WantNewSize(class view *requestor);
   void LinkTree(class view *parent);
   void UnlinkNotification(class view *unlinkedview);
   void ObservedChanged(class observable *changed, long status);
   void Print(FILE *f, const char *processor, const char *finalFormat, boolean toplevel);


  virtual void ToggleDebug();
  virtual void Paste();
  virtual void DrawBox();
  virtual int BoxWidth()  ;


  
	static boolean InitializeClass();

  
	boxview();
  
	virtual ~boxview();


	boolean	updateNeeded;		    /* the box itself needs to be updated */
	long lastUpdate;		    /* modification timestamp of last update */
	class view *child;		    /* view of contents */
};

/* end of boxview.ch */


#endif /* _boxview_H_ */
