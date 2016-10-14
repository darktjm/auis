#ifndef _viewref_H_
#define _viewref_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
#define viewref_VERSION 1

#include <observable.H>
class environment;
class viewref: public  observable {
public:
	TRACED_CLASS(viewref);


    
	viewref();
    
    static class viewref * Create(const char *viewtype, class dataobject *dataobject)  ;

    char *viewType;
    long viewID;
    class dataobject *dataObject;
    long desw,desh;
    environment *env;
    // Set & Get the environment where this viewref is located in the text.
    inline void SetEnvironment(environment *e) {
        env=e;
    }
    inline environment *Environment() {
        return env;
    }
 
};
/** @} */
#endif /* _viewref_H_ */
