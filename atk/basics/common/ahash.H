#ifndef _ahash_H_
#define _ahash_H_
ATK_INTER
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1996 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <mflex.H>
#include <oflex.H>
#include <util.h>


class AHashStringKey {
    size_t len;
    const char *key;
  public:
    inline AHashStringKey(const char *k) {
	key=NewString((char *)k);
	if(key) len=strlen(k);
	else len=0;
    }
    inline AHashStringKey() {
	key=NULL;
	len=0;
    }
    inline AHashStringKey(const AHashStringKey &src) {
	if(src.key) {
	    key=NewString((char *)src.key);
	    if(key) len=strlen(key);
	    else len=0;
	} else {
	    key=NULL;
	    len=0;
	}
    }
    inline ~AHashStringKey() {
	if(key) free((char *)key);
    }
    inline const char *CStr() {
	return key;
    }
    inline AHashStringKey &operator=(const AHashStringKey &src) {
	if(key) free((char *)key);
	if(src.key) {
	    key=NewString((char *)src.key);
	    if(key) len=strlen(key);
	    else len=0;
	} else {
	    key=NULL;
	    len=0;
	}
	return *this;
    }
    inline boolean operator==(const AHashStringKey &rhs) const {
	return (rhs.key && key)?(strcmp(rhs.key, key)==0):(rhs.key==key);
    }
    inline size_t Hash() const {
	return len? (((*(key)<<2)+((key)[(len)>>1])+((key)[(len)-1]<<2)+((key)[len>>2]<<2)+(len))):0;
    }
    inline const char *Key() {
	return key;
    }
};

class AHashLongKey {
    unsigned long key;
  public:
    inline AHashLongKey(unsigned long k) {
	key=k;
    }
    inline AHashLongKey() {
	key=0;
    }
    inline AHashLongKey(const AHashLongKey &src) {
	key=src.key;
    }
    inline ~AHashLongKey() {}
    inline AHashLongKey &operator=(const AHashLongKey &src) {
	key=src.key;
	return *this;
    }
    inline boolean operator==(const AHashLongKey &rhs) const {
	return key==rhs.key;
    }
    inline size_t Hash() const {
	return key;
    }
    inline unsigned long Key() {
	return key;
    }
};


class AHash {
  protected:
    size_t maxhash;
    size_t n;
  public:
    inline AHash(size_t max=57) {
	maxhash=max;
	n=0;
    }
    inline size_t MaxHash() {
	return maxhash;
    }
    virtual ~AHash();
    inline size_t GetN() { return n;}
    // oh the things we do because of broken compilers...
    // the below is broken out this way because of bugs in HP C++
    // (no while statements in inline functions), and in g++
    // forward static declarations don't work... -Rob Ryan
    // robr@cmu.edu (1/22/96)
    virtual size_t BucketN(size_t i);
    // gets the number of elements in the ith bucket.
    size_t FirstBucket(size_t i);
    // Returns the first bucket at or after the ith which
    // has at least one element.
};

#define DEFINE_AMHASH_CLASS(name,keytype,type) \
DEFINE_MFLEX_CLASS(Concat(name,_flex1),type,0); \
DEFINE_OFLEX_CLASS(Concat(name,_flex2),keytype,0); \
class name : public AHash { \
    Concat(name,_flex1) *buckets; \
    Concat(name,_flex2) *keys; \
  public: \
    name(size_t max=57): AHash(max) { \
	buckets=new Concat(name,_flex1)[maxhash]; \
	keys=new Concat(name,_flex2)[maxhash]; \
    } \
virtual ~name() { \
delete [] buckets; \
delete [] keys; \
} \
inline virtual size_t BucketN(size_t i) { \
return buckets[i].GetN(); \
} \
Concat(name,_flex2) *Keys() { \
return keys; \
} \
Concat(name,_flex1) *Buckets() { \
return buckets; \
} \
    type *Insert(const keytype &key, const type &item) { \
	size_t hash=key.Hash()%maxhash; \
	type *result=buckets[hash].Append(); \
	*result=item; \
	keytype *result2=keys[hash].Append(); \
*result2=key; \
n++; \
return result; \
    } \
type *Insert(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	type *result=buckets[hash].Append(); \
	keytype *result2=keys[hash].Append(); \
*result2=key; \
n++; \
return result; \
    } \
    type *Fetch(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	Concat(name,_flex2) &lkeys=keys[hash]; \
	size_t i, n=lkeys.GetN(); \
	for(i=0;i<n;i++) { \
	    if(lkeys[i]==key) { \
		return &buckets[hash][i]; \
	    } \
	} \
         return NULL; \
    } \
    void Remove(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	Concat(name,_flex2) &lkeys=keys[hash]; \
	size_t i=lkeys.GetN(); \
while(i>0) { \
i--; \
if(lkeys[i]==key) { \
n--; \
		lkeys.Remove((size_t)i); \
		  buckets[hash].Remove((size_t)i); \
		    break; \
	    } \
	} \
    } \
};

