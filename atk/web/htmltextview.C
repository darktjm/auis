/* File htmltextview.C created by Ward Nelson
       (c) Copyright IBM Corp 1995.  All rights reserved. 
       (c) Copyright Carnegie Mellon University 1996.  All rights reserved. 
 $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $

    htmltextview, an object for viewing HTML+ formatted files. 

*/

#include <andrewos.h>
ATK_IMPL("htmltextview.H")
#include <htmltextview.H>

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/web/RCS/htmltextview.C,v 1.7 1996/09/04 18:06:08 robr Exp $";
#endif

#include <view.H>
#include <viewref.H>
#include <bind.H>
#include <menulist.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <message.H>
#include <im.H>
#include <mark.H>
#include <buffer.H>
#include <frame.H>
#include <stylesheet.H>
#include <style.H>

#include <attlist.H>
#include <htmlenv.H>
#include <hidden.H>
#include <hiddenview.H>
#include <htmltext.H>

static keymap *html_Map;
static menulist *html_Menus;

ATKdefineRegistry(htmltextview, textview, htmltextview::InitializeClass);
static void showRaw(htmltextview *self, long rock);
static void showNormal(hiddenview *self, long rock);
static void showComments(htmltextview *self, long viewcomments);
static void fixList(htmltextview *self, long rock);
static void makeList(htmltextview *self, enum ListType listtype);
static void addItem(htmltextview *self, long rock);
static void newlineAddItem(htmltextview *self, long rock);
static void setTargetLink(frame *frame, long param);
static void setSourceLink(htmltextview *self, long param);
static void addLink(htmltextview *self, long rock);
static void addTarget(htmltextview *self, long rock);
static void showAttributes(htmltextview *self);
static void insertImage(htmltextview *self, long rock);

static struct bind_Description htmltextBindings[]={
    {"htmltextview-show-attributes", "\033a",0, 
	"Region~4,Edit Attributes~95",0,0, 
	(proctable_fptr)showAttributes, 
	"Display and edit attributes of style at cursor position.", NULL},
    {"htmltextview-add-link", NULL,0, "Region~4,Add Link (prompt)~80",0, textview_NotReadOnlyMenus, (proctable_fptr)addLink, "Make an anchor that points to the URL specified from a prompt.", NULL},
    {"htmltextview-add-target", NULL,0, "Region~4,Target for Link (prompt)~81",0, textview_NotReadOnlyMenus, (proctable_fptr)addTarget, "Gives the highlighted region a name that can be targeted by an anchor with a href=#name.", NULL},
#if 0 /*incomplete*/
    {"htmltextview-autolink", "\033z",0, 
	"Region~4,Auto Link..~85",0,textview_NotReadOnlyMenus, 
	(proctable_fptr)setSourceLink, 
	"Starts the autolink process on the highlighted text.  Waits for htmltextview-set-target to be invoked before adding the style.", NULL},
    {"htmltextview-set-target", NULL,0, 
	"Region~4,..Set Target~86",0,0/*readonly buffers OK too*/, 
	(proctable_fptr)setTargetLink, 
	"Invoke in the same ez session (any HTML+ buffer) after htmltextview-set-source was invoked; this notifies the pending autolink of its desired target, which can then be styled as a hyperlink."}, 
	/*XXX- incomplete.  We need a way to associate URLs 
	(not just AFS pathnames) with buffers somehow, 
	before this is of any use. */
#endif /*incomplete*/

    {"htmltextview-add-item", "\t",'\t', 
	"List~6,Add list item~20",'\t', textview_NotReadOnlyMenus, 
	(proctable_fptr)addItem, 
	"adds another list item to the current list.", NULL},
    {"htmltextview-newline-add-item", "\012",0, 
	NULL,0, textview_NotReadOnlyMenus, 
	(proctable_fptr)newlineAddItem, 
	"adds a blank line (with another item if used inside a list).", NULL},
    {"htmltextview-make-list", "\033'\033u",(long)listtype_UNORDERED, 
	"List~6,Unordered List~10",(long)listtype_UNORDERED, 
	textview_NotReadOnlyMenus, 
	(proctable_fptr)makeList, 
	"makes a list of the selected region", NULL},
    {"htmltextview-make-list", "\033'\033o",(long)listtype_ORDERED, 
	"List~6,Ordered List~10",(long)listtype_ORDERED, 
	textview_NotReadOnlyMenus},
    {"htmltextview-make-list", "\033'\033d",(long)listtype_DEFINITION, 
	"List~6,Definition List~10",(long)listtype_DEFINITION, 
	textview_NotReadOnlyMenus},
    {"htmltextview-renumber-list", "\033'\033r",0, 
	"List~6,Renumber List~30",0, textview_NotReadOnlyMenus, 
	(proctable_fptr)fixList, 
	"Replaces and reorders all the numbers in an ordered list.", NULL},
    {"htmltextview-expose-hidden", NULL,1, 
	"Page~9,Expose HTML+ insets~41",1, 0, 
	(proctable_fptr)showComments, 
	"Exposes insets containing raw HTML+ markup.", NULL},
    {"htmltextview-hide-hidden", NULL,0, 
	"Page~9,Hide HTML+ insets~42",0,0, 
	(proctable_fptr)showComments, 
	"Hides insets containing raw HTML+ markup.", NULL},
    {"htmltextview-show-raw", NULL,0, 
	"File~10,Show Raw HTML+~80",0,0, 
	(proctable_fptr)showRaw, 
	"Display file as raw HTML+ markup.", NULL},
    {"htmltextview-insert-image-here", NULL,0, 
	"Media,Image...~2",0, textview_NotReadOnlyMenus, 
	(proctable_fptr)insertImage, 
	"Prompts for URL or pathname of image and imbeds it at cursor position. (string parameter supersedes prompt)", NULL},
    NULL
};

