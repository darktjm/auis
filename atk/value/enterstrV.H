ATK_INTER
#ifndef _enterstrV_H_
#define _enterstrV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


 
#include <buttonV.H>
class enterstrV : public  buttonV {
public:
	virtual ATKregistryEntry *ATKregistry();


    class valueview * DoHit(enum view_MouseAction type, long x,long y,long hits)  ;
    void ObservedChanged( class observable * observed, long status );
    void LinkTree(class view *parent);
   void DrawButtonText(char *text,long len,struct rectangle *rect,struct rectangle*rect2,boolean pushd);
 class view * Hit(enum view_MouseAction type, long x, long y, long numberOfClicks)
     ;


   
	enterstrV();
   
	virtual ~enterstrV();

    class entertext *etext;
    class etextview *etextviewp;
};


#endif /* _enterstrV_H_ */
