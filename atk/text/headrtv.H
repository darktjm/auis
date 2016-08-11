ATK_INTER
#ifndef _headrtv_H_
#define _headrtv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
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

#include <header.H>
#include <view.H>
class headrtv: public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();
      headrtv();
      static boolean InitializeClass();
      static boolean atk_inited;

      void PostKeyState(class keystate *ks);
      void Print(FILE *file,char *processor, char *finalFormat, boolean topLevel);
      void WantInputFocus(class view *requestor);
      void LinkTree(class view *parent);
      class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
      void SetDataObject(class dataobject *object);
      void Update();
      void ReceiveInputFocus();
      void LoseInputFocus();
      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
      void GetOrigin(long width, long height, long *originX, long *originY);
      void *GetPSPrintInterface(char *printtype);

      virtual char *GetExpandedString(int pos, boolean *haspage);

      class fontdesc *font,*bfont,*bifont,*ifont;
      class keystate *keystate;
      boolean open;
      long top;
      long closebox, wherebox;
      int my_focus;
      class lpair *sections;
      class textview *textvs[header_TEXTS];
};

#endif /* _headrtv_H_ */
