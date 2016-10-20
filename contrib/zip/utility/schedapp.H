#ifndef _schedapp_H_
#define _schedapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsched
 * @{ */
#include <application.H>

/** An application for managing schedule documents.
 *  This application is required to start the \ref sched inset, since
 *  that requires an API call to initialize.  The application takes one
 *  primary argument: the name of the schedule file to load.  If that
 *  argument is missing, the sample schedule is loaded.  However, given
 *  that the sample is read-only, and the Save button doesn't allow
 *  changing the file name, and a new document can't be created, this
 *  is mostly useless.
 */
class schedapp : public  application {
public:

  virtual ATKregistryEntry *ATKregistry();
  schedapp();
  boolean ParseArgs( int argc, const char **argv);
  boolean Start();
  int Run();
  
  class frame		    	*framep;
  class im			*imp;
  class sched			*schedp;
  class schedv			*schedpview;
  char				 stream_name[512];

};
/** @} */
#endif /* _schedapp_H_ */
