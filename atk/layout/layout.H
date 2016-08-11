ATK_INTER
#ifndef _layout_H_
#define _layout_H_
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

/* $Header $ */
/* $ACIS$ */

 

/* layout.ch - layout data object definition and interface */

#define layout_VERSION 1
/* component data structure */

/*
Components are stored in a simple linked list.  Order is significant,
with earlier objects in the list drawn in front of later ones.

The forallcomponents(self, c) macro provides a loop down the list.
The cXxxxx macros provide geometry information.
*/

struct component {
    struct component *nextcomponent;	/* forward link in component list */
    class dataobject *data;		/* embedded data object or NULL if none */
    long left, top, width, height;	/* rectangle containing imbedded object */
    boolean varies:1;			/* TRUE if can be changed in execution mode */
};

#define forallcomponents(self, c) for (c = (self)->firstcomponent; c != NULL; c = c->nextcomponent)

#define cLeft(c)  ((c)->left)
#define cTop(c)  ((c)->top)
#define cRight(c) ((c)->left + (c)->width)
#define cBottom(c) ((c)->top + (c)->height)
#define cWidth(c) ((c)->width)
#define cHeight(c) ((c)->height)

#define cData(c) ((c)->data)
#define cVaries(c) ((c)->varies)


/* actual interface definition */

#include <dataobject.H>
class layout: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();



   char * ViewName()  ;
   long Read (FILE * f, long id)  ;
   long Write (FILE * f, long writeid, int level)  ;
   long GetModified()  ;


  virtual void ToggleDebug();
  virtual void RemoveComponent(struct component *c);
  virtual void FillInComponent(char *name, struct component *c);
  virtual struct component * CreateComponent()  ;
  virtual void SetComponentSize(struct component *c, long x, long y, long w, long h);
  virtual void Promote(struct component *c);
  virtual void Demote(struct component *c);
  virtual void MakeVariable(struct component *c);
  virtual void MakeFixed(struct component *c);


  inline struct component * GetFirstComponent() { return (this->firstcomponent); }


  
	static boolean InitializeClass();

  
	layout();
  
	virtual ~layout();


  struct component *firstcomponent;	    /* first in component list */
  int inGetModified;			    /* recursion protection switch */
};

/* end of layout.ch */

#endif /* _layout_H_ */
