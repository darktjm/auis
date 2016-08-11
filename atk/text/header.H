ATK_INTER
#ifndef _header_H_
#define _header_H_
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


#define header_HEADER 0
#define header_FOOTER 1

#define header_ltext 0
#define header_ctext 1
#define header_rtext 2
#define header_TEXTS 3

#define ALWAYS_ACTIVE_MODE

#include <dataobject.H>
class header: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


        static class header *Create(int  type, char *left, char *center, char *right)  ;
    
	header();
    
	virtual ~header();
    
	static boolean InitializeClass();
	static boolean atk_inited;



     void ObservedChanged(class observable *t,long value);
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
     char * ViewName()  ;

    virtual void SetHeader(int which, char *str);

    long  where;
    class text *texts[header_TEXTS];
    boolean active[header_TEXTS];
};

#endif /* _header_H_ */
