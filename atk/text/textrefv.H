#ifndef _textrefv_H_
#define _textrefv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtext
 * @{ */
#include <fnotev.H>

class textrefv: public  fnotev {
public:
	TRACED_CLASS_NODEST(textrefv);


     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);

    
	static boolean InitializeClass();

    
	textrefv();

    char *cref;
    long loc;
};
/** @} */
#endif /* _textrefv_H_ */
