#ifndef _AFORMULA_H_
#define _AFORMULA_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */

/** \addtogroup libawidget
 * @{ */
union avalue_u;
class ASlot;
#include <ATK.H>
#include <aaction.H>

typedef unsigned int ADependerKey;
class ADepender;
struct ADependNode {
    ADependNode *next;
    ADepender *depender;
    ADependerKey key; // the key for the portion of the depender which is invalidated
    ADependerKey skey; // the key for the portion of the dependee which changed.
};

class ASHook : public traced {
    aaction *setit;
    ADependNode *dependers;
    aaction *formula;
    boolean informula;
  protected:
    virtual ~ASHook();
  public:
    ASHook();
    inline void SetAssign(aaction *src) {
	if(src) src->Reference();
	if(setit) setit->Destroy();	
	setit=src; }
    inline void SetFormula(aaction *act) {
	if(act) act->Reference();
	if(formula) formula->Destroy();
	formula=act;
    }
    inline aaction *Formula() {
	return formula;
    }
    inline aaction *Assign() { return setit; }
    virtual void Assign(ASlot *dest,const avalue_u *src);
    virtual void Validate(ASlot *dest);
    virtual void Invalidate();
    void AddDepender(ADepender *n, ADependerKey=0 );
};

typedef unsigned char ADependerFlags;
#define ADEPENDERMODEBITS 6
#define ADEPENDERVALID (1<<0)
#define ADEPENDERMODEMASK (3)
#define ADEPENDERVALIDATING (1<<1)
#define ADEPENDERREQUESTEDUPDATE (1<<2)


class AVarSet;
class ADepender : public ATK {
    friend AVarSet;
  public: 
    void Push(ADependerKey key=0);
    void Pop(ADependerKey key=0);
    static void PushOffTag();
    static void PopOffTag();
    static ADependNode *first;
    static inline ADepender *Top() {
	return first?first->depender:NULL;
    }
    static inline ADependerKey TopPart() {
	return first?first->key:0;
    }
  public:
    virtual ATKregistryEntry *ATKregistry();
    virtual void Invalidate(ADependerKey key=0);
    virtual void Validate(ADependerKey key=0);
};

#include <abits.H>

#define AFORMULA_CURRENT ((AFormula *)ADepender::Top())

/* Use AFORMULA_BEGIN_NAMED to start an independent formula block which can later
  be referred to in a Validate(x) call as x. */
#define AFORMULA_BEGIN_NAMED(x) \
static ADependerKey x=AFORMULA_CURRENT->NewKey(); \
if(ADepender::TopPart()!=0) AFORMULA_CURRENT->AddDepender(ADepender::Top(), ADepender::TopPart(), x); \
if(!AFORMULA_CURRENT->CheckValid(x)) { \
AFORMULA_CURRENT->Push(x);

/* Use AFORMULA_END at the end of a formula block. */
#define AFORMULA_END \
AFORMULA_CURRENT->MarkValid(); \
AFORMULA_CURRENT->Pop(ADepender::TopPart()); \
}

#define AFORMULA_NAME(x) Concat(aformula,x)
/* Use AFORMULA_BEGIN to start an unnamed formula block, this can only be used once per source
 file line. */
#define AFORMULA_BEGIN AFORMULA_BEGIN_NAMED(AFORMULA_NAME(__LINE__))

class AFormula : public ADepender {
    ADependNode *dependers;
    unsigned char validbits[4];
    abits valid;
    ADependerKey keyid;
  protected:
    ADependerFlags flags;
    ADependerKey currentkey;
    // validation modes should include:
#define AFORMULAIMMEDIATE 0
    // immediate validation
#define AFORMULAWANTUPDATE 1
    // wantupdate/validation on screen update
#define AFORMULAACCESS 2
    // validation on access
#define AFORMULAEAGER 3
    // validation upon validation of any formula dependencies
  public:
    virtual ATKregistryEntry *ATKregistry();