static struct bind_Description addltextBindings[]={
    {"textview-insert-inset-here", NULL,0, "Page~9,Insert Horiz Rule~5",(long)"hr", textview_NotReadOnlyMenus},
    {"textview-insert-inset-here", NULL,0, "Page~9,Inset raw HTML+~40",(long)"hidden", textview_NotReadOnlyMenus},
    {"textview-insert-inset-here", NULL,0, "Media,Table~7",(long)"table", textview_NotReadOnlyMenus},
    NULL
};

/* buffer-replacement code stolen from framscmds.C */
struct bufferPair {
    buffer *buffer1, *buffer2;
};
static boolean ReplaceBufferWork(frame *fr, struct bufferPair *buffPair)
{
    if (fr->GetBuffer() == buffPair->buffer1)
	fr->SetBuffer(buffPair->buffer2, TRUE);
    return FALSE;
}
static void ReplaceBuffer(buffer *oldBuffer, buffer *newBuffer)
{
    struct bufferPair buffers;
    buffers.buffer1= oldBuffer;
    if ((buffers.buffer2= newBuffer) == oldBuffer)
	return;
    frame::Enumerate((frame_effptr)ReplaceBufferWork, (long)&buffers);
}

void showRaw(htmltextview *self, long rock)
{
    FILE *file= tmpfile();
    if (!file) {
        fprintf(stderr, "htmltext: Unable to open temporary file; can't display raw HTML\n");
        return;
    }

    htmltext *htxt= (htmltext *)(self)->GetDataObject();
    buffer *newbuf, *buf= buffer::FindBufferByData(htxt);
    if (!buf) {
        /* Won't work in bush, or if a mere inset has input focus */
        message::DisplayString(self, 0, "Cannot view raw version without buffer support.");
        return;
    }

    char *bufname= buf->GetName(), *fname= buf->GetFilename();
    hidden *hiddenobj= new hidden;
    boolean readonly= buf->GetReadOnly(), modified= buf->GetIsModified();

    htxt->Write(file, 0, 0);
    hiddenobj->SetVisibility(hidden_FULLSCREEN);
    rewind(file);
    hiddenobj->Read(file, 0);
    fclose(file);

    /* make the buffer contain the fullscreen hidden instead of the htmltext */
    newbuf= buffer::Create(bufname, fname, NULL, hiddenobj);
    newbuf->SetReadOnly(readonly);
    if (modified) {
	hiddenobj->SetModified();
	hiddenobj->NotifyObservers(0);
    }
    ReplaceBuffer(buf, newbuf);
    buf->Destroy();
 }

void showNormal(hiddenview *self, long rock)
{
    FILE *file= tmpfile();
    if (!file) {
        fprintf(stderr, "htmltext: Unable to open temporary file; can't display WYSIWYG HTML\n");
        return;
    }

    hidden *hid= (hidden *)(self)->GetDataObject();
    buffer *newbuf, *buf= buffer::FindBufferByData(hid);
    char *bufname= buf->GetName(), *fname= buf->GetFilename();
    htmltext *htmlobj= new htmltext;
    boolean readonly= buf->GetReadOnly(), modified= buf->GetIsModified();

    hid->Write(file, 0, 0);
    rewind(file);
    htmlobj->SetReadOnly(readonly);
    (htmlobj)->Read(file, 0);
    fclose(file);

    /* make the buffer contain htmltext instead of the fullscreen hidden */
    newbuf= buffer::Create(bufname, fname, NULL, htmlobj);
    newbuf->SetReadOnly(readonly);
    if (modified) {
	htmlobj->SetModified();
	htmlobj->NotifyObservers(0);
    }
    ReplaceBuffer(buf, newbuf);
    buf->Destroy();
}

void showComments(htmltextview *self, long viewcomments)
{
    htmltext *txtobj= (htmltext *)(self)->GetDataObject();
    if ((unsigned long)viewcomments>=256 && ((char *)viewcomments)[0]=='\0')
	/* user passed an empty string, call it 0 */
	viewcomments= 0;
    (txtobj)->ShowComments(viewcomments);
    (txtobj)->RegionModified(0, (txtobj)->GetLength());
    (txtobj)->NotifyObservers(0);
}

/* given a position or a starting environment, this finds the first list environment */
htmlenv *first_list_env(htmltextview *self, long pos, enum ListType *type, htmlenv *start_env)
{
    htmltext *txtobj= (htmltext *)(self)->GetDataObject();
    htmlenv *env;
    long o=0, u=0, d=0;
    style *style;
    char *sname;
    if (start_env) env= (htmlenv *)(start_env)->GetParent();
    else env= (htmlenv *)(self)->GetInsertEnvironment(pos);

    for (; env; env= (htmlenv *)(env)->GetParent())
	if (env->type==environment_Style && (style= env->environment::data.style)!=NULL && (sname= (style)->GetName())!=NULL)
	    if ((o= strcmp(sname, "ordered"))==0 ||
		(u= strcmp(sname, "unordered"))==0 ||
		(d= strcmp(sname, "definition"))==0)
		break;

    if (env) {
	if (o == 0) *type= listtype_ORDERED;
	else if (u == 0) *type= listtype_UNORDERED;
	else if (d == 0) *type= listtype_DEFINITION;
    }
    return env;
}

/* fixList renumbers the list at dot position */
void fixList(htmltextview *self, long rock)
{
    htmltext *txtobj= (htmltext *) (self)->GetDataObject();
    long pos= (self)->GetDotPosition();
    enum ListType ltype;
    htmlenv *env= first_list_env(self, pos, &ltype, NULL);

    if ((self)->ConfirmReadOnly()) return;

    if (env) {
	(txtobj)->RenumberList(pos, ltype, env);
	(txtobj)->RegionModified((env)->Eval(), (env)->GetLength());
	(txtobj)->NotifyObservers(0);
    }
}

