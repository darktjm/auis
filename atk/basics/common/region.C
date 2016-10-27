/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

 
#include <andrewos.h>
ATK_IMPL("region.H")
#include <point.h>
#include <rect.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <region.H>



/************** methods *********************/



ATKdefineRegistryNoInit(region, ATK);

void region::XorRegion(const class region  * RHS, class region  * Result)
{
    XXorRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::UnionRegion(const class region  * RHS , class region  * Result )
{
    XUnionRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::IntersectRegion(const class region  * RHS, class region  * Result )
{
    XIntersectRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::SubtractRegion(const class region  * RHS, class region  * Result )
{
    XSubtractRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::ClearRegion()
{
    if(this->regionData) {
	XDestroyRegion(this->regionData);
    }
    this->regionData = XCreateRegion();
}

void region::RectRegion(const struct rectangle  * NewRegnRect)
{
    XPoint rectPoint[5];
    rectPoint[0].x = rectPoint[4].x = rectangle_Left(NewRegnRect);
    rectPoint[0].y = rectPoint[4].y = rectangle_Top(NewRegnRect);
    rectPoint[1].x = rectangle_Right(NewRegnRect);
    rectPoint[1].y = rectangle_Top(NewRegnRect);
    rectPoint[2].x = rectangle_Right(NewRegnRect);
    rectPoint[2].y = rectangle_Bottom(NewRegnRect);
    rectPoint[3].x = rectangle_Left(NewRegnRect);
    rectPoint[3].y = rectangle_Bottom(NewRegnRect);
    if (this->regionData) {
	XDestroyRegion(this->regionData);
    }
    this->regionData = XPolygonRegion(rectPoint, 5,  EvenOddRule);
}

class region * region::DuplicateRegion()
{
    class region * retValue;

    retValue = region::CreateEmptyRegion();
    (this)->UnionRegion(retValue,retValue);
    return retValue;

}

void region::OffsetRegion(long  DeltaX,long  DeltaY)
{
    XOffsetRegion(this->regionData,DeltaX,DeltaY);
}

void region::InsetRegion(long  DeltaX, long  DeltaY)
{
    XShrinkRegion(this->regionData,DeltaX,DeltaY);
}

boolean region::IsRegionEmpty() const
{
    return XEmptyRegion(this->regionData);
}

boolean region::IsPointInRegion(const struct point  * TestPt) const
{
    return XPointInRegion(this->regionData,point_X(TestPt), point_Y(TestPt));
}

boolean region::IsRectInRegion(const struct rectangle  * TestRect) const
{
    return XRectInRegion(this->regionData, rectangle_Left(TestRect), rectangle_Top(TestRect), rectangle_Width(TestRect), rectangle_Height(TestRect));
}

boolean region::AreRegionsEqual(const class region  * TestRegion) const
{
    return XEqualRegion(this->regionData, TestRegion->regionData);
}

struct rectangle *region::GetBoundingBox(struct rectangle  *retRect) const
{
    XRectangle rect;

    if (retRect != NULL)  {
	XClipBox(this->regionData, &rect);
	
	rectangle_SetRectSize(retRect, rect.x, rect.y,
			      rect.width, rect.height);
    }
    return retRect;
}


/****************** classprocedures *****************/

class region * region::CreateEmptyRegion()
{
    class region * retValue;
    retValue = new region;
    return retValue;
}

class region * region::CreateRectRegion(const struct rectangle  * RegionShape)
{
    class region * self;

    self = new region;
    (self)->RectRegion(RegionShape);
    return self;
}

class region * region::CreateOvalRegion(const struct rectangle  * RegionShape)
{
    return region::CreateRectRegion(RegionShape);
}

class region * region::CreatePolyRegion(const struct point  * PointArray,short  PointCount)
{
    XPoint * polyPts;
    int i;
    class region * retValue;

    retValue = new region;
    if(!retValue) {
	return NULL;
    }
    
    polyPts = (XPoint *) malloc(sizeof(XPoint) * (PointCount + 1));

    if(!polyPts) return NULL;
    
    for (i=0;i<PointCount;i++) {
	polyPts[i].x = point_X(&PointArray[i]);
        polyPts[i].y = point_Y(&PointArray[i]);
    }
    polyPts[PointCount] = polyPts[0];

    XDestroyRegion(retValue->regionData);
    retValue->regionData = XPolygonRegion(polyPts, PointCount+1, EvenOddRule);
    free(polyPts);
    return retValue;
}

void region::CopyRegion(class region  * Destination,const class region  * Source)
{
    if (!Destination) return;
    if (Destination->regionData) {
	XDestroyRegion(Destination->regionData);
    }
    Destination->regionData = XCreateRegion();
    XUnionRegion(Destination->regionData, Source->regionData, Destination->regionData);
}

/************* Predefines *************/

region::region()
{
    this->regionData = XCreateRegion();
    if(this->regionData) THROWONFAILURE( TRUE);
    else THROWONFAILURE( FALSE);
}

region::~region()
{
    if (this->regionData) {
	XDestroyRegion(this->regionData);
    }
}
