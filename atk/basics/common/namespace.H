#ifndef _namespace_H_
#define _namespace_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <atom.H>


struct namespace_entry
{
  const class atom * name;
  long binding;
  short boundp;
};


#include <ATK.H>

	typedef boolean (*namespace_efptr)(long data, class Namespace *self, int x);
class Namespace : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
	Namespace();
	virtual ~Namespace();

  virtual int Enumerate( namespace_efptr proc, long data )  ;

  virtual void Clear();

  virtual int Lookup(const class atom * name)  ;
  virtual int LookupCreate(const class atom * name )  ;

  virtual short BoundpAt( int index )  ;
  virtual long ValueAt( int index )  ;
  virtual void UnbindAt( int index );
  virtual const class atom * NameAt( int index )  ;
  virtual void SetValueAt( int index, long value );

  virtual void SetValue( const class atom * name, long value );
  virtual long GetValue( const class atom * name )  ;
  virtual short Boundp( const class atom * name, long * data )  ;
  virtual void Unbind( const class atom * name);

  virtual const class atom * WhereIsValue( long value )  ;


  struct namespace_entry * nspace;
  short nspaceSize;
};
/** @} */
#endif /* _namespace_H_ */
