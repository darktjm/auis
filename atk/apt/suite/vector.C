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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/apt/suite/RCS/vector.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Vector-object

MODULE	vector.c

VERSION	0.0

AUTHOR	GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Vector-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  04/10/89	Created (GW Keim)
  07/19/89	In vector_RemoveItem() now index the vector explicitly instead
                of relying on a terminating NULL data field. It is perfectly OK
                to have NULL data anywhere in the vector; (GW Keim)
		NOTE: There may be more instances of this NULL-terminator reliance.
   07/28/89	Reset removed data items to NULL in vector_DestroyItem(); (GW Keim);

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
ATK_IMPL("vector.H")
#include <stdio.h>

#include <vector.H>

#define Data			(self->data)    
#define InitialDataSize		(self->initial_vector_count)
#define DataSize		(self->current_vector_count)
#define DataUsed		(self->current_used_count)
#define ReallocFactor		(self->reallocation_factor)
#define Sorter			(self->sorter)
#define Destroyer		(self->destroyer)
#define ApplyProc		(self->apply)
#define Debug			(self->debug)
#define DataSpaceAvailable	(DataSize > DataUsed)


ATKdefineRegistry(vector, ATK, NULL);
#ifndef NORCSID
#endif
static void ReallocData( register class vector    *self );


vector::vector( )
{
    class vector *self=this;
  Data = NULL;
  Debug = DataSize = DataUsed = InitialDataSize = ReallocFactor = 0;
  ApplyProc = NULL;
  Sorter =  NULL;
  Destroyer = NULL;
  THROWONFAILURE((TRUE));
}

class vector *
vector::Create( register long			      init_data_size , register long			      reallocfactor )
    {
  register class vector	    *self = NULL;

  if(!(self = new vector)) {
    printf("vector: couldn't allocate new object.\n");
    exit(-1);
  }
  DataSize = InitialDataSize = init_data_size;
  ReallocFactor = reallocfactor;
  if(!(Data = (long*) calloc(InitialDataSize + 1, sizeof(long)))) {
    printf("vector:couldn't allocate enough memory.\n");
    exit(-1);
  }
  return(self);
}

vector::~vector( )
    {
    class vector *self=this;
  register long			 i = 0;

  if(Data)
    if(Destroyer)
      while((i < DataUsed) && Data[i]) 
        Destroyer(Data[i++]);
  if(Data) {
    free(Data);
    Data = NULL;
  }
}


static void
ReallocData( register class vector    *self )
  {
  register long		    i = 0;

  DataSize *= ReallocFactor;
  if(!(Data = (long*) realloc(Data,sizeof(long) * (DataSize + 1)))) {
    printf("vector: couldn't allocate enough memory.\n");
    exit(-1);
  }
  i = DataUsed;
  while(i < (DataSize+1))
    Data[i++] = 0;
}

long
vector::AddItem( long				  item )
    {
    class vector *self=this;
  register long	 i = 0, insertOffset = 0, end = 0;

  if(!DataSpaceAvailable) 
    ReallocData(this);
  if(Sorter) {
    while(Data[i] && (Sorter(&Data[i],&item) < 0)) i++;
    insertOffset = i;
    end = DataUsed - 1;
    while(end >= insertOffset) {
      Data[end + 1] = Data[end];
      end--;
    }
  }
  else insertOffset = DataUsed;
  Data[insertOffset] = item;
  DataUsed++;
  return(insertOffset);
}

boolean
vector::ItemExists( register long			  item )
{
    
    class vector *self=this;
  if((this)->Subscript(item) != -1) 
    return(TRUE);
  else 
    return(FALSE);
}

long
vector::RemoveItem( register long		      item )
{
    
    class vector *self=this;
  register long		     i = 0, removeOffset = 0;

  while(Data[i]) {
    if(Data[i] == item) {
      removeOffset = i;
      while((i+1) < DataUsed) {
        Data[i] = Data[i+1];
	i++;
      }
      DataUsed--;
      Data[DataUsed] = 0;
      return(removeOffset);
    }
    else i++;
  }
  return(-1);
}

long
vector::Sort( )
  {
    class vector *self=this;
  register long		    status = 0;

  if(!Sorter) 
    status = vector_status_no_sort_routine;
  else 
    qsort(Data,DataUsed,sizeof(long),QSORTFUNC(Sorter));
  return(status);
}

long
vector::Subscript( register long			  item )
    {
    class vector *self=this;
  register long			 i = 0;

  if(Data)
    while((i < DataUsed) && Data[i]) 
      if(Data[i] == item) 
        return(i);
      else 
        i++;
  return(-1);
}

void
vector::Apply(vector_applyfptr proc )
    {
    class vector *self=this;
  register int		   i = 0, status = 0;

  if(Data && Data[0])
    for(i = 0 ; (i < DataUsed) && !status ; i++)
      status = proc(Data[i]);
}