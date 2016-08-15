/* user code begins here for HeaderInfo */

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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/controllers/RCS/calccon.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
 
/* user code ends here for HeaderInfo */
ATK_IMPL("calccon.H")

#include <proctable.H>
#include <view.H>
#include <arbiterview.H>
#include <calccon.H>
#include <celview.H>
#include <buttonV.H>
#include <controlV.H>
#include <cel.H>
#include <enterstrV.H>
#include <lsetview.H>
#include <lset.H>
#include <value.H>
#include <stringV.H>
/* user code begins here for includes */
#define ADD 1
#define SUB 2
#define MULT 3
#define DIV 4

ATKdefineRegistry(calccon, observable, calccon::InitializeClass);
#ifndef NORCSID
#endif
void displayval(class calccon  *self);
void clear(class calccon  *self);
static class calccon *FindSelf(class view  *v);
static void calcCallBack(class calccon  *self,class value  *val,long  r1,long  r2);
static void valenterCallBack(class calccon  *self,class value  *val,long  r1,long  r2);
static void decimalCallBack(class calccon  *self,class value  *val,long  r1,long  r2);
static void digitCallBack(class calccon  *self,class value  *val,long  r1,long  r2);
static void outputCallBack(class calccon  *self,class value  *val,long  r1,long  r2);
static void initself(class calccon  *self,class view  *v);
void calccon_clear(class view  *v,long  dat);


void displayval(class calccon  *self)
{
if(self->error){
    (self->output)->SetString("Error");
    return;
}
sprintf(self->buf,"%12.6g",self->val);
(self->output)->SetString(self->buf);
}
void clear(class calccon  *self)
{
self->val = 0.0;
self->saveval = 0.0;
*(self->buf) = '\0';
self->error = FALSE;self->op = 0;
self->dec = 0;
self->clear = TRUE;
displayval(self);
}
/* user code ends here for includes */

