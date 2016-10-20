#ifndef _ltapp_H_
#define _ltapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblt
 * @{ */
#include <application.H>
/** An application for managing LightTable documents.
 *  This application is required to start the \ref lt inset, since
 *  that requires an API call to initialize.  The application takes two
 *  primary arguments: the name of a zip file, followed by the name of
 *  a raster file.  If no file names are given, they are prompted for
 *  when a button is first pressed, if you manage to get that far without
 *  crashing.  Since it's not documented at all, and may well need a
 *  template file to operate (not supplied), I have no idea what this is
 *  supposed to do or what a "LightTable" is.  Maybe some time when I
 *  have more patience I'll try to analyze what the undocumented, poorly
 *  formatted, probably incorrect given the number bugs I've already found
 *  code does.
 */
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
