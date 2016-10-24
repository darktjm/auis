/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* Include file for cooridnate conversions */

class physical {
  public:
    static long LogicalPtToGlobalX(struct graphic  * CoordinateSystem,struct point  * LogicalPoint );
    static long LogicalPtToGlobalY(struct graphic  * CoordinateSystem,struct point  * LogicalPoint );
    static void LogicalPtToGlobalPt(struct graphic  * CoordinateSystem,struct point  * tempPoint );
    static void GlobalPtToLogicalPt(struct graphic  * CoordinateSystem,struct point  * tempPoint );
    static long LogicalXToGlobalX(struct graphic  * CoordinateSystem,long  LogicalX );
    static long LogicalYToGlobalY(struct graphic  * CoordinateSystem,long  LogicalY );
    static long GlobalXToLogicalX(struct graphic  * CoordinateSystem,long  PhysicalX );
    static long GlobalYToLogicalY(struct graphic  * CoordinateSystem,long  PhysicalY );
    static void LogicalToGlobalRect(struct graphic  * CoordinateSystem,struct rectangle  * TempRect );
    static void GlobalToLogicalRect(struct graphic  * CoordinateSystem,struct rectangle  * TempRect );
};
/** @} */
