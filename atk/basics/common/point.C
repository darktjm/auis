/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <point.h>

/* These have been macrofied */

/*
long point_X(Pt)
struct point * Pt; {
    return Pt->x;
}

long point_Y(Pt)
struct point * Pt; {
    return Pt->y;
}
*/

void point_SetX(struct point  * Pt,long  Value );
void point_SetY(struct point  * Pt,long  Value );
void point_OffsetPoint(struct point  * Pt,long  DeltaX , long  DeltaY);
void point_AddPt(struct point  * LHS,struct point  * RHS);
void point_SubPt(struct point  * LHS,struct point  * RHS );
boolean point_ArePtsEqual(struct point  * LHS,struct point  * RHS );
struct point * point_CreatePoint(long  InitX , long  InitY);


void point_SetX(struct point  * Pt,long  Value )
{
    Pt->x=Value;
}

void point_SetY(struct point  * Pt,long  Value )
{
     Pt->y=Value;
}

void point_OffsetPoint(struct point  * Pt,long  DeltaX , long  DeltaY)
{
    Pt->y += DeltaY;
    Pt->x += DeltaX;
}

void point_AddPt(struct point  * LHS,struct point  * RHS)
{
    LHS->y += RHS->y;
    LHS->x += RHS->x;
}

void point_SubPt(struct point  * LHS,struct point  * RHS )
{
    LHS->y -= RHS->y;
    LHS->x -= RHS->x;
}

/*
 More macrofied
*/
/*

void point_SetPt(Pt,NewX, NewY)
struct point * Pt;
long NewY;
long NewX;{
    Pt->y = NewY;
    Pt->x = NewX;
}

*/

boolean point_ArePtsEqual(struct point  * LHS,struct point  * RHS )
{
    return (LHS->x == RHS->x) && (LHS->y == RHS->y);
}

struct point * point_CreatePoint(long  InitX , long  InitY)
{
    struct point * RetValue;

    RetValue = (struct point *) malloc(sizeof(struct point));
    point_SetPt(RetValue,InitX,InitY);
    return RetValue;
}
