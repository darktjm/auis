ATK_INTER
#ifndef _text_H_
#define _text_H_
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


 

#define text_VERSION 1

enum textwritestyle {text_DefaultWrite, text_NoDataStream, text_DataStream};

typedef boolean (*text_eefptr)(long rock, class text *self, long pos, class environment * curenv);

#include <simpletext.H>
class text: public  simpletext {
  public:
    virtual ATKregistryEntry *ATKregistry();
    
    void SetReadOnly(boolean ro);
    void SetAttributes(struct attributes *attributes);
    void Clear();
    long GetModified()  ;
    boolean ReplaceCharacters(long pos, long len, 
			char *repStr, long replen)  ;
    void AlwaysReplaceCharacters(long pos, long len, 
			char *repStr, long replen);
    environment *ReplaceWithView(long pos, long len, 
			char *viewtype, class dataobject *dataobject);
    environment *AlwaysReplaceWithView(long pos, long len, 
			char *viewtype, class dataobject *dataobject);
    void LengthChanged(long pos, long len);
    long HandleKeyWord(long pos, char *keyword, FILE *file)  ;
    long HandleCloseBrace(long pos, FILE *file)  ;
    long Read(FILE *file, long id)  ;
    long Write(FILE *file, long writeID, int level)  ;
    long WriteOtherFormat(FILE *file, long writeid, int level, 
			int usagetype, char *boundary)  ;
    long ReadSubString(long pos, FILE *file, boolean quoteCharacters)  ;
    void WriteSubString(long pos, long len, FILE *file, boolean quoteCharacters);
    void ObservedChanged (class observable *changed, long value);
    void AlwaysCopyText(long pos, class simpletext *srctext, long srcpos, long len);
    void AlwaysDeleteCharacters(long position, long size);
    boolean CheckHighBit()  ;

    virtual class viewref * InsertObject(long pos, char *name,char *viewname)  ;
    virtual class environment * AddStyle(long pos, long len, class style *style)  ; 
    virtual class environment * AddView(long pos, char *viewtype, class dataobject *dataobject)  ;
    virtual class environment * AlwaysAddStyle(long pos, long len, class style *style)  ; 
    virtual class environment * AlwaysAddView(long pos, char *viewtype, class dataobject *dataobject)  ;
    virtual void SetEnvironmentStyle(class environment *envptr, class style *styleptr);
    virtual void SetGlobalStyle(class style *styleptr);
    virtual class style * GetGlobalStyle()  ;
    virtual long ReadTemplate(char *templateName, boolean inserttemplatetext)  ;
    virtual class viewref * FindViewreference(long pos, long len)  ;
    virtual long AlwaysInsertFile(FILE * file,const char *filename, long position)  ;
    virtual long InsertFile(FILE * file,const char *filename, long position)  ;
    virtual void ClearCompletely();
    virtual class environment * EnumerateEnvironments(long pos,long len,text_eefptr callBack,long rock)  ;
    virtual void SetBaseTemplateName(char *name);
    virtual class environment * AlwaysWrapViewChar(long pos, char *view, class dataobject *dobj)  ;
    virtual boolean CopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len)  ;
    virtual void AlwaysCopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len);
    

    inline void SetExportEnvironments(boolean val) { (this->exportEnvs = val); }
    inline boolean GetExportEnvironments() { return (this->exportEnvs); }
    inline class stylesheet * GetStyleSheet() { return (this->styleSheet); }
    inline void SetWriteAsText(boolean TorF)  { ((this)->WriteAsText = TorF); }
    inline boolean GetWriteAsText()  { return ((this)->WriteAsText); }
    inline void SetCopyAsText(boolean TorF)  { ((this)->CopyAsText = TorF); }
    inline boolean GetCopyAsText()  { return ((this)->CopyAsText); }
    inline textwritestyle GetWriteStyle() { return ((this)->writeStyle); }
    inline void SetWriteStyle(enum textwritestyle style) { ((this)->writeStyle = style); }
    inline char  * GetBaseTemplateName() { return ((this)->templateName); }
    
    virtual ~text();
    text();

    static void ApplyEnvironment(struct text_statevector *sv,
			style *defaultStyle, environment *env);
    static void InitStateVector(struct text_statevector *sv);
    static void FinalizeStateVector(struct text_statevector *sv);
    
    // ApplyEnvironmentsToStyle applies to dest 
    // all the style environments at or above env.
    static void ApplyEnvironmentsToStyle(environment *env, style *dest);

    class environment *rootEnvironment;
    class stylesheet *styleSheet;
    char *templateName; /* Template to read if uninterpreted document. */
    class viewref *currentViewreference;
    boolean exportEnvs;
    boolean executingGetModified;
    boolean WriteAsText,CopyAsText;
    enum textwritestyle writeStyle;
    class dataobject **objs;
    long nobjs, objssize;
};

#endif /* _text_H_ */
