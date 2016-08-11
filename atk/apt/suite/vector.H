ATK_INTER
#ifndef _vector_H_
#define _vector_H_
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
/* $Source $ */





/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Vector-object

MODULE	vector.ch

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

END-SPECIFICATION  ************************************************************/

#define vector_status_no_error			    0
#define vector_status_not_in_vector		    1
#define vector_status_end_of_vector		    3
#define vector_status_root_of_vector		    4
#define vector_status_empty			    5
#define vector_status_no_sort_routine		    6
#define vector_status_null_element		    7

#include <ATK.H>
typedef void (*vector_destroyfptr)(long d);
typedef long (*vector_sortfptr)(long *d1, long *d2);
typedef int (*vector_applyfptr)(long d);

class vector : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	vector();
	
	virtual ~vector();
	static class vector* Create(long init_data_count, long reallocfactor )  ;
    
	inline vector_destroyfptr SetDestroyRoutine(vector_destroyfptr routine) { return (this->destroyer=routine); }
	inline vector_sortfptr SetSortRoutine(vector_sortfptr routine) { return (this->sorter=routine); }
	inline long &Item(int i) { return (this->data[i]); }
	inline long &Count() { return (this->current_used_count); }
	inline long SetIncrement(long inc) { return (this->reallocation_factor = inc); }
    
	virtual long AddItem( long item )  ;
	virtual long RemoveItem( long item )  ;
	virtual boolean ItemExists( long item )  ;
	virtual long Sort()  ;
	virtual long Subscript( long item )  ;
	virtual void Apply(vector_applyfptr proc )  ;
    
	long		*data,
			 initial_vector_count,
			 current_vector_count,
			 current_used_count,
			 reallocation_factor,
			 debug,
		       (*apply)();
	vector_sortfptr sorter;
	vector_destroyfptr destroyer;
};



/*
    $Log: vector.H,v $
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1993/06/10  20:21:37  rr2b
// Great atk_inited removal.
//
# Revision 1.2  1993/05/18  15:53:25  rr2b
# Added typedefs for the destroy, sort and apply function pointers.
# Made vector::Item and vector::Count return references so they behave more macro
# like.
#
# Revision 1.1  1993/05/05  21:32:07  rr2b
# Initial revision
#
*Revision 1.4  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.2  1991/09/12  19:20:39  bobg
Update copyright notice

Revision 1.1  1989/04/28  20:26:46  tom
Initial revision

*/

#endif /* _vector_H_ */
