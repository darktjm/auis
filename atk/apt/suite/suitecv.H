#ifndef _suitecv_H_
#define _suitecv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsuite
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Suite-object

MODULE	suitecv.ch

VERSION: 0.0

AUTHOR	TC Peters & GW Keim
 	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Suite-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  11/01/88	Created (GW Keim)

END-SPECIFICATION  ************************************************************/

#include <textview.H>
class suitecv: public  textview  {
public:
	TRACED_CLASS(suitecv);

    
	
	static boolean InitializeClass();

	
	suitecv();
	

	 void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
	 void PostKeyState(class keystate *kstate);
	 void ReceiveInputFocus();
	 void LoseInputFocus();
    
	class keystate *kstate;
	struct suite_item *parent_item;
	class suiteev *parent_EV;
	int debug;
};



/*
    $Log: suitecv.H,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1993/06/10  20:21:09  rr2b
// Great atk_inited removal.
//
# Revision 1.3  1993/06/03  20:27:00  gk5g
# overrode FullUpdate so that we can update the Read/Write text and only change text if the caption has changed.
#
# Revision 1.2  1993/05/18  15:39:01  rr2b
# No changes.
#
# Revision 1.1  1993/05/05  21:32:07  rr2b
# Initial revision
#
*Revision 1.5  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.3  1991/09/12  19:20:30  bobg
Update copyright notice

Revision 1.2  1989/05/08  16:43:18  gk5g
changed references from suiteEV to suiteev

Revision 1.1  89/05/04  12:36:31  gk5g
Initial revision

Revision 1.1  89/04/28  20:26:19  tom
Initial revision

*/
/** @} */
#endif /* _suitecv_H_ */
