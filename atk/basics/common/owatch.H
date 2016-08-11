ATK_INTER
#ifndef _owatch_H_
#define _owatch_H_
/* Copyright 1992 Carnegie Mellon University, All rights reserved.
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
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/owatch.H,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $ */

struct owatch_data {
    class observable *obj;
    boolean alive;
    struct owatch_data *next, *prev;
    long refs;
};

#include <observable.H>

class owatch : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();


      
     void ObservedChanged(class observable *changed, long val);

    static inline boolean Check(struct owatch_data *tocheck) { return (tocheck?tocheck->alive:FALSE); }

    static struct owatch_data * Create(class observable *b)  ;
    static void Delete(struct owatch_data *d);
    static boolean CheckAndDelete(struct owatch_data *d)  ;

};

#endif /* _owatch_H_ */
