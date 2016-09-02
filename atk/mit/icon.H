#ifndef _icon_H_
#define _icon_H_
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


#define icon_TitleChanged 34935	/* random integers... */
#define icon_SizeChanged 1894
#define icon_ChildChanged 2435
#define icon_ChangedSomehow 90235

#include <dataobject.H>
class icon : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	icon();
	virtual ~icon();
  
	static boolean InitializeClass();
	static boolean atk_inited;


     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;

    virtual void SetSize(long w,long h);
    virtual void GetSize(long * w, long * h);
    virtual void SetChild(class dataobject * child);
    virtual class dataobject * GetChild()  ;
    virtual void SetTitle(const char * title);
    virtual const char * GetTitle()  ;

    class dataobject * child;
    long width, height;
    char * title;
};

#endif /* _icon_H_ */
