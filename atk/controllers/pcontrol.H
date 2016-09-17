ATK_INTER
#ifndef _pcontrol_H_
#define _pcontrol_H_
/* user code begins here for HeaderInfo */
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/* $Disclaimer: 
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
//  $ */

/* user code ends here for HeaderInfo */

#include <ATK.H>
class pcontrol : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        pcontrol();
	
	static boolean InitializeClass();

/* user code begins here for classprocedures */
/* user code ends here for classprocedures */

	class value *kb_2;
	class pianoV *kb_2View;
	class value *kb_3;
	class pianoV *kb_3View;
	class value *replay;
	class buttonV *replayView;
	class value *volume;
	class fourwayV *volumeView;
	class value *nt_10;
	class buttonV *nt_10View;
	class value *clear;
	class buttonV *clearView;
	class value *nt_11;
	class buttonV *nt_11View;
	class value *nt_0;
	class buttonV *nt_0View;
	class value *nt_1;
	class buttonV *nt_1View;
	class value *mode;
	class onoffV *modeView;
	class value *nt_2;
	class buttonV *nt_2View;
	class value *nt_3;
	class buttonV *nt_3View;
	class value *nt_4;
	class buttonV *nt_4View;
	class value *nt_5;
	class buttonV *nt_5View;
	class value *nt_6;
	class buttonV *nt_6View;
	class value *nt_7;
	class buttonV *nt_7View;
	class value *nt_8;
	class buttonV *nt_8View;
	class value *nt_9;
	class buttonV *nt_9View;
	class value *speed;
	class sliderV *speedView;
	class value *Read;
	class buttonV *ReadView;
	class value *noteout;
	class stringV *noteoutView;
	class value *undo;
	class buttonV *undoView;
	class text *score;
	class textview *scoreView;
	class arbiter *ab1;
	class arbiterview *ab1View;
	class lset *ab2;
	class lsetview *ab2View;
	class value *rest;
	class buttonV *restView;
	class value *kb_0;
	class pianoV *kb_0View;
	class value *Save;
	class buttonV *SaveView;
	class value *kb_1;
	class pianoV *kb_1View;
/* user code begins here for classdata */
	class value *duration;
	long lastnoteval,lastlen;
/* user code ends here for classdata */
	class view *v;
	class pcontrol *next;
};

#endif /* _pcontrol_H_ */