/* rip out old dingbats and replace newlines with a newline followed by a dingbat */
static void insert_dingbats(htmlenv *curr_env, htmltext *txtobj, long pos, long *plength, int sublist, enum ListType ltype)
{
    long chars_sofar=0, j=0, env_len, old_len, env_pos=0, nxt_chg;
    int cmpval;
    char c, prev;
    long dingbat_length;
    htmlenv *env=NULL;

    if (ltype == listtype_DEFINITION) return;

    if (!sublist && curr_env->type==environment_Style && strcmp((curr_env->environment::data.style)->GetName(),"dingbat") == 0) {
	(txtobj)->DeleteCharacters(pos, *plength);
	*plength= 0;
	return;
    }

    env= (htmlenv *)(curr_env)->GetChild(pos+chars_sofar);	/* see if there's an environment at the position I'm at */
    if (!env) env= (htmlenv *)(curr_env)->GetNextChild(NULL, pos+chars_sofar);

    while (chars_sofar<*plength) {
	if (env) {
	    env_len= (env)->GetLength();
	    cmpval= (env->type==environment_Style &&
		     ((strcmp((env->environment::data.style)->GetName(),"unordered") == 0) ||
		      (strcmp((env->environment::data.style)->GetName(),"ordered") == 0) ||
		      (strcmp((env->environment::data.style)->GetName(),"definition") == 0)));
	}
	if (env && (env_pos= (env)->Eval()) == pos+chars_sofar) {
	    old_len= env_len;
	    insert_dingbats(env, txtobj, pos+chars_sofar, &env_len, cmpval || sublist, ltype);
	    chars_sofar += env_len;
	    *plength += (env_len - old_len);
	} else {
	    if (env) nxt_chg= env_pos-(pos+chars_sofar);
	    else nxt_chg= *plength-chars_sofar;
	    if (nxt_chg > *plength) nxt_chg= *plength-chars_sofar;
	    c= (txtobj)->GetChar(pos+chars_sofar-1);
	    j=0;
	    while (j<nxt_chg) {
		prev= c;
		c= (txtobj)->GetChar(pos+j+chars_sofar);
		if (!sublist && (prev== '\n' || pos+j+chars_sofar == 0)) {
		    dingbat_length= ((txtobj)->AddDingbat(pos+j+chars_sofar, listtype_UNORDERED, curr_env))->GetLength(); /* always insert unordered dingbats */
		    nxt_chg += dingbat_length; j+= dingbat_length; *plength += dingbat_length;
		}
		++j;
	    }
	    chars_sofar += nxt_chg;
	    if (env) {
		old_len= env_len;
		insert_dingbats(env, txtobj, pos+chars_sofar, &env_len, cmpval || sublist, ltype);
		chars_sofar += env_len;
		*plength += (env_len - old_len);
	    }
	}
	env= (htmlenv *)(curr_env)->GetNextChild(NULL, pos +chars_sofar -1);
    }
}

int doproc(htmltextview *self, char *procname, long parm)
{
    struct proctable_Entry *proc= proctable::Lookup(procname);

    if (proc && proctable::Defined(proc))
	((self)->GetIM())->HandleMenu(proc, self, parm);
    else {
	char error[256];
	sprintf(error,"Unknown procedure '%s'.",procname);
	message::DisplayString(self, 0, error);
	return -1;
    }
    return 0;
}

