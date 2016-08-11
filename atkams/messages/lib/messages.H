#ifndef _messages_H_
#define _messages_H_
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


 

#define WHATIAM_UNDEFINED -1
#define WHATIAM_FOLDERS 0
#define WHATIAM_CAPTIONS 1
#define WHATIAM_BODIES 3

#include <textview.H>
class messages: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	messages();
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	virtual ~messages();
    
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       void ObservedChanged(class observable *changed, long change);
    
      virtual void SetWhatIAm(int WhatIAm);
      virtual void ResetFileIntoMenus();
      virtual int AppendOneMessageToFile(int cuid, char *fname, int DoRaw)  ;
    
      class menulist *mymenulist, *mypermmenulist, *fileintomenulist, *expandedmenuslist;
      class keystate *mykeys, *mypermkeys;
      int WhatIAm;
      class foldertreev *folderTree;
      class frame *folderFrame;
};


#endif /* _messages_H_ */
