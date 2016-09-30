#ifndef _tindex_H_
#define _tindex_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <stdio.h>
#include <ATK.H>
class tindex : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();
    
    static int IndexTerm(class text *d,const char *term,const char **error) ;
    static void WriteIndexList(class text *d,FILE *f);
    static const char * ReadIndexList(class text *d,FILE *f)  ;

    static boolean InitializeClass();

    static void PrintIndex(class view *self, boolean useps);
    static void PreviewIndex(class view *self, boolean useps);
    static int StyleNameIndexLevel(const char *stylename);
    static class text *BuildIndexText(struct textps_locatag *taglist);
};
/** @} */
#endif /* _tindex_H_ */
