ATK_INTER
#ifndef _tree23int_H_
#define _tree23int_H_
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


 

#define tree23int_VERSION 1

#include <ATK.H>

	typedef boolean (*tree23int_efptr)(long data, char *procdata);
	typedef void (*tree23int_filterfptr)(long data,char *procdata,class tree23int *newtree, class tree23int *lr);
	typedef void (*tree23int_mergefptr)(long data,char *procdata,class tree23int *newtree, class tree23int *lr);
	typedef void (*tree23int_applyfptr)(long data);

class tree23int : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    virtual void AddIn(long offset, class tree23int *newnode);
    virtual void Apply(tree23int_applyfptr proc);
    virtual void Filter(long offset, class tree23int *left, class tree23int *right, long lowval, long highval, tree23int_filterfptr proc, char *procdata);
    virtual long FindL(long offset, long key)  ;
    virtual long FindR(long offset, long key)  ;
    virtual void Free();
    virtual void Merge(class tree23int *ancestor, long offset, tree23int_mergefptr proc, char *procdata);
    virtual void Update(long pos, long size);
    virtual long Enumerate(tree23int_efptr proc, char *procdata)  ;
    virtual long NumberOfLeaves()  ; 
    virtual void Dump(long offset);
    virtual class tree23int * Delete()  ;
    virtual long Eval()  ;
    virtual class tree23int * Insert(long key, long data)  ;
    virtual void Jam(class tree23int *newnode);
    virtual class tree23int * Remove(class tree23int *child)  ;
    virtual void Twiddle(class tree23int *child);
    virtual class tree23int * GetLeftMostNode()  ;
    virtual class tree23int * GetNextNode(class tree23int *node)  ;
    virtual class tree23int * GetRightMostNode()  ;
    virtual class tree23int * GetPreviousNode(class tree23int *node)  ;

    inline long GetData() { return (this->data); }

    tree23int();
#if 0
	static class tree23int * Allocate()  ;
    static void Deallocate(class tree23int *self);
#endif
    class tree23int *parent;/* 	Parent of the node */
    long bump;			/* Amount to add to parents value to get key */
    long data;			/* The real data */
    boolean leaf;		/* True if the node is a leaf. */
    short nKids;
    class tree23int *kid[3];
};


#endif /* _tree23int_H_ */
