ATK_INTER
#ifndef _dictionary_H_
#define _dictionary_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
#include <ATK.H>

class dictionary : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void Insert(class view *view,char *id, char *object) ;
    static char * LookUp(class view *view,char *id)  ;
    static long CountViews(char *id)  ;
    static long ListViews(char *id,class view **viewarray,long len)  ;
    static long CountRefs(class view *view)  ;
    static long ListRefs(class view *view,char **refarray,long len)  ;
    static void Delete(class view *view,char *id);
    
	static boolean InitializeClass();

};
/** @} */
#endif /* _dictionary_H_ */
