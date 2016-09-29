#ifndef _AVALUE_H
#define _AVALUE_H 1
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
#include <andrewos.h>

#include <atom.H>
#include <oflex.H>

class avalueflex;
union avalue_u {
    long integer;
    double real;
    const char *cstr;
    void *obj;
};

static class avalue_init {
  public:
    avalue_init();
} avalue_init_foo;

typedef void (*avalue_destroyit)(void *);
class avalue {

    avalue_u val;
    const atom *type;
    const atom *name;
  public:
    // these allow easy reference to the built in types, for example:
    // if(a.Type()==avalue::real) dosomethingwith(a.Real());
    static const atom *integer;
    static const atom *real;
    static const atom *cstring;
    static const atom *voidptr;
    static const atom *valueflex;
    static const atom *none;
    static const atom *voidatom;
    static const atom *atomatom;
    static const atom *atkatom;
    avalue();
    avalue(const avalue &src);
    avalue &operator=(const avalue &src);
    ~avalue();
    // Accessor functions for the various types.  If the type given doesn't match the actual type 0 will be returned.  If no type is given the effect is as if the type indicated by the function name had been given.  The pointers returned are owned by the object which set the avalue.
    long Integer(const atom *type=NULL) const;
    double Real(const atom *type=NULL) const;
    const char *CString(const atom *type=NULL) const;
    void *VoidPtr(const atom *type=NULL) const;
    avalueflex *ValueFlex(const atom *type=NULL) const;
    const atom *Atom(const atom *type=NULL) const;
    ATK *ATKObject(const atom *type=NULL) const;
    // List is a duplicate of ValueFlex, for historical reasons.
    // It shouldn't outlive April 1995. -rr2b
    avalueflex *List(const atom *type=NULL) const;

    // These allow avalue objects to be cast implicitly to the indicated types.  The result is 0 if the avalue is not of the proper type.
    inline operator long() const{
	return Integer();
    }
    inline operator double() const {
	return Real();
    }
    inline operator const char *() const {
	return CString();
    }
    inline operator void *() const {
	return VoidPtr();
    }
    inline operator avalueflex *() const {
	return ValueFlex();
    }
    inline operator const atom *() const {
	return Atom();
    }
    inline const atom *Type() const {
	return type;
    }
    inline const atom *Name() const {
	return name;
    }

    // Sets the avalue to the indicated value, name and type.
    // Pointers are owned by the calling code.
    void Set(long integer,const atom *name=NULL, const atom *type=NULL);
    void Set(double real, const atom *name=NULL, const atom *type=NULL);
    void Set(const char *str, const atom *name=NULL, const atom *type=NULL);
    void Set(void *ptr, const atom *name=NULL, const atom *type=NULL);
    void Set(avalueflex *alist, const atom *name=NULL, const atom *type=NULL);
    void Set(const atom *a, const atom *name=NULL, const atom *type=NULL);
    void Set(ATK *obj, const atom *name=NULL);

    // These assignment operators allow the indicated types to be assigned to an avalue directly.
    inline avalue &operator=(long integer) {
	Set(integer);
	return *this;
    }
     inline avalue &operator=(ATK *obj) {
	Set(obj);
	return *this;
    }
    inline avalue &operator=(double real) {
	Set(real);
	return *this;
    }
    inline avalue &operator=(char *str) {
	Set(str);
	return *this;
    }
    inline avalue &operator=(void *ptr) {
	Set(ptr);
	return *this;
    }
    inline avalue &operator=(avalueflex *alist) {
	Set(alist);
	return *this;
    }
    inline avalue &operator=(const atom *a) {
	Set(a);
	return *this;
    }

    // Sets the name of an avalue.
    void SetName(const atom *name);

    // These constructors allow the indicated type wherever a const avalue is expected, and may be called explicitly as well.  (Both cases are probably rare, usually the avalue objects will be managed by an avalueflex or class derived therefrom.
    inline avalue(long integer,const atom *name=NULL, const atom *type=NULL) {
	Set(integer, name, type);
    }
    inline avalue(double real, const atom *name=NULL, const atom *type=NULL) {
	Set(real, name, type);
    }
    inline avalue(const char *str, const atom *name=NULL, const atom *type=NULL) {
	Set(str, name, type);
    }
    inline avalue(void *ptr, const atom *name=NULL, const atom *type=NULL) {
	Set(ptr, name, type);
    }
    inline avalue(avalueflex *alist, const atom *name=NULL, const atom *type=NULL){
	Set(alist, name, type);
    }
    inline avalue(const atom *a, const atom *name=NULL, const atom *type=NULL) {
	Set(a, name, type);
    }
    inline avalue(ATK *a, const atom *name=NULL) {
	Set(a, name);
    }
};

