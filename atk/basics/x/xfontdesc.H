#ifndef _xfontdesc_H_
#define _xfontdesc_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
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
/** @} */
#endif /* _xfontdesc_H_ */