/* makeList makes the selected region into a list.  If there is no selected region, the current insertion style is set */
void makeList(htmltextview *self, enum ListType listtype)
{
    htmltext *ht= (htmltext*) (self)->GetDataObject();
    long pos= (self)->GetDotPosition(); /* position of cursor */
    long length= (self)->GetDotLength(); /* length of cursor */
    long origpos= pos;
    htmlenv *env=NULL, *temp_env=NULL;
    style *list_style;
    char *list_name;
    boolean SibEndFlag, SibBeginFlag;
    enum ListType Sibltype;

    if ((self)->ConfirmReadOnly()) return;

    if (listtype==listtype_UNORDERED) list_name= "unordered";
    else if (listtype==listtype_ORDERED) list_name= "ordered";
    else if (listtype==listtype_DEFINITION) list_name= "definition";
    else if ((unsigned long)listtype >= 256) {
	/* custom user binding in .ezinit.  Try to oblige */
	list_name= (char *)listtype;
	if (strcmp(list_name,"unordered")==0) listtype= listtype_UNORDERED;
	else if (strcmp(list_name,"ordered")==0) listtype= listtype_ORDERED;
	else if (strcmp(list_name,"definition")==0) listtype= listtype_DEFINITION;
	else {
	    message::DisplayString(self, 0, "Unrecognized list type; must pass 'unordered', 'ordered', or 'definition' as a parameter.");
	    return;
	}
    }
    /* find or create necessary style */
    list_style= ((ht)->GetStyleSheet())->Find(list_name);
    if (!list_style) {
	list_style= new style;
	(list_style)->SetName(list_name);
	((ht)->GetStyleSheet())->Add(list_style);
    }
    /* clean up selected region (this may involve reducing length to 0) */
    if (length>0) {
	long startline= (ht)->GetBeginningOfLine(pos);
	long endline= (ht)->GetEndOfLine(pos+length-1);
	pos= startline;
	length= endline-startline;
	(self)->SetDotPosition(pos);
	(self)->SetDotLength(length);
    }
    /* wrap the style */
    if (length>0) {
	env= (htmlenv *)((htmlenv *)ht->text::rootEnvironment)->WrapStyle(pos, length, list_style);
    } else {
	/* no region selected; start a new empty list */
	if (pos>0 && (ht)->GetChar(pos-1)!='\n') {
	    /* need a new line to start on */
	    (self)->PrepareInsertion(FALSE /* a lie, to avoid dumb "feechur" */);
	    (ht)->InsertCharacters(pos++, "\n",1);
	    (self)->FinishInsertion();
	    (self)->SetDotPosition(pos);
	}
	if (listtype==listtype_DEFINITION) {
	    /* definition list: just set up the current insertion styles */
	    doproc(self, "textview-insert-environment", (long)list_name);
	    doproc(self, "textview-insert-environment", (long)"term");
	    (ht)->NotifyObservers(0);
	    return;
	}
	/* ordered or unordered list: wrap style and insert first bullet */
	(ht)->InsertCharacters(pos, " ",1); /* insert junk to wrap style around */
	env= (htmlenv *)((htmlenv *)ht->text::rootEnvironment)->WrapStyle(pos, 1, list_style);
    }
    /* don't let preceding style mess up dingbat insertion */
    temp_env= (htmlenv *) (env)->GetParent();
    if (temp_env) temp_env= (htmlenv *) (temp_env)->GetPreviousChild(env, pos);
    if (temp_env) {
	SibEndFlag= temp_env->nestedmark::includeEnding;
	SibBeginFlag= temp_env->nestedmark::includeBeginning;
	(temp_env)->SetStyle(SibBeginFlag, FALSE);
    }
    (env)->SetStyle(TRUE,TRUE);

    if (length>0) {
	/* add dingbat to every line in selected region */
	insert_dingbats(env, ht, pos, &length, 0, listtype);
    } else {
	/* add one dingbat for new list */
	htmlenv *dingbat_env;
	dingbat_env= (ht)->AddDingbat(pos, listtype, env);
	pos= (dingbat_env)->Eval() + (dingbat_env)->GetLength();
	(ht)->DeleteCharacters(pos, 1); /* delete junk we inserted for wrapping */
	(self)->SetDotPosition(pos);
	(ht)->RegionModified(origpos, pos-origpos+1);
    }

    /* set environment flags back */
    if (temp_env) (temp_env)->SetStyle(SibBeginFlag, SibEndFlag);
    (ht)->RenumberList(pos, listtype, env);
    /* (I think) just in case we changed some old ordered-list dingbats into nested unordered bullets, we need to renumber the parent ordered list */
    temp_env= first_list_env(self, pos, &Sibltype, env);
    if (temp_env) (ht)->RenumberList((temp_env)->Eval(), Sibltype, temp_env);

    (env)->SetStyle(FALSE, TRUE); /*XXX-hmm, this is kind of inconsiderate to people who use StylesIncludeEnd:no.  Oh well */
    (ht)->RegionModified(pos, length);
    (ht)->NotifyObservers(0);
}

void endTerm(htmltextview *self, long rock)
{
    htmltext *ht= (htmltext *)(self)->GetDataObject();
    long pos= (self)->CollapseDot();
    htmlenv *rootenv= (htmlenv *)ht->text::rootEnvironment, *env= (htmlenv *)(self)->GetEnclosingEnvironment(pos);
    boolean found= FALSE, begflg, endflg;

    while (env && env!=rootenv && (env->environment::type!=environment_Style || !(found= !strcmp((env->environment::data.style)->GetName(), "term")))) {
	env= (htmlenv *)(env)->GetParent();
    }
    if (found) {
	/* set end flag to false so the tab we insert won't be part of the term */
	begflg= env->nestedmark::includeBeginning;
	endflg= env->nestedmark::includeEnding;
	(env)->SetStyle(FALSE,FALSE);
    }

    if (rock=='\r') { /* \r means this was called indirectly from newlineAddItem, so create a new style */
	/* the "avoid stupid text feechur" statements are to fool tv-insert-env into doing what the environment flags TELL IT TO, rather than doing some ridiculous special-case code because there's a newline there */ /*XXX-fix this in the base text object, once the C++ version stabilizes. Limit the "feechur" to styles that have a special flag set in them, then we won't need this extra crap here. */
	(ht)->InsertCharacters(pos, " ",1); /* avoid stupid text feechur */
	doproc(self, "textview-insert-environment", (long)"term");
	(ht)->DeleteCharacters(pos, 1); /* avoid stupid text feechur */
    } else {
	/* whether a term style was already there or not, we want a tab inserted (UNLESS newlineAddItem called us) */
	(ht)->InsertCharacters(pos++, "\t", 1);
	(self)->SetDotPosition(pos);
	if (!found && pos>1 && (ht)->GetChar(pos-2)!='\n') {
	    /* no existing term style, and there's text in front of the tab; wrap term style around it */
	    long startline= (ht)->GetBeginningOfLine(--pos);
	    style *termsty= ((ht)->GetStyleSheet())->Find("term");
	    if (!termsty) { /* no term style; make one */
		termsty= new style;
		(termsty)->SetName("term");
		((ht)->GetStyleSheet())->Add(termsty);
	    }
	    (ht)->AddStyle(startline,pos-startline, termsty);
	    (ht)->RegionModified(startline,pos-startline+2);
	}
    }

    if (found)
	/* restore environment flags to whatever they were before */
	(env)->SetStyle(begflg, endflg);
    (ht)->NotifyObservers(0);
}

