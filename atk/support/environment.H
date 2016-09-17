ATK_INTER
#ifndef _environment_H_
#define _environment_H_
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


 

#define environment_VERSION 1

union environmentcontents {
    class style *style;
    class viewref *viewref;
};

enum environmenttype {
    environment_None,
    environment_Style,
    environment_View,
    environment_Any
};

#include <nestedmark.H>

class environment : public  nestedmark  {
public:
	virtual ATKregistryEntry *ATKregistry();


/* the next two are obsolete. Use WrapStyle, WrapView, InsertStyle, or
 * InsertView instead.
 */
    virtual class environment * Wrap(long pos, long length, enum environmenttype type, union environmentcontents data)  ;
    virtual class environment * Insert(long rpos, enum environmenttype type, union environmentcontents data, boolean doinsert)  ;
    virtual class environment * WrapStyle(long pos, long length, class style *style)  ;
    virtual class environment * WrapView(long pos, long length, class viewref *viewref)  ;
    virtual class environment * InsertStyle(long rpos, class style *style, boolean doinsert)  ;
    virtual class environment * InsertView(long rpos, class viewref *viewref, boolean doinsert)  ;
    virtual boolean Remove(long pos, long length, enum environmenttype type, boolean deleteall)  ;

     class nestedmark * NewButSimilar()  ;

    
	environment();
    
	virtual ~environment();
	static class environment * GetRootEnvironment()  ;
#if 0	
    static class environment * Allocate()  ;
    static void Deallocate(class environment *self);
#endif 
    enum environmenttype type;
    union environmentcontents data;
};

#endif /* _environment_H_ */
