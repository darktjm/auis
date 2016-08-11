/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


 


#include <atomlist.ih>

struct resourceList
{
  struct atomlist * name;
  struct atom * type;
  long data;
  short found;
};


package rm
{
 classprocedures:
  InitializeClass() returns boolean;
  ContextualPostResource( struct atomlist * context, struct atomlist *path,
			 long data, struct atom * type );
  PostResource( struct atomlist * path, long data,
	       struct atom * type );
  PostManyResources( struct resourceList * resources,
		    struct atomlist * context );
  GetResource( struct atomlist * name,
	      struct atomlist * className,
	      struct atom * type,
	      long * data ) returns short;
  GetManyResources( struct resourceList * resources,
		   struct atomlist * name,
		   struct atomlist * className );
  PostConverter( struct atom * fromtype, struct atom * totype, procedure converter );
};


/* conversion routines are called Convert( fromrock, outputrock )
   they should return FALSE (0) if the conversion fails */
