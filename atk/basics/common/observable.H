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
/**< A callback function for triggers.
 *   \param rcvr The object used when the function was registered.
 *   \param self The object which pulled the trigger.
 *   \param rock User data supplied when the function was registered. */

/** Base class for observables and observers.
 *  An observable class sends messages to observers when it changes.  These
 *  messages consist of the object which changed, and an integer code
 *  describing the change.  Any number of observers may attach to an
 *  observable object; they will all be notified on changes in an
 *  arbitrary order.  They may, of course, detach themselves as well,
 *  even while being notified.  The observer must also be of this class,
 *  as there is no separate observing class.  The observer overrides a
 *  method in order to receive messages.
 *
 *  This class also implements an alternate, more complex method
 *  of notification.  This method involves named triggers, which
 *  invoke callback functions.  For these, only the observable object
 *  needs to derive from this class.  The observer can be of any class,
 *  or even a non-class C routine.  However, most classes in ATK use the
 *  above, simple method.
 *
 *  There is a global trigger registry.  A class registers the triggers
 *  its objects may pull.  An object then registers a function to call for
 *  that specfic trigger.  When the event occurs on a triggering object,
 *  all functions are called, in an arbitrary order.  One way in which
 *  a function can be registered for calling is the Ness `on event`
 *  construct.
 */
class observable : public  traced {
  public:
    TRACED_CLASS(observable);
    virtual void Destroy();
    observable();

    virtual void AddObserver (class observable *observer);
    /**< Add an observer of this object.
     *   All observers added by this method will be notified on any
     *   change to this object.  Adding the same observer twice does
     *   not add it again.  While it is safe to call this while
     *   being notified, additions may not be notified until the
     *   next time.
     *   \param observer The new observer. */
    virtual boolean IsObserver (class observable *observer)  ;
    /**< Check if an object is already observing this.
     *   \param observer The observer to check for.
     *   \return TRUE if \e observer is an observer of this object. */
    virtual void RemoveObserver (class observable *observer);
    /**< Remove an observer of this object.
     *   It is safe to remove an observer while it is being notified,
     *   but removing other observers has unpredictable results.
     *   \param observer The observer to remove.  If it is not present,
     *                   nothing happens.  */
    static const long OBJECTDESTROYED = -1;
    /**< Value for NotifyObservers():  I'm about to be destroyed. */
    static const long OBJECTCHANGED = 0;
    /**< Value for NotifyObservers():  I'm changing in an unspecified way. */
    virtual void NotifyObservers (long value);
    /**< Notify all observers of this object of a change.
     *   The nature of the change is provided as well.  Derived classes
     *   may define their own; the only ones defined by this class are
     *   observable::OBJECTDESTROYED, which indicates that the observable
     *   object is about to be destroyed, and observable::OBJECTCHANGED,
     *   which is an unspecified change.  Child-defined codes should be
     *   greater than zero.   The order in which observers is
     *   called should not be relied on.
     *   \param value The nature of the change.  */
    virtual void ObservedChanged (class observable *changed, long value);
    /**< Receive notification of any observed objects.
     *   This method must be overriden to receive notifications.  It is
     *   recommended that the parent class' version be called as well.
     *   When an observable calls NotifyObservers(), it calls this method
     *   on all observers, giving it a pointer to the changed object and
     *   the integer giving the reason for the change.
     *   \param changed The object which changed.
     *   \param value The nature of the change, provided by the changing object. */

