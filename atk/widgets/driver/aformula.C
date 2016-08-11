/* Copyright 1996 Carnegie Mellon University All rights reserved.
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

#include <andrewos.h>

static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/widgets/driver/RCS/aformula.C,v 1.20 1996/11/15 16:39:05 robr Exp $";

ATK_IMPL("aformula.H")
#include <aformula.H>
#include <aslot.H>
#include <util.h>

ATKdefineRegistryNoInit(ADepender,ATK);
ATKdefineRegistryNoInit(ADependerPlaceHolder,ATK);
ATKdefineRegistryNoInit(AFormula,ADepender);
ATKdefineRegistryNoInit(AFormulaAct,AFormula);
ATKdefineRegistryNoInit(AFormulaSlot,AFormula);

ADependNode *ADepender::first=NULL;
static ADependNode *freenodes=NULL;

static ADependNode *NewNode() {
    ADependNode *result;
    if(freenodes) {
	result=freenodes;
	freenodes=freenodes->next;
    } else result=new ADependNode;
    result->key=~0u;
    result->skey=~0u;
    return result;
}

static void FreeNode(ADependNode *node) {
    node->next=freenodes;
    freenodes=node;
}

ASHook::ASHook() {
    refs=1;
    setit=NULL;
    dependers=NULL;
    formula=NULL;
    informula=FALSE;
}

ASHook::~ASHook() {
    if(setit) setit->Destroy();
    if(formula) formula->Destroy();
    while(dependers) {
	ADependNode *next=dependers->next;
	FreeNode(dependers);
	dependers=next;
    }
}

void ASHook::Destroy() {
    if(refs>0) {
	refs--;
	delete this;
    }
}
    
static atom_def union_src("union_src");
static atom_def in0type("avalue_u");
static ADependNode *pendingvalidations=NULL;
static int delayedvalidation=0;
void ASHook::Assign(ASlot *dest, const avalue_u *src) {
    boolean equal=dest->Equal(dest->val,*src);
    boolean assigned=FALSE;
    if(!equal) {
	if(setit) {
	    avalueflex out;
	    (*setit)(dest, 
		     avalueflex((avalue_u *)src, union_src, in0type)|(long)informula, 
                     out);
            if(out.GetN()==0) assigned=TRUE;
            else assigned=out[0].Integer();
            if(out.GetN()>=2) {
                equal=out[1].Integer();
            }
        } else if(formula==NULL || informula) {
            dest->val = *src;
            assigned=TRUE;
        }
    }
    if(assigned) {
        if (dest->source) FREESTRINGVAR(dest->source);
        if (dest->flags & ASlot::fromresources)  dest->Remember();
        dest->DisableFlags(ASlot::isdefault);
        if(!equal) {
            Invalidate();
        }
    }
}

void ASHook::Validate(ASlot *dest) {
    if(formula) {
        avalueflex out;
        informula=TRUE;
	(*formula)(dest, avalueflex(), out);
	if(out.GetN()>0) {
	    avalue &ref=out[0];
	    const atom *type=ref.Type();
            dest->EnableFlags(ASlot::valid);
	    if(type==avalue::integer) {
		*dest=ref.Integer();
	    } else if(type==avalue::integer) {
		*dest=ref.Real();
	    } else if(type==avalue::cstring) {
		*dest=ref.CString();
	    } else if(type==avalue::voidptr) {
		*dest=ref.VoidPtr();
	    } else if(type==avalue::atkatom) {
		*dest=ref.ATKObject();
            }
        }
        informula=FALSE;
    }
    return;
}

void ASHook::Invalidate() {
    ADependNode *adn=dependers;
    delayedvalidation++;
    while(adn) {
        if(adn->depender) adn->depender->Invalidate(adn->key);
        adn=adn->next;
    }
    delayedvalidation--;
    while(pendingvalidations) {
        ADependNode *self=pendingvalidations;
        pendingvalidations=self->next;
        self->depender->Validate(self->key);
        FreeNode(self);
    }
}

void ASHook::AddDepender(ADepender *n,ADependerKey key) {
    ADependNode *scan=dependers, **scanp=&dependers;
    while(scan && (scan->depender!=n || scan->key!=key)) {
	scanp=&scan->next;
	scan=*scanp;
    }
    if(scan) {
	*scanp=scan->next;
	scan->next=dependers;
	dependers=scan;
    } else {
	ADependNode *result=new ADependNode;
	result->depender=n;
	result->next=dependers;
	result->key=key;
	dependers=result;
    }
}

void ADepender::Invalidate(ADependerKey) {
}

void ADepender::Validate(ADependerKey) {
}

void ADepender::Push(ADependerKey key) {
    ADependNode *result=NewNode();
    result->depender=this;
    result->next=first;
    result->key=key;
    result->skey=0;
    first=result;
}

void ADepender::Pop(ADependerKey key) {
    if(!(first && first->depender==this && first->key==key)) {
	fprintf(stderr, "warning: ADepender: stack mismatch.\n");
	return;
    }
    ADependNode *next=first->next;
    FreeNode(first);
    first=next;
}

void ADepender::PushOffTag() {
    ADependNode *result=NewNode();
    result->depender=NULL;
    result->next=first;
    first=result;
}


void ADepender::PopOffTag() {
    if(!(first && first->depender==NULL)) {
	fprintf(stderr, "warning: ADepender: stack mismatch.\n");
	return;
    }
    ADependNode *next=first->next;
    FreeNode(first);
    first=next;
}

ADependerPlaceHolder::ADependerPlaceHolder() {
    dependers=NULL;
    name="NONE";
}

ADependerPlaceHolder::~ADependerPlaceHolder() {
    while(dependers) {
	ADependNode *next=dependers->next;
	FreeNode(dependers);
	dependers=next;
    }
}

void ADependerPlaceHolder::Invalidate(ADependerKey) {
    ADependNode *adn=dependers;
    while(adn) {
	if(adn->depender) adn->depender->Invalidate(adn->key);
	adn=adn->next;
    }
}

ADependerPlaceHolder::ADependerPlaceHolder(const ADependerPlaceHolder &src) {
    dependers=src.dependers;
    name=src.name;
    ((ADependerPlaceHolder *)&src)->dependers=NULL;
}

ADependerPlaceHolder &ADependerPlaceHolder::operator=(const ADependerPlaceHolder &src) {
    dependers=src.dependers;
    name=src.name;
    ((ADependerPlaceHolder *)&src)->dependers=NULL;
    return *this;
}

void ADependerPlaceHolder::AddDepender(ADepender *n, ADependerKey key) {
    if(this==n) return;
    ADependNode *scan=dependers, **scanp=&dependers;
    while(scan && (scan->depender!=n || scan->key!=key)) {
	scanp=&scan->next;
	scan=*scanp;
    }
    if(scan) {
	*scanp=scan->next;
	scan->next=dependers;
	dependers=scan;
    } else {
	ADependNode *result=NewNode();
	result->depender=n;
	result->next=dependers;
	result->key=key;
	result->skey=0;
	dependers=result;
    }
}

AFormula::AFormula() : valid(validbits,0){
    flags=0;
    dependers=NULL;
    currentkey=0;
    size_t i;
    for(i=0;i<sizeof(validbits);i++) validbits[i]=0;
    keyid=0;
}

AFormula::~AFormula() {
    while(dependers) {
	ADependNode *next=dependers->next;
	FreeNode(dependers);
	dependers=next;
    }
}

void AFormula::Push(ADependerKey key) {
    ADepender::Push(key);
    currentkey=key;
}

void AFormula::Pop(ADependerKey key) {
    ADepender::Pop(key);
    if(first) currentkey=first->key;
    else currentkey=0;
}

static void QueueValidate(AFormula *self, ADependerKey key) {
    ADependNode *result=NewNode();
    result->next=pendingvalidations;
    result->depender=self;
    result->key=key;
    pendingvalidations=result;
}

void AFormula::Invalidate(ADependerKey key) {
    if(key>=1) {
	valid[key-1]=FALSE;
    } else {
	size_t i;
	for(i=0;i<sizeof(validbits);i++) validbits[i]=0;
    }
    ADependNode *top=first;
    while(top) {
        if(top->depender==this) return;
        top=top->next;
    }
    flags&=~ADEPENDERVALID;
    ADependNode *adn=dependers;
    while(adn) {
	if(adn->depender && (adn->skey==key || adn->skey==0 || key==0)) adn->depender->Invalidate(adn->key);
	adn=adn->next;
    }
    switch(Mode()) {
	case AFORMULAIMMEDIATE:
	    if(delayedvalidation>0) QueueValidate(this, key);
	    else Validate(key);
	    break;
	case AFORMULAWANTUPDATE:
	    if(!(flags&ADEPENDERREQUESTEDUPDATE)) {
		WantUpdate(key);
		flags|=ADEPENDERREQUESTEDUPDATE;
	    }
	    break;
	case AFORMULAACCESS:
	    break;
	case AFORMULAEAGER:
	    break;
	default:
	    break;
    }
}

void AFormula::AddDepender(ADepender *n,ADependerKey key, ADependerKey skey) {
    if(this==n && key==0) return;
    ADependNode *scan=dependers, **scanp=&dependers;
    while(scan && (scan->depender!=n || scan->key!=key)) {
	scanp=&scan->next;
	scan=*scanp;
    }
    if(scan) {
	*scanp=scan->next;
	scan->next=dependers;
	dependers=scan;
    } else {
	ADependNode *result=NewNode();
	result->depender=n;
	result->next=dependers;
	result->key=key;
	result->skey=skey;
	dependers=result;
    }
}

void AFormula::Validate(ADependerKey key) {
    if(first && first->depender) AddDepender(first->depender, first->key, key);
    if(flags&ADEPENDERVALID) return;
    if(key>0 && valid[key-1]) return;
    if(flags&ADEPENDERVALIDATING) {
	fprintf(stderr, "warning: AFormula: recursive validation attempted.\n");
	return;
    }
    flags&=~ADEPENDERREQUESTEDUPDATE;
    flags|=ADEPENDERVALIDATING;
    Push();
    Evaluate();
    Pop();
    flags&=~ADEPENDERVALIDATING;
    flags|=ADEPENDERVALID;
    ADependNode *adn=dependers;
    ATK_CLASS(AFormula);
    while(adn) {
	if(adn->depender && (adn->skey==key || adn->skey==0 || key==0) && adn->depender->IsType(class_AFormula)  && ((AFormula *)(adn->depender))->Mode()==AFORMULAEAGER) adn->depender->Validate(adn->key);
	adn=adn->next;
    }
}

void AFormula::Evaluate() {
}

void AFormula::WantUpdate(ADependerKey key) {
    if(delayedvalidation>0) QueueValidate(this, key);
    else Validate(key);
}


void AFormulaAct::Evaluate() {
    if(formula) {
        avalueflex out;
        (*formula)(object, avalueflex(), out);
    }
}

void AFormulaAct::WantUpdate(ADependerKey) {
    if(wantUpdateFunction) {
        avalueflex out;
        (*wantUpdateFunction)(object, avalueflex(), out);
    }
}


void AFormulaSlot::Evaluate() {
    if(formula) {
        avalueflex out;
        (*(aaction *)*formula)(object, avalueflex(), out);
    }
}

void AFormulaSlot::WantUpdate(ADependerKey) {
    if(wantUpdateFunction) {
        avalueflex out;
        (*(aaction *)*wantUpdateFunction)(object, avalueflex(), out);
    }
}
