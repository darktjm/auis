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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/tindex.C,v 3.11 1995/08/06 23:05:41 wjh Stab74 $";
#endif


 
#include <andrewos.h>
ATK_IMPL("tindex.H")
#include <ctype.h>
#include <util.h>

#include <environ.H>
#include <print.H>
#include <completion.H>
#include <textview.H>
#include <text.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <proctable.H>
#include <environment.H>
#include <search.H>
#include <content.H>
#include <buffer.H>
#include <tindex.H>
#include <cursor.H>

#include "txtvpse.h"

#define tindex_HIDDEN -16
#define tindex_VISIBLE -17

#define Data(self) ((class content *)(((class view *) self)->dataobject))
#define Text(self) ((class text *) ((self)->dataobject))

static char *indexnames[] = {
    "index",
    "indexi",
    ""
};

static class atom *A_doindex;
static struct view_printopt po_doindex;

ATKdefineRegistry(tindex, ATK, tindex::InitializeClass);

static class view *FindView(class view  *v1,class view  *v2,class view  *v3,long  dat);
static class view *getcurrentview(class view  *self);
static class textview *getrealview(class view  *v);
static boolean skipnewlines(class text  *d,long  *pos,long  *len);
static void printindex(class view  *self, char *usepsstr);
static void previewindex(class view  *self, char *usepsstr);
static void tindex_IndexTermCmd(register class view  *v);
void tindex_ReadIndexFile(register class view  *v);
void tindex_WriteIndexFile(register class view  *v);
static void tindex_FudgeFonts(class text  *txt,char  *name , int  ftype);
void tindex_MakeIndexPlain(register class view  *v);
void tindex_MakeIndexItalic(register class view  *v);
void tindex_HideInvIndex(register class view  *v);
void tindex_ExposeInvIndex(register class view  *v);
static boolean isindexenv(class content  *self,class text  *text,long  pos,class environment  *env);
static void skipchapnumber(class text  *d,long  *pos,long  *len);
static boolean writeindex(FILE  *f,class text  *text,long  pos,class environment  *env);


static class view *FindView(class view  *v1,class view  *v2,class view  *v3,long  dat)
{
    if(ATK::IsTypeByName((v2)->GetTypeName(),"textview")){
	return v2;
    }
    return NULL;
}

static class view *getcurrentview(class view  *self)
{
    class buffer *buf;
    class content *ct;
    ct = Data(self);
    buf = buffer::FindBufferByData((class dataobject *)ct->srctext);
    if(buf)
	return (buf)->EnumerateViews((buffer_evfptr) FindView,0);
    return NULL;
}

#define indexnamecount 2
static class textview *getrealview(class view  *v)
{
    const char *name;
    name = (v)->GetTypeName();
    if(strcmp(name,"contentv") == 0)
	return (class textview *)getcurrentview(v);
    else if(ATK::IsTypeByName(name,"textview"))
	return (class textview *) v;
    else return NULL;
}
static class cursor *WaitCursor;
static boolean skipnewlines(class text  *d,long  *pos,long  *len)
{
    long i,end;
    int c;
    end = *pos + *len;
    for(i = *pos ; i < end; i++){
	c = (d)->GetChar(i);
	if(! isspace(c)) break;
    }
    *pos = i;
    for( ; i < end; i++){
	if((d)->GetChar(i) == '\n') {
	    end = i;
	    break;
	}
    }
    if(*pos > end) return FALSE;
    *len = end - *pos;
    return TRUE;
	
}
static void printindex(class view  *self, char *usepsstr)
{
    boolean useps;
    int pmode;

    pmode = (int)usepsstr;
    if (pmode >= 0 && pmode < 256) {
#ifdef PSPRINTING_ENV
	useps = TRUE;
#else
	useps = FALSE;
#endif
    }
    else {
	if (*usepsstr == 't' || *usepsstr == 'T')
	    useps = FALSE;
	else
	    useps = TRUE;
    }

    tindex::PrintIndex((class view *) getrealview(self), pmode);
}
static void previewindex(class view  *self, char *usepsstr)
{
    boolean useps;
    int pmode;

    pmode = (int)usepsstr;
    if (pmode >= 0 && pmode < 256) {
#ifdef PSPRINTING_ENV
	useps = TRUE;
#else
	useps = FALSE;
#endif
    }
    else {
	if (*usepsstr == 't' || *usepsstr == 'T')
	    useps = FALSE;
	else
	    useps = TRUE;
    }

    tindex::PreviewIndex((class view *) getrealview(self), pmode);
}

