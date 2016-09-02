/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */


/* Copyright 1992 Carnegie Mellon University, All rights reserved.
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
ATK_IMPL("observable.H")
#include <observable.H>
#include <atom.H>
#include <atomlist.H>
#include <owatch.H>
#include <aaction.H>

#define INITIALNUMOBSERVERS 4


/* a triggerinstance is a particular function to be called when a trigger fires */
struct triggerinstance {
	struct triggerinstance *next;
	observable_fptr func;
	aaction *act;
	ATK  *rcvr;
	long rock;
};

/* a triggerhousing is the intersection of a trigger atom with a particular object
	it contains the list of functions to be called when the trigger is pulled */
struct triggerhousing {
	struct triggerhousing *next;
	class atom *trigger;
	struct triggerinstance *instances;
	long disablecount;	/* enable if zero. disabled if > 0 */
	boolean firepending;	/* set True if fired while disabled */
};

/* a triggerclass is the list of all triggers defined for a class */
struct triggerclass {
	struct triggerclass *next;
	struct ATKregistryEntry  *class_c;   /* class_GetType(instance) */
	class atomlist *triggers;
};


static struct triggerclass *Triggers = NULL;	/* the list of defined triggers */





ATKdefineRegistry(observable, traced, NULL);
static int FindObserver(class observable  *self, class observable  *observer  );


observable::observable()
{
    this->nObservers = 0;
    this->maxObservers = 0;
    this->observers = NULL;
    this->triggers = NULL;
    THROWONFAILURE( TRUE);
}

observable::~observable()
{
    struct triggerinstance *ti, *tit;
    struct triggerhousing *th, *tht;
    if (this->observers)
	free(this->observers);

    /* free all triggerinstances and triggerhousings hanging from self->triggers */
    for (th = this->triggers; th != NULL; th = tht) {
	for (ti = th->instances; ti != NULL; ti = tit) {
		tit = ti->next;
		free(ti);
	}
	tht = th->next;
	free(th);
    }
}

void observable::Destroy() {
    if(refcount==1) NotifyObservers( observable_OBJECTDESTROYED);
    traced::Destroy();
}
    
/* Finds the index of the observer in self observers table.  Returns -1 if observer is not in the list
 */
static int FindObserver(class observable  *self, class observable  *observer  )
{
    int i = 0;
    class observable **observers;

    for (i = 0, observers = self->observers; i < self->nObservers; i++, observers++)
	if (*observers == observer) return i;
    
    return -1;
}

boolean observable::IsObserver(class observable  *observer  )
{
    return (FindObserver(this, observer) != -1 ? TRUE: FALSE);
}

void observable::AddObserver(class observable  *observer  )
{
    if (this->maxObservers == 0)  {
	this->maxObservers = INITIALNUMOBSERVERS;
	this->observers = (class observable **) malloc (INITIALNUMOBSERVERS * sizeof(class observable *));
    }
    else if (FindObserver(this, observer) != -1) return;
    else if (this->nObservers == this->maxObservers)  {
	this->maxObservers += this->maxObservers / 2;
	this->observers = (class observable **) realloc(this->observers, this->maxObservers * sizeof(class observable *));
    }
    this->observers[this->nObservers++] = observer;
}

void observable::RemoveObserver(class observable  *observer  )
{
    int i;

    if ((i = FindObserver(this, observer)) != -1)  {
	while (++i < this->nObservers) {
	    this->observers[i - 1] = this->observers[i];
	}
	this->nObservers -= 1;
    }
}

void observable::NotifyObservers(long  value  )
{
    int i;

    if(this->ReferenceCount()>0) this->Reference();
    
    /*
      We go backwards since objects may be removed from this list
      as we go along.
      */

    for (i = this->nObservers - 1;  i >= 0; i--) {
	(this->observers[i])->ObservedChanged( this, value);
    }

    if(this->ReferenceCount()>0) this->Destroy();
}


void observable::ObservedChanged(class observable  *changed, long  value  )
{
    
}
	/* the following methods implement a scheme for "triggers", 
		a set of named messages,  each trigger must first be
		defined with a call to DefineTrigger*/



/* observable__DefineTrigger(classID, classinstance, trigger)
	associate the atom as a possible trigger for the class 
*/
	void
