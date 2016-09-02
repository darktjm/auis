ATK_INTER
#ifndef _region_H_
#define _region_H_
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


 



#define region_PROGRAMMERVERSION 2

#include <ATK.H>

class region : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();



    virtual void XorRegion(class region * RHS, class region * Result);
    virtual void UnionRegion(class region * RHS, class region * Result);
    virtual void IntersectRegion(class region * RHS, class region * Result);
    virtual void SubtractRegion(class region * RHS, class region * Result);
    virtual void ClearRegion();
    virtual void RectRegion(struct rectangle * NewRegnRect);
    virtual class region * DuplicateRegion()  ;
    virtual void OffsetRegion(long DeltaX, long DeltaY);
    virtual void InsetRegion(long DeltaX, long DeltaY);

    virtual boolean IsRegionEmpty()  ;
    virtual boolean IsPointInRegion(struct point * TestPt)  ;
    virtual boolean IsRectInRegion(struct rectangle * TestRect)  ;
    virtual boolean AreRegionsEqual(class region * TestRegion)  ;

    virtual struct rectangle * GetBoundingBox(struct rectangle *rect)  ;


    inline struct _XRegion * GetRegionData() { return ((this)->regionData)    /* Only used in the X code.  probably should subclass region to do this */; }


    static class region * CreateEmptyRegion()  ;
    static class region * CreateRectRegion(struct rectangle * RegionShape)  ;
    static class region * CreateOvalRegion(struct rectangle * RegionShape)  ;
    static class region * CreatePolyRegion(struct point * PointArray, short PointCount)  ;
    static void CopyRegion(class region * Destination, class region * Source);
    
	virtual ~region();
    
	region();

    struct _XRegion * regionData; /* really XRegion, but I don't want to include all of X just to get the typedef */
};


#endif /* _region_H_ */
