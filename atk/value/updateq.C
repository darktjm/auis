/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("updateq.H")
#include <updateq.H>
#define New(TYPE) ( TYPE *)malloc( sizeof (TYPE) )

/**************** statics ****************/


/**************** class procedures ****************/

ATKdefineRegistryNoInit(updateq, dataobject);

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

