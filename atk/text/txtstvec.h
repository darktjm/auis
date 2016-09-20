/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#ifndef TEXTSTATEVECTOR_DEFINED
#define TEXTSTATEVECTOR_DEFINED

#include <style.H>

struct text_statevector {
    long CurLeftMargin;
    long CurRightMargin;
    long CurRightEdge;
    long CurLeftEdge;
    long CurTopMargin;
    long CurBottomMargin;
    long CurFontAttributes;
    long CurScriptMovement;
    long CurFontSize;
    long CurIndentation;
    enum style_Justification CurJustification;
    long CurSpacing;
    long CurSpread;
    long SpecialFlags;
    struct fontdesc *CurCachedFont;
    const char *CurFontFamily;
/* 
    struct dfamily * CurFontFamily;
    struct dfont * CurCachedFont;
 */
    struct tabs *tabs;
    struct view * CurView;		/* Pointer to view wrapped in the environment */
    char *CurColor; /* Color for  text */
    char *CurBGColor;
};
/* @} */
#endif /* TEXTSTATEVECTOR_DEFINED */
