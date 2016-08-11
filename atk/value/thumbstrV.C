/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/thumbstrV.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


#include <andrewos.h>
ATK_IMPL("thumbstrV.H")
#include <value.H>
#include <thumbstrV.H>


ATKdefineRegistry(thumbstrV, thumbV, NULL);
#ifndef NORCSID
#endif


char *thumbstrV::GetValueString()
{
    class value *w = (this)->Value();
    long len,val;
    char **arr;
    if(((len = (w)->GetArraySize()) == 0) || 
	((arr = (w)->GetStringArray()) == NULL) || 
	  ((val = (this)->GetTmpVal())< 0) ||
	  val >= len){
	return (this)->thumbV::GetValueString();
    }
    return(arr[val]);
}
class valueview * thumbstrV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
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
