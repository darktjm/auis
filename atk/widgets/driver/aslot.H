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
#ifndef _ASlot_H_
#define _ASlot_H_
ATK_INTER

/* 
	ASlot and derived classes form a family of types and slots
	for AWidget attributes.  The design permits a typed attribute 
	to be added to an AWidget even if that widget was compiled
	before the particular ASlot derived class was implemented.

Additional types are derived from ASlot

This file defines classes for these types:
		Int, Real, String, File, Ptr, ATK, Function, 
		DataObj, Figure, Font, Color, Bool
In each case, the full name begins with "ASlot".
Also supported:
	use of (ATK) value as a (text *) value.

*/

#include <avalue.H>
#include <aaction.H>
#include <color.H>

// const_ASlot is a type which can be assigned to an ASlot
//	it should be const, but that gives annoying stupid warnings
#define const_ASlot ASlot

#define SLOTINITPREFIX
#define SLOT(name) (SLOTINITPREFIX Baptize(SLOTINITPREFIX name, atom_def(Stringize(name)), TRUE))
#define SLOTINIT(name,val) ((SLOTINITPREFIX name=(val)),SLOTINITPREFIX Baptize(SLOTINITPREFIX name, atom_def(Stringize(name)), TRUE))

#define TSLOT(name) (Baptize(name, atom_def(Stringize(name)), FALSE))
#define TSLOTINIT(name,val) ((name=(val)),Baptize(name, atom_def(Stringize(name)), FALSE))


/* ASlot_NAME(string) declares slot_string to be a 
	variable referring to slot "string".
  A variable to contain an ASlot_NAME should be declared
	ASlot_NameVar
  A parameter to contain an ASlot_NAME is of type
	ASlot_NameParam
  For both variables and parameters, the source value can be 
  another such value or an atom, an atom *, an atom_def, 
  or a plain string.

	ASlot_NAME(height);	// slot_height
	ASlot_NameVar nv;
	nv = slot_height;		// ok
	nv = atom_def("height");	// ok
	nv = "height";			// even this is okay
*/
#define ASlot_NAME(name) \
		static atom_def Concat(slot_,name) (Stringize(name))
typedef atom_def ASlot_NameVar;
typedef const atom_def &ASlot_NameParam;

#define NULLSTR ((char *)NULL)
#define NULLVOID ((void *)NULL)
#define NULLATK ((ATK *)NULL)
#define NULLACT ((aaction *)NULL)
#define NULLPROC ((proctable_Entry *)NULL)
#define NULLFIG ((figure *)NULL)
#define NULLFONTDESC ((fontdesc *)NULL)

#ifdef ATK_NO_BOOL
typedef class abool_c {
    char val;
  public:
    inline abool_c(long b) {
        val=b?1:0;
    }
    inline abool_c(void *b) {
        val=b?1:0;
    }
    inline abool_c(const abool_c &b) {
        val=b.val;
    }
    operator int() {
        return val;
    }
    inline abool_c &operator=(long b) {
        val=(char)b;
        return *this;
    }
} abool;
#undef FALSE
#undef TRUE
#define FALSE ((long)0)
#define TRUE ((long)1)
#else
typedef bool abool;
#endif

class ASlot;
// ASlotAssign - class for aactions supplied as Assign function
DEFINE_AACTION_FUNC_CLASS(ASlotAssign, ASlot);

struct proctable_Entry;
class text;
class figure;
class fontdesc;
class dataobject;
class atom;
class AString;
class ATK;
class ASlotInt;
class ASlotReal;
class ASlotString;
class ASlotFile;
class ASlotPtr;
class ASlotATK;
class ASlotFunction;
class ASlotDataObj;
class ASlotFigure;

