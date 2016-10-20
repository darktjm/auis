#ifndef _writestamp_H_
#define _writestamp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/* #include <time.h> */
#include <fontdesc.H>
#include <event.H>
/** \addtogroup libtimeoday
 * @{ */
#include <timeoday.H>
/** Simple date stamp of last save date.
 *  When written to a stream, this updates its internal date and stores
 *  that, as well.
 *  View with \ref writestampv. */
class writestamp: public  timeoday {
public:
	TRACED_CLASS(writestamp);

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	writestamp();
      
    
       void WriteDataPart(FILE *fp);
       long ReadDataPart(FILE *fp)  ;
       void UpdateTime();
};
/** @} */
#endif /* _writestamp_H_ */
