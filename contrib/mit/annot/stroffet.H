#ifndef _stroffet_H_
#define _stroffet_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libstroffet Object stroffet
 *  A dynamically loadable ATK class file.  Contains stroffet, stroffetview.
 * @{ */
#include <icon.H>
class stroffet : public  icon {
public:
	TRACED_CLASS_NODEST(stroffet);

 
    void SetChild(class dataobject * child);
 
 

	stroffet();

	static boolean InitializeClass();
	static boolean atk_inited;

 
};

/** @} */
#endif /* _stroffet_H_ */
