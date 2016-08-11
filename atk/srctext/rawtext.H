#ifndef _rawtext_H_
#define _rawtext_H_
/* File rawtext.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1992-1995.  All rights reserved.

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

   rawtext.ch: Text subclass specialized for dealing with initfiles and other style-less text. */

#include <text.H>
class rawtext: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


  
     boolean InsertCharacters(long pos, char *str, long len)  ; /*RSK92overstrike*/
     void SetAttributes(struct attributes *atts);
     long Write(FILE *file, long writeID, int level)  ;

  
    virtual boolean JustInsertCharacters(long pos,char *str,long len)  ; /*RSK92overstrike*/
    virtual void OverstrikeAChar(long pos); /*RSK92overstrike*/

  
    
	rawtext();
    
	virtual ~rawtext();

  
    inline boolean   PutNewlineAtEOF() { return ((this)->NewlineEOF); }
    inline boolean   IsInOverstrikeMode() { return ((this)->OverstrikeMode) /*RSK92overstrike*/; }
    inline void ChangeOverstrikeMode(boolean newValue) { (((this)->OverstrikeMode) = (newValue)) /*RSK92overstrike*/; }

  
    boolean NewlineEOF;
    boolean OverstrikeMode; /*RSK92overstrike*/ /*ideally, this should be associated with the view; unfortunately, the view is not known to srctext_InsertCharacters, which is where the overstriking is being done*/
};

#endif /* _rawtext_H_ */
