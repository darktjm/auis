ATK_INTER
#ifndef _content_H_
#define _content_H_
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



struct content_chapentry {
    class mark *rem,*loc;
    struct content_chapentry *next;
    int which,space;
};

#include <text.H>

class content : public  text {
  public:
      virtual ATKregistryEntry *ATKregistry();


      void ObservedChanged (class observable *changed, long value);
      const char * ViewName()  ;

      virtual void reinit();
      virtual void SetSourceText(class text *txt);
      virtual void UpdateSource(long pos,long len);
      virtual long Enumerate(long pos,long len,const char *start);
      virtual void Denumerate(long pos,long len);
      virtual long StringToInts(long pos,int *lev);
      virtual class mark * locate(long pos);
      virtual struct content_chapentry* CopyEntry(long pos,long len,char *buf,long buflen);

      static int StyleNameContentLevel(const char *stylename);

      content();
      virtual ~content();

      struct content_chapentry *entry,*indexentry;
      class text *srctext;
      int InUpdate;
      int enumerate;
      char *ChapNumber;
      boolean doindent,isindented;
      int chapcount;
};

 

#endif /* _content_H_ */
