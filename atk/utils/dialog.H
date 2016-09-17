ATK_INTER
#ifndef _dialog_H_
#define _dialog_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <sbutton.H>
#define dialog_DS_VERSION 5

#include <dataobject.H>

class dialog : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	dialog();
    
	virtual ~dialog();
    
     const char * ViewName()  ;
     long Read (FILE *fp, long id)  ;
     long Write (FILE *fp, long id, int level)  ;


    inline int GetStyle() { return (sbutton::GetStyle(this->prefs)); }
    inline class text * GetText() { return ((this)->textp); }
    inline class sbutton * GetButtons() { return ((this)->buttons); }
    inline sbutton_hitfptr GetHitFunc() { return ((this)->buttons->hitfunc); }
    inline struct sbutton_prefs * GetPrefs() { return ((this)->prefs); }


    virtual void SetButtons(class sbutton *buttons);
    virtual const char * GetForeground()  ;
    virtual const char * GetBackground()  ;
    virtual void SetText(class text *text);
    

    class text *textp;
    class sbutton *buttons;
    struct sbutton_prefs *prefs;
};


#endif /* _dialog_H_ */
