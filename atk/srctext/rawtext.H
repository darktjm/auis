#ifndef _rawtext_H_
#define _rawtext_H_
/* File rawtext.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1992-1995.  All rights reserved.

   rawtext.ch: Text subclass specialized for dealing with initfiles and other style-less text. */

#include <text.H>
class rawtext: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


  
     boolean InsertCharacters(long pos, const char *str, long len)  ; /*RSK92overstrike*/
     void SetAttributes(struct attributes *atts);
     long Write(FILE *file, long writeID, int level)  ;

  
    virtual boolean JustInsertCharacters(long pos,const char *str,long len)  ; /*RSK92overstrike*/
    virtual void OverstrikeAChar(long pos); /*RSK92overstrike*/

  
    
	rawtext();
    
	virtual ~rawtext();

  
    inline boolean   PutNewlineAtEOF() { return ((this)->NewlineEOF); }
    inline boolean   IsInOverstrikeMode() { return ((this)->OverstrikeMode) /*RSK92overstrike*/; }
    inline void ChangeOverstrikeMode(boolean newValue) { (((this)->OverstrikeMode) = (newValue)) /*RSK92overstrike*/; }

  
    boolean NewlineEOF;
    boolean OverstrikeMode; /*RSK92overstrike*/ /*ideally, this should be associated with the view; unfortunately, the view is not known to srctext_InsertCharacters, which is where the overstriking is being done*/
};

#endif /* _rawtext_H_ */
