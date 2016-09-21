ATK_INTER
#ifndef _texttagv_H_
#define _texttagv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <fnotev.H>

class texttagv: public  fnotev {
public:
	virtual ATKregistryEntry *ATKregistry();


     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);

    
	static boolean InitializeClass();

    
	texttagv();
};
/** @} */
#endif /* _texttagv_H_ */
