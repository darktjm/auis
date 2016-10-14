#ifndef _ATK_H
#define _ATK_H

#include <andrewos.h>
#include <truth.h>
/*
		ATK.H - Base class for all of Andrew
		Provides class registry

 	Copyright Carnegie Mellon, 1996 - All Rights Reserved
	For full copyright information see: andrew/doc/COPYRITE 
*/

/** \addtogroup libbasics Object basics
 *  A statically loadable %ATK class file.  Contains application, atom,
 *  atomlist, bind, cursor, dataobject, environ, event, filetype, fontdesc,
 *  graphic, im, init, keymap, keystate, menulist, message, messitem,
 *  msghandler, Namespace, observable, path, pixelimage, proctable,
 *  region, rm, updatelist, view, windowsystem, colormap, image, imageio,
 *  traced, owatch, ATK, rastio, aaction, multiact, viewholderv, iddimage,
 *  WorkProc, xws, xim, xcursor, xfontdesc, xcolormap, xgraphic, xddimage.
 *  xddimage_draw is there as well, although it isn't technically exported.
 *
 *  This is the base library for %ATK, so it contains a lot more underneath,
 *  as well; it is one of the few objects which export C and C++ functions
 *  directly.  Note that environ's C++ class is named environclass.
 * @{ */

/** Raise an error if \p x is zero.  For use in class constructor functions.
 *  \param x Return code.  FALSE on failure.
 */
#define THROWONFAILURE(x) do { if(!(x)) ATKConstructorFailure(); return; } while (0)

/** A pointer to a class initialization function.  Returns 0 (FALSE) on error. */
typedef boolean (*ATKinitfptr)(void);
/**
 * Use this macro to enter a class into the runtime
 * registry.  It expands to a variable declaration and should probably
 * appear immediately after the include files in a class' implementation.
 * It exports a symbol called \c <em>class</em>_ATKregistry_, defines the
 * \c <em>class</em>::%ATKregistry() function, creates a static function called
 * \c <em>class</em>_new_ and declares a static variable called \c ATKinited_.
 * Note that the latter means that you can have no more than one call of this
 * macro in a given C++ file.
 *
 * The current dynamic object linker requires that you define each
 * class using either ATKdefineRegistry() or ATKdefineRegistryNoInit()
 * or a raw definition.  No additional macros will be expanded.
 *
 * Its use looks like this (using \ref textview as an example):
 * \qcode
 *     ATKdefineRegistry(textview, view, initfunc);
 * \endqcode
 *
 * \param me The class being registered
 * \param sup The superclass (immediate parent) of the class being registered
 * \param initf A function to call to initialize the class.  This is a
 *              function (pointer) of type \ref ATKinitfptr.  It is called
 *              automatically by ATK::LoadClass(), but not by
 *              ATK::DynamicLoad().  A class is expected to call this
 *              function itself in its own constructors or methods
 *              using the \ref ATKinit macro.  Calling the function by either
 *              of these methods will ensure it is called only once.
 *              If NULL, no initialization function is called.
 */
