#ifndef _region_H_
#define _region_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#define region_PROGRAMMERVERSION 2

#include <ATK.H>

class region : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();



    virtual void XorRegion(const class region * RHS, class region * Result);
    virtual void UnionRegion(const class region * RHS, class region * Result);
    virtual void IntersectRegion(const class region * RHS, class region * Result);
    virtual void SubtractRegion(const class region * RHS, class region * Result);
    virtual void ClearRegion();
    virtual void RectRegion(const struct rectangle * NewRegnRect);
    virtual class region * DuplicateRegion()  ;
    virtual void OffsetRegion(long DeltaX, long DeltaY);
    virtual void InsetRegion(long DeltaX, long DeltaY);

    virtual boolean IsRegionEmpty() const  ;
    virtual boolean IsPointInRegion(const struct point * TestPt) const  ;
    virtual boolean IsRectInRegion(const struct rectangle * TestRect) const  ;
    virtual boolean AreRegionsEqual(const class region * TestRegion) const  ;

    virtual struct rectangle * GetBoundingBox(struct rectangle *rect) const  ;


    inline struct _XRegion * GetRegionData() { return ((this)->regionData)    /* Only used in the X code.  probably should subclass region to do this */; }


    static class region * CreateEmptyRegion()  ;
    static class region * CreateRectRegion(const struct rectangle * RegionShape)  ;
    static class region * CreateOvalRegion(const struct rectangle * RegionShape)  ;
    static class region * CreatePolyRegion(const struct point * PointArray, short PointCount)  ;
    static void CopyRegion(class region * Destination, const class region * Source);
    
	virtual ~region();
    
	region();

    struct _XRegion * regionData; /* really XRegion, but I don't want to include all of X just to get the typedef */
};
/** @} */
#endif /* _region_H_ */
