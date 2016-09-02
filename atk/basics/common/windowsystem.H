ATK_INTER
#ifndef _windowsystem_H_
#define _windowsystem_H_
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


 

/* Window system package.
 * Provides an abstraction for dealing with multiple window management systems
 * using a singe executable.
 */

#include <ATK.H>
class im;
class graphic;
class cursor;
class fontdest;
class colormap;
class color;
class iddimage;
class image;
class windowsystem : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        virtual class im * CreateIM()  ;
        virtual class graphic * CreateGraphic()  ;
        virtual class cursor * CreateCursor()  ;
        virtual class fontdesc * CreateFontdesc()  ;
	virtual struct offscrwin * CreateOffScreenWindow(char * host, long width, long height)  ;
	virtual class colormap * CreateColormap(class im *im)  ;
	virtual class color * CreateColor(char *name, unsigned int r, unsigned int g, unsigned int b)  ;
        virtual void FlushAllWindows();
	virtual boolean HandleFiles(long WaitTime, boolean beCheap)  ;
	virtual iddimage *CreateDDImage(image *i, colormap **map);
};

#endif /* _windowsystem_H_ */
