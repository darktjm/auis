#ifndef _cel_H_
#define _cel_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter
 * @{ */
#include <atom.H>
#include <namespace.H>

#define cel_VERSION 1
#define cel_VISIBLE 0
#define cel_INVISIBLE 1
/* allowable args to cel_SetApplication */
#define cel_NORMAL 0
#define cel_APPLICATION 1
#define cel_NOTSET -1
#define cel_VALUE 10
#define cel_NeedsRepost 4242

#define cel_UNDEFINEDVALUE -47474747
#include <dataobject.H>
class cel: public  dataobject {
public:
	TRACED_CLASS(cel);


     void ObservedChanged (class observable *changed, long value);
     short Get( const class atom * property, const class atom ** type, long * rock )
       ;
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
     long GetModified()  ;

    static class cel * Create(char *viewtype, class dataobject *dataobject)  ;
    
	static boolean InitializeClass();

    
	cel();
    

    virtual boolean SetChildObject(class dataobject *newobject,const char *viewName)  ;
    virtual boolean SetObjectByName(const char *dataname)  ;
    virtual class dataobject * GetObject()  ;
    virtual void SetViewName(const char *viewname,boolean UseDefault);
    virtual void SetApplication(int app);
    virtual void InsertObject (class dataobject *newobject,char *dataname,const char *viewname,int usedefaultview);
    virtual const char * SetRefName(const char *refname)  ;
    virtual void UnsetRefName();
    virtual void SetVisibilityBit(int mode);
    virtual boolean SetObject(class dataobject *newobject)  ;
    virtual void SetLinkName(const char *linkname);
    virtual void SetObjectName(const char *dataname) ;
    virtual long WriteLink (FILE *file, long writeid, int level)  ;
    virtual char * GetStringAtt(char *attname,char *buf,long buflen)  ;
    virtual long GetLongAtt(char *attname)  ;
    virtual void SetLongAtt(char *attname,long val);
    virtual void SetStringAtt(char *attname,char *attval);
    virtual long ReadSup (FILE *file, long id)  ;
    virtual long WriteSup (FILE *file, long writeid, int level)  ;
    virtual long ReadFile(FILE *file)  ;
    virtual long InitDefault()  ;

    inline const char * GetRefName() { return (this->refname); }
    inline const char * GetViewName() { return (this->viewType); }
    inline const char * GetObjectName() { return (this->dataType); }
    inline const class atom * GetRefAtom() { return (this->refatm); }
    inline const class atom * GetViewAtom() { return (this->viewatm); }
    inline const class atom * GetObjectAtom() { return (this->dataatm); }
    inline const char * GetLinkName() { return (this->linkname); }
    inline const class atom * GetLinkAtom() { return (this->linkatm); }
    inline boolean GetApplication() { return (this->application); }
    inline class text * GetScript() { return (this->script); }
    inline void SetVisible() { (this)->SetVisibilityBit(cel_VISIBLE); }
    inline void SetInvisible() { (this)->SetVisibilityBit(cel_INVISIBLE); }
    inline boolean Visible() { return (this->mode == cel_VISIBLE); }
    inline void SetNoSave(boolean val) { (this->NoSave = val); }
    inline void SetDefaultStream(char *S) { (this->defaultStream = S); }
    inline void SetInitFile(char *S) { (this->initfile = S); }
    inline void SetWriteChild(boolean TF) { (this->WriteChild = TF); }
    inline char * GetDefaultStream() { return (this->defaultStream); }
    inline char * GetInitFile() { return (this->initfile); }
    inline boolean GetWriteChild() { return (this->WriteChild ); }
    inline long GetReadCount() { return (this->count); }
    inline class cel * GetNextChain() { return ((this)->chain); }
    inline class arbiter * GetArbiter() { return ((this)->ab); }
    inline void SetArbiter(class arbiter *AA) { ((this)->ab = (AA)); }

    const class atom *viewatm,*dataatm,*refatm,*linkatm;
    const char *viewType, *dataType, *refname, *linkname;
    long viewID;
    class dataobject *dataObject;
    long desw,desh;
    int application;
    boolean readfromfile,usedefaultview,NoSave;
    class text *script;
    int mode;
    char *defaultStream,*initfile;
    long count;
    boolean WriteChild;
    class cel *chain;
    class arbiter *ab;
};
/** @} */
#endif /* _cel_H_ */
