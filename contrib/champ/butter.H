#ifndef _butter_H_
#define _butter_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <dataobject.H>
#include <view.H>
class butter;
typedef void (*butter_hitfptr)(char *rockptr, long rocklong, class butter *self, enum view::MouseAction act);
class butter: public  dataobject {
public:
	TRACED_CLASS_NODEST(butter);

    
      
	butter();
    
      virtual void SetText(const char *txt);
      virtual void SetHitFunction(butter_hitfptr HitFunction);
      virtual void SetButtonFont(class fontdesc *f);
      virtual void SetRocks(char *r1, int r2);
    
      inline char * GetText() { return (this->text); }
      inline butter_hitfptr GetHitFunction() { return (this->HitFunction); }
      inline class fontdesc * GetButtonFont() { return (this->myfontdesc); }
      inline char * GetRockPtr() { return (this->rockptr); }
      inline long GetRockInt() { return (this->rockint); }
    
      char *text, *rockptr, *myfontname;
      butter_hitfptr HitFunction;
      long rockint, myfonttype, myfontsize;
      class fontdesc *myfontdesc;
      class cursor *mycursor;
};
/** @} */
#endif /* _butter_H_ */