#include <ATK.H>
#include <aformula.H>
class ASlot : public ADepender {
    friend ASHook;
  public:
    // ASlot: Constants for flag values.
	static const unsigned short fromresources;
		// value came from resource file or inherited in tree
	static const unsigned short todatastream;
		// will save in datastream
	static const unsigned short isowner;
		// ~ASlot must free val.obj
	static const unsigned short isdefault;
		// most recent value given by Default()
	static const unsigned short validating;
	// value is presently being computed.
	static const unsigned short valid;
	// value has been computed.

private:
	ASlot_NameVar name;		// can be set with Baptize
	ASHook *hook;		// usually NULL;  set w/ SetHook or via SetAssign
	unsigned short flags;	// see below under GetFlags

	static const char *client;	// for error messages
protected:
	char * source;		// how was value computed?
	avalue_u val;		// the actual value

	ASlot &Assign(const avalue_u *src);
		// used by derived classes to perform assignment
		// uses hook  (and thus the Assign function),
		// if defined
		// clears the source
	ASlot &Assign(const_ASlot *src);
		// uses setit;  retains source

	virtual void CantTake(const char *invalid);
		// string is invalid for type
	virtual void CantTake(const_ASlot *got);
		// got is an incorrect type for assignment to dest
	virtual void InvalidAs(const char *desiredtype) const;
		// slot cannot be converted to desiredtype

	inline void Check() const {
	    ADepender *top=Top();
	    if(top) {
		if(hook==NULL) ((ASlot *)this)->hook=new ASHook;
		hook->AddDepender(top, TopPart());
	    }
	    if(!(flags&ASlot::valid)) ((ASlot *)this)->Validate();
	}

	virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;
	
public:
	static void Init();
		// ASlot::Init must be called during initialization
		// of any module which uses one of the ASlot::xxxreg values.
	virtual ATKregistryEntry *ATKregistry();
	
	ASlot();
	virtual ~ASlot();
	virtual ASlot &Default();  // sets slot to its default value
	inline virtual boolean IsDefault()  const 
		// TRUE iff latest assign is via Default()
		{ return flags & isdefault; }

	inline operator avalue_u() const { Check(); return val; }
	virtual inline ASlot &operator=(const_ASlot *v) 
		{ return Assign(v); }

	// Input/output
	//	a line written has the form:   
	//  slot-name : ( type ) string-basis-for-value \n
	// where type is the suffix after "ASlot" in the class name
	void Write(FILE *fp) const;
	static ASlot *Read(FILE *fp);

	// value<->string conversion
	virtual void WriteValue(AString *dest) const;
		// writes value to dest
	virtual boolean ReadValue(const char *src);
		// reads value from src

	// each function converts from its argument to a string
	// 	and calls ReadValue on the string
	// interprets TAGS on the front of strings
	virtual boolean EatString(const char *src);
	virtual boolean EatFunction(const ASlotFunction &src);
	virtual boolean EatATK(const ATK *obj);
	virtual boolean EatAvalue(const avalue *v);
		
	// Flags: the next two set/reset the todatastream flag
	//	it will be set again if fromresources is true 
	//	and the value is modified
	inline void Remember() { flags |= todatastream; }
	inline void DisRemember() { flags &= ~todatastream; }

// ASlot: The following are for AWidget and other driver modules
// Normal clients should not need these

	static const char *ASlotPrefix(const char *name);
		// returns name prefixed with "ASlot"
	const char *GetSlotType() const;
		// could be {return GetTypeName();}
		// but strip off leading "ASlot"
	static ATKregistryEntry *LoadType(const char *typenm);
		// for AWidget::AddSlot(atom *name, char *typenm)
		//	creates a slot of the named type
		// almost is:  {return ATK::LoadClass(typenm);}
		// but have to prefix typename w/ "ASlot"
	static ASlot *NewASlot(const char *typenm);
		// typenm does not have leading "ASlot"

