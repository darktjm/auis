#ifndef _timeoday_H_
#define _timeoday_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtimeoday Object timeoday
 *  A dynamically loadable ATK class file.  Contains timeoday, timeodayview,
 *  writestamp, writestampview.
 * @{ */

#include <fontdesc.H>
#include <event.H>
#include <time.h>

#include <dataobject.H>
class timeoday: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	timeoday();
      
	virtual ~timeoday();
    
       long Read (FILE *fp, long id)  ;
       long Write (FILE *fp, long id, int level)  ;
    
      virtual void SetFormat(char *format);
      virtual void SetFont(class fontdesc *f);
      virtual void UpdateTime();
      virtual void FormatTime();
      virtual void WriteDataPart(FILE *fp);
      virtual long ReadDataPart(FILE *fp)  ;
      virtual boolean InitializeDefaults()  ;
    
      inline char  * GetTod() { return (this->tod); }
      inline class fontdesc  * GetFont() { return (this->myfontdesc); }
      inline time_t   GetTime() { return (this->now); }
      inline char  * GetFormat() { return (this->format); }
      inline class event  * GetEvent() { return (this->ev); }
      inline void SetTime(long thetime) { (this->now = (thetime)); }
    
      char *tod, *format;
      time_t now;
      long epoch;
      class event *ev;
      class fontdesc *myfontdesc;
};

/** @} */
#endif /* _timeoday_H_ */
