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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/stylesheet.C,v 3.6 1996/11/19 22:22:40 robr Exp $";
#endif


 



#include <andrewos.h>
ATK_IMPL("stylesheet.H")
#include <style.H>
#include <menulist.H>
#include <proctable.H>
#include <mflex.H>
#include <stylesheet.H>


#define iswhite(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')

#define INITIALNUMSTYLES 20



ATKdefineRegistry(stylesheet, observable, NULL);
ATKdefineRegistryNoInit(stylesheetInternal, traced);
static long FindStyle(class stylesheetInternal  *self, class style  *styleptr);


stylesheetInternal::stylesheetInternal()
{
    this->nstyles = 0;
    this->maxStyles = 0;
    this->styles = NULL;
    this->templateName = NULL;
    this->styleMenu = NULL;
    this->version = 0;

    THROWONFAILURE( TRUE);
}

stylesheetInternal::~stylesheetInternal()
{
    if (this->templateName)
	free(this->templateName);
    if (this->styleMenu != NULL)
        delete this->styleMenu;
    (this)->FreeStyles();
    if (this->styles)
	free(this->styles);
}

void stylesheetInternal::FreeStyles()
{
    register int i;
    register class style **styles;

    if (this->nstyles==0)
	return;

    for (i = 0, styles = this->styles; i < this->nstyles; i++, styles++)
	delete *styles;

    this->nstyles = 0;
    this->version++;
}

static long FindStyle(class stylesheetInternal  *self, class style  *styleptr)
{
    register int i;
    register class style **styles;
    
    for (i = 0, styles = self->styles; i < self->nstyles; i++, styles++)
        if (*styles == styleptr)
            return i;

    return -1;
}

void stylesheetInternal::Add(class style  *styleptr)
{
    if (this->maxStyles == 0) {
	this->maxStyles = INITIALNUMSTYLES;
	this->styles = (class style **)
          malloc(INITIALNUMSTYLES * sizeof(class style *));
    } else if (FindStyle(this, styleptr) != -1)
	return;
    else if (this->nstyles == this->maxStyles) {
	this->maxStyles += this->maxStyles / 2;
	this->styles = (class style **) realloc(this->styles, this->maxStyles * sizeof(class style *));
    }
    this->styles[this->nstyles++] = styleptr;
    this->version++;
}

void stylesheetInternal::Delete(class style  *styleptr)
{
    register int i;

    if ((i = FindStyle(this, styleptr)) != -1) {
	delete styleptr;
	for (this->nstyles--; i < this->nstyles; i++)
	    this->styles[i] = this->styles[i+1];
    }
    this->version++;
}

class style *stylesheetInternal::Find(char  *name)
{
    register int i;
    register class style **styles;
    
    for (i = 0, styles = this->styles; i < this->nstyles; i++, styles++)
	if (strcmp((*styles)->name, name) == 0)
            return *styles;

    return NULL;
}

class menulist *stylesheetInternal::GetMenuList(proctable_fptr procname, struct ATKregistryEntry   *infotype)
{
    struct proctable_Entry *proc;

    register int i;
    register class style **styles;

    if (!this->styleMenu)
        this->styleMenu = new menulist;
    else
        (this->styleMenu)->ClearML();

    proc = proctable::DefineProc("wraplook", procname, infotype, NULL, NULL);

    for (i = 0, styles = this->styles; i < this->nstyles; i++, styles++)
	(this->styleMenu)->AddToML( (*styles)->menuName, proc, (long)(*styles)->GetName(), 0);

    return this->styleMenu;
}

/* This routine parses the contents of a \define{} */
/* Assumes the "\define{" has already been read. */
/* The ending "}" is read by style_Read. */

long stylesheetInternal::Read(FILE  *fp, boolean  template_c)
{
    int c, i;
    class style *styleptr;
    char styleName[80];

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (iswhite(c));

    i = 0;

    if (c == '}')   /* Empty define */
        return 0;

    /* The style name is terminated by white space, usually '\n'. */

    while (! iswhite(c)) {
        if (i < sizeof (styleName) - 1)
            styleName[i++] = c;
        if ((c = getc(fp)) == EOF)
            return -1;
    }

    styleName[i] = '\0';

    if ((styleptr = (this)->Find( styleName)) != NULL) {
	class style nstyle;
	nstyle.Copy(styleptr);
    } else {
        styleptr = new style;
        (this)->Add( styleptr);
    }

    (styleptr)->SetName( styleName);
    styleptr->template_c = template_c;

    (styleptr)->Read( fp);

    return 0;
}

void stylesheetInternal::Write(FILE  *fp)
{
    register int i;
    register class style **styles;
    
    for (i = 0, styles = this->styles; i < this->nstyles; i++, styles++)
	if ((*styles)->template_c == 0)  {
	    fprintf(fp, "\\define{%s\n", (*styles)->GetName());
	    (*styles)->Write( fp);
	    fprintf(fp, "}\n");
	}
}

