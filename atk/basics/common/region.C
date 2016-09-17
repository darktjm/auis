/*LIBS: -lX11
*/

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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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



ATKdefineRegistry(region, ATK, NULL);

void region::XorRegion(class region  * RHS, class region  * Result)
{
    XXorRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::UnionRegion(class region  * RHS , class region  * Result )
{
    XUnionRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::IntersectRegion(class region  * RHS, class region  * Result )
{
    XIntersectRegion(this->regionData,RHS->regionData,Result->regionData);
}

void region::SubtractRegion(class region  * RHS, class region  * Result )
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

void region::RectRegion(struct rectangle  * NewRegnRect)
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

boolean region::IsRegionEmpty()
{
    return XEmptyRegion(this->regionData);
}

boolean region::IsPointInRegion(struct point  * TestPt)
{
    return XPointInRegion(this->regionData,point_X(TestPt), point_Y(TestPt));
}

boolean region::IsRectInRegion(struct rectangle  * TestRect)
{
    return XRectInRegion(this->regionData, rectangle_Left(TestRect), rectangle_Top(TestRect), rectangle_Width(TestRect), rectangle_Height(TestRect));
}

boolean region::AreRegionsEqual(class region  * TestRegion)
{
    return XEqualRegion(this->regionData, TestRegion->regionData);
}

struct rectangle *region::GetBoundingBox(struct rectangle  *retRect)
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

class region * region::CreateRectRegion(struct rectangle  * RegionShape)
{
    class region * self;

    self = new region;
    (self)->RectRegion(RegionShape);
    return self;
}

class region * region::CreateOvalRegion(struct rectangle  * RegionShape)
{
    return region::CreateRectRegion(RegionShape);
}

class region * region::CreatePolyRegion(struct point  * PointArray,short  PointCount)
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

void region::CopyRegion(class region  * Destination,class region  * Source)
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
