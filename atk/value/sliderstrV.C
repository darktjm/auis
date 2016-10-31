/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("sliderstrV.H")
#include <value.H>
#include <sliderstrV.H>


ATKdefineRegistryNoInit(sliderstrV, sliderV);

const char *sliderstrV::GetValueString()
{
    class value *w = (this)->Value();
    long len,val;
    const char * const *arr;
    if(((len = (w)->GetArraySize()) == 0) || 
	((arr = (w)->GetStringArray()) == NULL) || 
	  ((val = (this)->GetTmpVal())< 0) ||
	  val >= len)
	return (this)->sliderV::GetValueString();
    return(arr[val]);
}

// from sliderV.C
#define FUDGE2 4

view::DSattributes sliderstrV::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    int ret = sliderV::DesiredSize(width, height, pass, desiredwidth, desiredheight);
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
