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
/** A data object for storing a schedule document.
 *  This is basically a dummy wrapper for a zip document which requires
 *  an API call to properly initialize (thus requiring use of the \ref
 *  schedapp application, even though neither the app nor the data object
 *  do anything special).  Its primary viewer is \ref schedv.
 */
class sched : public  dataobject {
public:
  TRACED_CLASS_NODEST(sched);
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
