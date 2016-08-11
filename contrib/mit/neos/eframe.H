#ifndef _eframe_H_
#define _eframe_H_
/* $Author: wjh $ */

/*
	$Disclaimer: 
*Permission to use, copy, modify, and distribute this software and its 
*documentation for any purpose and without fee is hereby granted, provided 
*that the above copyright notice appear in all copies and that both that 
*copyright notice and this permission notice appear in supporting 
*documentation, and that the name of IBM not be used in advertising or 
*publicity pertaining to distribution of the software without specific, 
*written prior permission. 
*                        
*THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
*TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
*HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
*DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
*DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
*OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
*WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $
*/


 

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <mit-copyright.h>

#define EFRAMEMENUS_general       1


#include <view.H>
class eframe: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	eframe();
      
	virtual ~eframe();
      
	static boolean InitializeClass();

    
       void LinkTree(class view *parent);
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
       void Update();
       class view * Hit(enum view_MouseAction	action,	long x,	long y,	long clicks)  ;
       void PostMenus(class menulist *menulist);
       void PostKeyState(class keystate *ks);
       void ReceiveInputFocus(); 
    
      class frame *frame;
      class menulist *menus;
      class keystate *keys;
      int	menuflags;
};

#endif /* _eframe_H_ */