    AFormula();
    virtual ~AFormula();
    void Push(ADependerKey key=0);
    void Pop(ADependerKey key=0);
    virtual void Invalidate(ADependerKey key=0);
    virtual void Validate(ADependerKey key=0);
    virtual void Evaluate();
    virtual void WantUpdate(ADependerKey key=0);
    inline void SetMode(unsigned char c) {
        flags&=~(ADEPENDERMODEMASK<<ADEPENDERMODEBITS);
	flags|=(c<<ADEPENDERMODEBITS);
    }
    inline unsigned char Mode() {
	return (flags>>ADEPENDERMODEBITS)&ADEPENDERMODEMASK;
    }
    void AddDepender(ADepender *depender, ADependerKey key=0, ADependerKey skey=0);
    inline void MarkValid() {
	valid[currentkey-1]=TRUE;
    }
    inline void SetCurrentKey(ADependerKey key) {
	currentkey=key;
    }
    inline boolean CheckValid(ADependerKey key) {
	return valid[key-1];
    }
    inline ADependerKey NewKey() {
	if(keyid==32) {
	    fprintf(stderr, "WARNING: AFormula %p exceeded limit of 32 partial formulas!\n", this);
	}
	return keyid<32?++keyid:0;
    }
};

class AFormulaAct : public AFormula {
  protected:
    ATK *object;
    aaction *formula, *wantUpdateFunction;
  public:
    ATKregistryEntry *ATKregistry();
    inline AFormulaAct(ATK *obj=NULL, aaction *act=NULL, aaction *wantupdate=NULL)  {
        if(act) act->Reference();
        if(wantupdate) wantupdate->Reference();
        object=obj;
        formula=act;
        wantUpdateFunction=wantupdate;
        if(wantupdate) SetMode(AFORMULAWANTUPDATE);
    }
    virtual inline ~AFormulaAct() {
        if(formula) formula->Destroy();
        if(wantUpdateFunction) wantUpdateFunction->Destroy();
    }
    inline void SetFormula(aaction *act) {
	if(act) act->Reference();
	if(formula) formula->Destroy();
	formula=act;
    }
    inline aaction *Formula() {
	return formula;
    }
    inline void SetWantUpdateFunction(aaction *act) {
	if(act) act->Reference();
	if(wantUpdateFunction) wantUpdateFunction->Destroy();
	wantUpdateFunction=act;
        if(act) SetMode(AFORMULAWANTUPDATE);
    }
    inline aaction *WantUpdateFunction() {
        return wantUpdateFunction;
    }
    inline void SetObject(ATK *obj) {
	object=obj;
    }
    inline ATK *Object() {
	return object;
    }
    virtual void Evaluate();
    virtual void WantUpdate(ADependerKey key=0);
};

#include <aslot.H>

class ADependerPlaceHolder : public ADepender {
    ADependNode *dependers;
    ASlot_NameVar name;
  public:
    virtual ATKregistryEntry *ATKregistry();
    ADependerPlaceHolder();
    virtual ~ADependerPlaceHolder();
    virtual void Invalidate(ADependerKey key=0);
    void AddDepender(ADepender *depender,ADependerKey key=0);
    inline void SetName(ASlot_NameParam a) {
	name=a;
    }
    inline const atom_def &Name() {
	return name;
    }
    ADependerPlaceHolder(const ADependerPlaceHolder &src);
    ADependerPlaceHolder &operator=(const ADependerPlaceHolder &src);
};

class AFormulaSlot : public AFormula {
  protected:
    ATK *object;
    ASlotFunction *formula, *wantUpdateFunction;
  public:
    ATKregistryEntry *ATKregistry();
    inline AFormulaSlot(ATK *obj=NULL, ASlotFunction *act=NULL, ASlotFunction *wantupdate=NULL)  {
        object=obj;
        formula=act;
        if(wantupdate) SetMode(AFORMULAWANTUPDATE);
    }
    virtual inline ~AFormulaSlot() {
    }
    inline void SetFormula(ASlotFunction *act) {
        formula=act;
    }
    inline ASlotFunction *Formula() {
        return formula;
    }
    inline void SetWantUpdateFunction(ASlotFunction *act) {
        wantUpdateFunction=act;
        if(act) SetMode(AFORMULAWANTUPDATE);
    }
    inline ASlotFunction *WantUpdateFunction() {
        return wantUpdateFunction;
    }
    inline void SetObject(ATK *obj) {
        object=obj;
    }
    inline ATK *Object() {
        return object;
    }
    virtual void Evaluate();
    virtual void WantUpdate(ADependerKey key=0);
};
/** @} */
#endif
