#ifndef _t822view_H_
#define _t822view_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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


#include <messages.H>
class t822view: public  messages {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       void DeleteApplicationLayer(class view *scrollbar);
       void SetDataObject(class dataobject *dob);
       void ObservedChanged(class observable *changed, long value);
       void PrintPSDoc(FILE  *outfile, long  width, long  height);
    
      virtual void ShowHelp(const char *text);
      virtual void SetCaptions(class captions *cap);
      virtual class captions * NewCaptionsInNewWindow()  ;
      virtual class captions * GetCaptions()  ;
    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	t822view();
      
	virtual ~t822view();
    
      class captions *mycaps;
      class keystate *myks;
      class menulist *myml;
      boolean PriorReadOnliness;	/* for ESC-~ */
};

#endif /* _t822view_H_ */