/* if newlineAddItem (Ctrl-J) calls this, rock will be '\r'.  Otherwise, rock is irrelevant and is assumed to be the Tab key or an equivalent menu. */
void addItem(htmltextview *self, long rock)
{
    long pos= (self)->CollapseDot();
    long dingbat_len= 0, dingbat_pos= pos;
    htmltext *txtobj= (htmltext *)(self)->GetDataObject();
    htmlenv *env, *dingbat_env;
    boolean begin_flag=FALSE, end_flag=TRUE;
    enum ListType ltype;

    if ((self)->ConfirmReadOnly()) return;

    env= first_list_env(self,pos,&ltype,NULL);
    if (!env || ((txtobj)->GetChar(pos-1)!='\n' && ltype!=listtype_DEFINITION)) {
	/* either we're not in a list, or we're in the middle of a line in a non-defn list.  So just insert the tabs and quit */
	int count= ((self)->GetIM())->Argument();
	if (rock=='\r') /* Ctrl-J was pressed outside of a list; we don't WANT any tabs */
	    return;
	(self)->PrepareInsertion(FALSE);
	while (count--)
	    (txtobj)->InsertCharacters(pos++, "\t",1);
	(self)->FinishInsertion();
	(self)->SetDotPosition(pos);
	(txtobj)->NotifyObservers(0);
	return;
    }

    begin_flag= env->nestedmark::includeBeginning;
    end_flag= env->nestedmark::includeEnding;
    (env)->SetStyle(TRUE,TRUE);

    if (ltype==listtype_ORDERED || ltype==listtype_UNORDERED) {
	/* ordered or unordered list; add dingbat */
	dingbat_env= (txtobj)->AddDingbat(pos, ltype, env);
	dingbat_len= (dingbat_env)->GetLength();
	dingbat_pos= (dingbat_env)->Eval();
	(txtobj)->RegionModified(pos, dingbat_len+2);
	(self)->SetDotPosition(dingbat_pos + dingbat_len);
    } else
	/* definition list; terminate or apply term style as necessary */
	endTerm(self, rock);

    (env)->SetStyle(begin_flag,end_flag);
    (txtobj)->NotifyObservers(0);
}

void newlineAddItem(htmltextview *self, long rock)
{
    htmltext *ht= (htmltext *)(self)->GetDataObject();
    long pos= (self)->CollapseDot();
    htmlenv *env;
    boolean found= FALSE, begflg, endflg;
    int count= ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg(); /* don't want textview-insert-newline to see it */

    if ((self)->ConfirmReadOnly()) return;

    /* terminate existing term style if there */
    env= (htmlenv *)(self)->GetInsertEnvironment(pos);
    if (env && env->type==environment_Style && strcmp((env->environment::data.style)->GetName(), "term")==0)
	doproc(self, "textview-plainer", 0);

    /* add newline(s) with list items on them */
    while (count--) {
	doproc(self, "textview-insert-newline", (long)'\n'); /* cheesy way to insert a newline and get lastCmd set to lcInsertNL */
	addItem(self, '\r');
	pos= (self)->GetDotPosition();
    }
}


static mark *sourcemark= NULL;
static class linkview *autolink_source= NULL;

/* XXX- not totally completed.  can only set a link WITHIN a file, until a non-AFS URL can be associated with a buffer */
void setTargetLink(frame *frame, long param)
{
    /* First, checks to see if there is a link object waiting for an autolink.  Then, if checks to make sure this frame has a buffer, and that the buffer has a filename. */
    buffer *fb;
    char *fn;

    htmltextview *tar_hv;	/* view of the target */
    htmltext *src_hto;	/* the text object of the source of the link */
    htmltext *tar_hto;	/* the text object of the target of the link */
    char *name;			/* file name of the target */

    if (!sourcemark) {
	message::DisplayString(frame, 50, "Please set sourcelink source first.");
	return;
    }
    if ((sourcemark)->ObjectFree()) {
	message::DisplayString(frame, 50, "Please set sourcelink source first.");
	delete sourcemark;
	sourcemark= NULL;
	return;
    }
    if (!ATK::IsTypeByName((frame)->GetTypeName(),"frame")) {
	message::DisplayString(frame, 10, "Sourcelink works on frames only");
	return;
    }
    if ((fb= (buffer *)((frame))->GetBuffer())==NULL) {
	message::DisplayString(frame, 10, "Sourcelink works with buffers only");
	return;
    }
    if ((fn= ((buffer *)fb)->GetFilename())==NULL) {
	message::DisplayString(frame, 50, "Can only sourcelink to buffers on files.");
	return;
    } 
    name= (char *)malloc(strlen(fn) +1);
    strcpy(name, fn);
    printf("the file name for the target of the link = [%s]\n", name);

    src_hto= (htmltext*)(sourcemark)->GetObject();

    /* do something with the source textobject, like style with anchor style */
    /* if textview, also set pos and len */
    tar_hv= (htmltextview *)((frame))->GetView();
    tar_hto= (htmltext *) (tar_hv)->GetDataObject();

    if (tar_hv != NULL && ATK::IsTypeByName((tar_hv)->GetTypeName(), "htmltextview")) {
	/* do something like make an id if one isn't already there and stuff */
	long pos, len, i=0;
	htmlenv *first_env, *env, *rootenv;
	char *value, *begin, *end, hrefval[128];

	rootenv= (htmlenv *)tar_hto->text::rootEnvironment;

	/* get the position of the target of the mark */
	pos= (tar_hv)->GetDotPosition();
	len= (tar_hv)->GetDotLength();

	if (env= (htmlenv *)(rootenv)->GetInnerMost(pos)) {
	    first_env= env;
	    while (env != rootenv) {
		if (value= (env)->GetAttribValue("id"))
		    break;
		env= (htmlenv *)(env)->GetParent();
	    }
	    if (value)
		printf("id = %ld [%s]\n", i, value);
	    else {
		char newatts[128];
		long len;  int rc;  
		rc= message::AskForString(tar_hv, 0, "No id found at target. Enter one to be added there. id = ", NULL, newatts, sizeof(newatts));
		if (rc < 0) return;

		begin= skipwhitespace(newatts);
		if (*begin == '\0') return;

		end= skiptokenchars(begin); *end= '\0';
		if (*begin == '"') ++begin;
		if (*(end -1) == '"') *(--end) = '\0';

#define DOUBLEQUOTECHAR '"' /* work around a parser bug on OS/2 builds */
               ((first_env)->GetAttribs())-> AddAttribute("id", begin, *(begin-1) == DOUBLEQUOTECHAR);
#undef DOUBLEQUOTECHAR
	value= begin;
	    }
	}
	/* get the position of the source of the mark */
	pos= (sourcemark)->GetPos();
	len= (sourcemark)->GetLength();
	/* wrap anchor style and set it href to the id of the target */
	env=(htmlenv *)(src_hto)->AddStyle(pos, len, ((src_hto)->GetStyleSheet())->Find("anchor"));

	((env)->GetAttribs())->AddAttribute("href", hrefval, FALSE);
	(env)->SetStyle(FALSE, FALSE);
    }
    free(name);
    (src_hto)->RemoveMark(sourcemark);
    sourcemark= NULL;
}

