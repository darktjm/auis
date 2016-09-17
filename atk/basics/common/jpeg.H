ATK_INTER
#ifndef _jpeg_H_
#define _jpeg_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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
#include <image.H>

class jpeg : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	jpeg();
      
	virtual ~jpeg();

    
       long Read( FILE *file, long id )  ;
       long Write( FILE *file, long writeID, int level )  ;
       long WriteNative( FILE *file, const char *filename )  ;
       int Load( const char *fullname, FILE *fp )  ;

    
      static int Ident( const char *fullname )  ;

    
      inline void SetSaveQuality(int q) { ((this)->saveQuality = (q)); }

    
      int saveQuality;
};


#endif /* _jpeg_H_ */