static class calccon *firstcalccon;
static class calccon *FindSelf(class view  *v)
{
	class calccon *self,*last = NULL;
	class arbiterview *arbv =arbiterview::FindArb(v);
	for(self= firstcalccon; self != NULL; self = self->next){
		if(self->arbv == arbv) return self;
		last = self;
		}
	self = new calccon;
	self->arbv = arbv;
	initself(self,v);
	if(last == NULL) firstcalccon = self;
	else last->next = self;
	(self->arbv)->AddObserver(self);
	return self;
}
static void calcCallBack(class calccon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	if(self->calc_4 == val) self->calc_4 = NULL;
	if(self->calc_5 == val) self->calc_5 = NULL;
	if(self->calc_1 == val) self->calc_1 = NULL;
	if(self->calc_2 == val) self->calc_2 = NULL;
	if(self->calc_3 == val) self->calc_3 = NULL;
}
{
/* user code begins here for calcCallBack */
self->error = FALSE;
switch (self->op){
	case ADD:
	    self->val = self->val + self->saveval;
	    break;
	case SUB:
	    self->val =  self->saveval - self->val;
	    break;
	case MULT:
	    self->val = self->val * self->saveval;
	    break;
	case DIV:
	    if(self->val == 0.0){
		self->error = TRUE;
	    }
	    else self->val =  self->saveval / self->val;
	    break;
	default:
	    break;
}
self->saveval = self->val;
self->op = r1;
self->dec = 0;
self->clear = TRUE;
displayval(self);
/* user code ends here for calcCallBack */
}
}
static void valenterCallBack(class calccon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	if(self->valenter == val) self->valenter = NULL;
}
{
/* user code begins here for valenterCallBack */
{
    double f ;
    char *c;
    c  = (val)->GetString();
    switch (*c){
	case 0:
	    return;
	case 'c':
	case 'C':
	    ::clear(self);
	    break;
	case '=':
	    calcCallBack(self,NULL,0,0);
	    break;
	case '+':
	    calcCallBack(self,NULL,ADD,0);
	    break;
	case '-':
	    calcCallBack(self,NULL,SUB,0);
	    break;
	case '*':
	case 'X':
	    calcCallBack(self,NULL,MULT,0);
	    break;
	case '/':
	    calcCallBack(self,NULL,DIV,0);
	    break;
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9': case '.':
	    sscanf(c,"%lf",&f);
	    if(f != self->val){
		self->val = f;
		displayval(self);
	    }
	    break;
	default:
	    break;
    }
    (val)->SetString("");
}
/* user code ends here for valenterCallBack */
}
}
static void decimalCallBack(class calccon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	if(self->decimal == val) self->decimal = NULL;
}
{
/* user code begins here for decimalCallBack */
    if(self->dec == 0) self->dec = 10;
/* user code ends here for decimalCallBack */
}
}
static void digitCallBack(class calccon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	if(self->digit_0 == val) self->digit_0 = NULL;
	if(self->digit_1 == val) self->digit_1 = NULL;
	if(self->digit_2 == val) self->digit_2 = NULL;
	if(self->digit_3 == val) self->digit_3 = NULL;
	if(self->digit_4 == val) self->digit_4 = NULL;
	if(self->digit_5 == val) self->digit_5 = NULL;
	if(self->digit_6 == val) self->digit_6 = NULL;
	if(self->digit_7 == val) self->digit_7 = NULL;
	if(self->digit_8 == val) self->digit_8 = NULL;
	if(self->digit_9 == val) self->digit_9 = NULL;
}
{
/* user code begins here for digitCallBack */
    if(self->clear){
	self->val = 0.0;
	self->clear = FALSE;
    }
    if(self->dec != 0){
	self->val = self->val + ((double)r1 /(double) self->dec);
	self->dec *= 10;
    }
    else{
	self->val = self->val * 10.0 + r1;
    }
    displayval(self);
/* user code ends here for digitCallBack */
}
}
static void outputCallBack(class calccon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	if(self->output == val) self->output = NULL;
}
{
/* user code begins here for outputCallBack */
/* user code ends here for outputCallBack */
}
}
static void initself(class calccon  *self,class view  *v)
{
	self->v = v;
	self->calc_4View = (class buttonV *)arbiterview::GetNamedView(v,"calc-4");
	self->calc_4 = (class value *)arbiterview::GetNamedObject(v,"calc-4");
	if(self->calc_4) (self->calc_4)->AddCallBackObserver( self,(value_fptr)calcCallBack,4);
	if(self->calc_4View) (self->calc_4View)->AddObserver(self);
	self->calc_5View = (class buttonV *)arbiterview::GetNamedView(v,"calc-5");
	self->calc_5 = (class value *)arbiterview::GetNamedObject(v,"calc-5");
	if(self->calc_5) (self->calc_5)->AddCallBackObserver( self,(value_fptr)calcCallBack,5);
	if(self->calc_5View) (self->calc_5View)->AddObserver(self);
	self->valenterView = (class enterstrV *)arbiterview::GetNamedView(v,"valenter");
	self->valenter = (class value *)arbiterview::GetNamedObject(v,"valenter");
	if(self->valenter) (self->valenter)->AddCallBackObserver( self,(value_fptr)valenterCallBack,0);
	if(self->valenterView) (self->valenterView)->AddObserver(self);
	self->decimalView = (class buttonV *)arbiterview::GetNamedView(v,"decimal");
	self->decimal = (class value *)arbiterview::GetNamedObject(v,"decimal");
	if(self->decimal) (self->decimal)->AddCallBackObserver( self,(value_fptr)decimalCallBack,0);
	if(self->decimalView) (self->decimalView)->AddObserver(self);
	self->digit_0View = (class buttonV *)arbiterview::GetNamedView(v,"digit-0");
	self->digit_0 = (class value *)arbiterview::GetNamedObject(v,"digit-0");
	if(self->digit_0) (self->digit_0)->AddCallBackObserver( self,(value_fptr)digitCallBack,0);
	if(self->digit_0View) (self->digit_0View)->AddObserver(self);
	self->digit_1View = (class buttonV *)arbiterview::GetNamedView(v,"digit-1");
	self->digit_1 = (class value *)arbiterview::GetNamedObject(v,"digit-1");
	if(self->digit_1) (self->digit_1)->AddCallBackObserver( self,(value_fptr)digitCallBack,1);
	if(self->digit_1View) (self->digit_1View)->AddObserver(self);
	self->digit_2View = (class buttonV *)arbiterview::GetNamedView(v,"digit-2");
	self->digit_2 = (class value *)arbiterview::GetNamedObject(v,"digit-2");
	if(self->digit_2) (self->digit_2)->AddCallBackObserver( self,(value_fptr)digitCallBack,2);
	if(self->digit_2View) (self->digit_2View)->AddObserver(self);
	self->digit_3View = (class buttonV *)arbiterview::GetNamedView(v,"digit-3");
	self->digit_3 = (class value *)arbiterview::GetNamedObject(v,"digit-3");
	if(self->digit_3) (self->digit_3)->AddCallBackObserver( self,(value_fptr)digitCallBack,3);
	if(self->digit_3View) (self->digit_3View)->AddObserver(self);
	self->digit_4View = (class buttonV *)arbiterview::GetNamedView(v,"digit-4");
	self->digit_4 = (class value *)arbiterview::GetNamedObject(v,"digit-4");
	if(self->digit_4) (self->digit_4)->AddCallBackObserver( self,(value_fptr)digitCallBack,4);
	if(self->digit_4View) (self->digit_4View)->AddObserver(self);
	self->digit_5View = (class buttonV *)arbiterview::GetNamedView(v,"digit-5");
	self->digit_5 = (class value *)arbiterview::GetNamedObject(v,"digit-5");
	if(self->digit_5) (self->digit_5)->AddCallBackObserver( self,(value_fptr)digitCallBack,5);
	if(self->digit_5View) (self->digit_5View)->AddObserver(self);
	self->digit_6View = (class buttonV *)arbiterview::GetNamedView(v,"digit-6");
	self->digit_6 = (class value *)arbiterview::GetNamedObject(v,"digit-6");
	if(self->digit_6) (self->digit_6)->AddCallBackObserver( self,(value_fptr)digitCallBack,6);
	if(self->digit_6View) (self->digit_6View)->AddObserver(self);
	self->digit_7View = (class buttonV *)arbiterview::GetNamedView(v,"digit-7");
	self->digit_7 = (class value *)arbiterview::GetNamedObject(v,"digit-7");
	if(self->digit_7) (self->digit_7)->AddCallBackObserver( self,(value_fptr)digitCallBack,7);
	if(self->digit_7View) (self->digit_7View)->AddObserver(self);
	self->digit_8View = (class buttonV *)arbiterview::GetNamedView(v,"digit-8");
	self->digit_8 = (class value *)arbiterview::GetNamedObject(v,"digit-8");
	if(self->digit_8) (self->digit_8)->AddCallBackObserver( self,(value_fptr)digitCallBack,8);
	if(self->digit_8View) (self->digit_8View)->AddObserver(self);
	self->digit_9View = (class buttonV *)arbiterview::GetNamedView(v,"digit-9");
	self->digit_9 = (class value *)arbiterview::GetNamedObject(v,"digit-9");
	if(self->digit_9) (self->digit_9)->AddCallBackObserver( self,(value_fptr)digitCallBack,9);
	if(self->digit_9View) (self->digit_9View)->AddObserver(self);
	self->outputView = (class stringV *)arbiterview::GetNamedView(v,"output");
	self->output = (class value *)arbiterview::GetNamedObject(v,"output");
	if(self->output) (self->output)->AddCallBackObserver( self,(value_fptr)outputCallBack,0);
	if(self->outputView) (self->outputView)->AddObserver(self);
	self->calc_1View = (class buttonV *)arbiterview::GetNamedView(v,"calc-1");
	self->calc_1 = (class value *)arbiterview::GetNamedObject(v,"calc-1");
	if(self->calc_1) (self->calc_1)->AddCallBackObserver( self,(value_fptr)calcCallBack,1);
	if(self->calc_1View) (self->calc_1View)->AddObserver(self);
	self->calc_2View = (class buttonV *)arbiterview::GetNamedView(v,"calc-2");
	self->calc_2 = (class value *)arbiterview::GetNamedObject(v,"calc-2");
	if(self->calc_2) (self->calc_2)->AddCallBackObserver( self,(value_fptr)calcCallBack,2);
	if(self->calc_2View) (self->calc_2View)->AddObserver(self);
	self->calc_3View = (class buttonV *)arbiterview::GetNamedView(v,"calc-3");
	self->calc_3 = (class value *)arbiterview::GetNamedObject(v,"calc-3");
	if(self->calc_3) (self->calc_3)->AddCallBackObserver( self,(value_fptr)calcCallBack,3);
	if(self->calc_3View) (self->calc_3View)->AddObserver(self);
}
void calccon_clear(class view  *v,long  dat)
 {
class calccon *self;
if((self = FindSelf(v)) == NULL) return;
/* user code begins here for calccon_clear */
arbiterview::SetIgnoreUpdates(v, TRUE);
::clear(self);
/* user code ends here for calccon_clear */
}
void calccon::ObservedChanged(class observable  * observed,long  status)
{
/* user code begins here for ObservedChanged */
/* user code ends here for ObservedChanged */
if(observed == (class observable *) this->arbv){
	if (status == observable_OBJECTDESTROYED) this->arbv = NULL;
	 else initself(this,this->v);
}
if (status == observable_OBJECTDESTROYED) {
	if (observed == (class observable *) this->calc_4View) this->calc_4View=NULL;
	if (observed == (class observable *) this->calc_5View) this->calc_5View=NULL;
	if (observed == (class observable *) this->valenterView) this->valenterView=NULL;
	if (observed == (class observable *) this->decimalView) this->decimalView=NULL;
	if (observed == (class observable *) this->digit_0View) this->digit_0View=NULL;
	if (observed == (class observable *) this->digit_1View) this->digit_1View=NULL;
	if (observed == (class observable *) this->digit_2View) this->digit_2View=NULL;
	if (observed == (class observable *) this->digit_3View) this->digit_3View=NULL;
	if (observed == (class observable *) this->digit_4View) this->digit_4View=NULL;
	if (observed == (class observable *) this->digit_5View) this->digit_5View=NULL;
	if (observed == (class observable *) this->digit_6View) this->digit_6View=NULL;
	if (observed == (class observable *) this->digit_7View) this->digit_7View=NULL;
	if (observed == (class observable *) this->digit_8View) this->digit_8View=NULL;
	if (observed == (class observable *) this->digit_9View) this->digit_9View=NULL;
	if (observed == (class observable *) this->outputView) this->outputView=NULL;
	if (observed == (class observable *) this->calc_1View) this->calc_1View=NULL;
	if (observed == (class observable *) this->calc_2View) this->calc_2View=NULL;
	if (observed == (class observable *) this->calc_3View) this->calc_3View=NULL;
}
}
boolean calccon::InitializeClass()
{
struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
firstcalccon = NULL;
proctable::DefineProc("calccon-clear",(proctable_fptr)calccon_clear, viewtype,NULL,"calccon clear");
/* user code begins here for InitializeClass */
/* user code ends here for InitializeClass */
return TRUE;
}
calccon::~calccon()
{
	ATKinit;

	if(this->calc_4) (this->calc_4)->RemoveCallBackObserver( this);
	if(this->calc_5) (this->calc_5)->RemoveCallBackObserver( this);
	if(this->valenter) (this->valenter)->RemoveCallBackObserver( this);
	if(this->decimal) (this->decimal)->RemoveCallBackObserver( this);
	if(this->digit_0) (this->digit_0)->RemoveCallBackObserver( this);
	if(this->digit_1) (this->digit_1)->RemoveCallBackObserver( this);
	if(this->digit_2) (this->digit_2)->RemoveCallBackObserver( this);
	if(this->digit_3) (this->digit_3)->RemoveCallBackObserver( this);
	if(this->digit_4) (this->digit_4)->RemoveCallBackObserver( this);
	if(this->digit_5) (this->digit_5)->RemoveCallBackObserver( this);
	if(this->digit_6) (this->digit_6)->RemoveCallBackObserver( this);
	if(this->digit_7) (this->digit_7)->RemoveCallBackObserver( this);
	if(this->digit_8) (this->digit_8)->RemoveCallBackObserver( this);
	if(this->digit_9) (this->digit_9)->RemoveCallBackObserver( this);
	if(this->output) (this->output)->RemoveCallBackObserver( this);
	if(this->calc_1) (this->calc_1)->RemoveCallBackObserver( this);
	if(this->calc_2) (this->calc_2)->RemoveCallBackObserver( this);
	if(this->calc_3) (this->calc_3)->RemoveCallBackObserver( this);
/* user code begins here for FinalizeObject */
/* user code ends here for FinalizeObject */
}
calccon::calccon()
{
	ATKinit;

this->calc_4 = NULL;
this->calc_4View = NULL;
this->calc_5 = NULL;
this->calc_5View = NULL;
this->valenter = NULL;
this->valenterView = NULL;
this->decimal = NULL;
this->decimalView = NULL;
this->digit_0 = NULL;
this->digit_0View = NULL;
this->digit_1 = NULL;
this->digit_1View = NULL;
this->digit_2 = NULL;
this->digit_2View = NULL;
this->digit_3 = NULL;
this->digit_3View = NULL;
this->digit_4 = NULL;
this->digit_4View = NULL;
this->digit_5 = NULL;
this->digit_5View = NULL;
this->digit_6 = NULL;
this->digit_6View = NULL;
this->digit_7 = NULL;
this->digit_7View = NULL;
this->digit_8 = NULL;
this->digit_8View = NULL;
this->digit_9 = NULL;
this->digit_9View = NULL;
this->output = NULL;
this->outputView = NULL;
this->calc_1 = NULL;
this->calc_1View = NULL;
this->calc_2 = NULL;
this->calc_2View = NULL;
this->calc_3 = NULL;
this->calc_3View = NULL;
this->v = NULL;
this->next = NULL;
/* user code begins here for InitializeObject */
/* user code ends here for InitializeObject */
THROWONFAILURE( TRUE);}
/* user code begins here for Other Functions */
/* user code ends here for Other Functions */
