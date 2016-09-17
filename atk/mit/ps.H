#ifndef _ps_H_
#define _ps_H_
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


#include <icon.H>
class ps: public  icon {
public:
	virtual ATKregistryEntry *ATKregistry();

 
    void SetChild(class dataobject * child);
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
 
 
    inline long   GetPixelWidth() { return ((this)->pwidth); }
    inline long   GetPixelHeight() { return ((this)->pheight); }
    inline void SetPixelWidth(long value)  { ((this)->pwidth=(value)); }
    inline void SetPixelHeight(long value)  { ((this)->pheight=(value)); }
 
    
	ps();
    
	static boolean InitializeClass();
	static boolean atk_inited;

 
    long pwidth;
    long pheight;
};


#endif /* _ps_H_ */