// Define the generic part of the avalueflex functionality.
DEFINE_OFLEX_CLASS(avalueflex_base,avalue,5U);

// Define the specialized parts of the avalueflex functionality.
// The avalueflex is a flexible array supporting indexed access, insertion, and deletion of avalues.
class avalueflex : public avalueflex_base {
  protected:
    // These allow avalueflex objects to be quite cheap to create and throw away assuming they have fewer that 16 elements.
    char *Allocate(size_t len);
    
  public:
    // Constructors to allow building an avalueflex containing one value.
    // Only the first argument is required so where an avalueflex or
    // a reference to a const avalueflex are required any of long, double, char *
    // void *, avalueflex *, or atom * can be passed.
    avalueflex(long integer, const atom *name=NULL, const atom *type=NULL);
    avalueflex(double real, const atom *name=NULL, const atom *type=NULL);
    avalueflex(const char *str, const atom *name=NULL, const atom *type=NULL);
    avalueflex(void *ptr, const atom *name=NULL, const atom *type=NULL);
    avalueflex(avalueflex *avf, const atom *name=NULL, const atom *type=NULL);
    avalueflex(const atom *a, const atom *name=NULL, const atom *type=NULL);
    avalueflex(ATK *a, const atom *name=NULL);

    avalueflex();
    
    // Returns the first avalue with the given name and type.  NULL is returned if no matching avalue is present.
    avalue *Find(const atom *name, const atom *type=NULL);
    // Returns the first avalue with the given name and type.  NULL is returned if no matching avalue is present.  The returned avalue may not be modified.  This version is applied if called for a const avalueflex.  If the avalueflex contains a pointer to another that avalueflex will be searched as well at the point that its pointer is encountered in the original.
    inline const avalue *Find(const atom *n, const atom *t=NULL) const {
	avalueflex *huh=(avalueflex *)this;
	avalue *huh2=huh->Find(n, t);
	return huh2;
    }
    // Convenience operators to allow easy creation of argument lists without types or names.  Used like: avalueflex()|9|"foo"|((void *)bar)
    inline avalueflex &operator|(ATK *a) {
	Append()->Set(a);
	return *this;
    }
    inline avalueflex &operator|(long integer) {
	Append()->Set(integer);
	return *this;
    }
    inline avalueflex &operator|(double real) {
	Append()->Set(real);
	return *this;
    }
    inline avalueflex &operator|(char *str) {
	Append()->Set(str);
	return *this;
    }
    inline avalueflex &operator|(void *ptr) {
	Append()->Set(ptr);
	return *this;
    }
    inline avalueflex &operator|(avalueflex *avf) {
	Append()->Set(avf);
	return *this;
    }
    inline avalueflex &operator|(const atom *a) {
	Append()->Set(a);
	return *this;
    }
    // Support for adding named or typed elements in a more list-like syntax.
    inline avalueflex &add(ATK *a, const atom *name=NULL) {
	Append()->Set(a, name);
	return *this;
    }
    inline avalueflex &add(long integer, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(integer, name, type);
	return *this;
    }
    inline avalueflex &add(double real, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(real, name, type);
	return *this;
    }
    inline avalueflex &add(const char *str, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(str, name, type);
	return *this;
    }
    inline avalueflex &add(void *ptr, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(ptr, name, type);
	return *this;
    }
    inline avalueflex &add(avalueflex *avf, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(avf, name, type);
	return *this;
    }
    inline avalueflex &add(const atom *a, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(a, name, type);
	return *this;
    }
    int InterpretString(const char **str);
    
    // Returns TRUE if and only if the types of the unnamed arguments in this avalueargs object match the type atoms listed in argtypes.  If there are fewer elements in this avalueargs than in the argtypes FALSE will be returned.  If there are unnamed arguments in this avalueargs after the arguments matched to argtypes elemented FALSE will be returned.
    boolean TypeCheck(const avalueflex &argtypes) const;
    
    virtual ~avalueflex();
};