observable::DefineTrigger(ATKregistryEntry   *info, class atom  *trigger)
			{
	struct triggerclass *tc;
	for (tc = Triggers; 
			tc != NULL && tc->class_c != info;
			tc = tc->next)
		{};
	if (tc == NULL) {
		tc = (struct triggerclass *)malloc(sizeof (struct triggerclass));
		tc->class_c = info;
		tc->triggers = new atomlist;
		tc->next = Triggers;
		Triggers = tc;
	}
	if ( ! (tc->triggers)->Memberp( trigger))
		(tc->triggers)->Prepend( trigger);
}

/* observable__ListTriggers(classID, classinstance)
	returns a list of the triggers defined for the class 
	the caller must destroy the list 
*/
	class atomlist *
observable::ListTriggers(ATKregistryEntry   *info)
		{
	struct triggerclass *tc;
	class atomlist *result;
	result = new atomlist;

	for (tc = Triggers; tc != NULL; tc = tc->next)
		if ((info)->IsType( tc->class_c))
			(result)->JoinToEnd( tc->triggers);
	return result;
}

static struct triggerhousing *FindOrMakeTrigger(observable *self, atom *trigger) {
    struct triggerclass *tc;
    struct triggerhousing *th;
    for (th	= self->triggers; th !=	NULL &&	th->trigger != trigger; th = th->next) {};
    if (th == NULL) {
	/* find out if the trigger is defined */
	/* we have to loop through ALL the classes since triggers should be inheritable but some triggers may be defined for the superclass and others for the subclass */
	for (tc = Triggers;  tc != NULL; tc = tc->next) {
	    if((self)->IsType( tc->class_c) && (tc->triggers)->Memberp( trigger)) break;
	};
	if (tc == NULL)
	    /* not defined */
	    return NULL;
	/* make a new trigger housing: self is self's first recipient for self trigger */
	th = (struct triggerhousing *)malloc(sizeof(struct triggerhousing));
	th->trigger = trigger;
	th->instances = NULL;
	th->disablecount = 0;
	th->firepending = FALSE;
	th->next = self->triggers;
	self->triggers = th;
    }
    return th;
}

boolean observable::AddRecipient(class atom  *trigger, ATK   *rcvr, const aaction &act) {
    struct triggerhousing *th=FindOrMakeTrigger(this,trigger);
    struct triggerinstance *ti;
    if(th==NULL) return FALSE;
 /* th now points to an appropriate triggerhousing */

 /* find out if this is a redefinition */
    for (ti = th->instances; ti != NULL; ti = ti->next)
	if (ti->rcvr == rcvr) {
	    ti->func = NULL;
	    ti->act=(aaction *)&act;
	    ti->rock = 0;
	    return TRUE;
	}
 /* add a new trigger instacne */
    ti = (struct triggerinstance *)malloc(sizeof(struct triggerinstance));
    ti->func = NULL;
    ti->act=(aaction *)&act;
    ti->rcvr = rcvr;
    ti->rock = 0;
    ti->next = th->instances;
    th->instances = ti;
    return TRUE;
}



/* observable__AddRecipient(self, trigger, rcvr, func, rock)
	when the trigger is Pull'ed, the func will be called thus:
		func(rcvr, self, rock)
*/
	boolean
observable::AddRecipient(class atom  *trigger, ATK   *rcvr, observable_fptr func, long  rock)
					{
	struct triggerhousing *th=FindOrMakeTrigger(this,trigger);
	struct triggerinstance *ti;
	if(th==NULL) return FALSE;
	
	/* th now points to an appropriate triggerhousing */

	/* find out if this is a redefinition */
	for (ti = th->instances; ti != NULL; ti = ti->next)
		if (ti->rcvr == rcvr) {
			ti->func = func;
			ti->rock = rock;
			return TRUE;
		}
	/* add a new trigger instacne */
	ti = (struct triggerinstance *)malloc(sizeof(struct triggerinstance));
	ti->func = func;
	ti->rcvr = rcvr;
	ti->rock = rock;
	ti->next = th->instances;
	th->instances = ti;
	return TRUE;
}

