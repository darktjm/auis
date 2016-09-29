#ifndef _be1be2_H_
#define _be1be2_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtext
 * @{ */
/*
 * BE1 to BE2 conversion package
 */

#include <ATK.H>

class be1be2 : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();




    /*
     * CheckBE1 uses heuristics to decide if a document
     * is BE1.  It is fairly accurate although it's impossible
     * to do this task with the preferred efficiently.
     */

    static boolean CheckBE1(class text *text)  ;

    /*
     * Convert usually adds many entries to text's stylesht.
     * Returns a boolean indication of success.  However, it
     * is too picky and usually does better than it reports.
     */

    static boolean Convert(class text *text)  ;

};
/** @} */
#endif /* _be1be2_H_ */
