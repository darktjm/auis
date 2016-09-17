/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


 

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


#endif /* TEXTSTATEVECTOR_DEFINED */
