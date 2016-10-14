#ifndef _updateq_H_
#define _updateq_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libvalue
 * @{ */
#include <dataobject.H>
#include <view.H>
typedef void (*updateq_fptr)(class view *);

struct fp_queue {
  updateq_fptr fp;
  struct fp_queue * next;
};

class updateq : public  dataobject {
public:
	TRACED_CLASS(updateq);
	updateq();


 virtual void SetView( class view * view );
 virtual void EnqueueUpdateFunction( updateq_fptr fp );
 virtual void ExecuteUpdateQueue();
 virtual void ClearUpdateQueue();

 class view * view;
 struct fp_queue * queue;
};
/** @} */
#endif /* _updateq_H_ */