#define ATKdefineRegistry(me,sup, initf) \
  static ATK *me ## _new_() { return new me; };\
  extern ATKregistryEntry sup ## _ATKregistry_;\
  ATKregistryEntry me ## _ATKregistry_ = \
      {#me, me ## _new_, initf, {&sup ## _ATKregistry_ }};\
ATKregistryEntry *me ::ATKregistry() { return &me ## _ATKregistry_; }\
static UNUSED ATKinitfptr *ATKinited_= &(me ## _ATKregistry_.initfunc);
/*
 * For multiple inheritance, ATKdefineRegistry2(class, supclass1, supclass2)
 * can be defined, and so on.  We will defer defining these until
 * someone actually asks for them.  In any case, the extra classes
 * must themselves be subclasses of ATK, otherwise the runtime system
 * is not interested in them, and they may be safely omitted.
 */
/** This is the maximum number of parents supported by %ATK.  Currently,
 *  this is one. */
#define ATKmaxRegistryParents 1

/**
 * Use this macro to enter a class into the runtime
 * registry.  See \ref ATKdefineRegistry for details.
 * It should be used when no class initialization is required or
 * you will be providing your own initialization mechanism.
 * \qcode
 *     ATKdefineRegistryNoInit(textview, view);
 * \endqcode
 *
 * is equivalent to
 * \qcode
 *     ATKdefineRegistry(textview, view, NULL);
 * \endqcode
 *
 * Note that the lack of initialization also means that the static
 * ATKinited_ variable is no longer declared, so more than one of these
 * may appear in a C++ file.
 *
 * The current dynamic object linker requires that you define each
 * class using either ATKdefineRegistry() or ATKdefineRegistryNoInit()
 * or a raw definition.  No additional macros will be expanded.
 * \param me The class being registered
 * \param sup The superclass (immediate parent) of the class being registered
 */
#define ATKdefineRegistryNoInit(me, sup) \
  static ATK *me ## _new_() { return new me; };\
  extern ATKregistryEntry sup ## _ATKregistry_;\
  ATKregistryEntry me ## _ATKregistry_ = \
{#me, me ## _new_, NULL, {&sup ## _ATKregistry_ }};\
ATKregistryEntry *me ::ATKregistry() { return &me ## _ATKregistry_; }

/**
 * Use this macro to register a class.
 * It will automatically register this class and all superclasses
 * of this class.  It generates executable code.  Use it as follows:
 * \qcode
 *     ATKregister(textview);
 * \endqcode
 * There is never a reason to call this manually.  Instead, the
 * build system generates such calls for static loading via the
 * doStaticLoads() function or for dynamic loading to be executed
 * during static constructor execution when the object is loaded.
 * However, it is safe to call any time, as it will do nothing if
 * the class is already registered.
 *
 * \param classname The class to register.  The object file containing
 *                  the ATKdefineRegistry() or ATKdefineRegistryNoInit()
 *                  call for this class must be linked in at the time of
 *                  the call.
 */
#define ATKregister(classname) \
  extern ATKregistryEntry classname ## _ATKregistry_;\
  ATK::RegisterClass(&classname ## _ATKregistry_);

/**
 * Register all statically loadable classes.
 * The build process generates a list of statically bound classes
 * in statl.C (using genstatl), containing only one function to
 * call to register all of the statically bound classes:
 */
extern void doStaticLoads(void);

/**
 * Call the registered class initialization function, exactly once.
 * Use this macro at the start of each constructor,
 * and at the start of each static class function implementation.
 * It checks if the class has been initialized, and if not
 * calls the registered initialization function.
 *
 * This must appear in the same file as the ATKdefineRegistry()
 * macro call for the class.
 * If an implementation spans many files, the programmer must
 * code the initialization explicitly.  
 */
#define ATKinit do { \
    if (ATKinited_ && *ATKinited_) { \
	boolean (*initfunc)()=(*ATKinited_); \
	(*ATKinited_)=NULL; \
	ATKinited_=NULL; \
	if(!initfunc()) { \
	    /* don't know the class name */ \
	    fputs("Class initialization failed.\n", stderr); \
	    exit(1); /* no clean way to deal with this, I guess */ \
	} \
    } \
} while(0)

class ATK;

/** The registry structure used for dynamic %ATK class registration.
 *
 * This structure is not meant to be created or manipulated
 * manually.  It is the structure created by ATKdefineRegistry()
 * and ATKdefineRegistryNoInit(). */
struct ATKregistryEntry {
    const char *ClassName;
    ///< The name of the class.  The actual C++ name of the class
    ///< may (but should not) differ; this is what the class
    ///< system knows it by.
    ATK *(*New)();
    ///<  Create a new object of the class.
    ATKinitfptr initfunc;
    ///< This is normally called when the first static member
    ///< function, or constructor is called.  This is accomplished
    ///< by requiring that all static member functions and
    ///< constructors begin with \ref ATKinit;.
    ATKregistryEntry *parents[ATKmaxRegistryParents];
    ///< The parents of this class; so far, we only support
    ///< single inheritance in the run-time system.
    ATKregistryEntry *next;
    ///< The next ATKregistryEntry in the list of loaded
    ///< classes.  Maintained by the loader.  Must be
    ///< initialized to NULL.
    inline const char *GetTypeName() const { return ClassName; }
    ///< Returns the name of the class.

    // Analogs of the IsType methods listed below for ATK.
    boolean IsType(const ATKregistryEntry *typeobject) const;
    ///< Check if "this" is defined by \p typeobject or one
    ///< of its descendents.
    ///< \param typeobject The type descriptor of the type to test for.
    ///< \return TRUE iff "this" is defined by \p typeobject or one
    ///< of its descendents.
    boolean IsType(const char *classname) const;
    ///< Check if "this" is named \p classname or is one of \p classname's
    ///< descendents.
    ///< \param classname The registered name of a class
    ///< \return TRUE iff "this" is named \p classname or is one of \p classname's
    ///< descendents.
    boolean IsType(const ATK *typeobject) const;
    ///< Check if "this" is the same class as \p typeobject or one of
    ///< typeobject's class' descendents.
    ///< \param typeobject An object of an ATK-descendent type.
    ///< \return TRUE if "this" is the same class as \p typeobject or one of
    ///< \p typeobject's class' descendents.
};


/**
 * The ATK class is the root of entire %ATK class heirarchy.
 * Only subclasses of ATK may be registered with the runtime
 * system.
 */
class ATK {
  public:
    /// Add runtime registry information for a class.
    /// This function is generally not used directly by ATK users.  The
    /// ATKregister macro expands to a call to this function with the
    /// appropriate parameter for the class being registered.
    /// \param classent A registery structure for a class to register.
    static void RegisterClass(ATKregistryEntry *classent);

    /// Return the registry entry reperesenting the class of the object.
    /// All subclasses of ATK must provide their own registry entry.
    /// The \ref ATKdefineRegistry and \ref ATKdefineRegistryNoInit macros define
    /// this member function, but this declaration must nonetheless be
    /// added manually to the class definition.
    /// \return the registry entry.
    virtual ATKregistryEntry *ATKregistry();

    // Support for looking up an ATKregistryEntry by name,
    // and dynamically loading a class.
    static ATKregistryEntry *DynamicLoad(const char *classname, boolean trace=FALSE);
    ///< Dynamically load a class object.  If the object is already registered,
    ///< this loads nothing.  The naming conventions for class files are
    ///< system-dependent.  The location for class files is one of the
    ///< :-separated members of $CLASSPATH, or $ANDREWDIR/lib/atk by
    ///< default.
    ///< \param classname The name of the class to load.
    ///< \param trace TRUE if extra tracing of the load process is desired.
    ///<              The nature of the trace is system-defined, and may
    ///<              be nothing.
    ///< \return The registry definition structure for the loaded and
    ///<         registered class, or NULL on error.
    
    static int &DynamicLoadTrace();
    ///< Set to non-zero to write trace information to stdout.
    ///< Overrides the \p trace parameter's default for DynamicLoad().
    ///< \return a reference to a global trace setting.

    static ATKregistryEntry *LoadClass(const char *classname);
    ///< Load and initialize a class.  See DynamicLoad() for more
    ///< information.  Note that superclasses automatically loaded by
    ///< this function will not be initialized; either call LoadClass()
    ///< manually to perform immediate initialization, or wait for the
    ///< class to automatically initialize itself.
    ///< \param classname The name of the class to load and initialize.
    ///< \return The registry definition structure for the loaded,
    ///<         registered and initialized class, or NULL on error.
    static boolean IsLoaded(const char *classname);
    ///< Checks if a class is already loaded and registered.
    ///< \param classname The class to check.
    ///< \return TRUE iff the named class is already loaded.


    // IsType returns true if "this" is a subclass of the given
    // registry, classname or other ATK object.
    inline boolean IsType(const ATKregistryEntry *classent) const {
	return ((ATK *)this)->ATKregistry()->IsType(classent);
    }
    ///< Check if "this" is defined by \p classent or one
    ///< of its descendents.
    ///< \param classent The type descriptor of the type to test for.
    ///< \return TRUE iff "this" is defined by \p classent or one
    ///< of its descendents.
    inline boolean IsType(const char *classname) const {
	return ((ATK *)this)->ATKregistry()->IsType(classname);
    }
    ///< Check if "this" is named \p classname or is one of \p classname's
    ///< descendents.
    ///< \param classname The registered name of a class
    ///< \return TRUE iff "this" is named \p classname or is one of \p classname's
    ///< descendents.
    inline boolean IsType(const ATK *typeobject) const {
	return ((ATK *)this)->ATKregistry()->IsType(typeobject);
    }
    ///< Check if "this" is the same class as \p typeobject or one of
    ///< \p typeobject's class' descendents.
    ///< \param typeobject An object of an ATK-descendent type.
    ///< \return TRUE iff "this" is the same class as \p typeobject or one of
    ///< \p typeobject's class' descendents.

    /// Create a new object of the same type as this object.
    /// You will need to cast the return value to the appropriate
    /// type.
    /// \return A new object of the same type as this.
    inline ATK *NewFromObject() const {
	return ((ATK *)this)->ATKregistry()->New();
    }
    
    
    /// Check if classes are related.
    /// \param sub The name of a potential subclass.
    /// \param super The name of a potential superclass.
    /// \return TRUE iff sub is a subclass of super.
    static boolean IsTypeByName(const char *sub, const char *super);

    /// Obtain the registered name for this class.
    /// \return a string representing the classname
    ///         of any given ATK object.
    inline const char *GetTypeName() const { return ((ATK *)this)->ATKregistry()->ClassName; };

    /// Obtain a new object of any %ATK class that can be registered
    /// with the %ATK runtime system (with ATKregister()).
    /// The class is automatically loaded and initialized, if necessary.
    /// \param classname The name of the registered class.
    /// \return an instance of the class, or NULL if there is no such class
    ///         or there was an error creating the instance.
    static ATK *NewObject(const char *classname);

    /// Called by the THROWONFAILURE() macro when a constructor fails.
    /// This can be overridden to print a different message.
    /// Note that it CANNOT access virtual functions of the class.
    /// Also, it cannot be virtual.  Calling virtual functions in
    /// a constructor results in one of the base class versions
    /// of the function being called.
    static void ATKConstructorFailure();
};


/** The following class and macro simplify creation of 
 *	references to ATKregistry entries.  The macro call
 * \pcode
 *		\ref ATK_CLASS(<em>aclassname</em>)
 * \endpcode
 *	creates the declaration of identifier
 * \pcode
 *		class_<em>aclassname</em>
 * \endpcode
 *	This identifier can be used wherever an ATKregistryEntry is needed.
*/
class ATK_def {
    ATKregistryEntry *e;
    const char *classname;
  public:
    /// Constructor called by ATK_CLASS() to make a reference
    /// \param classnm The name of the class.
    inline ATK_def(const char *classnm) {
	classname = classnm;
	e = NULL;
    }
    /// Use the registry entry for the given class.
    inline operator ATKregistryEntry *() const {
	if ( ! e) ((ATK_def *)this)->e
	    = ATK::LoadClass(classname);
	return e;
    }
    /// Use the registry entry for the given class.
    inline ATKregistryEntry *operator->() const {
	if ( ! e) ((ATK_def *)this)->e
	    = ATK::LoadClass(classname);
	return e;
    }
};
/// Create a static object to reference a class' registry structure.
/// An object named \c class_<em>class</em> is created, which can be used in
/// places a pointer to the registry structure for that class would
/// normally be used.
/// \param class The class name
#define ATK_CLASS(class) \
  static ATK_def Concat(class_,class) (Stringize(class))
/** @} */
#endif /* _ATK_H */
