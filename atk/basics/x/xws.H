ATK_INTER
#ifndef _xws_H_
#define _xws_H_
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


 

/* X implementation of the windowsystem class.
 */

#include <windowsystem.H>
class iddimage;
class cursor;
class fontdesc;
class graphic;
class im;
class xws : public  windowsystem {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        virtual class cursor * CreateCursor()  ;
        virtual class fontdesc * CreateFontdesc()  ;
        virtual class graphic * CreateGraphic()  ;
        virtual class im * CreateIM()  ;
	virtual struct offscrwin * CreateOffScreenWindow(char * host, long width, long height)  ;
        virtual void FlushAllWindows();
        virtual boolean HandleFiles(long WaitTime, boolean beCheap)  ;
	virtual iddimage *CreateDDImage(image *i, colormap **map);

};

#endif /* _xws_H_ */
