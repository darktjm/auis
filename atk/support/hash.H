ATK_INTER
#ifndef _hash_H_
#define _hash_H_
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

#define hash_BUCKETS 32

#include <ATK.H>
#include <glist.H>
	typedef void (*hash_freefptr)(char *val);
	typedef int (*hash_hashfptr)(char *n);

class hash : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	hash();
    
	virtual ~hash();
    
	static boolean InitializeClass();


    virtual char * Lookup(char *key)  ;
    virtual void Store(char *key, char *value);
    virtual char * Delete(char *key)  ;
    virtual char * Rename(char *key, char *new_c)  ;
    virtual void Clear(hash_freefptr valFree);
    virtual void Debug();

    inline void SetHash(hash_hashfptr fn) { (this->hashfunc = (fn)); }

    class glist *buckets[hash_BUCKETS];
    hash_hashfptr hashfunc;
};

#endif /* _hash_H_ */
