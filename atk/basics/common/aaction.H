#ifndef _AACTION_H_
#define _AACTION_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
#include <andrewos.h>
#include <avalue.H>
#include <traced.H>

static atom_def aaction_error("error");

/** Generic function call mechanism.
 *
 *  An aaction is an object which executes a function with a given
 *  set of arguments and provides information about the arguments and
 *  return types.  It does nothing itself; only derived children actually
 *  implment anything.  */
class aaction : public traced {
  public:
    virtual ATKregistryEntry *ATKregistry();
    aaction() {}
    virtual ~aaction() {}
    virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out);
    /**< Invoke the action on the given object.
     *   This must be overridden by descendents in order for the action
     *   to have any effect.
     *   \param obj The object which is the primary input parameter.
     *   \param in Additional input arguments.
     *   \param out Array into which return value(s) will be placed. */
    virtual avalueflex &ArgTypes();
    /**< List the action's expected input arguments.
     *   The elements of the returned array are all of the type atom.
     *   First come the positional (unnamed) arguments, in order.  The value is
     *   the name of the type expected.  This allows the array to be
     *   used as the argtypes parameter of avalueflex::TypeCheck().
     *   Next come named parameters, in any order, whose name is the
     *   expected name and whose value is the name of the expected type.
     *   Note that named arguments not specified in this array may be
     *   passed, and may or may not be ignored by the action.
     *
     *   As with traditional C, if the array is empty, any arguments will
     *   be accepted (and may or may not be ignored by the action).
     *
     *   \return an avalueflex array representing the argument types. */

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
    /**< List the action's return types.
     *   The elements of the returned array are all of the type atom.
     *   First come the positional (unnamed) elements, in order.  The value is
     *   the name of the type returned.  This allows the array to be
     *   used as the argtypes parameter of avalueflex::TypeCheck(), although
     *   generally callers do not check the return types.
     *   Next come named parameters, in any order, whose name is the
     *   expected name and whose value is the name of the expected type.
     *   Note that named arguments not specified in this array may be
     *   returned.
     *
     *   As with traditional C, if the array is empty, rather than
     *   indicating no return, this indicates that the return values
     *   may be anything.
     *
     *   \return an avalueflex array representing the return types. */
};

#include <mflex.H>
/** An array containing pointers to multiple aactions. */
DEFINE_MFLEX_CLASS(multiact_list,aaction *,1);
/** A multiact is an action containing a list of actions to execute.
 *  All actions are executed, in the order that they have been added
 *  to the list.  The argument types are assumed to apply to all members.  */
class multiact : public aaction {
    multiact_list list;
  public:
    virtual ATKregistryEntry *ATKregistry();

    multiact &Prepend(aaction *act);
    /**< Prepend an action to the list
     *   \param act The action to prepend.  Since it is reference-counted,
     *              it is safe to Destroy() the action after adding.
     *   \return The list, so that calls to this function may be chained. */
    multiact &Append(aaction *act);
    /**< Append an action to the list
     *   \param act The action to append.  Since it is reference-counted,
     *              it is safe to Destroy() the action after adding.
     *   \return The list, so that calls to this function may be chained. */
    inline multiact &add(aaction *act) {
	return Append(act);
    }
    /**< Append an action to the list
     *   \param act The action to append.  Since it is reference-counted,
     *              it is safe to Destroy() the action after adding.
     *   \return The list, so that calls to this function may be chained. */
    static multiact *ChainBefore(aaction *current, aaction *newact);
    /**< Create a list with two elements in order.
     *   \param current An action.
     *   \param newact The action to run before current.
     *   \return A list containing current and newact, in the correct
     *           order.  If current is itself a list, that list is returned,
     *           with newact prepended. */
    static multiact *ChainAfter(aaction *current, aaction *newact);
    /**< Create a list with two elements in order.
     *   \param current An action.
     *   \param newact The action to run after current.
     *   \return A list containing current and newact, in the correct
     *           order.  If current is itself a list, that list is returned,
     *           with newact appended. */
    
    multiact &Remove(aaction *act);
    /**< Remove an action from the list.
     *   \param act The action to remove.
     *   \return The list.  If act was in the list, it is removed. */

    virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out);
};

/** Create an aaction which calls a configurable function.
 *  \param name The name of the aaction class.
 *  \param type The type of the primary argument to the function.  */
#define DEFINE_AACTION_FUNC_CLASS(name,type) \
/** An action which calls a configurable function whose primary argument
    is a type object. */ \
