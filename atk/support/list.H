#ifndef _list_H_
#define _list_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
/*
 * Generic Linked List / Stack / Queue datatype
 *
 * Although char *'s are used here, pointers to any type of object
 * or structure may be used with minimal casting.
 */

struct list_Entry {
    struct list_Entry *next;
    char *data;
};

#include <ATK.H>


	typedef int (*list_greaterfptr)(char *v1, char *v2);
	typedef boolean (*list_efptr)(char *value, char *rock);

class list : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
	
	list();
    
	virtual ~list();

    /* Copies dst to the end of src.   Src is left alone. */
    static void Merge(class list *dst, class list *src);

    /* Tacks src list onto the end of dst. Src is emptied. */
    static void Append(class list *dst, class list *src);


    virtual void InsertFront(char *data);
    virtual void InsertEnd(char *data);
    virtual boolean InsertUnique(char *data)  ;
    virtual void InsertSorted(char *data, list_greaterfptr compare);

    virtual char * RemoveFront()  ;

    virtual boolean Delete(char *data)  ;
    virtual boolean Member(char *data)  ;

    virtual boolean Sort(list_greaterfptr greater)  ;

    /* Enumerate proc receives (data, rock).  If the */
    /* proc returns FALSE, enumerate terminates and */
    /* returns the piece of data responsible. */

    virtual char * Enumerate(list_efptr proc, const char *rock)  ;

    virtual void Clear();


    inline void Enqueue(char *data) { ((this)->InsertEnd( data)); }
    inline char * Dequeue() { return ((this)->RemoveFront()); }

    inline void Push(char *data) { ((this)->InsertFront( data)); }
    inline char * Pop() { return ((this)->RemoveFront()); }

    inline long Size() { return ((this)->size); }

    inline char * First() { return ((this)->size == 0 ? NULL : (this)->head->data); }
    inline char * Last() { return ((this)->size == 0 ? NULL : (this)->tail->data); }

    /* Way to traverse the list from head to tail: */
    /* Start and Advance return NULL at end of list. */
    /* Data returns the data at the current position, NULL at end */

    inline struct list_Entry * Start() { return ((this)->trav = (this)->head); }
    inline struct list_Entry * Advance() { return ((this)->trav == NULL ? NULL :  
              ((this)->trav = (this)->trav->next)); }
    inline char * Data() { return ((this)->trav == NULL ? NULL :  
              (this)->trav->data); }


    struct list_Entry *head;
    struct list_Entry *tail;
    long size;

    struct list_Entry *trav;
};
/** @} */
#endif /* _list_H_ */
