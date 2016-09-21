#ifndef _icon_H_
#define _icon_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libnote
 * @{ */
#define icon_TitleChanged 34935	/* random integers... */
#define icon_SizeChanged 1894
#define icon_ChildChanged 2435
#define icon_ChangedSomehow 90235

#include <dataobject.H>
class icon : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	icon();
	virtual ~icon();
  
	static boolean InitializeClass();
	static boolean atk_inited;


     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;

    virtual void SetSize(long w,long h);
    virtual void GetSize(long * w, long * h);
    virtual void SetChild(class dataobject * child);
    virtual class dataobject * GetChild()  ;
    virtual void SetTitle(const char * title);
    virtual const char * GetTitle()  ;

    class dataobject * child;
    long width, height;
    char * title;
};
/** @} */
#endif /* _icon_H_ */
