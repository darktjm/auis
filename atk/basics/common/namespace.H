ATK_INTER
#ifndef _namespace_H_
#define _namespace_H_
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


 

/*
 *
 */

#include <atom.H>


struct namespace_entry
{
  class atom * name;
  long binding;
  short boundp;
};


#include <ATK.H>

#define namespace nmspace
	typedef boolean (*namespace_efptr)(long data, class namespace *self, int x);
class namespace : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
	namespace();
	virtual ~namespace();

  virtual int Enumerate( namespace_efptr proc, long data )  ;

  virtual void Clear();

  virtual int Lookup(class atom * name)  ;
  virtual int LookupCreate(class atom * name )  ;

  virtual short BoundpAt( int index )  ;
  virtual long ValueAt( int index )  ;
  virtual void UnbindAt( int index );
  virtual class atom * NameAt( int index )  ;
  virtual void SetValueAt( int index, long value );

  virtual void SetValue( class atom * name, long value );
  virtual long GetValue( class atom * name )  ;
  virtual short Boundp( class atom * name, long * data )  ;
  virtual void Unbind( class atom * name);

  virtual class atom * WhereIsValue( long value )  ;


  struct namespace_entry * nspace;
  short nspaceSize;
};



#endif /* _namespace_H_ */
