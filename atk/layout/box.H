ATK_INTER
#ifndef _box_H_
#define _box_H_
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

/* box.ch - box data object definition and interface */

#include <rect.h>

#define box_VERSION 1
/* globals for entire package */

/* actual interface definition */

#include <dataobject.H>
class box: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();



   const char * ViewName()  ;
   long Read (FILE * f, long id)  ;
   long Write (FILE * f, long writeid, int level)  ;
   long GetModified()  ;


  virtual void ToggleDebug();
  virtual void FillInContents(const char *name);




  
	static boolean InitializeClass();

  
	box();
  
	virtual ~box();


  int inGetModified;			    /* recursion protection switch */
  class dataobject *contents;		    /* contained object */
};

/* end of box.ch */

#endif /* _box_H_ */
