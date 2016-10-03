#ifndef _sched_H_
#define _sched_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsched Object sched
 *  A dynamically loadable ATK class file.  Contains sched, schedv, schedapp.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched Data-object Specification

MODULE	sched.ch

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable Data-object.

HISTORY
  01/20/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  sched_VERSION    1

#include <dataobject.H>
class sched : public  dataobject {
public:
  virtual ATKregistryEntry *ATKregistry();
  virtual void Set_Debug( boolean debug );
  static class sched *Create( char *stream_name );
  sched();

  class zip			     *zipp;
  struct zip_stream		     *stream;
  char				      stream_name[512];
  long				      id;
};

/** @} */
#endif /* _sched_H_ */
