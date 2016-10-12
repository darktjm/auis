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
/** \addtogroup libgtext
 * @{ */
/*****************************************************************************
 * 
 * gtextv.c -- The Gesture Text View Module
 *
 *****************************************************************************/

#include <textview.H>
class gtextv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


 
   class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)
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


/** @} */
#endif /* _gtextv_H_ */
