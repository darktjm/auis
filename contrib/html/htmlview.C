/* $Id$ */
/*
 * Copyright 1993, City University
 * Copyright 1993, 1994, Nick Williams. 
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose is hereby granted without fee, 
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice, this permission notice, and the following 
 * disclaimer appear in supporting documentation, and that the names of 
 * City University, Nick Williams, and other copyright holders, not be 
 * used in advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.
 *
 * City University, Nick Williams, AND THE OTHER COPYRIGHT HOLDERS 
 * DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT 
 * SHALL City University, Nick Williams, OR ANY OTHER COPYRIGHT HOLDER 
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS 
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <andrewos.h>


#include <bind.H>
#include <buffer.H>
#include <ctype.h>
#include <environment.H>
#include <environ.H>
#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <proctable.H>
#include <textview.H>
#include <txtstvec.h>
#include <style.H>
#include <view.H>

#include <html.H>
#include <htmlview.H>

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

static class keymap *Keymap;
static class menulist *Menus;














static char* bulletChars = "*+";
ATKdefineRegistry(htmlview, textview, htmlview::InitializeClass);
static boolean ConfirmReadOnly(class htmlview  *self, class html * html);
void htmlview_SetTitle(class htmlview * self, long  key);
void htmlview_SetIndex(class htmlview * self, long  key);
void htmlview_SetLink(class htmlview * self, long  key);
void htmlview_EditAttributes(class htmlview * self, long  key);
void htmlview_AddRandom(class htmlview * self, long  key);
void htmlview_AddImage(class htmlview * self, long  key);
void htmlview_SetImage(class htmlview * self, long  key);
void htmlview_AddHrule(class htmlview * self, long  key);
static int parse_num (class html  *html, int  start , int  end , int  *numret);
int checkEnumerate(class html * html, long  pos, long  end, int * the_number);
char* stringEnumerate(int * the_number /* datum1 */);
int lineEnumerate(class html * html, long * pos, long * end, int * the_number	 );
int checkBullet(class html * html, long  pos, long  end, int * datum);
char* stringBullet(int * datum);
int lineBullet(class html * html, long * pos, long * end, int * datum);
int checkGlossary(class html * html, long  pos, long  end, int * datum);
char* stringGlossary( int * datum);
int lineGlossary(class html * html, long * pos, long * end, int * datum);
void  htmlview_makeList (class htmlview  *self, char * listStyleName);
void htmlview_unlistify (class htmlview  *self, long  key);
void htmlview_modifyList(class htmlview * self, long  key);

/* These are bindings to commands only, not the menu bits themselves */
/* Place the menu entry for these commands in the stylesheet */
static struct bind_Description htmlBindings[]={
    {"htmlview-set-title",0,0,0,0, 0,(proctable_fptr)htmlview_SetTitle,"Set title of document."},
    {"htmlview-set-indexable",0,0,0,0, 0,(proctable_fptr)htmlview_SetIndex,"Set flag specifying if document is searchable."},
    {"htmlview-set-link",0,0,0,0,0,(proctable_fptr)htmlview_SetLink,"Turn a region into an active anchor"},
    {"htmlview-edit-attributes",0,0,0,0,0,(proctable_fptr)htmlview_EditAttributes,"Edit the attributes of tag at point"},
    {"htmlview-unlistify", 0,0,0,0,0, (proctable_fptr)htmlview_unlistify,"Remove all listyness about point or region"},
    {"htmlview-add-random", 0,0,0,0,0, (proctable_fptr)htmlview_AddRandom, "Add a random entity to a region of text"},
    {"htmlview-add-image", 0,0,0,0,0, (proctable_fptr)htmlview_AddImage, "Add an image"},
    {"htmlview-add-hrule", 0,0,0,0,0, (proctable_fptr)htmlview_AddHrule, "Add a horizontal rule"},
    {"htmlview-make-list",0,0,0,0,0, (proctable_fptr)htmlview_makeList, "Make a region into a list"},
    {"htmlview-set-img-src",0,0,0,0,0, (proctable_fptr)htmlview_SetImage, "Set the source of an image"},
    {"htmlview-modify-list",0,0,0,0,0, (proctable_fptr)htmlview_modifyList, "Modify an attribute of a list"},
    NULL
};

