/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * $Log: rect.C,v $
// Revision 3.2  1993/07/29  20:15:08  rr2b
// Great struct -> class conversion
//
// Revision 3.1  1993/04/26  20:54:27  rr2b
// Converted to new ATKdefineRegistry syntax.
// Converted calls to ATK::DynamicLoad to ATK::LoadClass.
//
// Revision 3.0  1993/04/24  18:33:18  rr2b
// Cfront port, see top level CFRONT.PORT
//
// Revision 2.1  1993/04/13  00:00:17  rr2b
// compiled cleanly with no changes
//
// Revision 2.0  1993/04/11  23:29:38  rr2b
// revised converter
//
 * Revision 2.8  1992/12/15  21:28:38  rr2b
 * more disclaimerization fixing
 *
 * Revision 2.7  1992/12/14  20:36:31  rr2b
 * disclaimerization
 *
 * Revision 2.6  1991/09/12  16:00:15  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.5  1989/02/17  18:40:04  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 *
 * Revision 2.4  89/02/08  15:59:18  ghoti
 * change copyright notice
 * 
 * Revision 2.3  89/02/06  19:32:47  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.2  89/02/06  15:32:53  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.1  88/09/27  14:29:53  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  01:14:35  ghoti
 * *** empty log message ***
 * 
 * Revision 1.1  88/09/01  01:47:18  zs01
 * "initial
 * 
 * Revision 5.0  88/05/29  21:10:14  ajp
 * Up to date - tested as of 5/29/88
 * bumped version number to 5.0
 * named "june88"
 * 
 * Revision 1.12  88/05/22  22:10:34  ajp
 * Update to bring in line with IBM/MIT Release
 * 
 * Revision 1.5  87/12/06  17:46:41  urling
 * cmu update
 * 
 * Revision 1.3  87/11/20  08:58:40  urling
 * cmu update
 * 
 * Revision 1.4  87/11/20  07:13:47  urling
 * *** empty log message ***
 * 
 * Revision 1.3  87/11/19  11:39:30  urling
 * Nov. 19 cmu update
 * 
 * Revision 1.10  87/11/11  19:03:47  wjh
 * added def'n of macro rectangle_GetRectSize
 * corrected the names of macros rectangle_IsEmptyRect & rectangle_IsEqualRect
 * fixed Union and Intersect to know about empty rectangles
 * made some arguments registers
 * fixed PtInRect to exclude points on the bottom and right
 * 
 *
 */

#include <andrewos.h>
#include <rect.h>

/******************** methods ******************/

void rectangle_UnionRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS);
void rectangle_IntersectRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS);
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
void rectangle_SetLeft(struct rectangle  * Rect,long  Value );
void rectangle_SetRight(struct rectangle  * Rect,long  Value );
void rectangle_SetWidth(struct rectangle  * Rect,long  Value );
void rectangle_SetHeight(struct rectangle  * Rect,long  Value );
void rectangle_SetTop(struct rectangle  * Rect,long  Value );
void rectangle_SetBottom(struct rectangle  * Rect,long  Value );
struct rectangle * rectangle_CreateRectCorners(struct point  * OneCorner,struct point  * OtherCorner );
struct rectangle * rectangle_CreateRectSize(long  left,long  top,long  width,long  height);
struct rectangle * rectangle_CreateRectSides(long  left,long  top,long  right, long  bottom);
struct rectangle * rectangle_Duplicate(struct rectangle  * Rect );


void
rectangle_UnionRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS)
		{
	long left, top;
		/* Result may be same as RHS or LHS, so we
		must defer changing Result until after compute is done */

	if (rectangle_IsEmptyRect(LHS))  {*Result = *RHS; return;}
	if (rectangle_IsEmptyRect(RHS))  {*Result = *LHS; return;}

	left = (LHS->left <  RHS->left) ? LHS->left : RHS->left;
	top = (LHS->top <  RHS->top) ? LHS->top : RHS->top;
	Result->width = (LHS->left+LHS->width <  RHS->left+RHS->width) 
			? RHS->left+RHS->width - left
			: LHS->left+LHS->width - left;
	Result->height = (LHS->top+LHS->height <  RHS->top+RHS->height) 
			? RHS->top+RHS->height - top
			: LHS->top+LHS->height - top;
	Result->left = left;
	Result->top = top;
}

	void
rectangle_IntersectRect(struct rectangle  *Result,struct rectangle  *LHS , struct rectangle  *RHS)
		{
	long left, top;
		/* Result may be same as RHS or LHS, so we
		must defer changing Result until after compute is done */

	if (rectangle_IsEmptyRect(LHS) || rectangle_IsEmptyRect(RHS)) 
		{rectangle_EmptyRect(Result);  return;}

	left = (LHS->left <  RHS->left) ? RHS->left : LHS->left;
	top = (LHS->top <  RHS->top) ? RHS->top : LHS->top;
	Result->width = (LHS->left+LHS->width <  RHS->left+RHS->width) 
			? LHS->left+LHS->width - left
			: RHS->left+RHS->width - left;
	Result->height = (LHS->top+LHS->height <  RHS->top+RHS->height) 
			? LHS->top+LHS->height - top
			: RHS->top+RHS->height - top;
	Result->left = left;
	Result->top = top;
}

/*
void rectangle_SetRectSize(LHS, left,top,width,height)
struct rectangle * LHS;
long top;
long left;
long height;
long width;{
    LHS->top = top;
    LHS->left = left;
    LHS->height = height;
    LHS->width = width;
}
*/

