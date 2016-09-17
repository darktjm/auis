ATK_INTER
#ifndef _ATK_H
#define _ATK_H

/*
		ATK.H - Base class for all of Andrew
		Provides class registry

 	Copyright Carnegie Mellon, 1996 - All Rights Reserved
	For full copyright information see: andrew/doc/COPYRITE 
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

#define THROWONFAILURE(x) do { if(!(x)) ATKConstructorFailure(); return; } while (0)
#ifndef boolean /* if somebody wants to use a macro for boolean, let them. */
typedef int boolean;
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef boolean (*ATKinitfptr)(void);
/*
 * Use the ATKdefineRegistry macro to enter a class into the runtime
 * registry.  It expands to a variable declaration and should probably
 * appear immediately after the include files in a class' implementation.
 * Its use looks like this (example):
 *
 *    ATKdefineRegistry(textview, view, initfunc);
 *
 * For multiple inheritance, ATKdefineRegistry2(class, supclass1, supclass2)
 * can be defined, and so on.  We will defer defining these until
 * someone actually askes for them.  In any case, the extra classes
 * must themselves be subclasses of ATK, otherwise the runtime system
 * is not interested in them, and they may be safely omitted.
 *
 *    ATKdefineRegistryNoInit(textview, view);
 *
 * Should be used when no class initialization is required or
 * you will be providing your own initialization mechanism.
 */
#define ATKdefineRegistry(me,sup, initf) \
  static ATK *me ## _new_() { return new me; };\
  extern ATKregistryEntry sup ## _ATKregistry_;\
  ATKregistryEntry me ## _ATKregistry_ = \
      {#me, me ## _new_, initf, {&sup ## _ATKregistry_ }};\
ATKregistryEntry *me ::ATKregistry() { return &me ## _ATKregistry_; }\
static UNUSED ATKinitfptr *ATKinited_= &(me ## _ATKregistry_.initfunc);

#define ATKdefineRegistryNoInit(me, sup) \
  static ATK *me ## _new_() { return new me; };\
  extern ATKregistryEntry sup ## _ATKregistry_;\
  ATKregistryEntry me ## _ATKregistry_ = \
{#me, me ## _new_, NULL, {&sup ## _ATKregistry_ }};\
ATKregistryEntry *me ::ATKregistry() { return &me ## _ATKregistry_; }

/*
 * Use the ATKregister macro to register a class.
 * It will automatically register this class and all superclasses
 * of this class.  It generates executable code.  Use it as follows:
 *
 *     ATKregister(textview);
 */
#define ATKregister(classname) \
  extern ATKregistryEntry classname ## _ATKregistry_;\
  ATK::RegisterClass(&classname ## _ATKregistry_);

/*
 * The build process generates a list of statically bound classes
 * in statl.C (using genstatl), containing only one function to
 * call to register all of the statically bound classes:
 */
extern void doStaticLoads(void);

/*
 * Use the ATKinit macro at the start of each constructor,
 * and at the start of each classprocedure implementation.
 * It checks if the class has been initialized, and if not
 * calls the function InitializeClass.
 *
 * If an implementation spans many files, the programmer must
 * code the initialization explicitly.  
 */
#define ATKinit {if (ATKinited_ && *ATKinited_) {boolean (*initfunc)()=(*ATKinited_); (*ATKinited_)=NULL; ATKinited_=NULL; initfunc();  }}

/*
 * The atk class is the root of entire atk class heirarchy.
 * Only subclasses of atk may be registered with the runtime
 * system.
 */
#define ATKmaxRegistryParents 1
class ATK;

struct ATKregistryEntry {
    const char *ClassName;
    ATK *(*New)();
	    //  Create a new object of the class.
    ATKinitfptr initfunc;    
	    // initfunc is called when the first static member
	    // function, or constructor is called.  (All static
	    // member functions and constructors should
	    // begin with ATKinit;)	
    ATKregistryEntry *parents[ATKmaxRegistryParents];
	    // The parents of this class, so far we only support
	    // single inheritance in the run-time system.
    ATKregistryEntry *next;
	    // The next ATKregistryEntry in the list of loaded
	    // classes.
    inline const char *GetTypeName() const { return ClassName; }
	    // Returns the name of the class.

    // Analogs of the IsType methods listed below for ATK.
    boolean IsType(const ATKregistryEntry *typeobject) const;
    boolean IsType(const char *classname) const;
    boolean IsType(const ATK *typeobject) const;
};


class ATK {
  public:
    // Runtime registry information for this class.
    // This function is generally not used by ATK users.  The
    // ATKregister macro expands to a call to this function.
    static void RegisterClass(ATKregistryEntry *classent);

    // All subclasses of ATK must provide their own registry entry.
    // The ATKregister macro defines this member function, but the ATK
    // programmer must nonetheless declare it in his class.
    virtual ATKregistryEntry *ATKregistry();

    // Support for looking up an ATKregistryEntry by name,
    // and dynamically loading a class.
    static ATKregistryEntry *DynamicLoad(const char *classname, boolean trace=FALSE);
    static int &DynamicLoadTrace();	/* non-zero writes msgs to stdout */
    
    static ATKregistryEntry *LoadClass(const char *classname);
    // Returns TRUE iff the named class is already loaded.
    static boolean IsLoaded(const char *classname);
    
    
    // IsType returns true if "this" is a subclass of the given
    // registry, classname or other ATK object.
    inline boolean IsType(const ATKregistryEntry *classent) const {
	return ((ATK *)this)->ATKregistry()->IsType(classent);
    }
    inline boolean IsType(const char *classname) const {
	return ((ATK *)this)->ATKregistry()->IsType(classname);
    }
    inline boolean IsType(const ATK *typeobject) const {
	return ((ATK *)this)->ATKregistry()->IsType(typeobject);
    }

    // Create a new object of the same type as this object.
    // You will need to cast the return value to the appropriate
    // type.
    inline ATK *NewFromObject() const {
	return ((ATK *)this)->ATKregistry()->New();
    }
    
    
    // IsTypeByName returns true if sub is a subclass of super.
    static boolean IsTypeByName(const char *sub, const char *super);

    // GetTypeName returns a string representing the classname
    // of any given ATK object.
    inline const char *GetTypeName() const { return ((ATK *)this)->ATKregistry()->ClassName; };

    // The NewObject function can create an instance of any ATK class
    // that has been registered with the ATK runtime system (with ATKregister).
    static ATK *NewObject(const char *classname);

    // Called by the THROWONFAILURE macro when a constructor fails.
    // This can be overridden to print a different message.
    // Note that it CANNOT access virtual functions of the class.
    // Also, it cannot be virtual.  Calling virtual functions in
    // a constructor results in one of the base class versions
    // of the function being called.
    static void ATKConstructorFailure();
};


/* The following class and macro simplify creation of 
	references to ATKregistry entries.  The macro call
		ATK_CLASS(aclassname)
	creates the declaration of identifier
		class_aclassname
	This identifier can be used wherever an ATKregistryEntry is needed.
*/
class ATK_def {
	ATKregistryEntry *e;
	const char *classname;
public:
	inline ATK_def(const char *classnm) {
		classname = classnm;
		e = NULL;
	}
	inline operator ATKregistryEntry *() const {
		if ( ! e) ((ATK_def *)this)->e 
				= ATK::LoadClass(classname);
		return e;
	}
	inline ATKregistryEntry *operator->() const {
		if ( ! e) ((ATK_def *)this)->e 
				= ATK::LoadClass(classname);
		return e;
	}
};
#define ATK_CLASS(class) \
	static ATK_def Concat(class_,class) (Stringize(class))

#endif /* _ATK_H */
