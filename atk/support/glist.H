ATK_INTER
#ifndef _glist_H_
#define _glist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
/* List object
 * for Graph Editor
 */

struct glistelt {
    void * this_c;
    struct glistelt *next;
    int position;
};

#include <ATK.H>

/* Greater takes two arguments, element_1 and element_2.
  It should return  a positive number if element_1 is greater than
  element_2, a negative number if element_1 is less than element_2,
  and zero if they are equal.
*/
	typedef int (*glist_greaterfptr)(const void *a, const void *b);
	typedef void (*glist_destroyfptr)(void *a);
	typedef boolean (*glist_findfptr)(const void *item, const void *rock);
	typedef void (*glist_efptr)(const void *item, unsigned long rock);
	
#define GLIST_CLASS(type) Concat(glist_,type)
#define DECLARE_GLIST_CLASS(type,findrocktype,enumrocktype) \
	typedef int (*Concat3(glist_,type,_greaterfptr))(const type *a, const type *b); \
	typedef void (*Concat3(glist_,type,_destroyfptr))(type *a); \
	typedef boolean (*Concat3(glist_,type,_findfptr))(const type *item, const findrocktype *rock); \
	typedef void (*Concat3(glist_,type,_efptr))(type *item, enumrocktype *rock); \
	class Concat(glist_,type) : private glist { \
	  public: \
	    static inline Concat(glist_,type) * Create(Concat3(glist_,type,_destroyfptr) Destroy)  { \
		Concat(glist_,type) *res=new Concat(glist_,type); \
		if(res==NULL) return res; \
	res->DestroyProc=(glist_destroyfptr) Destroy; \
	return res; \
	    } \
	    virtual ~Concat(glist_,type)() { \
	    } \
	    static inline void Copy(class Concat(glist_,type) *dest, Concat(glist_,type) *source) { \
		glist::Copy(dest, source); \
	    } \
	    inline boolean Insert(type *glist_arg, int glist_eposition=0) { \
		return glist::Insert((void *)glist_arg, glist_eposition); \
	    } \
	    inline boolean Push(type *glist_arg) { \
		return glist::Push((void *)glist_arg); \
	    } \
	inline boolean InsertSorted(type *glist_arg, Concat3(glist_,type,_greaterfptr) glist_greater, int glist_position) { \
		return glist::InsertSorted((void *)glist_arg, (glist_greaterfptr) glist_greater, glist_position); \
	    } \
	    inline boolean Sort(Concat3(glist_,type,_greaterfptr) glist_greater) { \
		return glist::Sort((glist_greaterfptr) glist_greater); \
	    } \
	    inline boolean InsertUnique(type *glist_arg, int glist_eposition=0) { \
		return glist::InsertUnique((void *)glist_arg, glist_eposition); \
	    } \
	    inline boolean Delete(type *glist_arg, boolean glist_destroy) { \
		return glist::Delete((void *)glist_arg, glist_destroy); \
	    } \
	    inline type *Find(Concat3(glist_,type,_findfptr) glist_filter, findrocktype glist_rock) { \
		return (type *)glist::Find((glist_findfptr)glist_filter,(void *)glist_rock); \
	    } \
	    inline void Enumerate(Concat3(glist_,type,_efptr) glist_proc, enumrocktype glist_rock) { \
		glist::Enumerate((glist_efptr)glist_proc, (unsigned long)glist_rock); \
	    } \
	    inline boolean Contains(type *glist_arg) { \
		return glist::Contains((void *)glist_arg); \
	    } \
	    inline void Clear(boolean glist_destroy) { \
		glist::Clear(glist_destroy); \
	    } \
	    inline type *First() { \
		return (type *)glist::First(); \
	    } \
	    inline type *Next() { \
		return (type *)glist::Next(); \
	    } \
	    inline type *Last() { \
		return (type *)glist::Last(); \
	    } \
	    inline long Size() { \
		return glist::Size(); \
	    } \
	};
	
class glist : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
    static class glist * Create(glist_destroyfptr Destroy)  ;
    
	virtual ~glist();
    
	glist();    
    static void Copy(class glist *dest, class glist *source);

    virtual boolean Insert(void * element, int eposition=0)  ;
    virtual boolean Push(void *element)  ;
    virtual void * Pop()  ;
    virtual boolean InsertSorted(void *element, glist_greaterfptr greater, int position=0)  ;
    virtual boolean Sort(glist_greaterfptr greater)  ;
    virtual boolean InsertUnique(void * element, int eposition=0)  ;
    virtual boolean Delete(const void * element, boolean destroy)  ;
    virtual void * Find(glist_findfptr filter, const void * rock)  ; /* filter is called: filter(void * element) */
    virtual void Enumerate(glist_efptr proc, unsigned long rock);
    virtual boolean Contains(const void * element)  ;
    virtual void Clear(boolean destroy);

    inline void * First() { return ((current=(this)->head)->this_c); }
    inline void * Next()  { return ((current=current->next)?current->this_c:NULL); }
    inline void * Last() { return ((this)->tail->this_c); }
    inline long Size() { return ((this)->size); }

    struct glistelt *head;
    struct glistelt *tail;
    struct glistelt *current;
    long size;
    glist_destroyfptr DestroyProc;
};
/** @} */
#endif /* _glist_H_ */
