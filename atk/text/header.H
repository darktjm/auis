#ifndef _header_H_
#define _header_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#define header_HEADER 0
#define header_FOOTER 1

#define header_ltext 0
#define header_ctext 1
#define header_rtext 2
#define header_TEXTS 3

#define ALWAYS_ACTIVE_MODE

#include <dataobject.H>
class header: public  dataobject {
public:
	TRACED_CLASS(header);


        static class header *Create(int  type, char *left, char *center, char *right)  ;
    
	header();
    
    
	static boolean InitializeClass();
	static boolean atk_inited;



     void ObservedChanged(class observable *t,long value);
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
     const char * ViewName()  ;

    virtual void SetHeader(int which, const char *str);

    long  where;
    class text *texts[header_TEXTS];
    boolean active[header_TEXTS];
};
/** @} */
#endif /* _header_H_ */
