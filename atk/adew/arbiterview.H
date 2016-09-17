ATK_INTER
#ifndef _arbiterview_H_
#define _arbiterview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


 
#define arbiterview_SEPCHAR ':'

#include <celview.H>
typedef void (*arbiterview_hfptr)(class celview *self, long rock);
class value;
class text;
class arbiterview: public  celview {
public:
	virtual ATKregistryEntry *ATKregistry();


     void ObservedChanged (class observable *changed, long value);
     void SetDataObject(class dataobject *ls);
     ATK  * WantHandler(const char *handlerName)  ;
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
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

    
	virtual ~arbiterview();
    
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


#endif /* _arbiterview_H_ */
