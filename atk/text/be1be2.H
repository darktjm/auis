ATK_INTER
#ifndef _be1be2_H_
#define _be1be2_H_
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

/*
 * BE1 to BE2 conversion package
 */

#include <ATK.H>

class be1be2 : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();




    /*
     * CheckBE1 uses heuristics to decide if a document
     * is BE1.  It is fairly accurate although it's impossible
     * to do this task with the preferred efficiently.
     */

    static boolean CheckBE1(class text *text)  ;

    /*
     * Convert usually adds many entries to text's stylesht.
     * Returns a boolean indication of success.  However, it
     * is too picky and usually does better than it reports.
     */

    static boolean Convert(class text *text)  ;

};

#endif /* _be1be2_H_ */
