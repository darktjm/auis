#ifndef _traced_H_
#define _traced_H_
/* Copyright 1992 Andrew Toolkit Consortium, Carnegie Mellon University */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>

/** Simple reference counting class.
 *  This class grew out of a need for reference counting in the observable
 *  class.  The reference counting was split off into this class.  It relies
 *  on calling Destroy() rather than the object destructor, since object
 *  destructors cannot abort.  All objects get a reference count of 1
 *  when created, and are really destroyed (i.e., the destructor called and
 *  memory deallocated) when the reference count becomes zero.
 *  References are added manually by calling Reference().
 */
class traced : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();

    /** Default constructor:  set reference count to 1. */
    traced();

    /** Call this instead of delete.  This will drop the reference count
     *  and delete the object if zero.  */
    virtual void Destroy();
    virtual ~traced();

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
/** @} */
#endif /* _traced_H_ */
