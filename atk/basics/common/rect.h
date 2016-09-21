/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#ifndef RECTANGLE_DEFINED

#define RECTANGLE_DEFINED

/** \addtogroup libbasics
 * @{ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

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

void rectangle_IntersectRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS);
void rectangle_UnionRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS);
/*void rectangle_SetRectSize(LHS, left,top,width,height); */
void rectangle_SetRectSides(struct rectangle  * LHS, long  left,long  top,long  right,long  bottom);
void rectangle_SetRectCorners(struct rectangle  * LHS, struct point  * OneCorner, struct point  * OtherCorner );
void rectangle_InsetRect(struct rectangle  * LHS, long  DeltaX, long  DeltaY);
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

ENDCPLUSPLUSPROTOS
/** @} */
#endif /* RECTANGLE_DEFINED */

