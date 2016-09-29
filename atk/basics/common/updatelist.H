#ifndef _updatelist_H_
#define _updatelist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
struct updateitem {
    class view *view;
    struct updateitem *next;
};

#include <ATK.H>

class updatelist : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual void AddTo(class view *view); /* Adds an update request. */
    virtual void DeleteTree(class view *tree); /* Removes update requests for views below the named view. */
    virtual void Clear(); /* Calls view_Update on all views that have been added and deletes them from the list. */

    inline boolean UpdatesPending() { return (this->updates != NULL); }

    
	virtual ~updatelist();
    
	updatelist();

    struct updateitem *updates;
};
/** @} */
#endif /* _updatelist_H_ */
