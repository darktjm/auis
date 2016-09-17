ATK_INTER
#ifndef _frameview_H_
#define _frameview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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


 

#define frameview_VERSION 1

#include <textview.H>

class frameview : public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
         void Update();
         void WantInputFocus(class view *requestor);
         void LoseInputFocus();
         void PostMenus(class menulist *menulistp);
         void PostKeyState(class keystate *keystatep);
	 view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
	 void WantNewSize(class view *requestor);
	 void LinkTree(class view *parent);
	
    
        virtual void SetMessageLine(class framemessage *messageLine);
        virtual class framemessage * GetMessageLine()  ;
	virtual void Return(long key); 
	virtual void Help(long key); 
	virtual void Complete(long key); 
	virtual void SetWantedLines(int lines);
	
    
        
	virtual ~frameview();
        static class frameview * Create(class framemessage *messageLine)  ;
        
	static boolean InitializeClass();

	
	frameview();    

    
        class framemessage *messageLine;
        class keystate *keystatep;
        class menulist *menulistp;
        class mark *transientMark;
	class event *eventp; /* Timer evet to erase transient messages. */
	boolean amLosingInputFocus;
	long lineHeight, lines;
	long wantsize;
	boolean dynamicsize;
	long minlines;
};


#endif /* _frameview_H_ */
