#ifndef _butactions_H_
#define _butactions_H_
ATK_INTER

/*  Copyright Carnegie Mellon, 1995 - All Rights Reserved
 For full copyright information see:'andrew/doc/COPYRITE'

	butactions.H   -  actions for Button Widget

 *    $Log: butactions.H,v $
 * Revision 1.1  1996/04/02  17:43:01  wjh
 * Initial revision
 *
 * Revision 1.1  1995/12/01   wjh
 * Initial revision
 */

#include <ATK.H>
class butactions : public  ATK {
  public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	static void Init();	// put entries in proctable
};


#endif /* _butactions_H_ */

