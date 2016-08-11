ATK_INTER
#ifndef _updatelist_H_
#define _updatelist_H_
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

#endif /* _updatelist_H_ */
