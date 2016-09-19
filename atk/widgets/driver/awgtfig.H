/* Copyright 1996 Carnegie Mellon University All rights reserved. */

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
