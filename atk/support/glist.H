ATK_INTER
#ifndef _glist_H_
#define _glist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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


 

/* List object
 * for Graph Editor
 */

struct glistelt {
    char * this_c;
    struct glistelt *next;
    int position;
};

#include <ATK.H>

/* Greater takes two arguments, element_1 and element_2.
  It should return  a positive number if element_1 is greater than
  element_2, a negative number if element_1 is less than element_2,
  and zero if they are equal.
*/
	typedef int (*glist_greaterfptr)(char *a, char *b);
	typedef void (*glist_destroyfptr)(char *a);
	typedef boolean (*glist_findfptr)(char *item, char *rock);
	typedef void (*glist_efptr)(char *item, unsigned long rock);
	
#define GLIST_CLASS(type) Concat(glist_,type)
#define DECLARE_GLIST_CLASS(type,findrocktype,enumrocktype) \
	typedef int (*Concat3(glist_,type,_greaterfptr))(type *a, type *b); \
	typedef void (*Concat3(glist_,type,_destroyfptr))(type *a); \
	typedef boolean (*Concat3(glist_,type,_findfptr))(type *item, findrocktype *rock); \
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
		return glist::Insert((char *)glist_arg, glist_eposition); \
	    } \
	    inline boolean Push(type *glist_arg) { \
		return glist::Push((char *)glist_arg); \
	    } \
	inline boolean InsertSorted(type *glist_arg, Concat3(glist_,type,_greaterfptr) glist_greater, int glist_position) { \
		return glist::InsertSorted((char *)glist_arg, (glist_greaterfptr) glist_greater, glist_position); \
	    } \
	    inline boolean Sort(Concat3(glist_,type,_greaterfptr) glist_greater) { \
		return glist::Sort((glist_greaterfptr) glist_greater); \
	    } \
	    inline boolean InsertUnique(type *glist_arg, int glist_eposition=0) { \
		return glist::InsertUnique((char *)glist_arg, glist_eposition); \
	    } \
	    inline boolean Delete(type *glist_arg, boolean glist_destroy) { \
		return glist::Delete((char *)glist_arg, glist_destroy); \
	    } \
	    inline type *Find(Concat3(glist_,type,_findfptr) glist_filter, findrocktype glist_rock) { \
		return (type *)glist::Find((glist_findfptr)glist_filter,(char *)glist_rock); \
	    } \
	    inline void Enumerate(Concat3(glist_,type,_efptr) glist_proc, enumrocktype glist_rock) { \
		glist::Enumerate((glist_efptr)glist_proc, (unsigned long)glist_rock); \
	    } \
	    inline boolean Contains(type *glist_arg) { \
		return glist::Contains((char *)glist_arg); \
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

    virtual boolean Insert(char * element, int eposition=0)  ;
    virtual boolean Push(char *element)  ;
    virtual char * Pop()  ;
    virtual boolean InsertSorted(char *element, glist_greaterfptr greater, int position=0)  ;
    virtual boolean Sort(glist_greaterfptr greater)  ;
    virtual boolean InsertUnique(char * element, int eposition=0)  ;
    virtual boolean Delete(char * element, boolean destroy)  ;
    virtual char * Find(glist_findfptr filter, char * rock)  ; /* filter is called: filter(char * element) */
    virtual void Enumerate(glist_efptr proc, unsigned long rock);
    virtual boolean Contains(char * element)  ;
    virtual void Clear(boolean destroy);

    inline char * First() { return ((current=(this)->head)->this_c); }
    inline char * Next()  { return ((current=current->next)?current->this_c:NULL); }
    inline char * Last() { return ((this)->tail->this_c); }
    inline long Size() { return ((this)->size); }

    struct glistelt *head;
    struct glistelt *tail;
    struct glistelt *current;
    long size;
    glist_destroyfptr DestroyProc;
};

#endif /* _glist_H_ */
