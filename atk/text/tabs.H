#ifndef _tabs_H_
#define _tabs_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
/* Nick Williams, August 1990 */

/* tabs are a package. 
 * However, we want to be able to define methods and data,
 * so we have it as a basic class.
 */

#include <ATK.H>
#include <style.H>

class tabs : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	tabs();
    
	virtual ~tabs();
    static class tabs * Create()  ;
    static void Death(class tabs *self);

    virtual void OutputTroff(long indent, FILE *file);
    virtual int Different(class tabs *b)  ;
    virtual class tabs * Delete(int n)  ;
    virtual class tabs * Add(long pos, enum style_TabAlignment op)  ;
    virtual class tabs * Clear()  ;
    virtual class tabs * ApplyChange(struct tabentry *change)  ;
    virtual class tabs * Copy()  ;

    long *Positions;
    long *Types;
    int number;
    int links;
};
/** @} */
#endif /* _tabs_H_ */
