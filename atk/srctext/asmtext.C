/* File asmtext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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
 
   asmtext, an object for editing Assembly Language code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <attribs.h>
#include <toolcnt.h>

#include "srctext.H"
#include "asmtext.H"

static Dict *words[TABLESIZE];

ATKdefineRegistry(asmtext, srctext, asmtext::InitializeClass);

/* addBangComment postpends a character to the string used to remember bang-comment chars.  This is of little use, except to make the info available to the VIEW, which needs it to create the proper keybindings */
static void addBangComment(asmtext *self, const char *ch)
{
    int indx=0;
    while (indx<MAX_BANGCHARS && self->bangComments[indx]!='\0')
	++indx;
    if (indx>=MAX_BANGCHARS) {
	printf("\nWARNING - you have exceeded the maximum of %d bang-comments.  `%c' ignored.", MAX_BANGCHARS, *ch);
	fflush(stdout);
	return;
    }
    /* if null string passed, assume semicolon */
    self->bangComments[indx]= (ch!=NULL)?(*ch):(';');
    self->bangComments[indx+1]= '\0';
}

/* setReindentFilterName remembers the name of the filter that was specified in ezinit */
static void setReindentFilterName(asmtext *self, const char *name)
{
    if (self->reindentFilterName!=NULL)
	free(self->reindentFilterName);
    self->reindentFilterName= strdup(name);
}

void asmtext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
    while (atts!=NULL) {
	if (strcmp(atts->key,"bang-comment")==0)
	    addBangComment(this,atts->value.string);
	else if (strcmp(atts->key,"c-comments")==0)
	    SetCComments(atoi(atts->value.string));
	else if (strcmp(atts->key,"reindent-filter-name")==0) {
	    setReindentFilterName(this,atts->value.string);
	    this->srctext::indentingEnabled= TRUE;
	}
	atts=atts->next;
    }
}

boolean asmtext::InitializeClass()
{
    static Dict asmkeywords[]={
	{NULL,0,0}
    };
    srctext::BuildTable("asmtext",::words,asmkeywords);
    return TRUE;
}

asmtext::asmtext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->srctext::useTabs= FALSE;
    this->bangComments[0]= '\0';
    this->CComments= FALSE;
    this->reindentFilterName= NULL;
    ToolCount("EditViews-asmtext",NULL);
    THROWONFAILURE(TRUE);
}

void asmtext::RedoStyles()
{
    long posn, len = GetLength();
    int prev=0, c = '\n';
    /* c is initialized to a newline so the start of the file looks like the start of line. */
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn=0; posn<len-1; ++posn) {
	prev = c;
	posn=SkipWhitespace(posn,len);
	c = GetChar(posn);
	switch (c) {
	    case '\n':
	    case ' ': case '\t':
		/* common characters that can't be token chars or bang comments-- skip for speed */
		break;
	    case '*':
		if (prev=='/' && UseCComments()) {
		    posn= CheckComment(posn-1);
		    c= '\n';
		    break;
		}
		/* fall thru in case no C comments, but * IS a bang comment */
	    default:
		/* see if this is a bang-comment character */
		if (strchr(this->bangComments,c)!=NULL) {
		    posn= CheckLinecomment(posn);
		    c= '\n';
		}
		if (IsTokenChar(c))
		    /* might be a user-defined keyword */
		    posn= CheckWord(posn,len);
	}
    }
}

/* override */
/* in srctext, Indent is called when Tab is hit with a region selected. This is being overridden in asmtext because asmtextview's Reindent should override srctextview's, and should never ever call this */
long asmtext::Indent(mark *mark)
{
    printf("\nERROR - asmtext__Indent empty override entered.\n");
    fflush(stdout);
    return 0;
}

/* asmtext_Keywordify makes buff all uppercase.  checkforceupper and Force Upper status are ignored. */
/* This function "keywordifies" a string. "Keywordify" means "make the word all upper case so it will be found in the hash table". */
char *asmtext::Keywordify(char *buff, boolean checkforceupper)
{
    if (buff!=NULL && strlen(buff)>0)
	makeupper(buff);
    return buff;
}
