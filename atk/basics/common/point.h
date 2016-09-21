/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#ifndef POINT_DEFINED

#define POINT_DEFINED

/** \addtogroup libbasics
 * @{ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS


struct point {
    long y,x;
};

#define point_X(thePoint) ((thePoint)->x)
#define point_Y(thePoint) ((thePoint)->y)
/* original unmacroified versions */
/*
long point_X();
long point_Y();
*/
void point_SetX(struct point  * Pt,long  Value );
void point_SetY(struct point  * Pt,long  Value );

void point_OffsetPoint(struct point  * Pt,long  DeltaX , long  DeltaY);
void point_AddPt(struct point  * LHS,struct point  * RHS);
void point_SubPt(struct point  * LHS,struct point  * RHS );
/*void point_SetPt(LHS, NewX, NewY); */
#define point_SetPt(PtToSet,NewX, NewY) \
(    ((PtToSet)->y = (NewY)), \
     ((PtToSet)->x = (NewX)) )

boolean point_ArePtsEqual(struct point  * LHS,struct point  * RHS );
struct point * point_CreatePoint(long  InitX , long  InitY);

ENDCPLUSPLUSPROTOS
/** @} */
#endif /* POINT_DEFINED */
