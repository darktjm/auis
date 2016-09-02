/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


 



#ifndef POINT_DEFINED

#define POINT_DEFINED



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
#endif /* POINT_DEFINED */

ENDCPLUSPLUSPROTOS