	inline void SetName(ASlot_NameParam nm)  {name = nm;}
	inline ASlot_NameVar GetName() const { return name; }
	void SetSource(const char *src);
	inline const char *GetSource() const { return source; }
	inline void SetHook(ASHook *h) {
	    if(h!=hook) {
		if(h) h->Reference();
		if(hook) hook->Destroy();
		hook=h;
	    }
	}
	inline ASHook *Hook() const { return hook; }
	inline void SetFormula(aaction *act) {
	    if(hook==NULL) hook=new ASHook;
	    hook->SetFormula(act);
	}
	inline aaction *Formula() {
	    if(hook) return hook->Formula();
	    else return NULL;
	}
	void SetAssign(aaction *act);
	inline aaction *GetAssign() {
	    if(hook) return hook->Assign();
	    else return NULL;
	}
	inline void UseOnlyFromAnAssignFunction
			(const avalue_u *src)  { val = *src; }
	void DumpSlot(FILE *f);
	static atom *in0type;

	inline static const char *Client(const char *objnm)
			{ const char *old = client;  
			client = objnm;  return old; }

// ASlot: Operations on flags	
	inline void EnableFlags(unsigned short f) { flags |= f; }
	inline void DisableFlags(unsigned short f)  { flags &= ~f; }
	inline unsigned int GetFlags(unsigned short mask =~0) const
			{ return flags & mask; }
	
// ASlot: Registry entry values for type tests
	// registry entries for ASlotXxx types
        // 	not used in aslot.C,  but used in avarset
        static ATKregistryEntry *fontslotreg;
        static ATKregistryEntry *boolslotreg;
	static ATKregistryEntry *intslotreg;
	static ATKregistryEntry *realslotreg;
	static ATKregistryEntry *stringslotreg;
	static ATKregistryEntry *fileslotreg;
	static ATKregistryEntry *ptrslotreg;
	static ATKregistryEntry *atkslotreg;
	static ATKregistryEntry *functionslotreg;
	static ATKregistryEntry *dataobjslotreg; 
	static ATKregistryEntry *figureslotreg;

	// registry entries for non-ASlotXxx types
	//	used in aslot.C
	static ATKregistryEntry *atkreg;
	static ATKregistryEntry *dataobjreg;
	static ATKregistryEntry *textreg;
	static ATKregistryEntry *figurereg;

// ASlot: operator=(type)  conversions
// 	must be overridden to be useful

#define ASEQ(typenm) 				\
	virtual inline ASlot & operator= (typenm);
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)
#undef ASEQ
// the following are used to avoid stupid msgs from HP compiler
#define ASEQ(typenm) \
	inline ASlot & operator= (typenm arg) { \
		return ASlot::operator=(arg);  }
#define ASEQC(typenm, parentclass) \
	inline ASlot &operator= (typenm arg) { \
		return Concat(ASlot,parentclass)::operator=(arg);  }

/* ASlot: operator typenm() conversions
		must be overridden to be useful
	These conversions permit arbitrary ASlot objects to be
	evaluated for specific types.  These are largely obviated by
		(a) the forms of Get with default values
		(b) 'operator typenm' conversions 
			within specific derived classes
	However, they are used by the Get methods in avarset.C 
*/
#define ASTYPE(typenm, slottype) 			\
		virtual inline operator typenm() const;
	ASTYPE(abool, Bool)
	ASTYPE(ATK *, ATK)
	ASTYPE(aaction *, Function)
	ASTYPE(const char *, String)
	ASTYPE(const color &, String)
	ASTYPE(dataobject *, DataObj)
	ASTYPE(double, Real)
	ASTYPE(figure *, Figure)
	ASTYPE(fontdesc *, Font)
	ASTYPE(int, Int)
	ASTYPE(long, Int)
	ASTYPE(text *, DataObj)
	ASTYPE(void *, Ptr)
#undef ASTYPE

// ASlot: Function call operations  (using aaction)
	virtual inline avalue operator()(ATK *obj=NULL) const;
	virtual inline avalue operator()(ATK *obj, 
			const avalueflex &in)const;
	virtual inline avalue operator()(ATK *obj, 
			const avalueflex &in, avalueflex &out) const;
	virtual void Invalidate(ADependerKey key=0);
	virtual void Validate(ADependerKey key=0);
};   // end of class ASlot{}


