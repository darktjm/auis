#ifndef _chomp_H_
#define _chomp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <dataobject.H>
/** Data object for loading \ref chompview.
 *  This is a dummy data object whose sole purpose is to use \ref chompview
 *  as an inset.  It stores and saves no data.
 */
class chomp: public  dataobject {
public:
	TRACED_CLASS_NODEST(chomp);

};
/** @} */
#endif /* _chomp_H_ */
