#ifndef _texttroff_H_
#define _texttroff_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
/* Flags for various things texttroff should or should not do. (each should be a power of 2) */
#define	texttroff_Revert 1	/* if set texttroff will revert to the parents state after processing, otherwise the state will be left however it ended up. */
				  
#include <ATK.H>

class texttroff : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static boolean InitializeClass();

    static void WriteSomeTroff(class view *view,class dataobject *dd,FILE *f,int toplevel,unsigned long revert);
    static void WriteTroff(class view *view,class dataobject *dd,FILE * f,int toplevel);

    /* BeginDoc / EndDoc
     These functions are called by standalone views which wish to use the
     troff environment as is established by text.
     They are called if the "toplevel" (fifth) parameter to xxx_Print is TRUE.
     The text generated before calling EndDoc should end with a newline;
     a .br should have been issued to send the last line to the output.
     */
    static void BeginDoc(FILE *f);
    static void EndDoc(FILE *f);


    /* BeginPS / EndPS
     These functions supply the transition for including a postscript rectangle
     in a troff page.  
     BeginPS sets up a postscript coordinate system for an image of size 
     (width x height) in postscript units (1/72 in.) 
     with the origin in the lower left corner.
     At the end of the postscript, the routine EndPS must be called.
     Each line between the two must begin with the two characters:  \!
     (backslash followed by exclamation mark)
     */
    static void BeginPS(FILE *f, long width, long height);
    static void EndPS(FILE *f, long width, long height);

};
/** @} */
#endif /* _texttroff_H_ */
