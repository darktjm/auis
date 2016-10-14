#ifndef _chimpview_H_
#define _chimpview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <lpair.H>
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
