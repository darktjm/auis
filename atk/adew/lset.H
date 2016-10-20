#ifndef _lset_H_
#define _lset_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter
 * @{ */
#define STRING 1
#define LONG 2
#define FLOAT 3

#include <dataobject.H>
class lset : public  dataobject {
public:
	TRACED_CLASS_NODEST(lset);


     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
     long GetModified()  ;

    
	static boolean InitializeClass();

    
	lset();

    virtual void InsertObject (const char *name,const char *viewname);
    virtual char * registername(const char *rf)  ;

	int type;
	int pct;
	char dataname[32];
	char viewname[32];
	char refname[64];
    class dataobject *dobj;
    class dataobject *left,*right;
    int application;
    class text *pdoc;
    int revision;
};
/** @} */
#endif /* _lset_H_ */