class ASlotInt : public ASlot {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotInt() { val.integer = -999; }
    inline ASlotInt(int i)  { *this = i; }
    inline ASlotInt(atom *name) 
		{ val.integer = -999;  SetName(name);}
    inline ASlotInt(atom *name, int i) 
		{*this = i; SetName(name);}
    ASlot &Default();

    inline operator int() const { Check(); return val.integer; }
    inline operator long() const { Check();  return val.integer; }
    virtual inline ASlot &operator=(const_ASlot *v) {
		if  (v->IsType(realslotreg))  
			return *this=(double)*v; // (discard source)
		return Assign(v);
    }
    inline ASlot &operator=(long i) 
		{avalue_u v;  v.integer = i;  Assign(&v);  return *this;}
    inline ASlot &operator=(int i) { return *this = (long)i; }
    inline ASlot &operator=(double i) { return *this = (long)i; }

	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)
	ASEQ(void *)
    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    boolean EatAvalue(const avalue *v);

    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;

};


class ASlotReal : public ASlot {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotReal() { val.real = -999.0; }
    inline ASlotReal(double r) { *this = r; }
    inline ASlotReal(atom *name) 
		{ val.real = -999.0;  SetName(name); }
    inline ASlotReal(atom *name, double r) 
		{ *this = r; SetName(name); }
    ASlot &Default(); 

    inline operator double() const {  Check(); return val.real; }
    virtual inline ASlot &operator=(const_ASlot *v) {
		if (v->IsType(intslotreg))  
			return *this=(long)*v;	// (discard 'source')
		return Assign(v);
    }
    inline ASlot &operator=(double r) 
		{ avalue_u v;  v.real = r;  Assign(&v);  return *this; }
    inline ASlot &operator=(int i) { return *this = (double)i; }
    inline ASlot &operator=(long i) { return *this = (double)i; }
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)
	ASEQ(void *)
    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    boolean EatAvalue(const avalue *v);
    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;

};


class ASlotString : public ASlot {
		// ASSUMPTION:  val.cstr is never NULL
		// the ASlotString always owns its pointee
  private:
    static const char *empty;
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotString() { val.cstr = empty; }
    inline ASlotString(const char* s) 
		{ val.cstr = empty; *this = s; }
    inline ASlotString(atom *name) 
		{ val.cstr = empty; SetName(name); }
    inline ASlotString(atom *name, const char *s) 
		{ val.cstr = empty; SetName(name); *this = s; }
    inline virtual ~ASlotString() 
		{ if (val.cstr && val.cstr != empty) free((void *)val.cstr); }
    ASlot &Default();

    inline operator const char *() const {  Check(); return val.cstr; }
    ASlot &operator=(const_ASlot *s);
    ASlot &operator=(const char *s);
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)
    inline ASlot &operator=(const char s)     // does not override
		{ char buf[2]; buf[0]=s; buf[1]='\0';  
		return *this = (const char *)buf; }

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;

};


// ASlotFile holds a string value during execution
//	it overrides ReadValue and WriteValue and 
//	saves various access paths to file
// Default is /dev/null

class ASlotFile : public ASlotString {
	// ASSERT:  val.cstr is never NULL
  private:
    static const char devnull[];
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotFile() { val.cstr = devnull; }
    inline ASlotFile(const char *s)  {val.cstr = devnull; *this = s;}
    inline ASlotFile(atom *name) 
		{val.cstr = devnull; SetName(name); }
    inline ASlotFile(atom *name, const char *s) 
		{ val.cstr = devnull; SetName(name); *this = s; }
    inline virtual ~ASlotFile() 
		{ if (val.cstr && val.cstr != devnull) free((void *)val.cstr); val.cstr = NULL;}
    ASlot &Default();

