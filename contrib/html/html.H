#ifndef _html_H_
#define _html_H_
/* HTML text, first run -*-c-*- */
/*
 * Copyright 1993, City University
 * Copyright 1993, 1994, Nick Williams. 
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose is hereby granted without fee, 
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice, this permission notice, and the following 
 * disclaimer appear in supporting documentation, and that the names of 
 * IBM, Carnegie Mellon University, and other copyright holders, not be 
 * used in advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.
 *
 * IBM, CARNEGIE MELLON UNIVERSITY, AND THE OTHER COPYRIGHT HOLDERS 
 * DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT 
 * SHALL IBM, CARNEGIE MELLON UNIVERSITY, OR ANY OTHER COPYRIGHT HOLDER 
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS 
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
/** \addtogroup libhtml Object html
 *  A dynamically loadable ATK class file.  Contains html, htmlview.
 * @{ */
#include <mark.H>

#include <text.H>
class html : public  text {
public:
	TRACED_CLASS(html);

  
     void Clear();
     long Read(FILE* file, long id)  ;
     long ReadSubString(long pos, FILE* file, int quoteCharacters)  ;
     long Write(FILE* file, long writeID, int level)  ;
     void WriteSubString(long pos, long len, FILE* file, int quoteChars);

  
    virtual char*  EnvStart(char* buf, class style *style, int* wantpar, int* br, int* newlines)  ;
    virtual char*  EnvEnd(char* buf, class style *style, int* par, int* br)  ;

    virtual void ChangeTitle(const char* name);
    virtual void ChangeIndexable(int flag);

    virtual void AddLink(long pos, long len, char* uri);
    virtual void AddEntity(long pos, long len, const char* name, char* vars);
      virtual void AddImage(long *pos, char* path);
    virtual char* GetAnchorDest(long pos)  ;
    virtual void GetAttributeList(class environment* env, char** list, int* count);
    virtual void ChangeAttribute(class view* tv, class environment* env, const char* attr, char* value);
    virtual class environment*  GetEntityEnvironment(long pos, class environment* env)  ;
    virtual char* GetAttribute(class environment* env, const char* attr)  ;
    virtual boolean TagItem(long pos, long len, const char* text, const char* styleName, class style* extra)  ;
    virtual int UntagItem(long pos)  ;
      virtual boolean HasErrors()  ;
      virtual void Inform(const char*msg);

  
    
	static boolean InitializeClass();

    
	html();
    

  
    inline const char *  GetTitle() { return ((this)->title); }
    inline int   GetIsIndex() { return ((this)->isindex); }
    inline void SetView(class htmlview *x) { ((this)->view = (x)); }

  
    char* base;     /* The base of the URI */
    char* title;    /* The title of the document */
    long lineNumber; /* when reading, used for displaying error lines */
    int nextid;     /* Next identifier to use for anchor */
    int isindex;    /* If document is searchable */
    int	withinPar;  /* When parsing, indicates if within a paragraph */
    int parbreak;   /* When writing, indicates if parbreak is pending */
    int adornment;  /* When writing, indicates if stuff really exists */
      class buffer* errorBuffer;
    class htmlview* view; /* For debugging, mainly */
    struct entityElement* entities; /* stack of current ents when parsing */
      class style* tagStyle;
      class style* itemStyle;
};

/** @} */
#endif /* _html_H_ */
