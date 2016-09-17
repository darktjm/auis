ATK_INTER
#ifndef _strinput_H_
#define _strinput_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


/* Changed June 1990 njw */
/* A simple string input view, with a fenced prompt */

#include <view.H>

class strinput : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	strinput();
  
	virtual ~strinput();
  
	static boolean InitializeClass();


   void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
   class view * Hit(enum view_MouseAction action, long x, long y, long clicks)  ;
   void ReceiveInputFocus();
   void LinkTree(class view *parent);

  virtual char * GetInput(int length)  ;
  virtual void SetInput(char *string);
  virtual void ClearInput();
  virtual void SetPrompt(char *string);
  virtual boolean HaveYouGotTheFocus()  ;

  inline boolean IsLinked() { return ((this)->parent && (this)->imPtr); }

  class text *textobj;
  class textview *textv;
  char *prompt;
};

#endif /* _strinput_H_ */