/* Added friendly read-only behavior from txtvcmds.c */



static boolean ConfirmReadOnly(class htmlview  *self, class html * html)
{
    if ((html)->GetReadOnly()) {
        message::DisplayString(self, 0,
          "Document is read only.");
        return TRUE;
    } else
        return FALSE;
}

boolean 
htmlview::InitializeClass()
{
    Menus  = new menulist;
    Keymap = new keymap;

    bind::BindList(htmlBindings,Keymap,Menus,&htmlview_ATKregistry_ );
    return TRUE;
}


htmlview::htmlview()
{
	ATKinit;

    this->kstate = keystate::Create(this, Keymap);
    this->menus = (Menus)->DuplicateML( this);
    (this)->SetBorder(5,5);
    THROWONFAILURE( TRUE);
}

htmlview::~htmlview()
{
	ATKinit;

    delete this->kstate;
    delete this->menus;
}

void
htmlview::PostKeyState(class keystate * keystate)
{
    this->kstate->next = NULL;
    (this->kstate)->AddBefore( keystate);
    (this)->textview::PostKeyState( this->kstate);
}

void
htmlview::PostMenus(class menulist * menulist)
{
    (this->menus)->ClearChain();

    if (menulist) {
	(this->menus)->ChainAfterML( menulist, 0);
    }

    (this)->textview::PostMenus( this->menus);
}

void
htmlview_SetTitle(class htmlview * self, long  key)
{
    char reply[80];
    class html* h = (class html*) self->view::dataobject;
    if (message::AskForString(self, 0, "New title: ", (h)->GetTitle(), reply, sizeof(reply)) < 0) {
	message::DisplayString(self, 0, "Cancelled");
	return;
    }
    (h)->ChangeTitle( reply);
    message::DisplayString(self, 0, "Done.");
}

void
htmlview_SetIndex(class htmlview * self, long  key)
{
    char reply[80];
    class html* h = (class html*) self->view::dataobject;

    if (message::AskForString(self, 0, "Is the document searchable? [yes/no] ", (h)->GetIsIndex() ? "yes" : "no", reply, sizeof(reply)) < 0) {
	message::DisplayString(self, 0, "Cancelled");
	return;
    }
    (h)->ChangeIndexable( reply[0] == 'y' || reply[0] == 'Y');
    message::DisplayString(self, 0, "Done.");
}

void
htmlview_SetLink(class htmlview * self, long  key)
{
    long pos, len;
    class html* html = (class html*) self->view::dataobject;
    char uri[MAXPATHLEN];

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len == 0) {
	/* XXX: set current insertion style... */
	message::DisplayString(self, 0, "Please mark some text first!");
	return;
    }
    if (message::AskForString(self, 100, "URI to link to: ", 0, uri, sizeof(uri)) < 0) {
	message::DisplayString(self, 0, "Cancelled");
	return;
    }

    (html)->AddLink( pos, len, uri);
}

class view*
htmlview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    class html* html = (class html*) this->view::dataobject;
    long pos, len;
    char* s;
    class view* retv = (this)->textview::Hit( action, x, y, numberOfClicks);
    /* Modified based on comments from Tom Neuendorffer, 28/04/94 */
    /* Now returns the correct view object */
    /* Will only determine anchor details if there was a mouse click */

    if (retv == (class view*) this && 
	(action==view_LeftDown || action==view_RightDown)) {
	pos = (this)->GetDotPosition();
	if (s = (html)->GetAnchorDest( pos)) {
	    message::DisplayString(this, 0, s);
	} else {
	    message::DisplayString(this, 0, "");
	}
    }
    return retv;
}

