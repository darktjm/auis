#ifndef _stylesheet_H_
#define _stylesheet_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
/* stylesht.H
 * class header file for stylesht.
 *
 */

#define stylesheet_VERSION 1

#include <observable.H>
#include <proctable.H>
#include <stdio.h>

	typedef boolean (*stylesheet_efptr)(class style *self, long data);

class stylesheetInternal : public  traced {
public:
	TRACED_CLASS(stylesheetInternal);
    virtual void FreeStyles();			/* clear out all styles in the stylesheet */
    virtual void Add(class style *styleptr);	/* add a style to the stylesheet */
    virtual void Delete(class style *styleptr);	/* delete a style from the stylesheet */
    virtual class style * Find(const char *name)  ;	/* find a style, given its name */
    virtual class menulist * GetMenuList(proctable_fptr procname, struct ATKregistryEntry  *infotype)  ;
					/* get a menulist for the styles */
    virtual long Read(FILE *fileptr, boolean template_c)  ;
					/* parse the contents of \\define{} */
    virtual void Write(FILE *fileptr);               /* write all externally defined styles */
    virtual void SetTemplateName(char *name);	/* save the name of the template being used */
    virtual char * GetTemplateName()  ;	/* retrieve the name of the template being used */
    virtual class style * EnumerateStyles(/* struct stylesheet *self, */ stylesheet_efptr func, long data) 
		 ;   /* calls func(style, data) for each style in self.  
			The boolean value returned by func is True if the function is
			through enumerating;  EnumerateStyles then returns the last style 
			processed, othewise it returns NULL */
    virtual class style * GetGlobalStyle()  ;


 /* copies the stylesheetInternal object for this stylesheet into
 the stylesheetInternal object pointed to by dest. */
    virtual void Copy(stylesheetInternal *dest);


    stylesheetInternal();

    char *templateName;			/* name of the template used, if any */
    long nstyles;			/* number of styles in this stylesheet */
    long maxStyles;			/* number of styles currently allocated */
    long version;			/* version used for updating menulist */
    class style **styles;		/* array of the styles themselves */
    class menulist *styleMenu;		/* current menulist for the styles */
};

struct stylesheet_maplist;
class stylesheet : public  observable {
  public:
    stylesheetInternal *si;
    TRACED_CLASS(stylesheet);
    inline void PrepareForChangesIfNeeded() {
	if(si->ReferenceCount()>1) {
	    PrepareForChanges();
	}
    }
    inline void UpdateCache() {
	nstyles=si->nstyles;
	maxStyles=si->maxStyles;
	version=si->version;
	styles=si->styles;
	styleMenu=si->styleMenu;
    }
 /* clear out all styles in the stylesheet */
    inline void FreeStyles() {
	si->Destroy();
	si=new stylesheetInternal;
	UpdateCache();
    }
 /* add a style to the stylesheet */
    inline void Add(class style *styleptr) {
	PrepareForChangesIfNeeded();
	si->Add(styleptr);
	UpdateCache();
    }
 /* delete a style from the stylesheet */
    inline void Delete(class style *styleptr) {
	PrepareForChangesIfNeeded();
	si->Delete(styleptr);
	UpdateCache();
    }
 /* find a style, given its name */
    inline class style * Find(char *name) {
	return si->Find(name);
    }
    inline class style *MakeWritable(char *name) {
	PrepareForChangesIfNeeded();
	UpdateCache();
	return si->Find(name);
    }
    inline class style * Find(const char *name) {
	return si->Find((char *)name);
    }
    inline class style *MakeWritable(const char *name) {
	PrepareForChangesIfNeeded();
	UpdateCache();
	return si->Find((char *)name);
    }
 /* get a menulist for the styles */
    inline class menulist * GetMenuList(proctable_fptr procname, struct ATKregistryEntry  *infotype) {
	menulist *m=si->GetMenuList(procname, infotype);
	UpdateCache();
	return m;
    }
 /* parse the contents of \\define{} */
    inline long Read(FILE *fileptr, boolean template_c) {
	PrepareForChangesIfNeeded();
	long r=si->Read(fileptr, template_c);
	UpdateCache();
	return r;
    }
 /* write all externally defined styles */
    inline void Write(FILE *fileptr) {
	si->Write(fileptr);
    }
 /* save the name of the template being used */
    inline void SetTemplateName(char *name) {
	si->SetTemplateName(name);
	templateName=si->templateName;
    }
    inline void SetTemplateName(const char *name) {
	si->SetTemplateName((char *)name);
	templateName=si->templateName;
    }
 /* retrieve the name of the template being used */    
    inline char * GetTemplateName() {
	return si->GetTemplateName();
    }
 /* calls func(style, data) for each style in self.  
 The boolean value returned by func is True if the function is
 through enumerating;  EnumerateStyles then returns the last style 
 processed, othewise it returns NULL */
    inline class style * EnumerateStyles(stylesheet_efptr func, long data) {
	return si->EnumerateStyles(func, data);
    }
    inline class style * GetGlobalStyle() {
	return si->GetGlobalStyle();
    }

    virtual void Copy(stylesheet *dest);

    void PrepareForChanges();
    boolean FindReplacement(style *oldstyle, style **newstyle);
    
    stylesheet();
    stylesheet(const stylesheet *s);
    

    char *templateName;			/* name of the template used, if any */
    long nstyles;			/* number of styles in this stylesheet */
    long maxStyles;			/* number of styles currently allocated */
    long version;			/* version used for updating menulist */
    class style **styles;		/* array of the styles themselves */
    class menulist *styleMenu;		/* current menulist for the styles */
    stylesheet_maplist *mapstyles;
};
/** @} */
#endif /* _stylesheet_H_ */
