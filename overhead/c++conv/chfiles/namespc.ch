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


 

/*
 *
 */

#include <atom.ih>


struct namespace_entry
{
  struct atom * name;
  long binding;
  short boundp;
};


class namespace[namespc] {
    classprocedures:
      InitializeObject(struct namespace *self) returns boolean;
      FinalizeObject(struct namespace *self);
methods:
  Enumerate( procedure proc, long data ) returns int;

  Clear();

  Lookup(struct atom * name) returns int;
  LookupCreate(struct atom * name ) returns int;

  BoundpAt( int index ) returns short;
  ValueAt( int index ) returns long;
  UnbindAt( int index );
  NameAt( int index ) returns struct atom *;
  SetValueAt( int index, long value );

  SetValue( struct atom * name, long value );
  GetValue( struct atom * name ) returns long;
  Boundp( struct atom * name, long * data ) returns short;
  Unbind( struct atom * name);

  WhereIsValue( long value ) returns struct atom *;

data:
  struct namespace_entry * namespace;
  short namespaceSize;
};


