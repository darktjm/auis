ATK_INTER
#ifndef _tabs_H_
#define _tabs_H_
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


/* Nick Williams, August 1990 */

/* tabs are a package. 
 * However, we want to be able to define methods and data,
 * so we have it as a basic class.
 */

#include <ATK.H>
#include <style.H>

class tabs : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	tabs();
    
	virtual ~tabs();
    static class tabs * Create()  ;
    static void Death(class tabs *self);

    virtual void OutputTroff(long indent, FILE *file);
    virtual int Different(class tabs *b)  ;
    virtual class tabs * Delete(int n)  ;
    virtual class tabs * Add(long pos, enum style_TabAlignment op)  ;
    virtual class tabs * Clear()  ;
    virtual class tabs * ApplyChange(struct tabentry *change)  ;
    virtual class tabs * Copy()  ;

    long *Positions;
    long *Types;
    int number;
    int links;
};

#endif /* _tabs_H_ */