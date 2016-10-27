#ifndef _arbiterview_H_
#define _arbiterview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter
 * @{ */
#define arbiterview_SEPCHAR ':'

#include <celview.H>
typedef void (*arbiterview_hfptr)(class celview *self, long rock);
class value;
class text;
/** A view to manage \ref arbiter objects.
 *  A \ref celview will try to search its parents for an \ref arbiterview.
 *  Its associated  \ref arbiter object ensures that the \ref celview's
 *  \ref cel has a unique name within its control.  If an \ref arbiterview
 *  is created under the control of another \ref arbiterview, it acts as a
 *  plain \ref celview. (FIXME: is this true?)
 */
class arbiterview: public  celview {
public:
	TRACED_CLASS(arbiterview);


     void ObservedChanged (class observable *changed, long value);
     void SetDataObject(class dataobject *ls);
     ATK  * WantHandler(const char *handlerName)  ;
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void LinkTree(class view *parent);

    virtual const char * GetDataName()  ;
    virtual int registername(class celview *cv,const char *refname)  ;
    virtual void InitCell(class celview *cv);
    virtual void DeleteCell(class celview *cv);
    virtual boolean CreateCon(class text *EditText)  ;
    virtual class celview * lookupname(const char *ViewName)  ;
    virtual void ArbRead(char *frs);
    virtual void InitArbcon();
    virtual long GetArbName(char *buf,long buflen)  ;
    virtual void AddHandler(arbiterview_hfptr handler,long rock);
    virtual boolean InTree()  ;

    inline class celview * GetCurrentCelview() { return (this->currentcelview); }
    inline boolean GetCopyMode() { return (this->CopyMode); }
    inline void SetCopyMode(boolean boolval) { this->CopyMode = boolval; }
    inline boolean GetCelMode() { return (this->CelMode); }
    inline void SetCelMode(boolean boolval) { this->CelMode = boolval;; }

    
	static boolean InitializeClass();

    
    
	arbiterview();
    static class dataobject * GetNamedObject(class view *vw,const char *ObjectName)  ;
    static class view * GetNamedView(class view *vw,const char *ViewName)  ;
    static class celview * GetNamedCelview(class view *vw,const char *ViewName)  ;
    static void SetIgnoreUpdates(class view *vw,boolean val);
    static class arbiterview* GetFirstLink()  ;
    static class arbiterview * FindArbByName(char *str)  ;
    static class arbiterview * FindArb(class view *vw)  ;

    class celview **celviewlist;
    class celview *currentcelview;
    int celcount , celsize;
    class value *obval,*vwval,*objectchoice,*viewchoice,*applicationchoice,*NameChoice,*CelNameVal;
    class text *EditText;
    class arbiterview *next;
    boolean NeedsInit,CopyMode,CelMode;
    arbiterview_hfptr handler;
    long hrock;
};
/** @} */
#endif /* _arbiterview_H_ */
