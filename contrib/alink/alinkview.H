#ifndef _alinkview_H_
#define _alinkview_H_
/* ********************************************************************** *\
 * Copyright (c) AT&T Bell Laboratories	1990  -	All Rights Reserved	  *
\* ********************************************************************** */

#include <pshbttnv.H>
class alinkview: public  pushbuttonview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	alinkview();
  
	virtual ~alinkview();
 
  virtual void RecordAudio();
 	
   void PostMenus(class menulist *ml);
 
  class menulist *ml;
};

#endif /* _alinkview_H_ */