void tindex::PrintIndex(class view  *self, boolean useps)
{
    ATKinit;
    boolean oldoption;

    if(self == NULL) return;
    oldoption = self->GetPrintOption(A_doindex);
    message::DisplayString(NULL,0,"Preparing index - please wait");
    environ::Put("IndexOnly",NULL);
    self->SetPrintOption(&po_doindex, TRUE);
    print::ProcessView(self, useps?print_PRINTPOSTSCRIPT:print_PRINTTROFF, 1, "Index", "");
    self->SetPrintOption(&po_doindex, oldoption);
    environ::Delete("IndexOnly");
    message::DisplayString(NULL,0,"Index print request submitted.");

}

void tindex::PreviewIndex(register class view  *self, boolean useps)
{
    ATKinit;
    boolean oldoption;

    if(self == NULL) return;
    oldoption = self->GetPrintOption(A_doindex);
    message::DisplayString(NULL,0,"Preparing index - please wait");
    environ::Put("IndexOnly",NULL);
    self->SetPrintOption(&po_doindex, TRUE);
    print::ProcessView(self, useps?print_PREVIEWPOSTSCRIPT:print_PREVIEWTROFF, 1, "Index", "");
    self->SetPrintOption(&po_doindex, oldoption);
    environ::Delete("IndexOnly");
    message::DisplayString(NULL,0,"Index preview window should appear soon.");
}

static void tindex_IndexTermCmd(register class view  *v)
{
    /* Prompt for an index term and call index_IndexTerm */
    char thisString[100],*error;
    class text *d;
    long gf,i;
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    d = Text(self);
    gf = message::AskForString(v, 0, "Term to index ", NULL, thisString, sizeof(thisString));
    if (gf < 0) return;
    i = tindex::IndexTerm(d,thisString,&error);
    if(i < 0){
	message::DisplayString(v,0,error);
    }
    else if(i == 0){
	message::DisplayString(v,0,"Term not found");
    }
    else {
	if(i == 1) sprintf(thisString,"one instance indexed");
	else sprintf(thisString,"%d instances found",i);
	message::DisplayString(v,0,thisString);
	if(v != (class view *)self) (Data(v))->reinit();
	(d)->RegionModified(0,(d)->GetLength());
	(d)->NotifyObservers(0);
    }
}
void tindex_ReadIndexFile(register class view  *v)
{
    /* Prompt for an index term and call index_IndexTerm */
    FILE *f;
    char thisString[1024],*error;
    class text *d;
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    d = Text(self);
    if(completion::GetFilename(v,"Index file to read: ",NULL,thisString, sizeof(thisString),FALSE,TRUE) == -1)
	return;
    if((f = fopen(thisString,"r")) == NULL){
	message::DisplayString(v,0,"Can't open file");
	return;
    }
    error = tindex::ReadIndexList(d,f);
    fclose(f);
    if(error != NULL){
	message::DisplayString(v,0,error);
    }
    else {
	message::DisplayString(v,0,"Done");
	if(v != (class view *)self) (Data(v))->reinit();
	(d)->RegionModified(0,(d)->GetLength());
	(d)->NotifyObservers(0);
    }
}
void tindex_WriteIndexFile(register class view  *v)
{
    /* Prompt for an index term and call index_IndexTerm */
    FILE *f;
    char thisString[1024],runit[2500];
    class text *d;
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    d = Text(self);
    if(completion::GetFilename(v,"Index file to Write: ",NULL,thisString, sizeof(thisString),FALSE,FALSE) == -1)
	return;
    if((f = fopen(thisString,"w")) == NULL){
	message::DisplayString(v,0,"Can't open file");
	return;
    }
    tindex::WriteIndexList(d,f);
    fclose(f);
    sprintf(runit,"sort -u -o %s %s &",thisString,thisString);
    system(runit);

    message::DisplayString(v,0,"Done");
}
static void tindex_FudgeFonts(class text  *txt,char  *name , int  ftype)
{
    class style *Style;
    if(txt && (Style = (txt->styleSheet)->Find(name )) != NULL){
	switch(ftype){
	    case fontdesc_Plain:
		(Style)->ClearOldFontFaces();
		(Style)->ClearNewFontFaces();
		break;
	    case tindex_HIDDEN:
		(Style)->AddHidden();
		break;
	    case tindex_VISIBLE:
		(Style)->RemoveHidden();
		break;
	    default:
		(Style)->AddNewFontFace(ftype);
	}
    }
    (txt)->RegionModified(0,(txt)->GetLength());
    (txt)->NotifyObservers(0);
}
void tindex_MakeIndexPlain(register class view  *v)
{
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    tindex_FudgeFonts(Text(self),"index",fontdesc_Plain);
}
void tindex_MakeIndexItalic(register class view  *v)
{
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    tindex_FudgeFonts(Text(self),"index",fontdesc_Italic);
}
void tindex_HideInvIndex(register class view  *v)
{
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    tindex_FudgeFonts(Text(self),"indexi", tindex_HIDDEN);
}
void tindex_ExposeInvIndex(register class view  *v)
{
    class textview *self;
    if((self = getrealview(v)) == NULL) return ;
    tindex_FudgeFonts(Text(self),"indexi", tindex_VISIBLE);
}

