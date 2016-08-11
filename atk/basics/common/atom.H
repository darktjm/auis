ATK_INTER
#ifndef _atom_H_
#define _atom_H_
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


 

#include <ATK.H>

class atom : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static class atom * Intern(const char *name)  ;
    static class atom * Intern(char *name)  ;
    static class atom * InternRock(long rock)  ;
    
	static boolean InitializeClass();

    
	atom();
    
	virtual ~atom();

    inline char * Name() const { return (this->name); }
    inline char * Name() { return (this->name); }

    char *name;
};

class atom_def {
    atom *a;
  public:
    inline atom_def() {
	a = NULL;
    }
    inline atom_def(const char *def) {
	a = atom::Intern(def);
    }
    inline atom_def(const atom *def) {
	a = (atom *)def;
    }
    inline operator const atom *() const {
	return a;
    }
    inline const atom *operator->() const {
	return a;
    }
};

#endif /* _atom_H_ */