char* editOptions[] = {
    "Add new variable",
    "Next environment",
    "Done",
    0
};

void
htmlview_EditAttributes(class htmlview * self, long  key)
{
    long pos, len;
    class html* html = (class html*) self->view::dataobject;
    class environment* startEnv;
    char* choices[16]; /* 3 internal, that leaves user with max of 13 */
    char answer[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char titlebuf[MAXPATHLEN];
    int i;
    int count;
    long result;

    pos= (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len == 0) {
	self->styleInQuestion = (html)->GetEntityEnvironment( pos, 0);
    } else {
	self->styleInQuestion = (html)->GetEntityEnvironment( pos, self->textview::displayEnvironment);
    }
    if (!self->styleInQuestion || self->styleInQuestion->type != environment_Style) {
	message::DisplayString(self, 0, "Select an entity first");
	return;
    }

    (html)->GetAttributeList( self->styleInQuestion, choices, &count);
    for (i = 0; i < sizeof(editOptions); i++) {
	choices[count+i] = editOptions[i];
    }
    startEnv = self->styleInQuestion;
    (self)->SetDotPosition( (self->styleInQuestion)->Eval());
    (self)->SetDotLength( (self->styleInQuestion)->GetLength());

    sprintf(titlebuf, "Attributes of this %s", self->styleInQuestion->data.style->name);
    while (message::MultipleChoiceQuestion(self, 100, titlebuf, count+2, &result, choices, NULL) >= 0) {
	if (result < count) {
	    /* Build up the prompt */
	    sprintf(buf, "Value for %s: ", choices[result]);
	    if (message::AskForString(self, 100, buf, (html)->GetAttribute( self->styleInQuestion,choices[result]), answer, sizeof(answer)) >= 0) {
		(html)->ChangeAttribute( self, self->styleInQuestion, choices[result], answer);
	    }
	} else if (result == count) {
	    /* ADD VAR */
	    if (message::AskForString(self, 100, "Give attribute (e.g. foo=bar): ", 0, answer, sizeof(answer)) >= 0) {
		char* s = (char*)strchr(answer, '=');
		if (s) {
		    *s++ = '\0';
		}
		(html)->ChangeAttribute( self, self->styleInQuestion, answer, s);
		/* Rebuild the options */
		(html)->GetAttributeList( self->styleInQuestion, choices, &count);
		for (i = 0; i < sizeof(editOptions); i++) {
		    choices[count+i] = editOptions[i];
		}
	    }
	} else if (result == (count+1)) {
	    /* NEXT */
	    /* first, free the old list */
	    for (i = 0; i < count; i++) {
		free(choices[i]);
	    }
	    /* Now find a new environment */
	    do {
		self->styleInQuestion = (class environment*)self->styleInQuestion->nestedmark::parent;
	    } while (self->styleInQuestion->type != environment_Style);
	    /* And cycle back to the start if we get to the root */
	    if (self->styleInQuestion == html->text::rootEnvironment) {
		self->styleInQuestion = startEnv;
	    }

	    /* build the new list */
	    (html)->GetAttributeList( self->styleInQuestion, choices, &count);
	    for (i = 0; i < sizeof(editOptions); i++) {
		choices[count+i] = editOptions[i];
	    }
	    (self)->SetDotPosition( (self->styleInQuestion)->Eval());
	    (self)->SetDotLength( (self->styleInQuestion)->GetLength());
	    sprintf(titlebuf, "Attributes of this %s", self->styleInQuestion->data.style->name);
	} else {
	    /* DONE */
	    goto endfn;  
	    /*NOTREACHED*/
	}
    }
    message::DisplayString(self, 0, "Cancelled");
    endfn:
      for (i = 0; i < count; i++) {
	  free(choices[i]);
      }
    self->styleInQuestion = 0;
    return;
}


void
htmlview_AddRandom(class htmlview * self, long  key)
{
    class html* html = (class html *)self->view::dataobject;
    char ename[MAXPATHLEN];
    long pos, len;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len == 0) {
	/* XXX: set current insertion style... */
	message::DisplayString(self, 0, "Please mark some text first!");
	return;
    }
    if (message::AskForString(self, 100, "Name of entity: ", 0, ename, sizeof(ename)) < 0) {
	message::DisplayString(self, 0, "Cancelled");
	return;
    }

    (html)->AddEntity( pos, len, ename, 0);
}

