#ifndef _schedapp_H_
#define _schedapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsched
 * @{ */
#include <application.H>
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
