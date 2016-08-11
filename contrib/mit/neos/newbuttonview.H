#ifndef _newbuttonview_H_
#define _newbuttonview_H_
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
 *   For full copyright information see:'mit-copyright.h'     *
 *  Adapted from ATK pushbuttonview by Nick Williams, July 1989
 *************************************************************/

#include <mit-copyright.h>

#include <view.H>
class newbuttonview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	static boolean InitializeClass();

    
	newbuttonview();
    
	virtual ~newbuttonview();
  	
     enum view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long * desired_width, long * desired_height)  ;
     void GetOrigin(long width, long height, long * originX, long * originY);
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)    ;
  
    virtual void Enable(boolean want_to_enable);
    virtual boolean IsEnabled()  ;
  
    short lit;
    short depth;
    char *bfont;
    int bfontsize;
    class cursor *cursor;
    boolean enabled;
};


#endif /* _newbuttonview_H_ */
