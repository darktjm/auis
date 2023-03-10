#ifndef _fontdesc_H_
#define _fontdesc_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#define fontdesc_All	-1
#define fontdesc_Plain	0
#define fontdesc_Bold	1
#define fontdesc_Italic	2
#define fontdesc_Shadow	4
#define fontdesc_Fixed	8
#define fontdesc_Outline 16
#define fontdesc_Thin	32
#define fontdesc_Black	64
#define fontdesc_Medium	128
#define fontdesc_Heavy	256
#define fontdesc_Condense 512

#define fontdesc_NumIcons 256


/* FaceCode flags - normally found in font.h for WM fonts*/
#define BoldFace 1
#define ItalicFace 2
#define ShadowFace 4
#define FixedWidthFace 010

struct fontnamedesc  {
    struct fontnamedesc *next;	/* next guy in list of all fonts */
    class fontdesc *fontList;	/* list of all fonts in this family */
    char *name;			/* family name: use = for equality check */
};

struct FontSummary {
    short maxWidth;	/* the max char width over the whole font */
    short maxHeight;	/* the max above+below for the whole font */
    short newlineHeight;/* recommended newline distance for the font */
    short maxSpacing;	/* the max spacing in x direction */
    short maxBelow;	/* the max below for the whole font */
    short maxLeft;	/* the max to the left of the origin */
};

struct fontdesc_charInfo {
    short width;			/* Width of bounding box */
    short height;			/* Height of bounding box */
    short xOriginOffset;	        /* X offset from left of bb */
    short yOriginOffset;		/* Y offset from top of bb */
    short xSpacing;		/* hor dist from this origin to next*/
    short ySpacing;		/* vert dist from this origin to next */
};


#include <ATK.H>

class fontdesc : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual char * GetFontFamily()  ;
    virtual struct fontnamedesc * GetFontFamilyDesc()  ;
    virtual long GetFontSize()  ;
    virtual long GetFontStyle()  ;

    virtual class graphic * CvtCharToGraphic(class graphic *gr, char SpecialChar)  ;
    virtual struct font * GetRealFontDesc(class graphic *gr)  ;

    virtual long StringSize(class graphic *gr, const char * string, long * XWidth, long * YWidth)  ;
    virtual long StringBoundingBox(class graphic *graphic, const char *string, int *width, int *height)  ;
    virtual long TextSize(class graphic *gr, const char * text, long TextLength, long *XWidth, long *YWidth)
		 ;
    virtual short* WidthTable(class graphic *gr)   ; /* actually an array of short's */
    virtual short * HeightTable(class graphic *gr)  ; /* actually an array of short's */
    virtual struct FontSummary * FontSummary(class graphic *gr)  ;
    virtual void CharSummary(class graphic *gr, char LookUpChar, struct fontdesc_charInfo * returendInfo);

    inline void SetCharValid(char c) { (this->charValid[((unsigned char)c) >> 3] |= (1 << (((unsigned char)c) & 7))); }
    inline void ClearCharValid(char c) { (this->charValid[((unsigned char)c) >> 3] &= ~(1 << (((unsigned char)c) & 7))); }
    inline boolean CharValid(char c) { return (this->charValid[((unsigned char)c) >> 3] & (1 << (((unsigned char)c) & 7))); }

    static class fontdesc * Create(const char * FontName, long FontStyle, long FontSize)  ;
    static struct fontnamedesc * GetFontNameDesc(char * FontName)  ;
    static boolean ExplodeFontName(const char *fontName, char *familyName, long bufSize, long *fontStyle, long *fontSize)  ;
    
	fontdesc();
    
	virtual ~fontdesc();
    static class fontdesc * Allocate()  ; 
    static void Deallocate(class fontdesc *self);

    struct fontnamedesc  *FontName;
    long   FontStyles;
    struct FontSummary summary;
    short *widthTable;		/* the full width table, if non-null */
    short *heightTable;		/* the full height table, if non-null*/
    short    FontSize;
    boolean DescValid;
    struct font * MachineDependentFontDescriptor;
    class fontdesc *next;
    unsigned char charValid[fontdesc_NumIcons / 8];
};
/** @} */
#endif /* _fontdesc_H_ */
