#ifndef _timeoday_H_
#define _timeoday_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <fontdesc.H>
#include <event.H>

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


#endif /* _timeoday_H_ */