void
htmlview_AddImage(class htmlview * self, long  key)
{
    class html* html = (class html *)self->view::dataobject;
    class environment* env;
    char ename[MAXPATHLEN];
    static char vars[256];
    long pos;

    pos = (self)->GetDotPosition();

    if (message::AskForString(self, 100, "Source of image: ", 0, ename, sizeof(ename)) < 0) {
	message::DisplayString(self, 0, "Cancelled");
	return;
    }

    sprintf(vars, "src=%s", ename);
    (html)->AddEntity( pos, 1L, "img", vars);
}

void
htmlview_SetImage(class htmlview * self, long  key)
{
    class html* html = (class html *)self->view::dataobject;
    long pos = (self)->GetDotPosition();
    char* newsrc;

    printf("in SetImage()\n");
    if (!self->styleInQuestion) {
	message::DisplayString(self, 0, "This callback should be used only when editing attributes");
	return;
    }
    printf("Getting attribute\n");
    newsrc = (html)->GetAttribute( self->styleInQuestion, "src");

    printf("Going for update\n");

    /* Find the image here and tell it to load using the new src. */
    /* XXX: Not yet Implemented */
    message::DisplayString(self, 0, "Sorry, image updates not yet implemented");
}

void
htmlview_AddHrule(class htmlview * self, long  key)
{
    class html* html = (class html*) self->view::dataobject;
    long pos = (self)->GetDotPosition();
    (html)->AddEntity( pos, 0, "hr", 0);
}

/* The following functions are all for handling lists */

/*
 * Parse a number out of a html object.
 *
 * Call this with start pointing at the first digit.
 * It will store the number parsed in numret.
 * It will return the count of characters scan'ed.
 * If the number is not immediately followed by a 
 * period and then a space or tab, the count is
 * returned as zero
 * signifying that this number is to be ignored.
 */
static int parse_num (class html  *html, int  start , int  end , int  *numret)
{
    int cur_num = 0, count = 0;
    long cur;

    while (start < end && isdigit (cur = (html)->GetChar( start))) {
	cur_num = 10 * cur_num + (cur - '0');
	start++;
	count++;
    }
    if (count == 0) return 0;
    /* skip over trailing period and whitespace if present */
    if (start < end && (cur = (html)->GetChar( start))	!= '.') return 0;	/* wrong format */
    if (start < end && ((cur = (html)->GetChar( start+1))
			 != ' ' && cur != '\t')) 
	return 0;	/* wrong format */
    count += 2;
    *numret = cur_num;
    return count;
}


int
checkEnumerate(class html * html, long  pos, long  end, int * the_number)
{
    int num;
    if (parse_num(html, pos, end, &num) > 0) {
	*the_number = num + 1;
	return 1;
    } else {
	return 0;
    }
}


char*
stringEnumerate(int * the_number /* datum1 */)
{
    static char numstring[16];
    sprintf(numstring, "%d.\t", *the_number);
    return numstring;
}

/*
 * parse the number at point.  If we have already found the
 * beginning of the list (i.e., we're redoing the numbers),
 * then we should check the number is correct and fix it if not
 */