/* returns 0 if not an index style, nonzero if it is */
int tindex::StyleNameIndexLevel(char *name)
{
    register char **sp;
    register int which = 0;
    if(name == NULL) return 0;
    for(sp = indexnames;which < indexnamecount && sp &&  *sp && **sp; sp++){
	which++;
	if(**sp == *name && strcmp(*sp,name) == 0){
	    return which;
	}
    }
    return 0;
}

static boolean isindexenv(class content  *self,class text  *text,long  pos,class environment  *env)
{
    char *sn;
    if (env->type == environment_Style){
	class style *style = env->data.style;
	if(style == NULL || ((sn = (style)->GetName()) == NULL)) return FALSE;
	if(tindex::StyleNameIndexLevel(sn)) return TRUE;
    }
    else return TRUE;
    return FALSE;
}
int tindex::IndexTerm(class text  *d,char  *term,char  **error)
{
	ATKinit;

    char *lastPattern = NULL;
    char  *tp;
    int pos = 0, len,dlen,c;
    register int j;
    class style *Style = NULL;
    class environment *te;
 
    tp =search::CompilePattern(term,(struct SearchPattern **)&lastPattern);
    if(tp != '\0'){
	if(error) *error = tp;
	return -1;
    }
    j = 0;len = 0;
    dlen = (d)->GetLength();
    for(pos = 0;(pos = search::MatchPattern(d,pos,(struct SearchPattern *)lastPattern)) >= 0;pos += len){
	len =  search::GetMatchLength();
	if(len == 0) {
	    return 0;
	}
	/* check if word or phrase is just part of another word */
	if(pos > 0){
	    c = (d)->GetChar(pos - 1);
	    if(isalpha(c)) continue;
	}
	if(pos + len < dlen){
	    c = (d)->GetChar(pos + len);
	    if(isalpha(c)) continue;
	}
	 /* check if already has an index style */
	if((d)->EnumerateEnvironments(pos,len,(text_eefptr)isindexenv,0) == NULL){
	   /* make it an index */
	    if(Style == NULL && (Style = (d->styleSheet)->Find("index" )) == NULL){
		Style = new style;
		(Style)->SetName( "index");
		(d->styleSheet)->Add( Style);
		/* should give some style attributes here */
	    }
	    te = (d->rootEnvironment)->WrapStyle(pos,len,Style);
	    (te)->SetStyle( FALSE, FALSE);
/*	    environment_Update(d->rootEnvironment, pos, len); */
	    j++;
	}
    }
    return(j);
}
static void skipchapnumber(class text  *d,long  *pos,long  *len)
{
    long i;
    int c;
    c = (d)->GetChar(*pos);
    if(c >= '0' && c <= '9'){
	i = (d)->Index(*pos,'\t',*len);
	i++;
	if(i > *pos && i < *pos + *len){
	    *len = *len - (i - *pos);
	    *pos = i;
	}
    }
fflush(stdout);
}
static boolean writeindex(FILE  *f,class text  *text,long  pos,class environment  *env)
{
    long len;
    char *sn,c;
    if (env->type == environment_Style){
	class style *style = env->data.style;
	if(style == NULL || ((sn = (style)->GetName()) == NULL)) return FALSE;
	if(tindex::StyleNameIndexLevel(sn)){
	    len = (env)->GetLength();
	    skipnewlines(text,&pos,&len);
	    skipchapnumber(text,&pos,&len);
	    for(len += pos ;pos < len ;pos++){
		c = (text)->GetChar(pos);
		if(isupper(c)) fputc(tolower(c),f);
		else fputc(c,f);
	    }
	    putc('\n',f);
	}
    }
    return FALSE;
}
void tindex::WriteIndexList(class text  *d,FILE  *f)
{
	ATKinit;

	(d)->EnumerateEnvironments(0,(d)->GetLength(),(text_eefptr)writeindex,(long)f);
}
char *tindex::ReadIndexList(class text  *d,FILE  *f)
{
	ATKinit;

    char buf[512],*error,*nl;
    while(fgets(buf,511,f) != NULL){
	if((nl  = strchr(buf,'\n')) != NULL) *nl = '\0';
	if(tindex::IndexTerm(d,buf,&error) < 0){
	    return error;
	}
    }
    return NULL;
}
boolean tindex::InitializeClass()
{
    struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");

    A_doindex = atom::Intern("doindex");
    po_doindex.name = A_doindex;
    po_doindex.type = atom::Intern("boolean");
    po_doindex.label = NULL;

    proctable::DefineProc("tindex-print",(proctable_fptr) printindex,viewtype,NULL,"print index");
    proctable::DefineProc("tindex-preview",(proctable_fptr) previewindex,viewtype,NULL,"preview index");
    proctable::DefineProc("tindex-index-term",(proctable_fptr) tindex_IndexTermCmd,viewtype,NULL,"index all instances of a term");
    proctable::DefineProc("tindex-read-index-file",(proctable_fptr) tindex_ReadIndexFile,viewtype,NULL,"read a list of index terms ");
    proctable::DefineProc("tindex-write-index-file",(proctable_fptr) tindex_WriteIndexFile,viewtype,NULL,"write a list of index terms ");
    proctable::DefineProc("tindex-index-italic",(proctable_fptr) tindex_MakeIndexItalic,viewtype,NULL,"make the index entries italic");
    proctable::DefineProc("tindex-index-plain",(proctable_fptr) tindex_MakeIndexPlain,viewtype,NULL,"make the index entries plain");
    proctable::DefineProc("tindex-expose-inv-index",(proctable_fptr) tindex_ExposeInvIndex,viewtype,NULL,"Expose the invisible index entries");
    proctable::DefineProc("tindex-hide-inv-index",(proctable_fptr) tindex_HideInvIndex,viewtype,NULL,"Hide the invisible index entries");
    WaitCursor = cursor::Create(NULL);
    if(WaitCursor) (WaitCursor)->SetStandard(Cursor_Wait);
    return TRUE;
}

