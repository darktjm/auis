#ifndef _text822_H_
#define _text822_H_
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


#define MODE822_NORMAL 1
#define MODE822_FORMAT 2
#define MODE822_FIXEDWIDTH 4
#define MODE822_ROT13 8
#define MODE822_NOAUTOFIX 16

#include <text.H>
class text822: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       long Read(FILE *file, long id)  ;
       long ReadSubString(long pos, FILE *file, boolean quoteCharacters)  ;
       long Write(FILE *file, long writeID, int level)  ;
       const char * ViewName()  ;
       void Clear();
       void ClearCompletely();
    
      virtual long ReadAsText(FILE *file, long id)  ;
    
      text822();
	
	static boolean InitializeClass();
	static boolean atk_inited;

      static boolean ReadIntoText(class text *d, FILE *fp, int Mode, const char *ContentTypeOverride, int *len, boolean IsReallyTextObject, int *bodystart, int *ignorepos, class text *auxheadtext)  ;
      static void ResetGlobalStyle(class text *t);
    
      class style *InstructionsStyle, *BigBoldStyle;

      int autofix;    /* TRUE if automatically made fixed width in Read. */
};

#endif /* _text822_H_ */
