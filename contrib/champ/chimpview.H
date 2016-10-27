#ifndef _chimpview_H_
#define _chimpview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <lpair.H>
/** A view for editing event calendars.
 *  This view provides a button list on the right and a list of events
 *  in a text area on the left.  The description is edited on the left,
 *  and the event dates are edited by clicking buttons on the right.
 */
class chimpview: public lpair {
public:
	TRACED_CLASS_NODEST(chimpview);

    
      
	chimpview();
    
       void SetDataObject(class dataobject *d);
       void LinkTree(class view *parent);
       
      class chlistview *lv;
      class enodeview *env;
};
/** @} */
#endif /* _chimpview_H_ */
