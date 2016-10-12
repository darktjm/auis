#ifndef _strinput_H_
#define _strinput_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libatkutils
 * @{ */
/* Changed June 1990 njw */
/* A simple string input view, with a fenced prompt */

#include <view.H>

class strinput : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	strinput();
  
	virtual ~strinput();
  
	static boolean InitializeClass();


   void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
   class view * Hit(enum view::MouseAction action, long x, long y, long clicks)  ;
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
/** @} */
#endif /* _strinput_H_ */
