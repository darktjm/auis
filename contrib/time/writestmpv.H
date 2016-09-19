#ifndef _writestampview_H_
#define _writestampview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <stdio.h>
#include <menulist.H>
#include <cursor.H>

#include <timeodayview.H>
class writestampview: public  timeodayview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	writestampview();
  
	virtual ~writestampview();
};


#endif /* _writestampview_H_ */
