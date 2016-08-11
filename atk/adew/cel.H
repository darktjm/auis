ATK_INTER
#ifndef _cel_H_
#define _cel_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

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
	virtual ATKregistryEntry *ATKregistry();


     void ObservedChanged (class observable *changed, long value);
     short Get( class atom * property, class atom ** type, long * rock )
       ;
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
     long GetModified()  ;

    static class cel * Create(char *viewtype, class dataobject *dataobject)  ;
    
	static boolean InitializeClass();

    
	cel();
    
	virtual ~cel();

    virtual boolean SetChildObject(class dataobject *newobject,char *viewName)  ;
    virtual boolean SetObjectByName(char *dataname)  ;
    virtual class dataobject * GetObject()  ;
    virtual void SetViewName(char *viewname,boolean UseDefault);
    virtual void SetApplication(int app);
    virtual void InsertObject (class dataobject *newobject,char *dataname,char *viewname,int usedefaultview);
    virtual char * SetRefName(char *refname)  ;
    virtual void UnsetRefName();
    virtual void SetVisibilityBit(int mode);
    virtual boolean SetObject(class dataobject *newobject)  ;
    virtual void SetLinkName(char *linkname);
    virtual void SetObjectName(char *dataname) ;
    virtual long WriteLink (FILE *file, long writeid, int level)  ;
    virtual char * GetStringAtt(char *attname,char *buf,long buflen)  ;
    virtual long GetLongAtt(char *attname)  ;
    virtual void SetLongAtt(char *attname,long val);
    virtual void SetStringAtt(char *attname,char *attval);
    virtual long ReadSup (FILE *file, long id)  ;
    virtual long WriteSup (FILE *file, long writeid, int level)  ;
    virtual long ReadFile(FILE *file)  ;
    virtual long InitDefault()  ;

    inline char * GetRefName() { return (this->refname); }
    inline char * GetViewName() { return (this->viewType); }
    inline char * GetObjectName() { return (this->dataType); }
    inline class atom * GetRefAtom() { return (this->refatm); }
    inline class atom * GetViewAtom() { return (this->viewatm); }
    inline class atom * GetObjectAtom() { return (this->dataatm); }
    inline char * GetLinkName() { return (this->linkname); }
    inline class atom * GetLinkAtom() { return (this->linkatm); }
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

    class atom *viewatm,*dataatm,*refatm,*linkatm;
    char *viewType, *dataType, *refname, *linkname;
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

#endif /* _cel_H_ */
