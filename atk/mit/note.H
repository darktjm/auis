#ifndef _note_H_
#define _note_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libnote Object note
 *  A dynamically loadable ATK class file.  Contains note, noteview,
 *  icon, iconview, ps, psview, gnucompat, vutils.
 * @{ */
#include <icon.H>
class note : public  icon {
public:
	TRACED_CLASS_NODEST(note);

 
    void SetChild(class dataobject * child);
 
 

	note();

	static boolean InitializeClass();
	static boolean atk_inited;

 
};
/** @} */
#endif /* _note_H_ */
