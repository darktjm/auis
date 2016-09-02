#ifndef _options_H_
#define _options_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <ATK.H>
class options : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      static void SetMessagesOptions(class t822view *bv)  ;
};

#endif /* _options_H_ */
