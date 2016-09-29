#ifndef _WSactions_H_
#define _WSactions_H_
/* ***************************************************
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'
 *  ***************************************************

	wsactions.H   -  actions for WidgetSelector

 *    $Log: wsactions.H,v $
 * Revision 1.1  1996/04/22  18:28:42  wjh
 * Initial revision
 *
 * Revision 1.1  1995/12/01   wjh
 * Initial revision
 */

#include <ATK.H>
class wsactions : public  ATK {
  public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	static void Init();	// put entries in proctable
};


#endif /* _WSactions_H_ */

