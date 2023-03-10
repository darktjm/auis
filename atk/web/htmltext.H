#ifndef _htmltext_H_
#define _htmltext_H_
/** \addtogroup libhtmltext Object htmltext
 *  A dynamically loadable ATK class file.  Contains htmltext, htmltextview,
 *  attlist, ez2htmlapp, hidden, hiddenview, hr, hrv, htablev, htmlenv,
 *  htmlform, htmlimagev, urlbutton, urlbuttonview, wbuttonv, webapp,
 *  webcom, weblink, weblinkview, web, webview.
 * @{ */
/* File htmltext.H created by Ward Nelson
       (c) Copyright IBM Corp 1995.  All rights reserved. 

    htmltext, an object for reading and writing HTML+ formatted files. */
#include <andrewos.h>
class style;
class attlist;
class htmltext;
class htmlenv;
class dataobject;
class table;
class image;
struct chunk;

enum ListType { 
		listtype_UNORDERED, 
		listtype_ORDERED, 
		listtype_DEFINITION 
};

enum AnchorType {NotAnchor, Anchor, AnchorTarget, IglAnch};

struct htmltaginfo {
    style *thestyle;
    attlist *atts;
    long startpos;
    enum AnchorType anchorcode;
    char *tagid;
};

struct namelist {
    char *name;
    char *codes;
    struct namelist *next;
};

typedef union {
    long integer;
    double real;
    const char *cstr;
    pointer obj;
    procedure f;
    char c;
    int i;
    boolean b;
} arbval;

/* htmlefptr - type for functions passed to EnumerateInsets.
	If function returns TRUE, 	the enumeration halts and
	returns the position of the inset most recently passed.
	For an error, EnumerateInsets returns -9 for an error
	and -1 if the function returns FALSE for all insets.

	self - the htmltext being scanned
	pos - the location of the embedded data object
	dobj - the dataobject at pos
	rock - arbitrary data for use by function
	needsgif - TRUE if htmltext cannot process this inset type	
*/
typedef boolean (*htmlefptr)(htmltext *self, long pos, htmlenv *env, dataobject *dobj, arbval rock);

#include <text.H>
class htmltext: public text {
  public:
    TRACED_CLASS(htmltext);
    htmltext();
    static boolean InitializeClass();

    long ReadSubString(long pos, FILE *file, 
		boolean quoteCharacters);
    void WriteSubString(long pos, long len, FILE *file, 
		boolean quoteCharacters);
    long Read(FILE *file, long id);
    long Write(FILE *file, long writeID, int level);
    void Clear();
    void ClearCompletely();
    long ReadTemplate(const char *templateName, boolean inserttemplatetext);
    virtual htmlenv *AddImage(long pos, const char *file,
		attlist  *atts=NULL);
    virtual void RenumberList(long pos, enum ListType ltype, 
		htmlenv *env);
    virtual htmlenv *AddDingbat(long pos, 
		enum ListType ltype, htmlenv *env);
    virtual long FindDestId(char *dest);
    virtual const char *GetAnchor(long pos);
    virtual void ShowComments(boolean viewcomments);
    virtual const char *GififyInset(long pos, htmlenv *env, const char *filename);
		// filename is path and name of gif file (w/  .gif)
		// returns static string with attributes:
		//		"WIDTH=ddd, HEIGHT=ddd"
		// 	or error string starting with "*"
		// caller must revise the text with AddImage
    virtual long EnumerateInsets(htmlefptr f, arbval rock);
    virtual const char *GetURL(long pos, long x=0, long y=0);
    virtual void ImbedTextInTable(table *t, chunk *c);
    inline const char *Title() {return title;}
    virtual long FindNamedAnchor(const char *target, 
		long start=0, long len=-1);
    virtual image *GetImage(const char *src, attlist *atts);
    boolean readonly;
    char *title;
};
/** @} */
#endif /* _htmltext_H_ */