/* sets the link from an anchor to destination */
void setSourceLink(htmltextview *self, long param)
{
    /* Start the autolink process.  Check to make sure we're not trouncing on another autolink first though.... */
    static char *conflict[]= {
	"Use new sourcelink source",
	"Use old sourcelink source",
	"Cancel sourcelink",
	NULL
    };
    long answer;
    long pos, len;
    htmltext *hto= (htmltext *)(self)->GetDataObject();

    pos= (self)->GetDotPosition();
    len= (self)->GetDotLength();

    if (sourcemark) {
	if (message::MultipleChoiceQuestion(self,99,"Already sourcelinking!", 2, &answer, conflict, NULL)>= 0) {
	    switch(answer) {
		case 0:
		    message::DisplayString(self, 10, "Please indicate sourcelink target buffer.");
		    message::DisplayString(self, 10, "Cancelled.  Using new source.");
		    (hto)->RemoveMark(sourcemark);
		    sourcemark= (hto)->CreateMark(pos, len);
		    return;
		case 1:
		    message::DisplayString(self, 10, "Cancelled.  Using old source");
		    message::DisplayString(self, 10, "Please indicate sourcelink target buffer with the linkview-set-target proc.");
		    return;
		case 2:
		    message::DisplayString(self, 10, "sourcelink cancelled.");
		    message::DisplayString(self, 10, "sourcelink cancelled.");
		    (hto)->RemoveMark(sourcemark);
		    sourcemark= NULL;
		    return;
	    }
	} else {
	    message::DisplayString(self, 10, "sourcelink cancelled.");
	    message::DisplayString(self, 10, "sourcelink cancelled.");
	    (hto)->RemoveMark(sourcemark);
	    sourcemark= NULL;
	    return;
	}
    }

    message::DisplayString(self, 10, "Please indicate sourcelink target buffer.");
    sourcemark= (hto)->CreateMark(pos, len);

}


void addLink(htmltextview *self, long rock)
{
    char newURL[256], *p= newURL;
    long pos, len;
    htmlenv *env;
    htmltext *hto= (htmltext *)(self)->GetDataObject();

    if ((self)->ConfirmReadOnly()) return;

    pos= (self)->GetDotPosition();
    len= (self)->GetDotLength();
    if (len == 0) {
	message::DisplayString(self, 10, "You must highlight the text you wish to make into a link.");
	return;
    }
    /* get URL, wrap anchor style, set href of anchor style */
    if (message::AskForString(self, 0, "URL to link to (prefix with '#' if within this document): ", NULL, newURL, sizeof(newURL)) < 0)
	return; /* Cancelled. */

    env= (htmlenv *)(hto)->AddStyle(pos, len, ((hto)->GetStyleSheet())->Find("anchor"));

    ((env)->GetAttribs())->AddAttribute("href", p, TRUE /* quote it */);
    (env)->SetStyle(FALSE, FALSE);
}

void addTarget(htmltextview *self, long rock)
{
    char newname[256];
    long pos, len;
    htmlenv *env;
    htmltext *hto= (htmltext *)(self)->GetDataObject();

    if (self->ConfirmReadOnly()) return;

    pos= self->GetDotPosition();
    len= self->GetDotLength();
    if (len == 0) {
	env= (htmlenv *)(hto->text::rootEnvironment)->GetInnerMost(pos);
	while (env) {
	    if (env->type==environment_Style && strcmp((env->data.style)->GetName(), "anchortarget")==0)
		break;
	    env= (htmlenv *)env->GetParent();
	}
	if (!env) { /* no text selected, and no anchortarget style already there; punt */
	    message::DisplayString(self, 10, "You must highlight the text you wish to make into an anchor target.");
	    return;
	}
	/* prompt for new name for existing anchortarget style */
	struct htmlatts *nameAtt= (env->GetAttribs())->GetAttribute("name");
	if (nameAtt) {
	    char *oldname= nameAtt->value;
	    strcpy(newname, oldname);
	    if (message::AskForString(self, 0, "New name for anchor target: ", newname, newname, sizeof(newname)) < 0)
		return; /* cancelled */
	    nameAtt->value= (char *)malloc(strlen(newname) +1);
	    strcpy(nameAtt->value, newname);
	}
    } else {
	/* get name, wrap anchortarget style, set name of anchortarget style */
	if (message::AskForString(self, 0, "Name of new anchor target: ", NULL, newname, sizeof(newname)) < 0)
	    return; /* Cancelled. */

	env= (htmlenv *)(hto)->AddStyle(pos, len, (hto->GetStyleSheet())->Find("anchortarget"));

	(env->GetAttribs())->AddAttribute("name", newname, TRUE /* quote it */);
	env->SetStyle(FALSE, FALSE);
    }
}


