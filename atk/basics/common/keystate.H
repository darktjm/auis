ATK_INTER
#ifndef _keystate_H_
#define _keystate_H_
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


 

/* keystate.ch -- A class that keep track of partially evaluated sequences of keystrokes.
December, 1986 */


#define keystate_VERSION	1

/* Return values from ApplyKey: */

enum keystate_ApplyKeyValues  {
    keystate_ProcFound,		/* pe has an entry */
    keystate_NoBinding,		/* key is undefined */
    keystate_Pending		/* keep feeding chars */
};

/* Return values from DoProc: */

enum keystate_DoProcValues {
    keystate_ProcCalled,	/* success */
    keystate_TypeMismatch,	/* couldn't find object of correc type */
    keystate_NoProc		/* couldn't load proc */
};

#include <keymap.H>

#include <ATK.H>

    typedef enum keymap_Types (*keystate_fptr)(long fdata, long key, struct proctable_Entry **p, long *rock);

class keystate : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
    virtual ~keystate() {}
    inline ATK  * GetObject() { return (this->object); }

    virtual void SetObject(ATK  *object);
    virtual class keystate * AddBefore(class keystate *ks)  ;
    virtual class keystate * AddAfter(class keystate *ks)  ;
    /* Routines for playing keystokes. */
    virtual void Reset();	/* inits whole chain and clears arg junk */
    virtual enum keystate_ApplyKeyValues ApplyKey(char key, struct proctable_Entry **ppe, long *rockP, ATK  **pobject)  ;
    virtual enum keystate_DoProcValues DoProc(struct proctable_Entry *pe, long rock, ATK  *object)  ;
    /* Zap a chain of keystates. */
    virtual void FreeChain();	/* Calls destroy on each keystate object */
    virtual void SetOverride(keystate_fptr function, long functionData);
    virtual void GetOverride(keystate_fptr *function, long *functionData);

    keystate();
	static class keystate * Create(ATK  *object, class keymap *keymap)  ;
    /* These are internal to the implementation. */
    static class keystate * Allocate()  ;
    static void Deallocate(class keystate *ks);


    class keystate *next;	/* links together keymaps */
    class keymap *orgMap;	/* keymap to use */
    class keymap *curMap;	/* where we are in traversal */
    ATK  *object;	/* first argument to function */
    keystate_fptr function; /* Override function to trap keystrokes. */
    long functionData; /* Rock for the override function. */
};


#endif /* _keystate_H_ */