int
lineEnumerate(class html * html, long * pos, long * end, int * the_number	 ) 
{
    int count;
    int newnum;

    *the_number += 1;

    if ((count = parse_num(html, *pos, *end, &newnum)) <= 0) {
	return 0;
    } else {
	/* We need to replace the number at point. */
	char* numstring;
	int tlen;

	if (newnum != *the_number) {
	    numstring = stringEnumerate(the_number);
	    tlen = strlen(numstring);
	    (html)->ReplaceCharacters( *pos, count, numstring, tlen);
	    *end += (tlen-count);
	    *pos += (tlen-count);
	}
	return 1;
    }
}

int
checkBullet(class html * html, long  pos, long  end, int * datum)
{
    if (strchr(bulletChars, (html)->GetChar( pos))) {
	return 1;
    } else {
	return 0;
    }
}



char*
stringBullet(int * datum)
{
    return "*\t";
}

int
lineBullet(class html * html, long * pos, long * end, int * datum)
{
    return checkBullet(html, *pos, *end, datum);
}

int
checkGlossary(class html * html, long  pos, long  end, int * datum)
{
    /* Need to check if the line has a glossary term at this point. */
    return 0;
}

char*
stringGlossary(int * datum)
{
    return "";
}

int
lineGlossary(class html * html, long * pos, long * end, int * datum)
{
    return 0;
}

typedef int (*html_LookAtFptr)(class html * html, long  pos, long  end, int * the_number);
typedef int (*html_LineFptr)(class html * html, long * pos, long * end, int * the_number	 );
typedef char *(*html_TagStringFptr)(int *datum);

struct listCompileTable {
      char* styleName;
      html_LookAtFptr lookAtFn; /* Check the tag at point to see relevance to new item */
      html_LineFptr lineFn;   /* For each line, this is called at beginning of line */
      html_TagStringFptr computeTagStringFn; /* When doing a tag, this is called to get the string */
      char* itemStyle;
} listTypes[] = {
    { "ol", checkEnumerate, lineEnumerate, stringEnumerate, 0 },
    { "ul", checkBullet, lineBullet, stringBullet, 0 },
    { "menu", checkBullet, lineBullet, stringBullet, 0 },
    { "dir", checkBullet, lineBullet, stringBullet, 0 },
    { "dl", checkGlossary, lineGlossary, stringGlossary, "data-description" },
    { 0 }
};