int displayAtts(htmltextview *self, htmlenv *env, htmlenv *rootenv)
{
    attlist *atts;
    char newatts[1024], *s, *label;
    const char *sname;
    int rc;
    if (env && env!=rootenv) {
	s= ((env)->GetAttribs())->MakeIntoString();

	if (env->environment::type == environment_Style)
	    sname= (env->environment::data.style)->GetName();
	else if (env->environment::type == environment_View) {  /* view */
	    sname= ((dataobject *) env->environment::data.viewref->dataObject)->GetTypeName();
	    if (ATK::IsTypeByName(sname, "img")) sname= "img";
	    else if (ATK::IsTypeByName(sname, "gif")) sname= "img";
	    else if (ATK::IsTypeByName(sname, "jpeg")) sname= "img";
	    else if (ATK::IsTypeByName(sname, "hr")) sname= "hr";
	    else if (ATK::IsTypeByName(sname, "table")) sname= "table";
	    else if (ATK::IsTypeByName(sname, "fnote")) sname= "fnote";
	}
	label= (char *)malloc(strlen(sname) + strlen (" Attributes: ") + 1);
	sprintf(label, "%s Attributes: ", sname);

	if ((self)->ConfirmReadOnly()) {
	    char *p= (char *)malloc(strlen(label) + strlen(s) + 1);
	    sprintf(p, "%s%s", label, s);
	    message::DisplayString(self, 0, p);
	    rc= -1;	/* we don't want to notifyobservers or setmodified  since we are only displaying the string */
	    free(p);
	} else {
	    rc= message::AskForString(self, 0, label, s, newatts, sizeof(newatts));
	    if (rc >= 0) {
		message::DisplayString(self, 0, "");
		atts= attlist::ParseAttributes(newatts);
		/* change the attributes of the environment */
		(env)->SetAttribs(atts);
	    }
	}
	free(label);
    }
    return rc;
}

/* displays the attributes of an environment in the message window */
void showAttributes(htmltextview *self)
{
    htmlenv *rootenv= (htmlenv *)(((htmltext *)(self)->GetDataObject())->text::rootEnvironment);
    htmlenv *env;

    long dis_pos= 0;	/* start position of the environment found by the last alt-s */
    long dis_len= 0;	/* length of the environment found by the last alt-s */
    long cur_pos= 0;
    long cur_len= 0;
    int rc= 0;    

    if (self->textview::displayEnvironment) {
	dis_pos= ((htmlenv *)self->textview::displayEnvironment)->Eval();
	dis_len= ((htmlenv *)self->textview::displayEnvironment)->GetLength();
    }

    cur_pos= (self)->GetDotPosition();
    cur_len= (self)->GetDotLength();

    /*    if (im_GetLastCmd(textview_GetIM(self)) == textview_lcCommand(textview_lcDisplayEnvironment)) { */
    if (dis_pos == cur_pos && dis_len == cur_len) {
	if (env= (htmlenv *)self->textview::displayEnvironment)
	    rc= displayAtts(self, env, rootenv);
    } else {
	env= rootenv;

	if (!(env)->GetChild(cur_pos)) {
	    long new_pos;
	    env= (htmlenv *)(env)->GetNextChild(NULL, cur_pos);
	    if (env) {
		new_pos= (env)->Eval();
		if (new_pos-cur_pos < cur_len) {
		    cur_len -= (new_pos -cur_pos);
		    cur_pos= (env)->Eval();
		} else /* no environments in the region highlighted */
		    goto NoStylesHere;
	    }
	}

	/* loop from the inner most environment to the root environment to find the best match  */
	env= (htmlenv *)(rootenv)->GetInnerMost(cur_pos);
	do  {
	    if (cur_pos==(env)->Eval() && cur_len<=(env)->GetLength())
		break;
	} while (env=(htmlenv *)(env)->GetParent());
	if (!env) env= (htmlenv *)(rootenv)->GetInnerMost(cur_pos);
	if (env && env!=rootenv)
	    rc= displayAtts(self, env, rootenv);
	else
	    goto NoStylesHere;
    }

    if (rc >= 0) {
	((htmltext *)(self)->GetDataObject())->SetModified();
	((htmltext *)(self)->GetDataObject())->NotifyObservers(0);
    }
    return;

    NoStylesHere: ; /* jump here if we can't find an environment to edit the attributes OF */
    message::DisplayString(self, 0, "No styles found at cursor position; cannot view or edit attributes.");
    return;
}

void insertImage(htmltextview *self, long rock)
{
    htmltext *txt;
    htmlenv *imgEnv;
    char url[1024];

    if ((self)->ConfirmReadOnly())
	return;
    txt= (htmltext *)(self)->GetDataObject();
    if (!rock || (unsigned long)rock<256) { /* only ask if no string parm given */
	if (message::AskForString(self,0, "Filename or URL of image: ", NULL,url,sizeof(url)) < 0)
	    return; /* cancelled */
    } else strcpy(url, (char *)rock); /* use the one given in .ezinit */
    if (url[0]=='\0') {
	message::DisplayString(self,0, "No URL or path specified; cancelled.");
	return;
    }
    imgEnv= (txt)->AddImage((self)->CollapseDot(), url);
    if (imgEnv)
	/* add attributes to environment so we can save a meaningful HTML+ tag */
	((imgEnv)->GetAttribs())->AddAttribute("src", url, TRUE);
}

/* friendly read-only behavior stolen from txtvcmod.c */
boolean htmltextview::ConfirmReadOnly()
{
    if (((htmltext *)GetDataObject())->GetReadOnly()) {
	message::DisplayString(this, 0, "Document is read only.");
	return TRUE;
    } else
	return FALSE;
}

void htmltextview::SetDataObject(class dataobject *dataObject)
{
    htmltext *htmlobj= (htmltext *)dataObject;
    this->textview::SetDataObject(dataObject);
    if (!ATK::IsTypeByName(dataObject->GetTypeName(), "htmltext"))
	return;
    if (htmlobj->text::templateName==NULL) {
	/* must force-read template for "Show Normal", cells in table insets, and when user didn't specify template on addfiletype line */
	htmlobj->text::templateName= (char *)malloc(5);
	strcpy(htmlobj->text::templateName, "html");
	htmlobj->ReadTemplate("html", FALSE);
    }
}