struct indexent {
    struct textps_locatag *tag;
    char *term, *subterm;
};

static int indexentsort(struct indexent *v1, struct indexent *v2)
{
    int res;

    res = ULstrcmp(v1->term, v2->term);
    if (res != 0)
	return res;

    if (!v1->subterm && v2->subterm) {
	return (-1);
    }
    else if (v1->subterm && !v2->subterm) {
	return (1);
    }
    else {
	res = ULstrcmp(v1->subterm, v2->subterm);
	if (res != 0)
	    return res;
    }

    /* term and subterm are the same (or both NULL) */
    return (v1->tag->pagenum - v2->tag->pagenum);
}

class text *tindex::BuildIndexText(struct textps_locatag *taglist)
{
    class text *res = new text;
    struct textps_locatag *tag;
    struct indexent *entlist, *tent, *tent2;
    int numents, entlist_size;
    char *term, *subterm, *cx;
    class style *titlestyle;
    class environment *env;
    long pos, pos2;
    long rangebeg, rangeend;
    char buf[128], curchar, thischar, *curterm, *cursubterm;
    int ix, jx, kx, kkx, len;

    if (!res)
	return NULL;

    numents = 0;
    entlist_size = 8;
    entlist = (struct indexent *)malloc(sizeof(struct indexent) * entlist_size);
    for (tag=taglist; tag; tag=tag->next) {
	if (tag->type==locatag_Index) {
	    if (numents >= entlist_size) {
		entlist_size *= 2;
		entlist = (struct indexent *)realloc(entlist, sizeof(struct indexent) * entlist_size);
	    }
	    entlist[numents].tag = tag;
	    cx = strstr(tag->name, "++");
	    if (!cx) {
		term = NewString(tag->name);
		subterm = NULL;
	    }
	    else {
		len = cx-tag->name;
		term = (char *)malloc(len+1);
		strncpy(term, tag->name, len);
		term[len] = '\0';
		subterm = NewString(cx+2);
	    }
	    if (islower(*term))
		*term = (toupper(*term));
	    entlist[numents].term = term;
	    entlist[numents].subterm = subterm;
	    numents++;
	}
    }
    qsort(entlist, numents, sizeof(struct indexent), QSORTFUNC(indexentsort));

