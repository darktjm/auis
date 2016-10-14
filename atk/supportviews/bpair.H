#ifndef _bpair_H_
#define _bpair_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews Object supviews
 *  A statically loadable ATK class file.  Contains bpair, label, labelview,
 *  lpair, lprruler, lprrulerview, matte, oscroll, palette, panner, sbttnav,
 *  sbutton, sbuttonv, scroll, scrollandpanner, stringtbl, strtblview,
 *  AmenuCard, AmenuCardv, AmenuOption, AmenuOptionv, AmenuOptionCardv,
 *  popupwin.
 * @{ */
/*
bpair.H

	Class definitions for bpair

*/

#define  bpair_PROGRAMMERVERSION    1

#include <lpair.H>

class bpair: public  lpair
{
public:
	TRACED_CLASS(bpair);



	 void FullUpdate(/* struct bpair *self, */ enum view::UpdateType type, 
			long left, long top, long width, long height);



	
	bpair();
	

};
/** @} */
#endif /* _bpair_H_ */
