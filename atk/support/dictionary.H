ATK_INTER
#ifndef _dictionary_H_
#define _dictionary_H_
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

class dictionary : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void Insert(class view *view,char *id, char *object) ;
    static char * LookUp(class view *view,char *id)  ;
    static long CountViews(char *id)  ;
    static long ListViews(char *id,class view **viewarray,long len)  ;
    static long CountRefs(class view *view)  ;
    static long ListRefs(class view *view,char **refarray,long len)  ;
    static void Delete(class view *view,char *id);
    
	static boolean InitializeClass();

};



#endif /* _dictionary_H_ */
