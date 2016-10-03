#ifndef _ltapp_H_
#define _ltapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblt
 * @{ */
#include <application.H>
class ltapp : public  application {
public:
  virtual ATKregistryEntry *ATKregistry();
 
  ltapp();
  
  boolean ParseArgs( int argc, const char **argv);
  boolean Start();

private:
  class frame		    	*framep;
  class im			*imp;
  class lt			*ltp;
  class ltv	    		*ltpview;
  char				 trace_stream_name[512];
  char				 trace_raster_name[512];

};
/** @} */
#endif /* _ltapp_H_ */
