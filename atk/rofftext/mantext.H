#ifndef _mantext_H_
#define _mantext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup librofftext
 * @{ */
/* mantext.ch: link to rofftext -man */

#include <mark.H>

#include <rofftext.H>
class mantext: public  rofftext {
public:
	TRACED_CLASS_NODEST(mantext);


    
	mantext();
  
     long Read(FILE *file, long id)  ;
  
    virtual long GetLinePos(long line)  ;
    virtual void SetLinePos(long line, long pos);
    virtual void GetFilename(char *filename);
  
    int nLines, nAlloc;
    class mark **lineMark;
    char filename[500];
};
/** @} */
#endif /* _mantext_H_ */
