ATK_INTER
#ifndef _strcache_H_
#define _strcache_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
/* A string cache
 *
 */

#include <ghash.H>

class strcache : public  ghash {
public:
	virtual ATKregistryEntry *ATKregistry();



    
	strcache();
    
	virtual ~strcache();
    
	static boolean InitializeClass();

    static const char * SaveStr(const char *str)  ;

     char * Delete(char *key)  ;
     char * Rename(char *key, char *new_c)  ;

};
/** @} */
#endif /* _strcache_H_ */
