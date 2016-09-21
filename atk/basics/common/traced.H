ATK_INTER
#ifndef _traced_H_
#define _traced_H_
/* Copyright 1992 Andrew Toolkit Consortium, Carnegie Mellon University */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>

class traced : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


	traced();

	virtual void Destroy();
	virtual ~traced();

	inline unsigned long Reference() { return ((this)->refcount++); }
	inline unsigned long UnReference() { return (((this)->refcount>0)?(this)->refcount--:0); }
	inline unsigned long   ReferenceCount() { return ((this)->refcount); }

	unsigned long refcount;
};
/** @} */
#endif /* _traced_H_ */