#ifdef USE_OBSOLETE_AVALUEARGS_CLASS
//  Avalueargs is a convenience class to allow the easy construction of avalueargs objects initialized with a list of values.
// For a single argument an expression of the appropriate type may be used directly where a const avalueargs is expected.
// For lists of multiple arguments .arg(expression) may be used like: avalueargs().arg(arg1).arg(arg2,name).arg(arg3,name3,type)
//  Alternately if names and type aren't needed  the following syntax may be used:
// avalueargs()|arg1|arg2|arg3
// Type checking is available, e.g.:
// if(av.ArgsOK(avalueargs().arg(argtype1).arg(argtype2)) ok;
//  else bad.
class avalueargs : public avalueflex {
  public:
    // An avalueargs may be constructed empty, or with a single element.  Where a const avalueargs is expected and no name or type is needed an expression of the appropriate type may be used.
    avalueargs();
    avalueargs(long integer, const atom *name=NULL, const atom *type=NULL);
    avalueargs(double real, const atom *name=NULL, const atom *type=NULL);
    avalueargs(const char *str, const atom *name=NULL, const atom *type=NULL);
    avalueargs(void *ptr, const atom *name=NULL, const atom *type=NULL);
    avalueargs(avalueflex *avf, const atom *name=NULL, const atom *type=NULL);
    avalueargs(const atom *a, const atom *name=NULL, const atom *type=NULL);
    avalueargs(ATK *a, const atom *name=NULL);
    
    // Support for adding named or typed arguments.
    inline avalueargs &arg(ATK *a, const atom *name=NULL) {
	Append()->Set(a, name);
	return *this;
    }
    inline avalueargs &arg(long integer, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(integer, name, type);
	return *this;
    }
    inline avalueargs &arg(double real, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(real, name, type);
	return *this;
    }
    inline avalueargs &arg(const char *str, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(str, name, type);
	return *this;
    }
    inline avalueargs &arg(void *ptr, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(ptr, name, type);
	return *this;
    }
    inline avalueargs &arg(avalueflex *avf, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(avf, name, type);
	return *this;
    }
    inline avalueargs &arg(const atom *a, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(a, name, type);
	return *this;
    }

};
#endif

 // These next two #defines aren't done yet.  Eventually arbitrary pointers and objects will hopefully be allowed in avalues.
#define DEFINE_AVALUE_POINTER_TYPE(name,type) \
static atom_def Concat(avalue_atom_,name)(Stringize(name)); \
class Concat(avalue_,name) : public avalue { \
public: \
inline Concat(Set_,name)(type const &o,const atom *atype=NULL, const atom *name=NULL) { \
Set(o, atype, name?name:Concat(avalue_atom_,name)); \
} \
inline Concat(avalue_,name)(type const &o,const atom *atype=NULL, const atom *name=NULL) { \
Concat(Set_,name)(o,atype,name); \
} \
inline operator type() { \
return (type)VoidPtr(Concat(avalue_atom_,name)); \
} \
inline type operator *() { \
return (type) VoidPtr(Concat(avalue_atom_,name)); \
} \
static inline type Cast(avalue *a) { \
return (type)a->VoidPtr(Concat(avalue_atom_,name)); \
} \
}

#define DEFINE_LARGE_AVALUE_TYPE(name,type) \
static atom_def Concat(avalue_atom_,name)(Stringize(name)); \
class Concat(avalue_,name) : public avalue { \
public: \
static inline void Delete(void *o) { \
delete (type)o; \
} \
inline Concat(Set_,name)(type const *o, const atom *atype=NULL, const atom *name=NULL) { \
Set(new type(*o), Delete, atype, name?name:Concat(avalue_atom_,name)); \
} \
inline Concat(avalue_,name)(type const *o,const atom *atype=NULL, const atom *name=NULL) { \
Concat(Set_,name)(o,atype,name); \
} \
inline operator type() { \
return (type)VoidPtr(Concat(avalue_atom_,name)); \
} \
inline type operator *() { \
return (type) VoidPtr(Concat(avalue_atom_,name)); \
} \
static inline type Cast(avalue *a) { \
return (type)a->VoidPtr(Concat(avalue_atom_,name)); \
} \
}

DEFINE_OFLEX_CONST_ITER(avalueflex,avalue);
DEFINE_OFLEX_ITER(avalueflex, avalue);
/** @} */
#endif /* _AVALUE_H */
