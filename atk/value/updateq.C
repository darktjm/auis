/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

#include <andrewos.h>
ATK_IMPL("updateq.H")
#include <updateq.H>
#define New(TYPE) ( TYPE *)malloc( sizeof (TYPE) )

/**************** statics ****************/


/**************** class procedures ****************/

ATKdefineRegistry(updateq, dataobject, NULL);

updateq::updateq()
     {
  this->view = NULL;
  this->queue = NULL;
  THROWONFAILURE( TRUE);
}


updateq::~updateq( )
     {
  (this)->ClearUpdateQueue();
}


void updateq::SetView( class view  * view )
          {
  this->view = view;
}


void updateq::EnqueueUpdateFunction( updateq_fptr  fp )
          {
  struct fp_queue *  newEntry;
  struct fp_queue ** qpointer;

  newEntry = New( struct fp_queue );
  newEntry->fp = fp;
  newEntry->next = NULL;
  for (qpointer = &this->queue; *qpointer != NULL;
       qpointer = &((*qpointer)->next));

  *qpointer = newEntry;
}


void updateq::ExecuteUpdateQueue()
     {
  struct fp_queue * qpointer;

  /* The implimentation of this allows the clearq and enque methods
    to be envoked by one of the functions that gets called here.  if you
    break this, you will loose in a serious way */

  while (this->queue != NULL)
    {
      qpointer = this->queue;
      this->queue = this->queue->next;
      (*(qpointer->fp))(this->view);
      free(qpointer);
    }

}


void updateq::ClearUpdateQueue()
     {
  struct fp_queue * qpointer;
  
  while (this->queue != NULL)
    {
      qpointer = this->queue;
      this->queue = this->queue->next;
      free(qpointer);
    }
}

