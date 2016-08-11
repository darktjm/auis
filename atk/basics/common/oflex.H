#ifndef _OFLEX_H
#define _OFLEX_H
/* Copyright 1995 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */
#include <new.h>
#include <mflex.H>

class oflex : public mflex {
  protected:
    virtual void MemCpy(char *dest, char *src, size_t bytelength);

    virtual void MemMove(char *dest, char *src, size_t bytelength);

    virtual void DestroyElement(void *src);

    virtual void ConstructElement(void *src);

    virtual void CopyElement(void *dest, void *src);

    virtual void Invalidate(size_t i, size_t len);
    
  public:
    
    oflex(size_t typesize=1, size_t n=5);
    oflex(const oflex &src);
    
    virtual ~oflex();
    char *Insert(size_t i, size_t len=1);

    inline char *Append(size_t len=1) {
	return Insert(GetN(), len);
    }

};


#define DEFINE_OFLEX_CLASS(name,type,minsize) \
class name : public oflex { \
  public: \
    inline name(size_t n=minsize) : oflex(sizeof(type), n) { \
    } \
inline name(const name &src) : oflex(sizeof(type), src.GetN()) { \
if(src.GetN()>0) { \
type *pelts=Insert(0, src.GetN()); \
MemCpy((char *)pelts, (char *)&src[0], typesize*src.GetN()); } \
    } \
inline virtual ~name() { \
for(size_t i=0;i<GetN();i++) DestroyElement(&((*this)[i])); \
    } \
inline const type &operator[](size_t i) const{ \
	return *(type *)&(*(oflex *)this)[i]; \
    } \
inline type &operator[](size_t i) { \
	return *(type *)&(*(oflex *)this)[i]; \
    } \
    inline type *Insert(size_t i, size_t len=1) { \
	return (type *)oflex::Insert(i,len); \
    } \
    inline type *Append(size_t len=1) { \
	return (type *)oflex::Insert(GetN(),len); \
    } \
inline type *Insert(size_t i, const type &o) { \
void *res=mflex::Insert(i,1); \
type *res2=new(res) type(o); \
return (type *)res; \
} \
inline type *Insert(size_t i, const type *o) { \
return Insert(i,*o); \
} \
inline type *Append(const type &o) { \
return Insert(GetN(),o); \
} \
inline type *Append(const type *o) { \
    return Append(*o); \
} \
    inline size_t Find(const type &o) { \
	return oflex::Find((const char *)&o); \
    } \
inline size_t Find( const type *o ) {return Find( *o );}; \
    inline void Remove(size_t i, size_t len=1) { \
	oflex::Remove(i, len); \
    } \
    inline void Remove(const type *o, size_t len=1) { \
	oflex::Remove((char *)o, len); \
    } \
    inline type *GetBuf(size_t i, size_t len, size_t *gotlenp) { \
	return (type *)oflex::GetBuf(i, len, gotlenp); \
    } \
inline virtual void ConstructElement(void *src) { \
type *res=new((void *)src) type(); } \
inline virtual void DestroyElement(void *src) { \
((type *)src)->~type(); } \
inline virtual void CopyElement(void *dest, void *src) { \
new(dest) type(*(const type *)src); } \
}

#define DEFINE_OFLEX_ITER(name,type) \
class Concat(name,_iter) { \
    size_t i, n, ci; \
    name &obj; \
  public: \
    inline Concat(name,_iter)(name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na<=0) n=o->GetN(); \
	else n=na; \
    } \
    inline Concat(name,_iter)(name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na<=0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    inline Concat(name,_iter) &first() { \
	ci=i; \
	return *this; \
    } \
    inline Concat(name,_iter) &last() { \
	ci=i+n-1; \
	return *this; \
    } \
    inline type *operator->() { \
	return &obj[ci]; \
    } \
    inline type &operator*() { \
	return obj[ci]; \
    } \
    inline Concat(name,_iter) &operator++(int) { \
	ci++; \
	return *this; \
    } \
    inline Concat(name,_iter) &operator--(int) { \
	ci--; \
	return *this; \
 \
    } \
    inline boolean done() { \
	if(ci>=i+n || ci<0) return TRUE; \
	else return FALSE; \
    } \
}

#define DEFINE_OFLEX_CONST_ITER(name,type) \
class Concat(name,_citer) { \
    size_t i, n, ci; \
    const name &obj; \
  public: \
    inline Concat(name,_citer)(const name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na<=0) n=o->GetN(); \
	else n=na; \
    } \
    inline Concat(name,_citer)(const name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na<=0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    inline Concat(name,_citer) &first() { \
	ci=i; \
	return *this; \
    } \
    inline Concat(name,_citer) &last() { \
	ci=i+n-1; \
	return *this; \
    } \
    inline const type *operator->() { \
	return &obj[ci]; \
    } \
    inline const type &operator*() { \
	return obj[ci]; \
    } \
    inline Concat(name,_citer) &operator++(int) { \
	ci++; \
	return *this; \
    } \
    inline Concat(name,_citer) &operator--(int) { \
	ci--; \
	return *this; \
 \
    } \
    inline boolean done() { \
	if(ci>=i+n || ci<0) return TRUE; \
	else return FALSE; \
    } \
}
	
#endif

