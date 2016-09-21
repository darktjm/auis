ATK_INTER
#ifndef _bind_H_
#define _bind_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>
#include <proctable.H>

struct bind_Description {
    const char *procName;
    const char *keyVector;
    long    keyRock;
    const char *menuEntry;
    long    menuRock;
    long    menuMask;
    proctable_fptr proc;
    const char *doc;
    const char *module;
};

struct bind_ActionDescription {
    const char *procName;
    const char *keyVector;
    char	    *keyRock;
    const char *menuEntry;
    char	    *menuRock;
    long    menuMask;
    aaction *act;
    const char    *doc;
    const char    *module;
};
class bind : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void BindList(const struct bind_Description *bl, class keymap *km, class menulist *ml, const struct ATKregistryEntry  *type);
    static void BindList(const struct bind_ActionDescription *bl, class keymap *km, class menulist *ml, const struct ATKregistryEntry  *type);
};
/** @} */
#endif /* _bind_H_ */
