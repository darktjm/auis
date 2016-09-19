ATK_INTER
#ifndef _hash_H_
#define _hash_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* A hash table
 *
 */

#define hash_BUCKETS 32

#include <ATK.H>
#include <glist.H>
	typedef void (*hash_freefptr)(char *val);
	typedef int (*hash_hashfptr)(const char *n);

class hash : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	hash();
    
	virtual ~hash();
    
	static boolean InitializeClass();


    virtual const char * Lookup(const char *key)  ;
    virtual void Store(const char *key, const char *value);
    virtual const char * Delete(const char *key)  ;
    virtual const char * Rename(const char *key, const char *new_c)  ;
    virtual void Clear(hash_freefptr valFree);
    virtual void Debug();

    inline void SetHash(hash_hashfptr fn) { (this->hashfunc = (fn)); }

    class glist *buckets[hash_BUCKETS];
    hash_hashfptr hashfunc;
};

#endif /* _hash_H_ */
