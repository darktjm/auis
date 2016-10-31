/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("thumbstrV.H")
#include <value.H>
#include <thumbstrV.H>


ATKdefineRegistryNoInit(thumbstrV, thumbV);

const char *thumbstrV::GetValueString()
{
    class value *w = (this)->Value();
    long len,val;
    const char * const *arr;
    if(((len = (w)->GetArraySize()) == 0) || 
	((arr = (w)->GetStringArray()) == NULL) || 
	  ((val = (this)->GetTmpVal())< 0) ||
	  val >= len){
	return (this)->thumbV::GetValueString();
    }
    return(arr[val]);
}
class valueview * thumbstrV::DoHit( enum view::MouseAction  type,long  x,long  y,long  hits )
{
    int mv = ((this)->Value())->GetArraySize();
    class thumbV *sf = (class thumbV *) this;
    if(mv > 0 && mv != sf->maxval + 1 ){
	int diff ;
	sf->maxval = mv -1 ;
	diff = sf->maxval - sf->minval;
	if(diff < 20) sf->granular = 6;
	else if(diff < 50) sf->granular = 4;
	else if(diff < 100) sf->granular = 2;
	else sf->granular = 0;
    }
    return (this)->thumbV::DoHit( type,x,y,hits );
}

// from thumbV:
#define FUDGE2 8

view::DSattributes thumbstrV::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    int ret = thumbV::DesiredSize(width, height, pass, desiredwidth, desiredheight);
    long len;
    const char * const *arr;
    class value *w = this->Value();
    if(!this->fontname || !w || ((len = w->GetArraySize()) == 0) || 
	((arr = w->GetStringArray()) == NULL))
	return (view::DSattributes)ret;
    for(long i = 0; i < len; i++) {
	long twidth, theight;
	boldfont->StringSize(GetDrawable(), arr[i],&twidth,&theight);
	twidth += 2 * (x + FUDGE2);
	if(twidth > *desiredwidth) {
	    ret &= ~view::WidthSmaller;
	    *desiredwidth = twidth;
	}
    }
    return (view::DSattributes)ret ;
}
