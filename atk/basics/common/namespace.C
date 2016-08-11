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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/namespace.C,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 



#include <andrewos.h>
ATK_IMPL("namespace.H")
#include <namespace.H>
#include <atom.H>
#define namespace_InitialSize 4
#define Empty
#ifndef True
#define True 1
#define False 0
#endif /* True */

/* style notes
 * internally, offsets into self->namespace are the handle passed around.
 * -1 for invalid entries
 * namespace_entry.name == NULL indicates a malloced but empty entry
 */

/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(namespace, ATK, NULL);
#ifndef NORCSID
#endif
#ifndef True
#endif /* True */
static int Index(class namespace  * self,class atom  * key);
static int Create( class namespace  * self, class atom  * key );


static int Index(class namespace  * self,class atom  * key)
          {
  int i;
  for (i = 0; i < self->nspaceSize; ++i)
    if (self->nspace[i].name == key)
      return i;
  return -1;
}


static int Create( class namespace  * self, class atom  * key )
          {
  struct namespace_entry * de;

  if (self->nspace[self->nspaceSize - 1].name != NULL)
    {
      int i = self->nspaceSize * 2;
      self->nspace =
	(struct namespace_entry *)realloc(self->nspace,
					   i * sizeof (struct namespace_entry));
      while (self->nspaceSize < i)
	self->nspace[self->nspaceSize++].name = NULL;
    }

  for (de = self->nspace; de->name != NULL; ++de)
    Empty;

  de->name = key;
  de->boundp = False;
  de->binding = -69;
  return de - self->nspace;
}


/****************************************************************/
/*		instance methods				*/
/****************************************************************/


/* (just in case your wondering, this is not a method) */
namespace::namespace( )
          {
  this->nspace =
    (struct namespace_entry *)malloc(namespace_InitialSize *
				      sizeof (struct namespace_entry));
  for (this->nspaceSize = 0; this->nspaceSize < namespace_InitialSize;
       ++(this->nspaceSize))
    this->nspace[this->nspaceSize].name = NULL;

  THROWONFAILURE( TRUE);
}

namespace::~namespace()
{
    if(this->nspace) free(this->nspace);
    this->nspace=0;
}


int namespace::Lookup( class atom  * name )
          {
  return Index(this,name);
}


int namespace::LookupCreate( class atom  * name )
          {
  int index = Index( this, name );

  if (index < 0)
    index = Create( this, name );
  return index;
}


short namespace::BoundpAt( int  index )
          {
  return (index >= 0) && (index < this->nspaceSize)
    && (this->nspace[index].name != NULL)
    && (this->nspace[index].boundp);
}

long namespace::ValueAt( int  index )
          {
  return (index >= 0) && (index < this->nspaceSize)
    && (this->nspace[index].name != NULL)
    && (this->nspace[index].boundp)
    ? this->nspace[index].binding : -69;
}


void namespace::UnbindAt( int  index )
          {
  if (index >= 0 && index < this->nspaceSize)
    this->nspace[index].boundp = False;
}


class atom * namespace::NameAt( int  index )
          {
  return (index >= 0 && index < this->nspaceSize) ?
    this->nspace[index].name : NULL;
}



void namespace::SetValueAt( int  index, long  value )
               {
  if (index < this->nspaceSize && index >= 0
      && this->nspace[index].name != NULL)
    {
      this->nspace[index].binding = value;
      this->nspace[index].boundp = True;
    }
}


void namespace::SetValue( class atom  * name, long  value )
               {
  (this)->SetValueAt(  (this)->LookupCreate(name), value );
}


long namespace::GetValue( class atom  * name )
          {
  return (this)->ValueAt(  (this)->Lookup( name) );
}


short namespace::Boundp( class atom  * name, long  * value )
               {
  int index = Index( this, name );
  if (index >= 0 && this->nspace[index].boundp && value != NULL)
    *value = this->nspace[index].binding;
  return index >= 0 && this->nspace[index].boundp;
}

void namespace::Unbind( class atom  * name )
          {
  int index = Index( this, name );
  if (index >= 0)
    {
      this->nspace[index].boundp = False;
      this->nspace[index].binding = -69;
    }
}


class atom * namespace::WhereIsValue( long  value )
          {
  int x;
  for (x = 0; x < this->nspaceSize; ++x)
    if (this->nspace[x].name != NULL && this->nspace[x].boundp
	&& this->nspace[x].binding == value)
      break;
  return x < this->nspaceSize ? this->nspace[x].name : NULL;
}




int namespace::Enumerate( namespace_efptr  proc, long  procdata )
               {
  int x;
  for ( x = 0;
       (x < this->nspaceSize)
       && this->nspace[x].name != NULL
       &&  ((*proc)( procdata, this, x ));
       ++x)
    Empty;
  return (x < this->nspaceSize) &&
    (this->nspace[x].name != NULL) ? x : -69;
}


void namespace::Clear()
     {
  int x;
  for (x = 0;
       x < this->nspaceSize
       && this->nspace[x].name == NULL;
       ++x)
    this->nspace[x].name = NULL;
}