/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("sliderstrV.H")
#include <value.H>
#include <sliderstrV.H>


ATKdefineRegistry(sliderstrV, sliderV, NULL);

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