#define DEFINE_AOHASH_CLASS(name,keytype,type) \
DEFINE_OFLEX_CLASS(Concat(name,_flex1),type,0); \
DEFINE_OFLEX_CLASS(Concat(name,_flex2),keytype,0); \
class name : public AHash { \
    Concat(name,_flex1) *buckets; \
    Concat(name,_flex2) *keys; \
  public: \
    name(size_t max=57): AHash(max) { \
	buckets=new Concat(name,_flex1)[maxhash]; \
	keys=new Concat(name,_flex2)[maxhash]; \
    } \
virtual ~name() { \
delete [] buckets; \
delete [] keys; \
    } \
inline virtual size_t BucketN(size_t i) { \
return buckets[i].GetN(); \
} \
Concat(name,_flex2) *Keys() { \
return keys; \
} \
Concat(name,_flex1) *Buckets() { \
return buckets; \
} \
    type *Insert(const keytype &key, const type &item) { \
	size_t hash=key.Hash()%maxhash; \
	type *result=buckets[hash].Append(); \
	*result=item; \
	keytype *result2=keys[hash].Append(); \
*result2=key; \
n++; \
return result; \
    } \
type *Insert(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	type *result=buckets[hash].Append(); \
	keytype *result2=keys[hash].Append(); \
*result2=key; \
n++; \
return result; \
    } \
    type *Fetch(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	Concat(name,_flex2) &lkeys=keys[hash]; \
	size_t i, n=lkeys.GetN(); \
	for(i=0;i<n;i++) { \
	    if(lkeys[i]==key) { \
		return &buckets[hash][i]; \
	    } \
	} \
	return NULL; \
    } \
     void Remove(const keytype &key) { \
	size_t hash=key.Hash()%maxhash; \
	Concat(name,_flex2) &lkeys=keys[hash]; \
	size_t i=lkeys.GetN(); \
while(i>0) { \
i--; \
if(lkeys[i]==key) { \
n--; \
		lkeys.Remove((size_t)i); \
		  buckets[hash].Remove((size_t)i); \
		    break; \
	    } \
	} \
    } \
};

#define DEFINE_AHASH_CLASS_ITER(name,keytype,type) \
class Concat(name,_iter) { \
public: \
size_t i, j; \
name &obj; \
Concat(name,_flex2) *keys; \
Concat(name,_flex1) *buckets; \
  inline Concat(name,_iter)(name *o) : obj(*o) { \
      keys=obj.Keys(); \
buckets=obj.Buckets(); \
i=obj.FirstBucket(0); \
if(i<obj.MaxHash()) j=buckets[i].GetN()-1; \
  } \
  inline Concat(name,_iter)(name &o) : obj(o) { \
      keys=obj.Keys(); \
buckets=obj.Buckets(); \
i=obj.FirstBucket(0); \
j=-1; \
if(i<obj.MaxHash()) j=buckets[i].GetN()-1; \
  } \
  inline type &operator*() { \
      return buckets[i][j]; \
  } \
  inline keytype *KeyPtr() { \
      return &(keys[i][j]); \
  } \
  inline keytype &Key() { \
      return keys[i][j]; \
  } \
  inline Concat(name,_iter) &operator++() { \
if(j>0) j--; \
else { \
i=obj.FirstBucket(i+1); \
if(i<obj.MaxHash()) j=buckets[i].GetN()-1; \
} \
return *this; \
  } \
  inline boolean done() const { \
      if(i>=obj.MaxHash()) return TRUE; \
      else return FALSE; \
  } \
}
#endif
