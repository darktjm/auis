#ifndef _traced_H_
#define _traced_H_
/* Copyright 1992 Andrew Toolkit Consortium, Carnegie Mellon University */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>

/** Use this macro at the beginning of a class definition for some boilerplate.
 *  Use this instead of the usual ATKregistry() definition.  This just adds
 *  that and declares the default destructor as protected.
 *  \param c this class name
 */
#define TRACED_CLASS(c) \
  protected: \
    virtual ~c(); \
  public: \
    virtual ATKregistryEntry *ATKregistry()

/** Use this macro at the beginning of a class definition for some boilerplate.
 *  Use this instead of the usual ATKregistry() definition.  This just adds
 *  that and defines a dummy protected destructor.
 *  \param c this class name
 */
#define TRACED_CLASS_NODEST(c) \
  protected: \
    virtual ~c() {} \
  public: \
    virtual ATKregistryEntry *ATKregistry()

/** Simple reference counting class.
 *  This class grew out of a need for reference counting in the observable
 *  class.  The reference counting was split off into this class.  It relies
 *  on calling Destroy() rather than the object destructor, since object
 *  destructors cannot abort.  All objects get a reference count of 1
 *  when created, and are really destroyed (i.e., the destructor called and
 *  memory deallocated) when the reference count becomes zero.
 *  References are added manually by calling Reference().
 *
 *  Derived classes must never be allocated statically.  To make this
 *  mostly illegal, use protected destructors.  Even classes with nothing
 *  to do for destruction must define a destructor in order to make it
 *  protected.  To assist in following this rule, use TRACED_CLASS() to
 *  both make the destructor protected and document the reason for doing
 *  so.  If there is no destructor, use TRACED_CLASS_NODEST() instead.
 *
 *  Making destructors protected forces non-derived classes
 *  to do the right thing (i.e., never call the destructor, not even
 *  implicitly).  It is not possible to force derived or friend
 *  classes do the right thing (classes with private destructors would
 *  add another layer of security while making it impossible to further
 *  derive those classes).
 */
class traced : public ATK  {
  public:
    TRACED_CLASS(traced);

    /** Default constructor:  set reference count to 1. */
    traced();

    /** Call this instead of delete.  This will drop the reference count
     *  and delete the object if zero.  */
    virtual void Destroy();

    inline unsigned long Reference() { return ((this)->refcount++); }
    /**< Increments the object's reference count.
     *   \return the previous value of the reference count. */
    inline unsigned long UnReference() { return (((this)->refcount>0)?(this)->refcount--:0); }
    /**< Decrements the object's reference count.  This should be used only
     *   in extraordinary situations, normally the reference count is
     *   decremented by calling the appropriate Destroy procedure.
     *   \return the previous value of the reference count. */
    inline unsigned long   ReferenceCount() { return ((this)->refcount); }
    /**< Get the current reference count.
     *   \return the current value of the reference count. */
  private:
    unsigned long refcount;
};

// not quite auto_ptr
// unique_ptr wouldn't really help much, and is C++-11
/// This is a pointer which automatically deletes a traced object when it
/// is deleted.  This somewhat simulates direct object creation, but the
/// usage is that of a pointer, rather than an object.
/// \param T The traced descendent type.
template <class T> class traced_ptr {
  private:
    T *ptr;
  public:
    // The additional reference here is to match the takeover constructor.
    /// Construction creates a new T from the heap.
    inline traced_ptr() { ptr = new T;  ptr->Reference(); }
    // The additional reference here is to ensure that the user doesn't
    // accidentally deref the pointer right away, causing it to be
    // freed, making for hard-to-trace crashes later on.
    /// Construction may just take ownership of an existing heap object.
    /// This is necessary if you want to use a constructor other than
    /// the default.
    /// \param p The pointer to take over.
    inline traced_ptr(T *p) { ptr = p;  ptr->Reference(); }
    /// Destruction calls T's Destroy().
    inline ~traced_ptr() { ptr->UnReference(); ptr->Destroy(); }
    /// Convert this to a pointer to T.
    inline operator T *() const { return ptr; }
    /// Convert this to a pointer to T.
    inline T *operator->() { return ptr; }
};

/** @} */
#endif /* _traced_H_ */
