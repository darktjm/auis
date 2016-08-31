ATK_INTER
#ifndef _tindex_H_
#define _tindex_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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


#endif /* _tindex_H_ */
