#ifndef _AACTION_H_
#define _AACTION_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
#include <andrewos.h>
#include <avalue.H>

static atom_def aaction_error("error");

class aaction : public ATK {
    unsigned long refs;
  public:
    virtual ~aaction();
    inline aaction() : refs(1) {
    }
    virtual ATKregistryEntry *ATKregistry();
    // The function call operator invokes the action on the given object, passing the arguments in in.
    // and returning any result in out.
    virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out);
    // ArgTypes() returns an avalueflex representing the argument types.
    // The elements of the returned avalueflex are:  (all are of type atom)
    // index	name	value
    // 0		none	1st argument type
    // etc...
    // last	positional	none	lastpositional argument type
    // any		name	any named argument type
    // etc...
    // If the avalueflex is empty then any arguments will be accepted.
    // (They may however be ignored.)
    // In addition named arguments not in the list may still be passed.
    virtual avalueflex &ArgTypes();
           // RetTypes() returns an avalueflex representing the return types.
    // The elements of the returned avalueflex are:  (all are of type atom)
    // index	name	value
    // 0		none	1st return value type
    // etc...
    // last	positional	none	lastpositional return value type
    // any		name	any named return value type
    // etc...
    // Named arguments not listed may be present in an actual return value.
    // if the avalueflex is empty any arguments may be returned.
    virtual avalueflex &RetTypes();

    virtual void Destroy();
    inline void Reference() {
	refs++;
    }
    inline void DeReference() {
	refs--;
    }
    unsigned long Refs() {
	return refs;
    }
};

#include <mflex.H>
DEFINE_MFLEX_CLASS(multiact_list,aaction *,1);
class multiact : public aaction {
    multiact_list list;
  public:
    virtual ATKregistryEntry *ATKregistry();
    multiact &Prepend(aaction *act);
    multiact &Append(aaction *act);
    inline multiact &add(aaction *act) {
	return Append(act);
    }
    static multiact *ChainBefore(aaction *current, aaction *newact);
    static multiact *ChainAfter(aaction *current, aaction *newact);
    
    multiact &Remove(aaction *act);
    virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out);
};

#define AACTION_DUMMY_CLASS class

#define DEFINE_AACTION_FUNC_CLASS(name,type) \
AACTION_DUMMY_CLASS name : public aaction { \
protected: \
	void (*func)(type *o, const avalueflex &aux, const avalueflex &in, avalueflex &out); \
	avalueflex auxargs; \
	avalueflex intypes, outtypes; \
  public: \
	inline name(void (*p)(type *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : func(p), auxargs(a) { \
    	} \
	inline name(void (*p)(type *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t, const avalueflex &r) : func(p), auxargs(a), intypes(t), outtypes(r) { \
    	} \
	inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) { \
		func(((type *)obj), auxargs, in, out); \
	} \
	inline virtual avalueflex &ArgTypes() { \
		return intypes; \
	} \
	inline virtual avalueflex &RetTypes() { \
		return outtypes; \
	} \
};

#define DEFINE_AACTION_CLASS(name,type) \
AACTION_DUMMY_CLASS name : public aaction { \
protected: \
	void (type::*pmf)(const avalueflex &aux,  const avalueflex &in, avalueflex &out); \
	avalueflex auxargs; \
	avalueflex intypes, outtypes; \
  public: \
	inline name(void (type::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : pmf(p), auxargs(a) { \
    	} \
	inline name(void (type::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t,  const avalueflex &r) : pmf(p), auxargs(a), intypes(t), outtypes(r) { \
    	} \
	inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) { \
 		void (type::*p)(const avalueflex &in, const avalueflex &aux, avalueflex &out)=pmf;  \
		(((type *)obj)->*p)(auxargs, in, out); \
	} \
	inline virtual avalueflex &ArgTypes() { \
		return intypes; \
	} \
	inline virtual avalueflex &RetTypes() { \
		return outtypes; \
	} \
};

#ifndef AACTION_FUNC_LINKAGE
#define AACTION_FUNC_LINKAGE static
#endif


/* To have an AACTION_FUNC with 'target', use this macro
	to define two things needed by the target 
*/
#define AACTION_FUNC_CLASS(target) \
	DEFINE_AACTION_FUNC_CLASS(Concat(target,Act), target); \
	ATK_CLASS(target)

#define AACTION_ARG_CLASS(target) AACTION_FUNC_CLASS(target)

#define AACTION_METHOD_CLASS(target) \
DEFINE_AACTION_CLASS(Concat(target,MethodAct), target); \
ATK_CLASS(target);


#define AACTION_FUNC_AND_METHOD_CLASS(target) \
DEFINE_AACTION_FUNC_CLASS(Concat(target,Act), target); \
DEFINE_AACTION_CLASS(Concat(target,MethodAct), target); \
ATK_CLASS(target);

#define DECLARE_AACTION_METHOD(name) void name(const avalueflex &aux, const avalueflex &in, avalueflex &out)

/* An AACTION_FUNC is a function suitable to be the
		procedure body of an aaction object  
*/
/* note: methodprefix is assumed to be C++ class prefix, e.g. class:: */
#define AACTION_FUNC_GENERAL(name,target,reglinkage,funclinkage,methodprefix,objarg,method) \
	typedef target Concat(name,Target); \
	reglinkage  ATK_def \
		&Concat(name,TargetClass) = Concat(class_,target); \
	typedef Concat3(target,method,Act) Concat(name,TargetAct); \
funclinkage void methodprefix name(objarg \
		const avalueflex &aux, const avalueflex &in, avalueflex &out) 

#define AACTION_FUNC_INTERNAL(arg) arg,
#define AACTION_FUNC(name,target) AACTION_FUNC_GENERAL(name,target,AACTION_FUNC_LINKAGE,AACTION_FUNC_LINKAGE,/* */,AACTION_FUNC_INTERNAL(target *self), /* */)

#define AACTION_METHOD(name,target) AACTION_FUNC_GENERAL(name,target,AACTION_FUNC_LINKAGE,/* */,target::,/* */, Method)

/* use this macro to enter an AACTION_FUNC
	into the proctable (as from an InitializeClass function)
*/
#define ENTER_AACTION_FUNC(name,procname,auxvalues,doc)  { \
	static Concat(name,TargetAct) act(&name,auxvalues); \
	proctable::DefineAction(procname,&act,  \
		Concat(name,TargetClass),NULL,doc); \
}

#define ENTER_AACTION_METHOD(name,target,procname,auxvalues,doc)  { \
static Concat(name,TargetAct) act(&target::name,auxvalues); \
	proctable::DefineAction(procname,&act,  \
		Concat(name,TargetClass),NULL,doc); \
}
/** @} */
#endif