void rectangle_SetRectSides(struct rectangle  * LHS, long  left,long  top,long  right,long  bottom)
{
    LHS->top = top;
    LHS->left = left;
    LHS->height = bottom-top;
    LHS->width = right-left;
}

void rectangle_SetRectCorners(struct rectangle  * LHS, struct point  * OneCorner, struct point  * OtherCorner )
{

    long LeftEdge;
    long RightEdge;
    long TopEdge;
    long BottomEdge;

    LeftEdge = RightEdge = point_X(OneCorner);
    if (point_X(OneCorner) < point_X(OtherCorner)) {
	RightEdge = point_X(OtherCorner);
        }
    else {
	LeftEdge = point_X(OtherCorner);
        }
    TopEdge = BottomEdge = point_Y(OneCorner);
    if (point_Y(OneCorner) < point_Y(OtherCorner)) {
	BottomEdge = point_Y(OtherCorner);
        }
    else {
	TopEdge = point_Y(OtherCorner);
        }
    rectangle_SetRectSides(LHS,LeftEdge,TopEdge,RightEdge,BottomEdge);
}

void rectangle_InsetRect(struct rectangle  * LHS, long  DeltaX, long  DeltaY)
{
    LHS->top += DeltaY;
    LHS->height -= 2*DeltaY;
    LHS->left += DeltaX;
    LHS->width -= 2*DeltaX;
}

void rectangle_OffsetRect(struct rectangle  * LHS, long  DeltaX, long  DeltaY)
{
    LHS->top += DeltaY;
    LHS->left += DeltaX;
}

void rectangle_EmptyRect(struct rectangle  * Rect )
{
    rectangle_SetRectSize(Rect,0,0,-1,-1);
}

boolean rectangle_IsEmptyRect(struct rectangle  * TestRect)
{
    return (TestRect->width <= 0) || (TestRect->height <= 0);
}

boolean rectangle_IsEqualRect(struct rectangle  * LHS, struct rectangle  * RHS)
{
    return ( (LHS->left == RHS->left) && (LHS->top == RHS->top) &&
	     (LHS->width == RHS->width) &&
	     (LHS->height == RHS->height) ) ||
	   (rectangle_IsEmptyRect(LHS) && rectangle_IsEmptyRect(RHS));
}

boolean rectangle_IsEnclosedBy(struct rectangle  * InnerRect, struct rectangle  * OuterRect)
/* Tests to see if InnerRect is enclosed by he OuterRect, i.e., InnerRect is
in OuterRect */
{
    if (InnerRect->left < OuterRect->left) return FALSE;
    if (InnerRect->top < OuterRect->top) return FALSE;
    if (InnerRect->left+InnerRect->width > OuterRect->left+OuterRect->width) return FALSE;
    if (InnerRect->top+InnerRect->height > OuterRect->top+OuterRect->height) return FALSE;
    return TRUE;
}

boolean rectangle_IsPtInRect(struct point  * TestPoint ,struct rectangle  * TestRect)
{
    if (point_X(TestPoint) < TestRect->left) return FALSE;
    if (point_Y(TestPoint) < TestRect->top) return FALSE;
    if (point_X(TestPoint) >= TestRect->left+TestRect->width) return FALSE;
    if (point_Y(TestPoint) >= TestRect->top+TestRect->height) return FALSE;
    return TRUE;
}

short rectangle_PtToAngle(struct point  * SamplePoint ,struct rectangle  * ReferenceRect)
{
    /* I need a precise definition of this to implement it,
	so for now, punt */
    return 30; /* 30 degrees is a nice number */
}


void rectangle_SetLeft(struct rectangle  * Rect,long  Value )
{
    Rect->left = Value;
}

void rectangle_SetRight(struct rectangle  * Rect,long  Value )
{
    Rect->width = Value - Rect->left;
}

void rectangle_SetWidth(struct rectangle  * Rect,long  Value )
{
    Rect->width = Value;
}

void rectangle_SetHeight(struct rectangle  * Rect,long  Value )
{
    Rect->height = Value;
}

void rectangle_SetTop(struct rectangle  * Rect,long  Value )
{
    Rect->top = Value;
}

void rectangle_SetBottom(struct rectangle  * Rect,long  Value )
{
    Rect->height = Value - Rect->top;
}

/************** class procedures *******************/


struct rectangle * rectangle_CreateRectCorners(struct point  * OneCorner,struct point  * OtherCorner )
{
    struct rectangle * RetValue;

    RetValue = (struct rectangle *) malloc(sizeof(struct rectangle));
    rectangle_SetRectCorners(RetValue,OneCorner,OtherCorner);
    return RetValue;
}

struct rectangle * rectangle_CreateRectSize(long  left,long  top,long  width,long  height)
{
    struct rectangle * RetValue;

    RetValue = (struct rectangle *) malloc(sizeof(struct rectangle));
    rectangle_SetRectSize(RetValue,left,top,width,height);
    return RetValue;
}


struct rectangle * rectangle_CreateRectSides(long  left,long  top,long  right, long  bottom)
{
    struct rectangle * RetValue;

    RetValue = (struct rectangle *) malloc(sizeof(struct rectangle));
    rectangle_SetRectSides(RetValue,left,top,right,bottom);
    return RetValue;
}

struct rectangle * rectangle_Duplicate(struct rectangle  * Rect )
{
    struct rectangle * RetValue;

    RetValue = (struct rectangle *) malloc(sizeof(struct rectangle));
    *RetValue = *Rect;
    return RetValue;
}
