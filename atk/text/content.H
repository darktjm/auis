ATK_INTER
#ifndef _content_H_
#define _content_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
