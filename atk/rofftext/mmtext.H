#ifndef _mmtext_H_
#define _mmtext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup librofftext
 * @{ */
/* mmtext.ch: link to rofftext -mm */

#include <mark.H>

#include <rofftext.H>
class mmtext: public  rofftext {
public:
	TRACED_CLASS_NODEST(mmtext);


    
	mmtext();
  
     long Read(FILE *file, long id)  ;
  
    virtual long GetLinePos(long line)  ;
    virtual void SetLinePos(long line, long pos);
    virtual void GetFilename(char *filename);
  
    int nLines, nAlloc;
    class mark **lineMark;
    char filename[500];
};
/** @} */
#endif /* _mmtext_H_ */