/* observable__DeleteRecipient(self, trigger, rcvr)
	removes the receiver from the list of recipients
*/
	void
observable::DeleteRecipient(class atom  *trigger, ATK   *rcvr)
			{
	struct triggerhousing *th;
	struct triggerinstance *ti, *pi;
	for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
		{};
	if (th == NULL)  return;
	
	for (pi = NULL, ti = th->instances; ti != NULL && ti->rcvr != rcvr; pi = ti, ti = ti->next)
		{};
	if (ti == NULL) return;

	if (pi == NULL)
		th->instances = ti->next;
	else
		pi->next = ti->next;
	free(ti);
}


/* observable__PullTrigger(self, trigger)
	call all funcs associated with this trigger on this object
 */
void observable::PullTrigger(class atom  *trigger, const avalueflex &args) {
    struct triggerhousing *th;
    struct triggerinstance *ti;
    struct owatch_data *w1;
    for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
    {};
    if (th == NULL)
	return;

    if (th->disablecount > 0) {
	th->firepending = TRUE;
	return;
    }
    /* add check to see if this object gets destroyed by the
     trigger recipient. */
    w1=owatch::Create(this);
    for (ti = th->instances; ti != NULL; ti = ti->next) {
	if(ti->func) (ti->func)(ti->rcvr, this, ti->rock);
	else if(ti->act) {
	    avalueflex out;
	    (*ti->act)(ti->rcvr, args, out);
	}
	if(!owatch::Check(w1)) return;
    }
    owatch::Delete(w1);
}
	void
observable::PullTrigger(class atom  *trigger)
		{
	struct triggerhousing *th;
	struct triggerinstance *ti;
	struct owatch_data *w1;
	for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
		{};
	if (th == NULL)
		return;
	if (th->disablecount > 0) {
		th->firepending = TRUE;
		return;
	}
	/* add check to see if this object gets destroyed by the
	 trigger recipient. */
	w1=owatch::Create(this);
	for (ti = th->instances; ti != NULL; ti = ti->next) {
	    if(ti->func) (ti->func)(ti->rcvr, this, ti->rock);
	    else if(ti->act) {
		static atom_def object("object");
		avalueflex in;
		avalueflex out;
		in.add(this,object);
		(*ti->act)(ti->rcvr, in, out);
	    }
	    if(!owatch::Check(w1)) return;
	}
	owatch::Delete(w1);
}

	/* if a client is calling a number of operations which would pull a trigger 
		too many times, it can disable the trigger temporarily.  
		It must later Enable the trigger.  At that time one call back 
		is made for the trigger if it has been Pulled one or more times in the interim.  */

/* observable__DisableTrigger(self, trigger)
	until Enabled, this trigger will no produce call backs 
			Enable MUST be called once 
			for each time Disable has been called.
*/
	void
observable::DisableTrigger(class atom  *trigger)
		{
	struct triggerhousing *th;
	for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
		{};
	if (th == NULL)
		return;
	if (th->disablecount == 0)
		th->firepending = FALSE;
	th->disablecount ++;
}

/* observable__EnableTrigger(self, trigger)
	this trigger will once again produce call backs
*/
	void
observable::EnableTrigger(class atom  *trigger)
		{
	struct triggerhousing *th;
	for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
		{};
	if (th == NULL)
		return;
	if (th->disablecount > 0)
		th->disablecount --;
	else th->disablecount = 0;
	if (th->disablecount == 0  &&  th->firepending)
		(this)->PullTrigger( trigger);
}

/* observable__DisableCount(self, trigger)
	return the number of outstanding DisableTrigger calls
*/
long observable::DisableCount(class atom  *trigger)
{
    struct triggerhousing *th;
    struct triggerclass *tc;

    for (th = this->triggers; th != NULL && th->trigger != trigger; th = th->next) 
    {};
    if (th != NULL)
	return th->disablecount;

	/* find out if the trigger is defined */
    for (tc = Triggers;  tc != NULL && tc->class_c != (this)->ATKregistry();
    tc = tc->next)
    {};
    if (tc == NULL || ! (tc->triggers)->Memberp( trigger))
		/* not defined */
	return -1;
	/* is defined, but has no recipients.  Is not disabled */
    return 0;
}
