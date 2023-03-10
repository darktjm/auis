#ifndef _keymap_H_
#define _keymap_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* keymap.H -- A class that provides mappings of keys to procedures.
December, 1986 */

/* Number of entries in a keymap.  Should be power of two. */
#define keymap_MAXKEYS	256
/* Number of entries in a sparse map. Should be a multiple of 4 for good
 * structure packing. */
#define keymap_SPARSESIZE 16

#include <ATK.H>
/* These are the values that the return values from Lookup can take on. */

enum keymap_Types  {
    keymap_Empty,	/* no binding for this key */
    keymap_Proc,	/* binding is a proctable structure */
    keymap_Keymap	/* binding is another keymap */
};

struct keymap_sparsetable {
    int numValid;
    char keys[keymap_SPARSESIZE];
    enum keymap_Types types[keymap_SPARSESIZE];
    ATK  *objects[keymap_SPARSESIZE];
    long rocks[keymap_SPARSESIZE];
};    

struct keymap_fulltable {
    enum keymap_Types types[keymap_MAXKEYS];
    ATK  *objects[keymap_MAXKEYS];
    long rocks[keymap_MAXKEYS];
};


class keymap : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	keymap();
        keymap(const keymap &o);
	virtual ~keymap();

    virtual boolean BindToKey(const char *keys, const struct proctable_Entry *pe, long rock)	 ;
    virtual void RemoveBinding(const char *keys);
    virtual void InsertObject(long slot, ATK  *object, long rock, enum keymap_Types type);
    /* Lookup stuffs an object in the char * and returns an int that describes its type. */
    virtual enum keymap_Types Lookup(char key, ATK  **object, long *rockP)  ;

    union {
        struct keymap_sparsetable *sparse;
        struct keymap_fulltable *full;
    } table;
    boolean sparsep;
};
/** @} */
#endif /* _keymap_H_ */
