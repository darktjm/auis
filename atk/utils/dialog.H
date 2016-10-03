#ifndef _dialog_H_
#define _dialog_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libatkutils Object atkutils
 *  A statically loadable ATK class file.  Contains dialog, dialogv,
 *  strinput.
 * @{ */
#include <sbutton.H>
#define dialog_DS_VERSION 5

#include <dataobject.H>

class dialog : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	dialog();
    
	virtual ~dialog();
    
     const char * ViewName()  ;
     long Read (FILE *fp, long id)  ;
     long Write (FILE *fp, long id, int level)  ;


    inline int GetStyle() { return (sbutton::GetStyle(this->prefs)); }
    inline class text * GetText() { return ((this)->textp); }
    inline class sbutton * GetButtons() { return ((this)->buttons); }
    inline sbutton_hitfptr GetHitFunc() { return ((this)->buttons->hitfunc); }
    inline struct sbutton_prefs * GetPrefs() { return ((this)->prefs); }


    virtual void SetButtons(class sbutton *buttons);
    virtual const char * GetForeground()  ;
    virtual const char * GetBackground()  ;
    virtual void SetText(class text *text);
    

    class text *textp;
    class sbutton *buttons;
    struct sbutton_prefs *prefs;
};
/** @} */
#endif /* _dialog_H_ */