    ASlot &operator=(const_ASlot *s);
    ASlot &operator=(const char *s);
    	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)

    inline operator const char *() const {  Check(); return val.cstr; }

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);

    virtual boolean ReadFile(AString *dest) const;
		// read the entire file into dest
		// returns FALSE for failure
    virtual inline int OpenFile(int flags) const
		// return a file descriptor for the file (or -1)
		{ const char *nm = *this; 
		return (nm) ? open(nm, flags, 0644) : -1; }
    
};


class ASlotPtr : public ASlot {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotPtr() { }
    inline ASlotPtr(atom *n)
		{ SetName(n); }
    inline ASlotPtr(void *s) { *this = s; }
    inline ASlotPtr(atom *n, void *s)
		{ SetName(n);  *this = s; }
	// There is no destructor to free the pointee
    ASlot &Default();

    virtual inline operator void *() const
			{  Check(); return (void *)val.obj; }
    virtual operator ATK *() const;
    virtual inline ASlot &operator=(const_ASlot *v) {
	return Assign(v);
    }
    virtual inline ASlot &operator=(void *s) 
		{ avalue_u v;  v.obj = s;  Assign(&v);  return *this; }
    virtual ASlot &operator=(long s);
    virtual inline ASlot &operator=(int s)
		{ return *this = (long)s; } 
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)

    void WriteValue(AString *dest) const;
    // Derived classes MUST override WriteValue & ReadValue.
    boolean EatAvalue(const avalue *v);
    boolean EatATK(const ATK *obj);
    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;
};


class ASlotATK : public ASlotPtr {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotATK() { }
    inline ASlotATK(ATK *s)   { *this = s; }
    inline ASlotATK(atom *n) { SetName(n);  }
    inline ASlotATK(atom *n, ATK *s)  
		{ SetName(n);  *this=s; }

    inline operator ATK *() const {  Check();  return (ATK *)val.obj; }
    operator figure *() const;
    operator text *() const;
    operator dataobject *() const;
    inline ASlot &operator= (ATK *s) 
		{ avalue_u v;  v.obj = (void *)s;  
		Assign(&v);  return *this; }
    virtual inline ASlot &operator=(const_ASlot *v) {
	return Assign(v);
    }
	ASEQC(int, Ptr)
	ASEQC(long, Ptr)
	ASEQC(void *, Ptr)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)

    boolean EatAvalue(const avalue *v);
    boolean EatATK(const ATK *obj);
};


class ASlotFunction : public ASlotATK {
// (derives from ASlotATK because aaction derives from ATK)
  private:
    proctable_Entry *pe;
  public:
    ATKregistryEntry *ATKregistry();
    ASlotFunction();
    ASlotFunction(atom *n);
    ASlotFunction(aaction *s);
    ASlotFunction(atom *n, aaction *s);
    inline virtual ~ASlotFunction() 
		{ if (GetFlags(isowner)) 
				((aaction *)val.obj)->Destroy(); }
    ASlot &Default();

    inline operator aaction *() const {  Check();  return (aaction *)val.obj; }
    virtual ASlot &operator=(const_ASlot *v);
    ASlot &operator=(aaction *a);
    ASlot &operator=(const char *s);
    ASlot &operator=(proctable_Entry *p);
    inline ASlot &operator=(void *a) 
		{ if (a) CantTake("void *"); else Default(); return *this;}  
		// xxx ??? is it appropriate to use Default to assign NULL

	ASEQC(int, Ptr)
	ASEQC(long, Ptr)
	ASEQC(ATK *, ATK)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
    ASlot &operator=(const ASlotFunction *s); // does not override
    ASlot &operator=(const ASlotFile *s); // does not override

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    boolean EatFunction(const ASlotFunction &src);
    boolean EatATK(const ATK *obj);
};


class ASlotDataObj : public ASlotATK {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotDataObj() { val.obj = NULL; }
    inline ASlotDataObj(dataobject *s) 
		{ val.obj = NULL; *this = s; }
    inline ASlotDataObj(atom *n) 
		{ val.obj = NULL; SetName(n); }
    inline ASlotDataObj(atom *n, dataobject *s)  
		{ val.obj = NULL; SetName(n);  *this=s; }
    virtual ~ASlotDataObj();
    ASlot &Default();

