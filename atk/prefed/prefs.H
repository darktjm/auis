ATK_INTER
#ifndef _prefs_H_
#define _prefs_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

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



#include <list.H>
#include <prefval.H>
#include <mark.H>
#include <style.H>
#include <atom.H>

struct prefdesc {
    char *name;
    char *app;
    char *group;
    char *seps;
    char *cond;
    char *val;
    class prefval *obj;
    class list *defviews;
    class list *prevlines;
    enum prefval_Type type;
    long low, high;
    long listsize;
    boolean writetype;
    boolean writeviews;
    char *views;
    class mark *mark;
    boolean shadow;
    boolean indefs;
    int order;
    boolean expert;
    long helppos;
    boolean freehelp;
    boolean freeobject;
    class mark *pm;
};

struct prefline {
    class list *prevlines;
    char *type;
    class list *vl;
    char *app;
    char *name;
    char *group;
    char *cond;
    char *val;
    char *views;
    boolean shadow;
    long low, high;
    long helppos;
    int expert;
};

struct prefgroup {
    long grouphelp;
    char *name;
};

enum style_type {
    PrefsSubsection,
    PrefsGroupname
};

struct hstyles {
    long pos;
    long len;
    enum style_type type;
};

enum prefs_SortType {
    prefs_Name,
    prefs_App,
    prefs_Group,
    prefs_Order,
    prefs_MaxSortType
};

#define prefs_DS_VERSION 1

#include <text.H>
class prefs : public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	prefs();
    
	virtual ~prefs();
    static char * TranslateViewName(char *name)  ;

    virtual long ReadDataPart()  ;
    virtual void Sort(enum prefs_SortType sortby, boolean perm);
    virtual struct prefdesc * DuplicatePref(struct prefdesc *pd, char *newapp, char *newcond)  ;
    virtual void DeletePref(struct prefdesc *pd);
    virtual long WritePlain(FILE *fp, long id, int level)  ;
    virtual void UpdateText();
    virtual void UpdateOneInText(struct prefdesc *pd);
    virtual long ReScan()  ;

     void ObservedChanged(class observable *changed, long val);
     char * ViewName()  ;
     long Write(FILE *fp, long writeID, int level)  ;
     long Read(FILE *fp, long id)  ;


    inline void SetReadingDefaults(boolean rdefaults) { (this->readingdefaults=(rdefaults)); }

    class list *prefsp;
    boolean readingdefaults;
    struct prefline lastline;
    enum prefs_SortType sortby;
    char *lastheader;
    class style *hstyle;
    class style *pstyle;
    char *lastgroup;
    class list *categories;
    boolean sane;
    class text *help;
    long version;
    struct hstyles *llist;
    long llistsize, llistcnt;
    long maxorder;
    boolean selfmod;
};

#endif /* _prefs_H_ */
