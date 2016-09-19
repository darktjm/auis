ATK_INTER
#ifndef _rm_H_
#define _rm_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
