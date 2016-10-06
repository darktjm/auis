/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
 * internally, offsets into self->Namespace are the handle passed around.
 * -1 for invalid entries
 * namespace_entry.name == NULL indicates a malloced but empty entry
 */

/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(Namespace, ATK, NULL);
static int Index(class Namespace  * self,const class atom  * key);
static int Create( class Namespace  * self, const class atom  * key );


static int Index(class Namespace  * self,const class atom  * key)
          {
  int i;
  for (i = 0; i < self->nspaceSize; ++i)
    if (self->nspace[i].name == key)
      return i;
  return -1;
}


static int Create( class Namespace  * self, const class atom  * key )
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
Namespace::Namespace( )
          {
  this->nspace =
    (struct namespace_entry *)malloc(namespace_InitialSize *
				      sizeof (struct namespace_entry));
  for (this->nspaceSize = 0; this->nspaceSize < namespace_InitialSize;
       ++(this->nspaceSize))
    this->nspace[this->nspaceSize].name = NULL;

  THROWONFAILURE( TRUE);
}

Namespace::~Namespace()
{
    if(this->nspace) free(this->nspace);
    this->nspace=0;
}


int Namespace::Lookup( const class atom  * name )
          {
  return Index(this,name);
}


int Namespace::LookupCreate( const class atom  * name )
          {
  int index = Index( this, name );

  if (index < 0)
    index = Create( this, name );
  return index;
}


short Namespace::BoundpAt( int  index )
          {
  return (index >= 0) && (index < this->nspaceSize)
    && (this->nspace[index].name != NULL)
    && (this->nspace[index].boundp);
}

long Namespace::ValueAt( int  index )
          {
  return (index >= 0) && (index < this->nspaceSize)
    && (this->nspace[index].name != NULL)
    && (this->nspace[index].boundp)
    ? this->nspace[index].binding : -69;
}


void Namespace::UnbindAt( int  index )
          {
  if (index >= 0 && index < this->nspaceSize)
    this->nspace[index].boundp = False;
}


const class atom * Namespace::NameAt( int  index )
          {
  return (index >= 0 && index < this->nspaceSize) ?
    this->nspace[index].name : NULL;
}



void Namespace::SetValueAt( int  index, long  value )
               {
  if (index < this->nspaceSize && index >= 0
      && this->nspace[index].name != NULL)
    {
      this->nspace[index].binding = value;
      this->nspace[index].boundp = True;
    }
}


void Namespace::SetValue( const class atom  * name, long  value )
               {
  (this)->SetValueAt(  (this)->LookupCreate(name), value );
}


long Namespace::GetValue( const class atom  * name )
          {
  return (this)->ValueAt(  (this)->Lookup( name) );
}


short Namespace::Boundp( const class atom  * name, long  * value )
               {
  int index = Index( this, name );
  if (index >= 0 && this->nspace[index].boundp && value != NULL)
    *value = this->nspace[index].binding;
  return index >= 0 && this->nspace[index].boundp;
}

void Namespace::Unbind( const class atom  * name )
          {
  int index = Index( this, name );
  if (index >= 0)
    {
      this->nspace[index].boundp = False;
      this->nspace[index].binding = -69;
    }
}


const class atom * Namespace::WhereIsValue( long  value )
          {
  int x;
  for (x = 0; x < this->nspaceSize; ++x)
    if (this->nspace[x].name != NULL && this->nspace[x].boundp
	&& this->nspace[x].binding == value)
      break;
  return x < this->nspaceSize ? this->nspace[x].name : NULL;
}




int Namespace::Enumerate( namespace_efptr  proc, long  procdata )
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


void Namespace::Clear()
     {
  int x;
  for (x = 0;
       x < this->nspaceSize
       && this->nspace[x].name == NULL;
       ++x)
    this->nspace[x].name = NULL;
}
