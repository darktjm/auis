ATK_INTER
#ifndef _updateq_H_
#define _updateq_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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


#include <dataobject.H>
#include <view.H>
typedef void (*updateq_fptr)(class view *);

struct fp_queue {
  updateq_fptr fp;
  struct fp_queue * next;
};

class updateq : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	updateq();
	~updateq();


 virtual void SetView( class view * view );
 virtual void EnqueueUpdateFunction( updateq_fptr fp );
 virtual void ExecuteUpdateQueue();
 virtual void ClearUpdateQueue();

 class view * view;
 struct fp_queue * queue;
};


#endif /* _updateq_H_ */
