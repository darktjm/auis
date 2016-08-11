#ifndef _gtextv_H_
#define _gtextv_H_
/******************************************************************************
 *
 * gtextv - Gesture Text View
 * Medical Informatics 
 * Washington University, St. Louis
 * July 29, 1991
 *
 * Scott Hassan
 * Steve Cousins
 * Mark Frisse
 *
 *****************************************************************************/

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


/*****************************************************************************
 * 
 * gtextv.c -- The Gesture Text View Module
 *
 *****************************************************************************/

#include <textview.H>
class gtextv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


 
   class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)
     ;
 
 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	gtextv();
  
	virtual ~gtextv();

 
  int *xp, *yp;          /* Mouse Buffering Arrays */
  int index;             /* The current length of the buffers */
  int limit;             /* The current limit of the buffers */
  long parstart, parend; /* The starting and ending positions for */
                         /* the selected region. */
}; 



#endif /* _gtextv_H_ */
