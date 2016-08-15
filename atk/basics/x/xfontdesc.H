ATK_INTER
#ifndef _xfontdesc_H_
#define _xfontdesc_H_
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


 


#include <fontdesc.H>
class xfontdesc : public  fontdesc {
public:
	virtual ATKregistryEntry *ATKregistry();



	virtual class graphic * CvtCharToGraphic(class graphic *gr, char SpecialChar)  ;
	virtual struct font * GetRealFontDesc(class graphic *gr)  ;

	virtual long StringSize(class graphic *gr, const char * string, long * XWidth, long * YWidth)  ;
	virtual long TextSize(class graphic *gr, const char * text, long TextLength, long *XWidth, long *YWidth)
		 ;
	virtual short* WidthTable(class graphic *gr)   ; /* actually an array of short's */
	virtual short * HeightTable(class graphic *gr)  ; /* actually an array of short's */
	virtual void CharSummary(class graphic *gr, char LookUpChar, 
			struct fontdesc_charInfo * returnedInfo);


	
	xfontdesc();
	
	virtual ~xfontdesc();
	
	static boolean InitializeClass();

};

#endif /* _xfontdesc_H_ */
