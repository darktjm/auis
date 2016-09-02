ATK_INTER
#ifndef _sbttnav_H_
#define _sbttnav_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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


struct groups {
    struct sbutton_prefs *prefs;
    class menulist *ml;
    struct groups *next;
    int prio;
};

#include <sbuttonv.H>

class sbttnav : public  sbuttonv {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean Touch(int ind, enum view_MouseAction action)  ;
     void PostMenus(class menulist *ml);

    
	static boolean InitializeClass();

    
	sbttnav();
    
	virtual ~sbttnav();

    class menulist *ml;
    struct groups *groups;
    int groupcount;
    int dragging;
    class cursor *cursor;
};


#endif /* _sbttnav_H_ */
