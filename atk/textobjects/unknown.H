#ifndef _unknown_H_
#define _unknown_H_
/* Copyright 1994 Carnegie Mellon University All rights reserved. */
/** \addtogroup libtextobjs
 * @{ */
#include <text.H>
class unknown : public  text {
public:
	TRACED_CLASS(unknown);

    
      virtual const char * GetRealClass()  ;
    virtual void SetRealClass(char *rclass);
    
       long Read(FILE *file, long id)  ;
     long Write(FILE *file, long writeid, int level)
       ;
    
      
	unknown();
    
    
      char *realclass;
    class text *odata;
    boolean wrapped;
};
/** @} */
#endif /* _unknown_H_ */
