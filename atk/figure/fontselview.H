ATK_INTER
#ifndef _fontselview_H_
#define _fontselview_H_
/* fontselv.ch - font selection inset view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/fontselview.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $
*/

#include <lpair.H>

class fontselview  : public  lpair {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();
 
      
	fontselview();
      
	virtual ~fontselview();

    
       void ObservedChanged(class observable *fontsel, long status);
       void SetDataObject(class dataobject *dobj);

    
      virtual void ShowExtraOption();
      virtual void SetExtraOptionString(char *val);

    
      class lpair *lp1, *lp2;
      class fontsample *sample;
      boolean showdefault;
      char *defaultstring;

      class stringtbl *familytbl;
      class strtblview *vfamilytbl;
      short *familyacc, familyextra;
      char **familylist;
      int families_num;
      int familynum;

      class stringtbl *styletbl;
      class strtblview *vstyletbl;
      short *styleacc, styleextra;
      long *stylelist;
      int styles_num;
      long style_mask;

      class stringtbl *sizetbl;
      class strtblview *vsizetbl;
      short *sizeacc, sizeextra;
      short *sizelist;
      int sizes_num;
      int sizenum;

};

#endif /* _fontselview_H_ */
