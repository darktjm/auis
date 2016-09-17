/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


 

/* Include file for cooridnate conversions */

long physical_LogicalPtToGlobalX(struct graphic  * CoordinateSystem,struct point  * LogicalPoint );
long physical_LogicalPtToGlobalY(struct graphic  * CoordinateSystem,struct point  * LogicalPoint );
void physical_LogicalPtToGlobalPt(struct graphic  * CoordinateSystem,struct point  * tempPoint );
void physical_GlobalPtToLogicalPt(struct graphic  * CoordinateSystem,struct point  * tempPoint );
long physical_LogicalXToGlobalX(struct graphic  * CoordinateSystem,long  LogicalX );
long physical_LogicalYToGlobalY(struct graphic  * CoordinateSystem,long  LogicalY );
long physical_GlobalXToLogicalX(struct graphic  * CoordinateSystem,long  PhysicalX );
long physical_GlobalYToLogicalY(struct graphic  * CoordinateSystem,long  PhysicalY );
void physical_LogicalToGlobalRect(struct graphic  * CoordinateSystem,struct rectangle  * TempRect );
void physical_GlobalToLogicalRect(struct graphic  * CoordinateSystem,struct rectangle  * TempRect );
