ATK_INTER
#ifndef _ghash_H_
#define _ghash_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/* A hash table
 *
 */

#define ghash_BUCKETS 32

#include <ATK.H>


	typedef boolean (*ghash_efptr)(long rock, char *value, const char *key, class ghash *self);
	typedef void (*ghash_freevalfptr)(char *value);
	typedef void (*ghash_freekeyfptr)(char *key);
	typedef char *(*ghash_copyvalfptr)(const char *value);
	typedef char *(*ghash_copykeyfptr)(const char *key);
	typedef int (*ghash_comparekeyfptr)(const char *key1, const char *key2);
	typedef int (*ghash_hashfptr)(const char *key);

class ghash : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
	ghash();
    
	virtual ~ghash();
    
	static boolean InitializeClass();


    virtual char * Lookup(const char *key)  ;
    virtual const char * LookupKey(const char *key)  ;
    virtual boolean Store(const char *key, const char *value)  ;
    virtual char * Delete(const char *key)  ;
    virtual char * Rename(const char *key, char *new_c)  ;
    virtual void Clear();
    virtual char * Enumerate(ghash_efptr proc, long rock)  ;
    virtual void Debug();

    inline void SetHash(ghash_hashfptr fn) { (this->hash = (fn)); }
    inline void SetFreeVal(ghash_freevalfptr fn) { (this->freeVal = (fn)); }
    inline void SetFreeKey(ghash_freekeyfptr fn) { (this->freeKey = (fn)); }
    inline void SetCopyKey(ghash_copykeyfptr fn) { (this->copyKey = (fn)); }
    inline void SetCopyVal(ghash_copyvalfptr fn) { (this->copyVal = (fn)); }
    inline void SetCompareKey(ghash_comparekeyfptr fn) { (this->compKey = (fn)); }
    inline ghash_freevalfptr   GetFreeVal() { return (this->freeVal); }
    inline ghash_freekeyfptr   GetFreeKey() { return (this->freeKey); }
    inline ghash_copykeyfptr   GetCopyKey() { return (this->copyKey); }
    inline ghash_copyvalfptr   GetCopyVal() { return (this->copyVal); }
    inline ghash_comparekeyfptr   GetCompareKey() { return (this->compKey); }


    class glist *buckets[ghash_BUCKETS];
    ghash_hashfptr hash;
    ghash_freevalfptr freeVal;
    ghash_freekeyfptr freeKey;
    ghash_copyvalfptr copyVal;
    ghash_copykeyfptr copyKey;
    ghash_comparekeyfptr compKey;
};

#endif /* _ghash_H_ */
