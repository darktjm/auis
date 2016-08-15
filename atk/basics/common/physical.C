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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/physical.C,v 3.2 1993/07/29 20:15:08 rr2b Stab74 $";
#endif


 

/* This is a utility package only to be used by im and graphic (and
 perhaps other machine dependent modules that I do not yet know about,
 such as the cursor system. This package provides a way to convert
 between the coordinate spaces of a graphic and wm "hardware" */


#include <graphic.H>


#define SINGLEWINDOW 1

#ifndef NORCSID
#endif
long physical_LogicalPtToGlobalX(class graphic  * CoordinateSystem,struct point  * LogicalPoint );
long physical_LogicalPtToGlobalY(class graphic  * CoordinateSystem,struct point  * LogicalPoint );
void physical_LogicalPtToGlobalPt(class graphic  * CoordinateSystem,struct point  * tempPoint );
void physical_GlobalPtToLogicalPt(class graphic  * CoordinateSystem,struct point  * tempPoint );
long physical_LogicalXToGlobalX(class graphic  * CoordinateSystem,long  LogicalX );
long physical_LogicalYToGlobalY(class graphic  * CoordinateSystem,long  LogicalY );
long physical_GlobalXToLogicalX(class graphic  * CoordinateSystem,long  PhysicalX );
long physical_GlobalYToLogicalY(class graphic  * CoordinateSystem,long  PhysicalY );
void physical_LogicalToGlobalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect );
void physical_GlobalToLogicalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect );


long physical_LogicalPtToGlobalX(class graphic  * CoordinateSystem,struct point  * LogicalPoint )
{
#if SINGLEWINDOW
    return point_X(LogicalPoint)+
	   point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds);
#else /* SINGLEWINDOW */
    return point_X(LogicalPoint)-
	   rectangle_Left(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}   

long physical_LogicalPtToGlobalY(class graphic  * CoordinateSystem,struct point  * LogicalPoint )
{
#if SINGLEWINDOW
    return point_Y(LogicalPoint)+
	   point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds);
#else /* SINGLEWINDOW */
    return point_Y(LogicalPoint)-
	   rectangle_Top(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}   

void physical_LogicalPtToGlobalPt(class graphic  * CoordinateSystem,struct point  * tempPoint )
{
#if SINGLEWINDOW
    point_OffsetPoint(tempPoint,
	   point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds),
	   point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds));
#else /* SINGLEWINDOW */
    point_OffsetPoint(tempPoint,
	   -rectangle_Left(&CoordinateSystem->localBounds),
	   -rectangle_Top(&CoordinateSystem->localBounds));
#endif /* SINGLEWINDOW */
}   

void physical_GlobalPtToLogicalPt(class graphic  * CoordinateSystem,struct point  * tempPoint )
{
#if SINGLEWINDOW
    point_OffsetPoint(tempPoint,
	   -(point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds)),
	   -(point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds)));
#else /* SINGLEWINDOW */
    point_OffsetPoint(tempPoint,
	   rectangle_Left(&CoordinateSystem->localBounds),
	   rectangle_Top(&CoordinateSystem->localBounds));
#endif /* SINGLEWINDOW */
}   



long physical_LogicalXToGlobalX(class graphic  * CoordinateSystem,long  LogicalX )
{
#if SINGLEWINDOW
    return LogicalX +
	   point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds);
#else /* SINGLEWINDOW */
    return LogicalX -
	   rectangle_Left(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}   

long physical_LogicalYToGlobalY(class graphic  * CoordinateSystem,long  LogicalY )
{
#if SINGLEWINDOW
    return LogicalY +
	   point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds);
#else /* SINGLEWINDOW */
    return LogicalY -
	   rectangle_Top(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}   

long physical_GlobalXToLogicalX(class graphic  * CoordinateSystem,long  PhysicalX )
{
#if SINGLEWINDOW
    return PhysicalX - (
	   point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds));
#else /* SINGLEWINDOW */
    return PhysicalX +
	   rectangle_Left(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}

long physical_GlobalYToLogicalY(class graphic  * CoordinateSystem,long  PhysicalY )
{
#if SINGLEWINDOW
    return PhysicalY - (
	   point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds));
#else /* SINGLEWINDOW */
    return PhysicalY +
	   rectangle_Top(&CoordinateSystem->localBounds);
#endif /* SINGLEWINDOW */
}

void physical_LogicalToGlobalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect )
{
#if SINGLEWINDOW
    rectangle_OffsetRect(TempRect,
	   point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds),
	   point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds));
#else /* SINGLEWINDOW */
    rectangle_OffsetRect(TempRect,
	   -rectangle_Left(&CoordinateSystem->localBounds),
	   -rectangle_Top(&CoordinateSystem->localBounds));
#endif /* SINGLEWINDOW */
}

void physical_GlobalToLogicalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect )
{
#if SINGLEWINDOW
    rectangle_OffsetRect(TempRect,
	   -(point_X(&CoordinateSystem->physicalOrigin)-
	   rectangle_Left(&CoordinateSystem->localBounds)),
	   -(point_Y(&CoordinateSystem->physicalOrigin)-
	   rectangle_Top(&CoordinateSystem->localBounds)));
#else /* SINGLEWINDOW */
    rectangle_OffsetRect(TempRect,
	   rectangle_Left(&CoordinateSystem->localBounds),
	   rectangle_Top(&CoordinateSystem->localBounds));
#endif /* SINGLEWINDOW */
}

