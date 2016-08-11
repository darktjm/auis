/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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


 



#ifndef RECTANGLE_DEFINED

#define RECTANGLE_DEFINED

#include <point.h>

struct rectangle {
    long top,left,height,width;
};

struct bothrectangle {
    union  {
	struct rectangle rectformat;
	struct { struct point upperLeft, lowerRightOffset;} pointformat;
    } bothformats;
} ;


#define rectangle_SetRectSize(LHS,sleft,stop,swidth,sheight) \
(    ((LHS)->top = (stop)), \
    ((LHS)->left = (sleft)), \
    ((LHS)->height = (sheight)), \
    ((LHS)->width = (swidth)) )

#define rectangle_Left(Rect)  ((Rect)->left)
#define rectangle_Top(Rect)  ((Rect)->top)
#define rectangle_Right(Rect) ((Rect)->left + (Rect)->width)
#define rectangle_Bottom(Rect) ((Rect)->top + (Rect)->height)
#define rectangle_Width(Rect) ((Rect)->width)
#define rectangle_Height(Rect) ((Rect)->height)
#define rectangle_GetRectSize(r, x, y, w, h) \
		(*(x) = (r)->left, *(y) = (r)->top, \
		*(w) = (r)->width, *(h) = (r)->height)

void rectangle_IntersectRect(register struct rectangle  *Result,register struct rectangle  *LHS , register struct rectangle  *RHS);
void rectangle_UnionRect(register struct rectangle  *Result,register struct rectangle  *LHS , register struct rectangle  *RHS);
/*void rectangle_SetRectSize(LHS, left,top,width,height); */
void rectangle_SetRectSides(register struct rectangle  * LHS, long  left,long  top,long  right,long  bottom);
void rectangle_SetRectCorners(register struct rectangle  * LHS, register struct point  * OneCorner, register struct point  * OtherCorner );
void rectangle_InsetRect(register struct rectangle  * LHS, long  DeltaX, long  DeltaY);
void rectangle_OffsetRect(struct rectangle  * LHS, long  DeltaX, long  DeltaY);
void rectangle_EmptyRect(struct rectangle  * Rect );
boolean rectangle_IsEmptyRect(struct rectangle  * TestRect);
boolean rectangle_IsEqualRect(struct rectangle  * LHS, struct rectangle  * RHS);
boolean rectangle_IsEnclosedBy(struct rectangle  * InnerRect, struct rectangle  * OuterRect);
boolean rectangle_IsPtInRect(struct point  * TestPoint ,struct rectangle  * TestRect);
short rectangle_PtToAngle(struct point  * SamplePoint ,struct rectangle  * ReferenceRect);
/*long rectangle_Left(Rect);*/
/*long rectangle_Right(Rect);*/
/*long rectangle_Top(Rect);*/
/*long rectangle_Bottom(Rect);*/
/*long rectangle_Width(Rect);*/
/*long rectangle_Height(Rect);*/
void rectangle_SetLeft(struct rectangle  * Rect,long  Value );
void rectangle_SetRight(struct rectangle  * Rect,long  Value );
void rectangle_SetHeight(struct rectangle  * Rect,long  Value );
void rectangle_SetWidth(struct rectangle  * Rect,long  Value );
void rectangle_SetTop(struct rectangle  * Rect,long  Value );
void rectangle_SetBottom(struct rectangle  * Rect,long  Value );

struct rectangle * rectangle_CreateRectCorners(struct point  * OneCorner,struct point  * OtherCorner );
struct rectangle * rectangle_CreateRectSize(long  left,long  top,long  width,long  height);
struct rectangle * rectangle_CreateRectSides(long  left,long  top,long  right, long  bottom);
struct rectangle * rectangle_Duplicate(struct rectangle  * Rect );

#endif /* RECTANGLE_DEFINED */

ENDCPLUSPLUSPROTOS

