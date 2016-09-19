/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
