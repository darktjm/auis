/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* This is a utility package only to be used by im and graphic (and
 perhaps other machine dependent modules that I do not yet know about,
 such as the cursor system. This package provides a way to convert
 between the coordinate spaces of a graphic and wm "hardware" */

#include <andrewos.h>

#include <graphic.H>
#include <physical.h>

#define SINGLEWINDOW 1

long physical::LogicalPtToGlobalX(class graphic  * CoordinateSystem,struct point  * LogicalPoint )
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

long physical::LogicalPtToGlobalY(class graphic  * CoordinateSystem,struct point  * LogicalPoint )
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

void physical::LogicalPtToGlobalPt(class graphic  * CoordinateSystem,struct point  * tempPoint )
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

void physical::GlobalPtToLogicalPt(class graphic  * CoordinateSystem,struct point  * tempPoint )
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



long physical::LogicalXToGlobalX(class graphic  * CoordinateSystem,long  LogicalX )
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

long physical::LogicalYToGlobalY(class graphic  * CoordinateSystem,long  LogicalY )
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

long physical::GlobalXToLogicalX(class graphic  * CoordinateSystem,long  PhysicalX )
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

long physical::GlobalYToLogicalY(class graphic  * CoordinateSystem,long  PhysicalY )
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

void physical::LogicalToGlobalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect )
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

void physical::GlobalToLogicalRect(class graphic  * CoordinateSystem,struct rectangle  * TempRect )
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