    res->ReadTemplate("index", TRUE);
    titlestyle = res->GetStyleSheet()->Find("indextitle");
    if (!titlestyle) {
	titlestyle = new style;
	titlestyle->SetName("indextitle");
	res->GetStyleSheet()->Add(titlestyle);
    }

    curchar = 0;
    curterm = NULL;
    for (ix=0; ix<numents; ) {
	tent = (&(entlist[ix]));
	thischar = tent->term[0];
	if (thischar < 'A' || thischar > 'Z')
	    thischar = 0;
	if (thischar != curchar) {
	    curchar = thischar;
	    if (curchar) {
		pos = res->GetLength();
		sprintf(buf, "\n\t\t- %c -\n", curchar);
		res->InsertCharacters(pos, buf, strlen(buf));
		pos2 = res->GetLength();
		env = res->AddStyle(pos, pos2-pos, titlestyle);
		env->SetStyle(FALSE, FALSE);
	    }
	}
	if (ULstrcmp(tent->term, curterm)) {
	    /* new term */
	    curterm = tent->term;
	    res->InsertCharacters(res->GetLength(), "\n", 1);
	    if (tent->subterm) {
		/* oop, need a numberless term here */
		res->InsertCharacters(res->GetLength(), curterm, strlen(curterm));
		res->InsertCharacters(res->GetLength(), "\n", 1);
	    }
	}
	for (jx=ix+1; jx<numents; jx++) {
	    tent2 = (&(entlist[jx]));
	    if (ULstrcmp(tent->term, tent2->term) || ULstrcmp(tent->subterm, tent2->subterm)) {
		break;
	    }
	}
	if (!tent->subterm) {
	    /* top-level term */
	    res->InsertCharacters(res->GetLength(), tent->term, strlen(tent->term));
	}
	else {
	    res->InsertCharacters(res->GetLength(), "\t", 1);
	    res->InsertCharacters(res->GetLength(), tent->subterm, strlen(tent->subterm));
	}
	for (kx = ix; kx < jx; ) {
	    kkx = kx;
	    rangebeg = entlist[kx].tag->pagenum;
	    rangeend = rangebeg;
	    while (kkx < jx) {
		tent2 = (&(entlist[kkx]));
		if (tent2->tag->pagenum > rangeend+1)
		    break;
		rangeend = tent2->tag->pagenum;
		kkx++;
	    }
	    if (rangebeg==rangeend)
		sprintf(buf, "%s%d", (kx==ix)?",  ":", ", rangebeg);
	    else
		sprintf(buf, "%s%d-%d", (kx==ix)?",  ":", ", rangebeg, rangeend);
	    res->InsertCharacters(res->GetLength(), buf, strlen(buf));
	    kx = kkx;
	}
	res->InsertCharacters(res->GetLength(), "\n", 1);
	ix = jx;
    }

    for (ix=0; ix<numents; ix++) {
	if (entlist[ix].term)
	    free (entlist[ix].term);
	if (entlist[ix].subterm)
	    free (entlist[ix].subterm);
    }
    free(entlist);

    return res;
}

