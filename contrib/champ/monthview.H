#ifndef _monthview_H_
#define _monthview_H_
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

#include <view.H>
class monthview: public view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	monthview();
      
	virtual ~monthview();
    
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
       void LinkTree(class view *parent);
    
      virtual void SetTextview(class textview *tv);
      virtual void GrabTextview(boolean ForceRedraw);
      virtual void ResetMonth(boolean ForceRedraw);
    
      class textview *tv;
      int mymonth, myyear;
      int skippedatstart;
      int EventCt[31];
      int textpositions[31];
      struct tm FullTimes[31];
      class text *t;
      const char *AnnounceArray[11];
      int AnnounceArraySize;
      int pendingtop;
      int firstpendingtop;
};

#endif /* _monthview_H_ */