void htmltextview::PostMenus(menulist *menulist)
{
    (this->html_menus)->ChainBeforeML(menulist, (long)this);
    (this->html_menus)->SetMask((menulist)->GetMask());
    (this)->textview::PostMenus(this->html_menus);
}

void htmltextview::PostKeyState(class keystate *keystate)
{
    /* XXX- Generic textview bindings defined by the user can override ours.  "That ain't good" */
    if (this->textview::hasInputFocus) {
	(this->html_state)->AddBefore(keystate);
	(this)->textview::PostKeyState(this->html_state);
    } else 
	(this)->textview::PostKeyState(keystate);
}


boolean htmltextview::InitializeClass()  {
    struct proctable_Entry *proc;

    html_Menus= new menulist;
    html_Map= new keymap;

    bind::BindList(htmltextBindings,html_Map,html_Menus,
                   &htmltextview_ATKregistry_);
    bind::BindList(addltextBindings, html_Map, html_Menus,ATK::LoadClass("textview"));

    proctable::DefineProc("htmltextview-set-target", 
		(proctable_fptr)setTargetLink, ATK::LoadClass("frame"), NULL, 
		"Execute this proc from the frame of the the buffer for the target file of a link.  To be called after htmltextview-set-source.");

    proc= proctable::DefineProc("hiddenview-show-normal", 
		(proctable_fptr)showNormal, ATK::LoadClass("hiddenview"), 
		"hiddenview", "Return to WYSIWYG HTML+ view.");
    hiddenview::hid_Menus->AddToML("File~10,Show Normal HTML+~80",
		proc, 0, hiddenview_FullScreenMenus);

#if 0 /* not in RobK's version... */
   proc = proctable::DefineProc("textview-insert-inset-here",  
			NULL, NULL, NULL, NULL);
    (html_Menus)->AddToML("Page~9,Insert Horiz Rule~5", proc, 
			(long)"hr", textview_NotReadOnlyMenus);
    (html_Menus)->AddToML("Page~9,Inset raw HTML+~40", proc, 
			(long)"hidden", textview_NotReadOnlyMenus);
    (html_Menus)->AddToML("Media,Table", proc, 
                          (long)"table", textview_NotReadOnlyMenus);
#endif
    return TRUE;
}

htmltextview::htmltextview()
{
    ATKinit;
    this->html_state= keystate::Create(this, html_Map);
    this->html_state->next= NULL;
    this->html_menus= (html_Menus)->DuplicateML(this);
    DisplayingHref = FALSE;
    THROWONFAILURE(TRUE);
}

htmltextview::~htmltextview()
{
    ATKinit;
    return;
}


void lookforid(htmltextview *self, long param)
{
    htmltext *hto= (htmltext *)(self)->GetDataObject();
    long len= (hto)->GetLength(), val_len, pos;
    char id[64], *begin, *end, *value=NULL;
    int rc;
    char *lookfor= (char *)param;

    if (!lookfor) {
	rc= message::AskForString(self, 0, "id = ", NULL, id, sizeof(id));
	if (rc < 0) return;

	begin= skipwhitespace(id);
	if (*begin == '\0') return;
	end= skiptokenchars(begin);
	val_len= end-begin;
	value= (char *) malloc (val_len+1);
	strncpy(value, begin, val_len); value[val_len]= '\0';

	lookfor= value;
    }
    pos= (hto)->FindDestId(lookfor);
    (self)->SetDotPosition(pos);
    (self)->SetDotLength(0);
    (self)->SetTopPosition(pos);

    if (value) free(value);
}

view *htmltextview::Hit(enum view_MouseAction action, long x, long y, long numberOfClicks) {
    view *vptr;
    if(action==view_UpMovement || action==view_LeftUp) hitpos= Locate(x,y, &vptr);

    if (action == view_UpMovement)  {
	htmlenv *rootenv=(htmlenv*)
	  (((htmltext*)GetDataObject())
	   ->text::rootEnvironment);
	htmlenv *env= (htmlenv *)
	  (rootenv)->GetInnerMost(hitpos);
	if (env) {
	    while (env != rootenv) {
		const char *p= (env->type==environment_Style) 
		  ? (env->environment::data.style)
		  ->GetName() 
		  : "";
		if (strcmp(p, "anchor") == 0) break;
		env= (htmlenv *)(env)->GetParent();
	    }
	}
	if (env && env != rootenv) {
	    // it is an anchor
	    char *href_value
	      = (env)->GetAttribValue("href");
	    if (href_value) {
		message::DisplayString(this, 0, href_value);
		DisplayingHref = TRUE;
	    }
	    // no further processing of anchor
	    return this;
	}
	else if (DisplayingHref) {
	    message::DisplayString(this, 0, "");
	    DisplayingHref = FALSE;
	}
    } 
    return (this)->textview::Hit(action, x, y, numberOfClicks);
}


/* 
 * $Log: htmltextview.C,v $
 * Revision 1.7  1996/09/04  18:06:08  robr
 * Syncing with rochester
 *
 * Revision 1.6  1996/07/26  17:42:56  robr
 * Fixed Hit method to never return NULL and to record the position of
 * the last LeftUp mouse event.
 * FEATURE/BUG
 *
 * Revision 1.5  1996/07/06  03:07:34  wjh
 * 	update to display URL as cursor moves over anchor
 *
 * Revision 1.4  1996/05/02  01:58:36  wjh
 * reformatted bind desription entries
 * fixed bug: claimed that it implemented textview-insert-inset-here
 * added std hdr
 *
 */
