#ifndef _observable_H_
#define _observable_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#define observable_VERSION 1

#include <traced.H>

class aaction;
class avalueflex;
class observable;
typedef void (*observable_fptr)(ATK *rcvr, observable *self, long rock);

class observable : public  traced {
public:
	virtual ATKregistryEntry *ATKregistry();
	virtual void AddObserver (class observable *observer);
        virtual boolean IsObserver (class observable *observer)  ;
	virtual void RemoveObserver (class observable *observer);
	virtual void NotifyObservers (long value);
	virtual void ObservedChanged (class observable *changed, long value);

	/* the following methods implement a scheme for "triggers", 
		a set of named messages,  each trigger must first be
		defined with a call to DefineTrigger */
	virtual boolean AddRecipient(/* struct thisobject *self, */ class atom *trigger,
			ATK  *rcvr, observable_fptr func, 
			long rock)  ;
		/* when the trigger is Pulled, the 'func' will be called thus:
			func(rcvr, self, rock)  
		    AddRecipient returns FALSE if the trigger is not defined for the class 
		    (each rcvr can be registered only once with each trigger 
			on a given object)
		 */
	virtual boolean AddRecipient(class atom *trigger, ATK  *rcvr, const aaction &act);
	virtual void DeleteRecipient(/* struct thisobject *self, */ class atom *trigger,
			ATK  *rcvr);
		/* removes the receiver from the list of recipients */
	virtual void PullTrigger(/* struct thisobject *self, */ class atom *trigger);
		/* call all funcs associated with this trigger on this object 
			Only the object itself should call PullTrigger.  */
	virtual void PullTrigger(class atom *trigger, const avalueflex &args);
	/* if a client is calling a number of operations which would pull a trigger 
		too many times, it can disable the trigger temporarily.  
		It must later Enable the trigger.  At that time one call back 
		is autometically made for the trigger if it has 
		been Pulled one or more times in the interim.  */
	virtual void DisableTrigger(/* struct thisobject *self, */ class atom *trigger);
		/* until Enabled, this trigger will no produce call backs 
			Enable MUST be called once 
			for each time Disable has been called. */
	virtual void EnableTrigger(/* struct thisobject *self, */ class atom *trigger);
		/* this trigger will once again produce call backs  */
	virtual long DisableCount(/* struct thisobject *self, */ class atom *trigger)  ;
		/* returns a non-negative integer giving the
			number of outstanding calls to DisableTrigger
		   if the trigger does not exist, returns -1  */

	virtual void Destroy();


	
	observable();
	
	virtual ~observable();

	static void DefineTrigger(ATKregistryEntry  *classinstance, class atom *trigger);
		/* associate the atom as a possible trigger for the class.
			The xxx_InitializeClass routine can use
			xxx_classinfo as the first argument. */
	static class atomlist * ListTriggers(ATKregistryEntry  *classinstance)  ;
		/* returns a list of the triggers defined for the class and all super classes.
			The returned value has been newly created and must
			be atomlist_Destroy()ed by the caller.
			As a value for 'classinstance', those objects outside class xxx 
			can use an instance of xxx or 
				((struct xxx *)class_Load("xxx")). */



	short nObservers;			/* number of observers */
	short maxObservers;		/* number of entries in observers table */
	class observable **observers;	/* table of observers */
	struct triggerhousing *triggers;
};

#define observable_OBJECTDESTROYED -1
#define observable_OBJECTCHANGED 0
/** @} */
#endif /* _observable_H_ */