void stylesheetInternal::SetTemplateName(char  *templateName)
{
    if (this->templateName != NULL) {
	if (templateName!=NULL && strcmp(this->templateName, templateName)==0)
	    return;
	else
	    free(this->templateName);
    }

    if (templateName == NULL)
	this->templateName = NULL;
    else {
	this->templateName = (char *) malloc(strlen(templateName)+ 1);
	strcpy(this->templateName, templateName);
    }
    this->version++;
}

char *stylesheetInternal::GetTemplateName()
{
    return this->templateName;
}

 /* EnumerateStyles calls func(style, data) for each style in self.  
			The boolean value returned by func is True if the function is
			through enumerating;  EnumerateStyles then returns the last style 
			processed, othewise it returns NULL 
		WJHansen, 17 Aug, 1987 */

class style *
stylesheetInternal::EnumerateStyles(stylesheet_efptr func, long  data)
{
	class style **curr;
	for (curr = this->styles; curr < this->styles + this->nstyles; ) {
		class style *t = *curr;
		if ((*func)(*curr, data))
			return *curr;
		if (t == *curr) curr++;	/* try to be reasonable if a style has been deleted */
	}
	return NULL;
}

class style *stylesheetInternal::GetGlobalStyle()
{
    return (this)->Find( "global");
}

stylesheet::stylesheet() {
    si=new stylesheetInternal;
    UpdateCache();
    templateName=si->templateName;
    mapstyles=NULL;
}

stylesheet::stylesheet(const stylesheet *s) {
    if(s) {
	si=s->si;
	si->Reference();
	UpdateCache();
	templateName=si->templateName;
    }
    mapstyles=NULL;
}

stylesheet::~stylesheet() {
    if(si) si->Destroy();
}

struct stylesheet_mapentry {
    style *oldstyle, *newstyle;
};

DEFINE_MFLEX_CLASS(stylesheet_maplist,stylesheet_mapentry,10);

void stylesheet::Copy(stylesheet *dest) {
    int i;
    boolean forcecopy=FALSE;
    register class style **styles, *overridestyle;
    if(si==dest->si) {
	// The dest is already using the same exact data.
	return;
    }
    stylesheetInternal *old=dest->si;
    dest->si=si;
    if(si) {
	si->Reference();
	templateName=si->templateName;
	dest->UpdateCache();
    } else {
	templateName=NULL;
	nstyles=maxStyles=version=0;
	styles=NULL;
	styleMenu=NULL;
    }
    if(old) {
	dest->mapstyles=new stylesheet_maplist;
	for (i = 0, styles = old->styles; i < old->nstyles; i++, styles++) {
	    style *n;
	    n=dest->Find((*styles)->GetName());
	    stylesheet_mapentry *e=dest->mapstyles->Append();
	    if(e) {
		e->oldstyle=(*styles);
		e->newstyle=n;
	    }
	}
	dest->NotifyObservers(observable_OBJECTCHANGED);
	delete dest->mapstyles;
	dest->mapstyles=NULL;
	old->Destroy();
    }
}


void stylesheet::PrepareForChanges() {
    stylesheetInternal *dest=new stylesheetInternal;
    if(si) {
	mapstyles=new stylesheet_maplist;
	class style **curr;
	dest->FreeStyles();
	for(curr=si->styles;curr<si->styles+si->nstyles;curr++) {
	    style *n=new style;
	    stylesheet_mapentry *e=mapstyles->Append();
	    if(e) {
		e->oldstyle=(*curr);
		e->newstyle=n;
	    }
	    (*curr)->Copy(n);
	    dest->Add(n);
	}
	dest->SetTemplateName(GetTemplateName());
	stylesheetInternal *old=si;
	si=dest;
	NotifyObservers(observable_OBJECTCHANGED);
	delete mapstyles;
	mapstyles=NULL;
	old->Destroy();
    } else si=dest;
    if(si) UpdateCache();
}

boolean stylesheet::FindReplacement(style *oldstyle, style **newstyle) {
    if(mapstyles) {
	size_t i;
	for(i=0;i<mapstyles->GetN();i++) {
	    if((*mapstyles)[i].oldstyle==oldstyle) {
		*newstyle=(*mapstyles)[i].newstyle;
		return TRUE;
	    }
	}
    }
    return FALSE;
}
    

void stylesheetInternal::Copy(stylesheetInternal *dest) {
    class style **curr;
    dest->FreeStyles();
    for(curr=styles;curr<styles+nstyles;curr++) {
	style *n=new style;
	(*curr)->Copy(n);
	dest->Add(n);
    }
    dest->SetTemplateName(GetTemplateName());
}
