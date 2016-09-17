ATK_INTER
#ifndef _hlptextview_H_
#define _hlptextview_H_
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

/* $ACIS$ */

#include <textview.H>
class hlptextview: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
     void GetClickPosition(long position,long numberOfClicks,enum view_MouseAction action,long startLeft,long startRight,long *leftPos,long *rightPos);

  
 
  
    
	static boolean InitializeClass();

    
	hlptextview();
    
	virtual ~hlptextview();
 
  
    inline void SetHyperlinkCheck(boolean newValue) { ((this)->checkForHyperlinks = (newValue)); }
    inline boolean   GetHyperlinkCheck() { return ((this)->checkForHyperlinks); }
    inline char  * GetHyperTopic() { return ((this)->hyperTopic); }
    inline view_MouseAction   GetMouseAction() { return ((this)->action); }

  
    boolean checkForHyperlinks;	    /* TRUE when watching for hypertext styles */
    char *hyperTopic;		    /* Non-NULL if checkForHyperlinks & we found one. */
    enum view_MouseAction action;   /* Action that set hyperTopic */
};

#endif /* _hlptextview_H_ */