void 
htmlview_makeList (class htmlview  *self, char * listStyleName)
{
    class html* html = (class html *)self->view::dataobject;
    struct text_statevector sv;
    long dot, pos, npos, count, tlen, len, end, origLen;
    long startPos, itemPos;
    long cur, indent, left;
    int one_only = 0, modified = 0;
    int datum = 0;
    int oldList = 0;
    char* tagString;
    char* itemStyle;
    int diff;

    int i;
    html_LookAtFptr lookAtFn = 0;
    html_LineFptr lineFn = 0;
    html_TagStringFptr computeTagStringFn = 0;

    if (ConfirmReadOnly(self, html)) {
	return;
    }

    /* Work out the functions we want... */
    for (i = 0; listTypes[i].styleName; i++) {
	if (strcmp(listTypes[i].styleName, listStyleName) == 0) {
	    itemStyle = listTypes[i].itemStyle;
	    lookAtFn	       = listTypes[i].lookAtFn;
	    lineFn	       = listTypes[i].lineFn;
	    computeTagStringFn = listTypes[i].computeTagStringFn;
	    break;
	}
    }
    if (lookAtFn == 0) {
	return;
    }

    dot = pos = (self)->GetDotPosition();
    origLen = len = (self)->GetDotLength();

    end = pos + len;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	int newlines = 0;
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((html)->GetChar( pos - 1)) != '\n') pos--;
	end = (html)->GetLength ();
	len = end - pos;
	one_only = 1;

	npos = pos;
	if (npos > 0) 
	    npos--; /* back up over newline we just saw */

	/* back up over additional whitespace between paragraphs */
	while (npos > 0) {
	    cur = (html)->GetChar( npos);
	    if (cur == '\n') {
		if (++newlines > 1) {
		    npos = -1;
		    break;
		}
	    }
	    if (!(cur == ' ' || cur == '\n' || cur == '\t'))
		break;
	    npos--;
	    continue;
	}
	    
	/* Now go to begining of the paragraph */
	while (npos > 0 && ((html)->GetChar( npos - 1)) != '\n') 
	    npos--;

	/* See if the previous paragraph begins with a tag'd item. */
	if (datum = lookAtFn(html, npos, pos, &datum)) {
	    oldList = 1;
	}
    }
    
    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    startPos = pos;
    itemPos = -1;
    while (pos < end) {
	cur = (html)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	/* pos is the start of the line (first non-blank char) */
	(void) (self)->GetStyleInformation( &sv, pos, NULL);
	if (sv.CurLeftMargin == left && sv.CurIndentation == indent) {
	    /* Do the line function... */
	    if (diff = lineFn(html, &pos, &end, &datum)) {
		/* They did it themselves */
		itemPos = -1;
	    } else {
		itemPos = pos;
	    }
	}

	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((html)->GetChar( pos) == '\n') {
		break;
	    }
	}
	/* At end of line, here (pos == '\n') */
	if (itemPos >= 0) {
	    int tlen;

	    tagString = computeTagStringFn(&datum);
	    tlen = strlen(tagString);
	    (html)->TagItem( itemPos, pos-itemPos, tagString, itemStyle, 0);
	    pos += tlen;
	    len += tlen;
	    end += tlen;
	    itemPos = -1;
	    modified = 1;
	}
	pos++;
	if (one_only) break;
    }
    /* End of region */

    /* We only add the list over everything if we didn't find an old wrapping. */
    if (!oldList || origLen > 0) {
	(html)->AddEntity( startPos, pos-startPos, listStyleName, 0);
    }
    if (modified) {
	(html)->NotifyObservers( 0);
	if (!one_only) {
	    (self)->SetDotPosition ( dot);
	    (self)->SetDotLength ( len);
	}
    }
}


void htmlview_unlistify (class htmlview  *self, long  key)
{
    class html *html = (class html *)self->view::dataobject;
    struct text_statevector sv;
    int pos, count, len, end;
    long cur, indent, left;
    int one_only = 0, modified = 0;
    int cur_num;

    if (ConfirmReadOnly(self, html)) {
        return;
    }

    (html)->SetView(self);
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((html)->GetChar( pos - 1)) != '\n') pos--;
	end = (html)->GetLength ();
	len = end - pos;
	one_only = 1;
    }

    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    while (pos < end) {
	cur = (html)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	else {
	    (void) (self)->GetStyleInformation( &sv, pos, NULL);
	    if (sv.CurLeftMargin == left && sv.CurIndentation == indent)
		if (count = (html)->UntagItem( pos)) {
		    end -= count;
		    len -= count;
		    modified = 1;
		}
	}
	if (one_only) break;
	/* go to end of paragraph */
	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((html)->GetChar( pos) == '\n') break;
	}
	pos++;
    }
    if (modified) {
	(html)->NotifyObservers( 0);
	if (!one_only) (self)->SetDotLength ( len);
    }
}


void
htmlview_modifyList(class htmlview * self, long  key)
{
    class html *html = (class html *)self->view::dataobject;
    char* ptr;
    if (!self->styleInQuestion) {
	message::DisplayString(self, 0, "Need to use Edit Attributes to call this");
	return;
    }
    if (ptr = (html)->GetAttribute( self->styleInQuestion, "compact")) {
	/* style_SetNewInterlineSpacing(self->styleInQuestion->data.style, style_InterlineSpacing, -2, style_Points); */
    } else {
	/* style_SetNewInterlineSpacing(self->styleInQuestion->data.style, style_InterlineSpacing, 0, style_Points); */
    }
    (html)->RegionModified( (self)->GetDotPosition(), (self)->GetDotLength());
    (html)->NotifyObservers( 0);
}

