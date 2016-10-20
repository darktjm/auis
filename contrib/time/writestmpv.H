#ifndef _writestampview_H_
#define _writestampview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <stdio.h>
#include <menulist.H>
#include <cursor.H>
/** \addtogroup libtimeoday
 * @{ */
#include <timeodayview.H>
/** A view for \ref writestamp objects.
 *  Provides actions and menu to change date format.
 */
class writestampview: public  timeodayview {
public:
	TRACED_CLASS(writestampview);

 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	writestampview();
  
};

/** @} */
#endif /* _writestampview_H_ */
