#ifndef _chimpview_H_
#define _chimpview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <lpair.H>
class chimpview: public lpair {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	chimpview();
    
       void SetDataObject(class dataobject *d);
       void LinkTree(class view *parent);
       
      class chlistview *lv;
      class enodeview *env;
};

#endif /* _chimpview_H_ */
