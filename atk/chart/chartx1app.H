ATK_INTER
#ifndef _chartx1app_H_
#define _chartx1app_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchart
 * @{ */
/*****************************************\

  Chart Programming Guide --  Example #1

\*****************************************/

#include <application.H>
class chartx1app : public  application 
  {
public:
	virtual ATKregistryEntry *ATKregistry();
	chartx1app();

  
    boolean Start()				     ;

  
    class chart			   *chart_data_object;
    class chartv			   *chart_view_object;
    class im				   *imp;
    class frame			   *framep;
  };
/** @} */
#endif /* _chartx1app_H_ */
