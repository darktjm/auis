#ifndef _lt_H_
#define _lt_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblt Object lt
 *  A dynamically loadable ATK class file.  Contains ltapp, lt, ltv.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The LightTable Data-object Specification

MODULE	lt.H

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable Data-object.

HISTORY
  10/10/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  lt_VERSION    1

#include <dataobject.H>
class lt : public  dataobject  {
public:
  virtual ATKregistryEntry *ATKregistry();
  virtual void Set_Debug( boolean debug );
  virtual long Read_Visuals( char *foreground, char *background );
 
  lt();

private:
  friend class ltv; /* ick */
  friend class ltv_private; /* double ick */
  class zip			     *zipp;
  struct zip_stream		     *foreground_stream;
  char				      foreground_stream_name[512];
  class raster			     *rasterp;
  char				      background_raster_name[512];
  long				      background_width, background_height;
  long				      id;
};

/** @} */
#endif /* _lt_H_ */
