/* Copyright 1996 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
  */

#include <awidget.H>

class AWgtFig : public AWidget {
  public:
	virtual ATKregistryEntry *ATKregistry();

	AWgtFig();
	virtual ~AWgtFig();

	ASlotFigure initialImage;  // a figure to start from

	// The initialImage figure is copied for AWgtFigView
	// It can be revised at any time by action functions.
	// (The image does NOT include the border.)
	// When the size changes, one of the following is called.
	ASlotFunction prepImage;   	// revise figure the first time
		// If initialImage is empty, prepImage must insert
		// all image components in the figure.
	ASlotFunction reviseImage;  	// revise figure subsequent times

	ASlotBool passMouse;  // (default FALSE)
	// If True, mouse hits are passed to viewer.
	// otherwise call one of Mouse{Left,Right}{Down,Move,Up}
};
