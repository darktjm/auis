ATK_INTER
#ifndef _rm_H_
#define _rm_H_
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


 


#include <atomlist.H>

struct resourceList
{
  class atomlist * name;
  class atom * type;
  long data;
  short found;
};


#include <ATK.H>

	typedef short (*rm_fptr)(long data, long *converted);

class rm : public ATK 
{
public:
	virtual ATKregistryEntry *ATKregistry();
  
	static boolean InitializeClass();

  static void ContextualPostResource( class atomlist * context, class atomlist *path,
			 long data, class atom * type );
  static void PostResource( class atomlist * path, long data,
	       class atom * type );
  static void PostManyResources( struct resourceList * resources,
		    class atomlist * context );
  static short GetResource( class atomlist * name,
	      class atomlist * className,
	      class atom * type,
	      long * data )  ;
  static void GetManyResources( struct resourceList * resources,
		   class atomlist * name,
		   class atomlist * className );
  static void PostConverter( class atom * fromtype, class atom * totype, rm_fptr converter );
};


/* conversion routines are called Convert( fromrock, outputrock )
   they should return FALSE (0) if the conversion fails */

#endif /* _rm_H_ */