	/* the following methods implement a scheme for "triggers", 
		a set of named messages,  each trigger must first be
		defined with a call to DefineTrigger */
    virtual boolean AddRecipient(const class atom *trigger,
			ATK  *rcvr, observable_fptr func, 
			long rock)  ;
    /**< Register a function to be called for a trigger.
     *   The trigger must be a registered trigger for this object's class
     *   or one of its ancestors.  The function will silently overwrite
     *   any previously added function or action for this receiver object.
     *   \param trigger The name of the trigger.
     *   \param rcvr The object pointer passed to the callback.  This
     *               is expected to be the object of the caller.  Only
     *               one callback can be registered per rcvr.
     *    \param func The function to call.
     *    \param rock User data to supply to the function when it is called.
     *    \return TRUE if successful.  Reasons for failure include the
     *            trigger not being defined, or memory allocation errors. */
    virtual boolean AddRecipient(const class atom *trigger, ATK  *rcvr, const aaction &act);
    /**< Register an action to be called for a trigger.
     *   The trigger must be a registered trigger for this object's class
     *   or one of its ancestors.  The action will silently overwrite
     *   any previously added function or action for this receiver object.
     *   \param trigger The name of the trigger.
     *   \param rcvr The object pointer passed to the callback.  This
     *               is expected to be the object of the caller.  Only
     *               one callback can be registered per rcvr.
     *    \param act The action to perform.
     *    \return TRUE if successful.  Reasons for failure include the
     *            trigger not being defined, or memory allocation errors. */
    virtual void DeleteRecipient(const class atom *trigger,
			ATK  *rcvr);
    /**< Remove the registered function or action to be called for an object.
     *   There will be at most one action for each object; after this call,
     *   there will be none.
     *   \param trigger The name of the trigger.
     *   \param rcvr The object pointer used with AddRecipient(). */
    virtual void PullTrigger(const class atom *trigger);
    /**< Call all functions associated with a trigger for this object.
     *   Functions are called with the registered object, the registered
     *   rock, and this object.  Actions are called with a single input
     *   parameter:  the object pulling the trigger.  Its name is "object".
     *   Return values from the action are ignored.
     *   If the trigger callee deletes the object pulling the trigger,
     *   further functions and actions will not be called.
     *   \param trigger The name of the trigger. */
    virtual void PullTrigger(const class atom *trigger, const avalueflex &args);
    /**< Call all functions associated with a trigger for this object.
     *   Functions are called with the registered object, the registered
     *   rock, and this object.  Actions are called with the given input
     *   arguments.  Return values from the action are ignored.
     *   If the trigger callee deletes the object pulling the trigger,
     *   further functions and actions will not be called.
     *   \param trigger The name of the trigger.
     *   \param args The input arguments for the trigger.  For
     *               consistency, the calling object should add a pointer
     *               to itself as an argument named "object".  */
    virtual void DisableTrigger(const class atom *trigger);
    /**< Globally disable a trigger.
     *   Calls to DisableTrigger() can be nested; each must have a matching
     *   call to EnableTrigger().  This disables the trigger for all
     *   receivers until re-enabled.  If the trigger is pulled one or
     *   more times while disabled, it will be pulled once automatically
     *   when re-enabled.  This should be used sparingly, only if an action
     *   is known to cause numerous triggers when only one would suffice.
     *   Always re-enable as soon as possible, as forgetting will leave
     *   the trigger permanently disabled, for all receivers.
     *   \param trigger The trigger. */
    virtual void EnableTrigger(const class atom *trigger);
    /**< Globally re-enable a trigger.
     *   This *must* be called once for every call to DisableTrigger().
     *   If the trigger is enabled as a result of this call (it may not
     *   if multiple disables are pending), and the trigger was pulled
     *   by this object at least once while disabled, the trigger will
     *   be pulled using only the default argument list for actions.
     *   \param trigger The trigger. */
    virtual long DisableCount(const class atom *trigger)  ;
    /**< Indicate how many times DisableTrigger() has been called.
     *   \param trigger The trigger to check.
     *   \return the number of times EnableTrigger() must be called to
     *           enable the trigger, if greater than or equal to zero;
     *           otherwise, the requested trigger does not exist. */

    static void DefineTrigger(ATKregistryEntry  *classinstance, const class atom *trigger);
    /**< Create a trigger for a given class.
     *   This creates a potential event trigger on objects of the given class
     *   with the name given by an atom.
     *   \param classinstance The ATK registry information for the class.
     *                        Since these registrations should be done in
     *                        the class-wide initialization routine,
     *                        &<em>class</em>_ATKregistry_ can be used.
     *   \param trigger The name of the trigger. */
    static class atomlist * ListTriggers(ATKregistryEntry  *classinstance)  ;
    /**< Find triggers registered for a class.
     *   \param classinstance The ATK registry information for the class.
     *                        If you have an object of the class, use
     *                        the ATKregistry() method.  Otherwise, you can
     *                        use the ATKCLASS() macro to obtain a pointer.
     *   \return a list of trigger atoms defined for the class and all of its
     *           ancestors.  The returned value must be destroyed using its
     *           Destroy() method. */

    /* I guess allowing clients to modify the observer itself is OK */
    /* In any case, all current clients cast to non-const, so I can't stop it */
    inline observable * const *GetObservers(int *n) { *n = nObservers; return observers; }
    /**< Retrieve list of observers.
     *   This returns the current list of observers.  It returns pointers to
     *   static space which may change if observers are added or deleted.
     *   \param n The number of observers is returned here.
     *   \return An array of observers.
     */
  private:
    int FindObserver(class observable *observer); // ick
    struct triggerhousing *FindOrMakeTrigger(const atom *trigger); // ick
    short maxObservers;		/* number of entries in observers table */
    struct triggerhousing *triggers;
    // access via GetObservers to preserve constness.
    short nObservers;			/* Number of observers. */
    class observable **observers;	/* Table of observers. */
};
/** @} */
#endif /* _observable_H_ */