class name : public aaction { \
  protected: \
    void (*func)(type *o, const avalueflex &aux, const avalueflex &in, avalueflex &out); \
    /**< The function to execute.  */ \
    avalueflex auxargs; /**< The user data (aux arguments) */ \
    avalueflex intypes /** The input parameter types */, outtypes /** The return value types */; \
  public: \
    inline name(void (*p)(type *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : func(p), auxargs(a) { \
    } /**< Create action to call given fuction.
       *   The input and output parameter type arrays are initially empty (anything).
       *   \param p The function.
       *   \param aux User data (additional parameters to pass to the function). */ \
    inline name(void (*p)(type *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t, const avalueflex &r) : func(p), auxargs(a), intypes(t), outtypes(r) { \
    }  /**< Create action to call given fuction.
       *   \param p The function.
       *   \param aux User data (additional parameters to pass to the function).
       *   \param t The array describing the expected input parameter types.
       *   \param r The array describing the return types. */ \
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

/** Create an aaction which calls a configurable class method.
 *  \param name The name of the aaction class.
 *  \param type The type of the class from which the method is taken.
 */
#define DEFINE_AACTION_CLASS(name,type) \
/** An action which calls a configurable method of type */ \
class name : public aaction { \
  protected: \
    void (type::*pmf)(const avalueflex &aux,  const avalueflex &in, avalueflex &out); \
    /**< The method to execute.  */ \
    avalueflex auxargs; /**< The user data (aux arguments) */ \
    avalueflex intypes /** The input parameter types */, outtypes /** The return value types */; \
  public: \
    inline name(void (type::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : pmf(p), auxargs(a) { \
    } /**< Create action to call given method.
       *   The input and output parameter type arrays are initially empty (anything).
       *   \param p The method.
       *   \param aux User data (additional parameters to pass to the method). */ \
    inline name(void (type::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t,  const avalueflex &r) : pmf(p), auxargs(a), intypes(t), outtypes(r) { \
    }  /**< Create action to call given method.
       *   \param p The method.
       *   \param aux User data (additional parameters to pass to the method).
       *   \param t The array describing the expected input parameter types.
       *   \param r The array describing the return types. */ \
    inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) { \
	void (type::*p)(const avalueflex &in, const avalueflex &aux, avalueflex &out)=pmf; \
	(((type *)obj)->*p)(auxargs, in, out); \
    } \
    inline virtual avalueflex &ArgTypes() { \
	return intypes; \
    } \
    inline virtual avalueflex &RetTypes() { \
	return outtypes; \
    } \
};


/* To have an AACTION_FUNC with 'target', use this macro
	to define two things needed by the target
        [tjm - removed ATK_CLASS() call; not needed by current linker/loader]
*/
/** Create an aaction which calls a configurable function.
 *  The name of the aaction-derived class is <em>type</em>Act.
 *  \param target The type of the primary argument to the function. */
#define AACTION_FUNC_CLASS(target) \
	DEFINE_AACTION_FUNC_CLASS(Concat(target,Act), target);

/** Create an aaction which calls a configurable class method.
 *  The name of the aaction-derived class is <em>type</em>MethodAct.
 *  \param target The type of the primary argument to the method. */
#define AACTION_METHOD_CLASS(target) \
DEFINE_AACTION_CLASS(Concat(target,MethodAct), target);

/** Create an aaction which calls a configurable function and one which calls
 *  a configurable method.
 *  The names of the aaction-derived classes are <em>type</em>Act for the
 *  function caller, and <em>type</em>MethodAct for the method caller.
 *  \param target The type of the primary argument to the function/method. */
#define AACTION_FUNC_AND_METHOD_CLASS(target) \
DEFINE_AACTION_FUNC_CLASS(Concat(target,Act), target); \
DEFINE_AACTION_CLASS(Concat(target,MethodAct), target);

/** Prototype for class method usable as AACTION_METHOD_CLASS() callbacks.
 *  This can be used both in the class, to declare the function, and in
 *  the code, to define the method code.
 *  Arguments to the method are \e aux, \e in, and \e out, corresponding to the
 *  user (auxiliary), input, and output avalueflex array references,
 *  respectively.
 *  \param name The method name.  When using this macro to define the
 *              method code, prefix it with <em>class</em>::. */
#define DECLARE_AACTION_METHOD(name) void name(const avalueflex &aux, const avalueflex &in, avalueflex &out)


/* An AACTION_FUNC is a function suitable to be the
		procedure body of an aaction object  
*/
/** Definition prototype for creating functions for function calling aactions.
 *  This generic version is internal, and not meant to be used directly;
 *  use AACTION_FUNC() and AACTION_METHOD() insetead.
 *  It can be used to create functions or methods.  Declaration prototypes
 *  are not necessary for static functions if they appear before use.
 *  Declaration prototypes for methods should use DECLARE_AACTION_METHOD().
 *  The arguments to the function or method are the \e objarg, and
 *  \e aux, \e in and \e out, corresponding to the user (auxiliary), input, and
 *  output avalueflex array references, respectively.
 *  In addition to the prototype, two typedefs and one object are created:
 *  <em>name</em>Target, which is the primary parameter type,
 *  <em>name</em>TargetAct, which is the aaction class this is intended for and
 *  <em>name</em>TargetClass, which is the ATK class definition structure
 *   for the primary parameter type, via ATK_def.
 *  \param name The name of the function.
 *  \param target The primary argument type for the action.
 *  \param reglinkage Linkage prefix for <em>name</em>TargetClass class reference.
 *         (static or nothing are the only options that make sense)
 *  \param funclinkage Linkage prefix for function (static or nothing are the
 *         only options that make sense)
 *  \param methodprefix Name prefix for function (nothing or <em>class</em>:: are
 *         the only options that make sense)
 *  \param objarg The primary argument as it appears in the prototype (or not).
 *                This includes the trailing comma if the parameter exists.
 *  \param method "Method" or nothing, depending on whether or not this is a method.
 */
#define AACTION_FUNC_GENERAL(name,target,reglinkage,funclinkage,methodprefix,objarg,method) \
	typedef target Concat(name,Target); \
	reglinkage  ATK_def \
		&Concat(name,TargetClass) = Concat(class_,target); \
	typedef Concat3(target,method,Act) Concat(name,TargetAct); \
funclinkage void methodprefix name(objarg \
		const avalueflex &aux, const avalueflex &in, avalueflex &out) 

/** The default linkage for functions created by AACTION_FUNC().
 *  Define this to override the default of static. */
#ifndef AACTION_FUNC_LINKAGE
#define AACTION_FUNC_LINKAGE static
#endif

/** This macro is a hack to allow a comma in a macro argument. */
#define AACTION_FUNC_INTERNAL(arg) arg,
/** Definition prototype for creating functions for function calling aactions.
 *  Declaration prototypes are not necessary for static functions if they
 *  appear before use.  No macro is provided for easily declaring functions
 *  if they are exported or needed before definition.  The function linkage
 *  and class reference object linkage are set using AACTION_FUNC_LINKAGE.
 *  The arguments to the function are the \e self, the primary argument, and
 *  \e aux, \e in and \e out, corresponding to the user (auxiliary), input, and
 *  output avalueflex array references, respectively.
 *  In addition to the prototype, two typedefs and one object are created:
 *  <em>name</em>Target, which is the primary parameter type (\e target), and
 *  <em>name</em>TargetAct, which is the aaction class (<em>target</em>Act)
 *  this is intended for and
 *  <em>name</em>TargetClass, which is the ATK class definition structure
 *  for the primary parameter type, via ATK_def.
 *  \param name The name of the function.
 *  \param target The primary argument type for the action.
 */
#define AACTION_FUNC(name,target) AACTION_FUNC_GENERAL(name,target,AACTION_FUNC_LINKAGE,AACTION_FUNC_LINKAGE,/* */,AACTION_FUNC_INTERNAL(target *self), /* */)

/** Definition prototype for creating methodss for method calling aactions.
 *  Declaration prototypes in class definitions should use
 *  DECLARE_AACTION_METHOD().  The class reference object linkage is set
 *  using AACTION_FUNC_LINKAGE.  The arguments to the method are
 *  \e aux, \e in and \e out, corresponding to the user (auxiliary), input, and
 *  output avalueflex array references, respectively.
 *  In addition to the prototype, two typedefs are created:
 *  <em>name</em>Target, which is the primary parameter type (\e target), and
 *  <em>name</em>TargetAct, which is the aaction class (<em>target</em>MethodAct)
 *  this is intended for and
 *  <em>name</em>TargetClass, which is the ATK class definition structure
 *  for the primary parameter type, via ATK_def.
 *  \param name The name of the method.
 *  \param target The primary argument type for the action.  The method
 *                is a member of this class.
 */
#define AACTION_METHOD(name,target) AACTION_FUNC_GENERAL(name,target,AACTION_FUNC_LINKAGE,/* */,target::,/* */, Method)

/** Create a named procedure from a function-calling aaction.
 *  Calls proctable::DefineAction() to enter the function in the global
 *  named procedure table.  This is normally used e.g. during class
 *  initialization to register all procedures.
 *  \param name The function name.
 *  \param procname The proctable name (a string).
 *  \param auxvalues The user data for the function, as an avalueflex array.
 *  \param doc The documentation string for this procedure. */
#define ENTER_AACTION_FUNC(name,procname,auxvalues,doc)  { \
	static Concat(name,TargetAct) act(&name,auxvalues); \
	proctable::DefineAction(procname,&act,  \
		Concat(name,TargetClass),NULL,doc); \
}

/** Create a named procedure from a method-calling aaction.
 *  Calls proctable::DefineAction() to enter the method in the global
 *  named procedure table.  This is normally used e.g. during class
 *  initialization to register all procedures.
 *  \param name The method name.
 *  \param target The method's class.
 *  \param procname The proctable name (a string).
 *  \param auxvalues The user data for the function, as an avalueflex array.
 *  \param doc The documentation string for this procedure. */
#define ENTER_AACTION_METHOD(name,target,procname,auxvalues,doc)  { \
static Concat(name,TargetAct) act(&target::name,auxvalues); \
	proctable::DefineAction(procname,&act,  \
		Concat(name,TargetClass),NULL,doc); \
}
/** @} */
#endif