    inline operator dataobject *() const 
		{ Check();  return (dataobject *)val.obj; }
    ASlot &operator=(const_ASlot *src); 
    ASlot &operator=(dataobject *s);
    inline ASlot &operator=(figure *s) 
		{ return ASlotDataObj::operator=((dataobject *)s); }
    inline ASlot &operator=(void *a) 
		{ if (a) CantTake("void *"); else Default(); return *this;}
	ASEQC(ATK *, ATK)
	ASEQC(int, Ptr)
	ASEQC(long, Ptr)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(double)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    virtual boolean EatATK(const ATK *obj);
};


class ASlotFigure : public ASlotDataObj {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotFigure() { }
    inline ASlotFigure(atom *n)  { SetName(n); }
    inline ASlotFigure(figure *s)   { *this=s; }
    inline ASlotFigure(atom *n, figure *s) 
		{ SetName(n);  *this=s; }

    inline operator figure *() const { Check();  return (figure *)val.obj; }
    virtual inline ASlot &operator=(const_ASlot *v) {
		if (v->IsType(atkslotreg))  {
			*this=(ATK *)*v;
			if (v->GetSource()) SetSource(v->GetSource());
			return *this;
		}
		return Assign(v);
    }
    ASlot &operator=(figure *s);
    virtual ASlot &operator=(ATK *src);
	ASEQC(int, Ptr)
	ASEQC(long, Ptr)
	ASEQC(dataobject *, DataObj)
	ASEQC(void *, DataObj)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(double)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)

    virtual boolean EatATK(const ATK *obj);
};

class ASlotFont : public ASlot {
	const char *fontname;
  public:
	ATKregistryEntry *ATKregistry();
	ASlotFont(const char *s = NULL); 
	ASlotFont(atom *name, const char *s = NULL); 
	inline virtual ~ASlotFont() 
		{ if(fontname) free((void *)fontname); }
	ASlot &Default();

	inline operator fontdesc *() const 
			{ Check();  return (fontdesc *)val.obj; }
	inline operator const char *() const { Check();  return fontname; }
	ASlot &operator=(const_ASlot *s);
	ASlot &operator=(const char *s);
	ASlot &operator=(fontdesc *s);
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)

	void WriteValue(AString *dest) const;
	boolean ReadValue(const char *src);

	virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;
};

class ASlotBool : public ASlotInt {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotBool() { val.integer = FALSE; }
    inline ASlotBool(boolean i)  { val.integer = i; }
    inline ASlotBool(atom *name) 
		{ val.integer = FALSE;  SetName(name);}
    inline ASlotBool(atom *name, boolean i)
		{ val.integer = i; SetName(name); }
    ASlot &Default();

    inline operator abool() const { Check();  return val.integer; }
    inline operator boolean() const { Check();  return val.integer; }
    inline operator long() const { Check();  return val.integer; }

    inline ASlot &operator=(boolean i) {avalue_u v;  v.integer = i;  Assign(&v);  return *this;}
    inline ASlot &operator=(long i) {avalue_u v;  v.integer = i;  Assign(&v);  return *this;}
    virtual inline ASlot &operator=(const_ASlot *v) {
	return Assign(v);
    }
    	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(proctable_Entry *)
	ASEQ(void *)

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    boolean EatAvalue(const avalue *v);
    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;
};

