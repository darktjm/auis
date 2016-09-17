ATK_INTER
#ifndef _calccon_H_
#define _calccon_H_
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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */
 
/* user code ends here for HeaderInfo */
#include <observable.H>
class calccon : public  observable  {
public:
	virtual ATKregistryEntry *ATKregistry();
 

        
	static boolean InitializeClass();

	
	virtual ~calccon();
	
	calccon();
/* user code begins here for classprocedures */
/* user code ends here for classprocedures */

	 void ObservedChanged( class observable * observed, long status );	/* user code begins here for overrides */
/* user code ends here for overrides */

	class value *calc_4;
	class buttonV *calc_4View;
	class value *calc_5;
	class buttonV *calc_5View;
	class value *valenter;
	class enterstrV *valenterView;
	class value *decimal;
	class buttonV *decimalView;
	class value *digit_0;
	class buttonV *digit_0View;
	class value *digit_1;
	class buttonV *digit_1View;
	class value *digit_2;
	class buttonV *digit_2View;
	class value *digit_3;
	class buttonV *digit_3View;
	class value *digit_4;
	class buttonV *digit_4View;
	class value *digit_5;
	class buttonV *digit_5View;
	class value *digit_6;
	class buttonV *digit_6View;
	class value *digit_7;
	class buttonV *digit_7View;
	class value *digit_8;
	class buttonV *digit_8View;
	class value *digit_9;
	class buttonV *digit_9View;
	class value *output;
	class stringV *outputView;
	class value *calc_1;
	class buttonV *calc_1View;
	class value *calc_2;
	class buttonV *calc_2View;
	class value *calc_3;
	class buttonV *calc_3View;
/* user code begins here for classdata */
	char buf[64];
	double val,saveval;
	boolean error,clear;
	long op,dec;
/* user code ends here for classdata */
	class view *v;
	class arbiterview *arbv;
	class calccon *next;
};


#endif /* _calccon_H_ */
