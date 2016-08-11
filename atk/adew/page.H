ATK_INTER
#ifndef _page_H_
#define _page_H_
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


 

/* The following may be use in place of a specific position  
	in the methods taking position arguments below  */
#define page_CURRENT		 -1
#define page_AFTERCURRENT	 -2
#define page_BEFORECURRENT	 -3
#define page_ATEND 		 -4
#define page_ATBEGINING	 -5

struct page_switchee {
    class dataobject *d;
    char *viewname;
    char *label;
    struct page_switchee *next; 
};

#include <dataobject.H>
class page : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	page();
      
	virtual ~page();
    
       long Read (FILE *file, long id)  ;
       long Write (FILE *file, long writeid, int level)  ;
       char * ViewName()  ;
       long GetModified()  ;
    
      inline boolean GetPostMenus() { return ((this)->PostMenusFlag); }
      inline void SetPostMenus(boolean foo) { (((this)->PostMenusFlag) = foo); }
      inline class dataobject * GetFirstObject() { return ((((this)->FirstSwitchee) == NULL) ? NULL: (this)->FirstSwitchee->d); }
      inline class dataobject * GetNowPlaying() { return ((((this)->NowPlaying) == NULL) ? NULL: (this)->NowPlaying->d); }

       /* The following macro methods are private */
      inline struct page_switchee * GetFirstSwitchee() { return ((this)->FirstSwitchee); }
      inline struct page_switchee * GetNowPlayingSwitchee() { return ((this)->NowPlaying); }
       
    
      virtual boolean AddObject(class dataobject *d, char *label,
		 char *viewname,long position)  ;
      virtual boolean DeleteObject(class dataobject *d)  ;
      virtual boolean SetNowPlaying(class dataobject *d)  ;
      virtual boolean SetNowPlayingByName(char *name)  ;
      virtual char * GetNowPlayingName()  ;
      virtual boolean SetNowPlayingByPosition(long position)  ;
      virtual long GetPositionOfObject(class dataobject *d)  ;
      virtual char * GetNameOfObject(class dataobject *d)  ;	
      virtual class dataobject * GetObjectAtPosition(long position)  ;
      virtual class dataobject * GetObjectByName(char *name)  ;
      virtual long GetObjectCount()  ;
/* The following methods are private */
      virtual char * GetSwitcheeName(struct page_switchee *sw)  ;
    
      struct page_switchee *FirstSwitchee, *NowPlaying;
      boolean PostMenusFlag,executingGetModified;
};

#endif /* _page_H_ */