class ASlotColor : public ASlot {
  public:
    ATKregistryEntry *ATKregistry();
    inline ASlotColor() 
    {val.obj = icolor::Create("NONE"); }
    inline ASlotColor(const char* s) 
		{ val.obj = icolor::Create(s); }
    inline ASlotColor(const color& s) {
	color &lcol=(color &)s;
	if(lcol.Name()) val.obj = icolor::Create(lcol.Name());
	else {
	    unsigned short r, g, b;
	    lcol.RGB(r, g, b);
	    val.obj = icolor::Create(r, g, b);
	}
    }
    inline ASlotColor(atom *name)
		{ SetName(name); 
		val.obj = icolor::Create("NONE"); }
    inline ASlotColor(atom *name, const char *s) 
		{ SetName(name); 
		val.obj = icolor::Create(s); }
    inline ASlotColor(atom *name, const color &s) {
	color &lcol=(color &)s;
	SetName(name);
	if(lcol.Name()) val.obj = icolor::Create(lcol.Name());
	else {
	    unsigned short r, g, b;
	    lcol.RGB(r, g, b);
	    val.obj = icolor::Create(r, g, b);
	}
    }
    inline virtual ~ASlotColor() { if(val.obj) ((icolor *)val.obj)->Destroy();}
    ASlot &Default();

    inline operator const color &() const  { Check();  return *(icolor *)val.obj; }
    inline const color &Color() const { Check();  return *(icolor *)val.obj; }
    inline operator const char *() const { Check();  return ((icolor *)val.obj)->Name(); }
    ASlot &operator=(const_ASlot *s);
    ASlot &operator=(const char *s);
    ASlot &operator=(const color &c);
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)

    void WriteValue(AString *dest) const;
    boolean ReadValue(const char *src);
    virtual boolean Equal(const avalue_u &dest, const avalue_u &src) const;
};


// EPILOGUE - methods for ASlot
// (for good, but unremembered, reasons these 
// cannot be within the class declaration for ASlot)

#undef ASEQ
#define ASEQ(typenm) 				\
	inline ASlot & ASlot::operator= (typenm ) {  	\
		CantTake(Stringize(typenm));  			\
		return *this;						\
	}
	ASEQ(ATK *)
	ASEQ(aaction *)
	ASEQ(const char *)
	ASEQ(const color &)
	ASEQ(dataobject *)
	ASEQ(double)
	ASEQ(figure *)
	ASEQ(fontdesc *)
	ASEQ(int)
	ASEQ(long)
	ASEQ(proctable_Entry *)
	ASEQ(void *)
#undef ASEQ


// define inline functions described in ASlot declaration

#define ASTYPE(typenm, slottype) 			\
	inline ASlot::operator typenm() const {  	\
		InvalidAs(Stringize(typenm));	\
		return (typenm)Concat(ASlot,slottype)();	\
	}
	ASTYPE(abool, Bool)
	ASTYPE(ATK *, ATK)
	ASTYPE(aaction *, Function)
	ASTYPE(const char *, String)
	ASTYPE(const color &, String)
	ASTYPE(dataobject *, DataObj)
	ASTYPE(double, Real)
	ASTYPE(figure *, Figure)
	ASTYPE(fontdesc *, Font)
	ASTYPE(int, Int)
	ASTYPE(long, Int)
	ASTYPE(text *, DataObj)
	ASTYPE(void *, Ptr)
#undef ASTYPE

// aaction calls 
	// no in or out
	inline avalue ASlot::operator()(ATK *obj) const {
		aaction *act=*this;
		if(act) {
			avalueflex in, out;
			(*act)(obj, in, out);
			if(out.GetN()) return out[0];
		}
		return 0L;
	}

	// no out
	inline avalue ASlot::operator()(ATK *obj, 
			const avalueflex &in) const { 
		aaction *act=*this;
		if(act) {
			avalueflex out;
			(*act)(obj, in, out);
			if(out.GetN()) return out[0];
		}
		return 0L;
	}

	// both in and out supplied
	inline avalue ASlot::operator()(ATK *obj, 
			const avalueflex &in, avalueflex &out) const { 
		aaction *act=*this;
		if(act) {
			(*act)(obj, in, out);
			if(out.GetN()) return out[0];
		}
		return 0L;
	}

// END of EPILOGUE


#endif  /* _ASlot_H_ */
