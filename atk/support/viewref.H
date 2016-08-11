ATK_INTER
#ifndef _viewref_H_
#define _viewref_H_
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


 

#define viewref_VERSION 1

#include <observable.H>
class environment;
class viewref: public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	viewref();
    
	virtual ~viewref();
    static class viewref * Create(char *viewtype, class dataobject *dataobject)  ;

    char *viewType;
    long viewID;
    class dataobject *dataObject;
    long desw,desh;
    environment *env;
    // Set & Get the environment where this viewref is located in the text.
    inline void SetEnvironment(environment *e) {
        env=e;
    }
    inline environment *Environment() {
        return env;
    }
 
};

#endif /* _viewref_H_ */
