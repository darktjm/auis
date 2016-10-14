#ifndef _enodeview_H_
#define _enodeview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#define BUTT_SYS 0
#define BUTT_YEAR 1
#define BUTT_MON 2
#define BUTT_DAY 3
#define BUTT_WKDAY 4
#define BUTT_WKDAYSELECT 5
#define BUTT_HR 6
#define BUTT_MIN 7
#define BUTT_ADD 8
#define BUTT_DEL 9
#define NUMBUTTS 10 /* one plus previous */

#include <lpair.H>
class enodeview: public lpair {
public:
	TRACED_CLASS_NODEST(enodeview);

    
      
	enodeview();
    
       void ObservedChanged (class observable *changed, long value);
    
      void SetChimpview(class chimpview *cv);
    
      class butter *butts[NUMBUTTS];
      class chimpview *mychimpview;
};
/** @} */
#endif /* _enodeview_H_ */
