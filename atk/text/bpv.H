ATK_INTER
#ifndef _bpv_H_
#define _bpv_H_
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

 



#include <view.H>

class bpv: public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();
      static boolean InitializeClass();
      bpv();

      void SetDataObject(class dataobject *dataobject);
      void ObservedChanged (class observable *changed, long value);
      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
      void Update();
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
      void PostMenus(class menulist  *ml);
      class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
      void ReceiveInputFocus();
      void LoseInputFocus();
      void Print(FILE *file, char *processor, char *finalFormat, boolean topLevel);
      void *GetPSPrintInterface(char *printtype);

      boolean HasInputFocus;
      boolean NowBig;
      class menulist *Menus;
};


#endif /* _bpv_H_ */
