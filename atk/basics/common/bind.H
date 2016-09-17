ATK_INTER
#ifndef _bind_H_
#define _bind_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
#include <proctable.H>

struct bind_Description {
    const char *procName;
    const char *keyVector;
    long    keyRock;
    const char *menuEntry;
    long    menuRock;
    long    menuMask;
    proctable_fptr proc;
    const char *doc;
    const char *module;
};

struct bind_ActionDescription {
    const char *procName;
    const char *keyVector;
    char	    *keyRock;
    const char *menuEntry;
    char	    *menuRock;
    long    menuMask;
    aaction *act;
    const char    *doc;
    const char    *module;
};
class bind : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void BindList(const struct bind_Description *bl, class keymap *km, class menulist *ml, const struct ATKregistryEntry  *type);
    static void BindList(const struct bind_ActionDescription *bl, class keymap *km, class menulist *ml, const struct ATKregistryEntry  *type);
};

#endif /* _bind_H_ */
