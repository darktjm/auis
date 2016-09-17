ATK_INTER
#ifndef _rectlist_H_
#define _rectlist_H_
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


 

#define rectlist_VERSION 1

#include <ATK.H>
#include <color.H>
#include <rect.h>
typedef color tempcolor;
struct spotcolor : public rectangle {
    long ox, oy;
    tempcolor color;
    tempcolor bgcolor;
    spotcolor *next, *nextgroup;
};

class rectlist : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void ResetList();
    static void AddOldRectangle(long bottom, long top, long left, long right);
    static void AddNewRectangle(long bottom, long top, long left, long right, long startScan);
    static void InvertRectangles(class view *vPtr, spotcolor *spots=NULL);
    static struct rectangle &Bounds();
};

#endif /* _rectlist_H_ */
