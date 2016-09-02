ATK_INTER
#ifndef _fillerview_H_
#define _fillerview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

/* $ACIS$ */

 

/* fillerview.ch - filler view definition and interface */
#define fillerview_VERSION 1

/* Interface definition */

#include <celview.H>
class fillerview: public  celview {
public:
	virtual ATKregistryEntry *ATKregistry();



   void FullUpdate(enum view_UpdateType how, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
   void ReceiveInputFocus();
   void LoseInputFocus();


  virtual void SetDataObjectByName(const char *dataname);	/* replace myself */


  
	static boolean InitializeClass();
	/* initialize class */
  
	fillerview();	/* initialize instance */
  
	virtual ~fillerview();	/* clean up instance */


	boolean hasInputFocus;
        class menulist *menulistp;
	int hitindex;
};

/* end of fillerview.ch */


#endif /* _fillerview_H_ */
