#ifndef _alink_H_
#define _alink_H_
/* ********************************************************************** *\
 * Copyright (c) AT&T Bell Laboratories	1990  -	All Rights Reserved	  *
\* ********************************************************************** */

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
       boolean ReadOtherFormat(FILE *file, const char *fmt, const char *encoding, const char *description)  ;
       long WriteOtherFormat(FILE *file, long writeid, int level, dataobject::otherformat usagetype, const char *boundary)  ;
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
