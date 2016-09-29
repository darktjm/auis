#ifndef _typetext_H_
#define _typetext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtypescript
 * @{ */
/* typetext.H
 * Class header for a basic fenced view.
 *
 */

#include <text.H>
class typetext: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();
	typetext();


     long HandleBegindata(long pos,FILE *file)  ;
     long HandleKeyWord(long pos, char *keyword, FILE *file)  ;
     long HandleCloseBrace(long pos, FILE *file)  ;
     const char * ViewName()  ;
     long GetModified()  ;

int hashandler;
};
/** @} */
#endif /* _typetext_H_ */
