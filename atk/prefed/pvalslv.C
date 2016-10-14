/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvalslv.H")
#include <math.h>


#include <scroll.H>
#include <prefval.H>
#include "pvalslv.H"

#define zfree(x) do { if(x) { free(x); (x)=NULL;}} while (0)
#define DATA(self) ((class prefval *)(self)->GetDataObject())
#define VALUE(self) ((DATA(self))->GetValue()?(DATA(self))->GetValue()->v.ival:0)


ATKdefineRegistryNoInit(pvalslv, ssliderv);

pvalslv::pvalslv()
{
    (this)->SetLocation( scroll_BOTTOM);
    THROWONFAILURE( TRUE);
}

pvalslv::~pvalslv()
{
}

void pvalslv::GetInfo(struct range  *total , struct range  *seen , struct range  *dot)
{

    total->beg=(DATA(this))->GetRangeLow();
    total->end=(DATA(this))->GetRangeHigh();

    switch(DATA(this)->type) {
	case prefval_Integer:
	    seen->beg=VALUE(this);
	    break;
	default: ;
    }
    seen->end=seen->beg;

    dot->beg = -1;
    dot->end = -1;
}

long pvalslv::WhatIsAt(long  numerator , long  denominator)
{
    long coord;

    coord = numerator * ((DATA(this))->GetRangeHigh() - (DATA(this))->GetRangeLow());
    coord /= denominator;

    if(DATA(this)->type==prefval_Integer) {
	coord+=VALUE(this);
    }
    
    return coord;
}

void pvalslv::SetFrame(long  pos , long  num , long  denom)
{
    struct prefval_value v;
    long coord;

    coord = num * ((DATA(this))->GetRangeHigh() - (DATA(this))->GetRangeLow());
    coord /= denom;

    (DATA(this))->InitValue( &v);
    if(DATA(this)->type==prefval_Integer) {
	v.v.ival=pos-coord;
	if(v.v.ival<(DATA(this))->GetRangeLow()) v.v.ival=(DATA(this))->GetRangeLow();
	else if(v.v.ival>(DATA(this))->GetRangeHigh()) v.v.ival=(DATA(this))->GetRangeHigh();
    }
    (DATA(this))->SetValue( &v);
    (DATA(this))->NotifyObservers( prefval_ValuesChanged);
    (DATA(this))->FreeValue( &v);

}

void pvalslv::Endzone(int  end, enum view::MouseAction  action)
{
    long diff=0;
    struct prefval_value v;

    
    if(DATA(this)->type!=prefval_Integer || !(DATA(this))->GetValue()) return;

    if(action != view::LeftDown && action != view::RightDown) return;

    (DATA(this))->InitValue( &v);
    (DATA(this))->CopyValue( &v, (DATA(this))->GetValue());
    
    if(end== scroll_MOTIFTOPENDZONE) end=scroll_TOPENDZONE;
    else if(end==scroll_MOTIFBOTTOMENDZONE) end=scroll_BOTTOMENDZONE;
    
    if(action == view::LeftDown) {
	if(end==scroll_BOTTOMENDZONE) diff=(DATA(this))->GetRangeHigh()-v.v.ival;
	else if(end==scroll_TOPENDZONE) diff=(DATA(this))->GetRangeLow()-v.v.ival;
    } else {
	if(end==scroll_BOTTOMENDZONE) diff=1;
	else if(end==scroll_TOPENDZONE) diff=(-1);
    }
    v.v.ival+=diff;
    if(v.v.ival<(DATA(this))->GetRangeLow()) v.v.ival=(DATA(this))->GetRangeLow();
    else if(v.v.ival>(DATA(this))->GetRangeHigh()) v.v.ival=(DATA(this))->GetRangeHigh();
 
    (DATA(this))->SetValue( &v);
    (DATA(this))->NotifyObservers( prefval_ValuesChanged);
    (DATA(this))->FreeValue( &v);
}

