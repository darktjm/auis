ATK_INTER
#ifndef _buttonV_H_
#define _buttonV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <fontdesc.H>
#include <sbutton.H>

#define buttonV_NONE 0
#define buttonV_SURROUND 1
#define buttonV_ADJACENT 2
#define buttonV_EITHER 3
#include <valueview.H>
class buttonV : public  valueview
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
 class view * Hit(enum view_MouseAction type, long x, long y, long numberOfClicks)
     ;
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview * DoHit(enum view_MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  virtual void CacheSettings();
  virtual void DrawButtonText(const char *text,long len,struct rectangle *rect,struct rectangle *rect2,boolean pushd);
  virtual void HandleStyleString(char *s);

  
	static boolean InitializeClass();

  
	buttonV();
  
	virtual ~buttonV(); 

  inline void SetButtonType(short TYPE) { (this->buttontype = TYPE); }
  inline short GetButtonType() { return (this->buttontype); }
  inline void SetFixedCount(short VAL) { (this->fixedcount = (VAL)); }
  inline short GetFixedCount() { return (this->fixedcount); }
  inline void SetFourway(boolean VAL) { (this->fourway = (VAL)); }
  inline boolean GetFourway() { return (this->fourway); }
  inline void SetFixedColumns(long VAL) { (this->fixedcolumns = this->columns = (VAL)); }
  inline long GetFixedColumns() { return (this->fixedcolumns); }
 
  const char * label;
  char *l[4];
  const char * fontname;
  short fontsize;
  short buttontype;
  short count,fixedcount;
  long max,rtl,rhw,x,y;
  class fontdesc * activefont;
  boolean pushed,valueset,movedoff,vertical,topdown,fourway;
  class list *listp;
  struct buttonV_rl *current;
  long bcnt,offset,columns,bsize,fixedcolumns; 
  struct sbutton_info si; /* TO BE REMOVED */
  struct sbutton_prefs *prefs;
  int mono;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
*/

#endif /* _buttonV_H_ */
