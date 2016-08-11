#ifndef _alink_H_
#define _alink_H_
/* ********************************************************************** *\
 * Copyright (c) AT&T Bell Laboratories	1990  -	All Rights Reserved	  *
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


#include <pushbutton.H>
class alink: public  pushbutton {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	alink();
      
	virtual ~alink();
    
       long Read (FILE *file, long id)  ;
       long Write (FILE *file, long writeid, int level)  ;
       boolean ReadOtherFormat(FILE *file, char *fmt, char *encoding, char *description)  ;
       long WriteOtherFormat(FILE *file, long writeid, int level, int usagetype, char *boundary)  ;
       void SetText(char *txt);
    
      virtual void SetAudio(long len, char *audio);
    
      inline char  * GetAudio() { return (this->audio); }
      inline long   GetAudioLength() { return (this->audioLength); }
      inline int   LabelSetP() { return (this->label_set); }
    
      long audioLength;
      char *audio;
      int label_set;
};

#endif /* _alink_H_ */
