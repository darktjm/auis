/*
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
*/

#include <andrewos.h> /* sys/types.h sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/recsearch.C,v 1.5 1994/12/13 20:35:03 rr2b Stab74 $";
#endif


 

ATK_IMPL("recsearch.H")

#include <im.H>
#include <simpletext.H>
#include <message.H>
#include <search.H>
#include <proctable.H>

#include <recsearch.H>

static void RecSearchCmd(class im  *self, long rock);
static void RecSearchAtTopCmd(class im  *self, long rock);
static void RecSearchResumeCmd(class im  *self, long rock);
static void RecSearchReplaceCmd(class im  *self, long rock);

ATKdefineRegistry(recsearch, ATK, recsearch::InitializeClass);

#define SRCHSTRLEN 100
static view *lastStart;
static struct SearchPattern *lastPattern = NULL;
static char searchString[SRCHSTRLEN] = "";
static class simpletext *replacement;

boolean recsearch::InitializeClass()
{
    ATKregistryEntry *imclass = ATK::LoadClass("im");

    proctable::DefineProc("recsearch-begin",  (proctable_fptr)RecSearchCmd, imclass, NULL, "Begin recursive search.");

    proctable::DefineProc("recsearch-begin-at-top",  (proctable_fptr)RecSearchAtTopCmd, imclass, NULL, "Begin recursive search at top (IM) level.");

    proctable::DefineProc("recsearch-resume",  (proctable_fptr)RecSearchResumeCmd, imclass, NULL, "Resume recursive search.");

    proctable::DefineProc("recsearch-replace",  (proctable_fptr)RecSearchReplaceCmd, imclass, NULL, "Replace target of recsearch with a string.");

    replacement = NULL;
    lastStart = NULL;

    return TRUE;
}

static void DoRecSearch(class im  *self, class view *start, char *givenstr)
{
    char defSrchString[SRCHSTRLEN], *tp, messageBuf[120], *prompt;
    int pos = 0, gf;
    boolean defaultExists = FALSE;
    boolean res;

    if (givenstr && (*givenstr)) {
	strncpy(searchString, givenstr, SRCHSTRLEN);
	searchString[SRCHSTRLEN-1] = '\0';
    }
    else {
	if (*searchString != '\0'){
	    defaultExists = TRUE;
	    strcpy(defSrchString, searchString);
	}
	if (defaultExists) {
	    sprintf(messageBuf, "RecSearch for [%s] : ", defSrchString);
	    prompt = messageBuf;
	}
	else
	    prompt = "RecSearch for: ";
	gf = message::AskForString(self, 0, prompt, NULL, searchString, sizeof(searchString));
	if (gf < 0) return;
	if (defaultExists && *searchString == '\0')
	    strcpy(searchString, defSrchString);
    }

    if (!defaultExists || *searchString != '\0') {
	tp = search::CompilePattern(searchString, &lastPattern);
	if (tp) {
	    message::DisplayString(self, 0, tp);
	    return;
	}
    }

    /*printf("### recsearch: starting at |%s|\n", start->GetTypeName());*/
    lastStart = start;
    res = lastStart->RecSearch(lastPattern, TRUE);
    if (res) {
	struct rectangle dummy,hit;
	lastStart->GetLogicalBounds(&dummy);
	lastStart->RecSrchExpose(dummy,hit);
	message::DisplayString(self, 0, "Found.");
    }
    else
	message::DisplayString(self, 0, "Not found.");

    return;
} 

static void RecSearchCmd(class im  *self, long rock) 
{
    char *givenstr;

    if (rock >= 0 && rock < 256) {
	givenstr = NULL;
    }
    else {
	givenstr = (char *)rock;
    }
    DoRecSearch(self, self->GetInputFocus(), givenstr);
}

static void RecSearchAtTopCmd(class im  *self, long rock) 
{
    char *givenstr;

    if (rock >= 0 && rock < 256) {
	givenstr = NULL;
    }
    else {
	givenstr = (char *)rock;
    }
    DoRecSearch(self, self, givenstr);
}

static void RecSearchResumeCmd(class im  *self, long rock) 
{
    boolean res = FALSE;

    if (!lastStart) {
	message::DisplayString(self, 0, "There is no recsearch in progress.");
	return;
    }

    /*printf("### recsearch: starting at |%s|\n", lastStart->GetTypeName());*/
    res = lastStart->RecSrchResume(lastPattern);

    if (res) {
	struct rectangle dummy,hit;
	lastStart->GetLogicalBounds(&dummy);
	lastStart->RecSrchExpose(dummy,hit);
	message::DisplayString(self, 0, "Found.");
    }
    else
	message::DisplayString(self, 0, "Not found.");

    return;
} 

static void RecSearchReplaceCmd(class im  *self, long rock) 
{
    boolean res;
    int gf;
    char replacestr[SRCHSTRLEN];
    char *givenstr;

    if (!lastStart) {
	message::DisplayString(self, 0, "There is no recsearch in progress.");
	return;
    }

    if (rock >= 0 && rock < 256) {
	givenstr = NULL;
    }
    else {
	givenstr = (char *)rock;
    }

    if (!replacement) 
	replacement = new simpletext;
    else
	replacement->Clear();

    if (givenstr) {
	replacement->InsertCharacters(0, givenstr, strlen(givenstr));
    }
    else {
	gf = message::AskForString(self, 0, "New string: ", NULL, replacestr, sizeof(replacestr));
	if (gf < 0) return;
	replacement->InsertCharacters(0, replacestr, strlen(replacestr));
    }

    res = lastStart->RecSrchReplace(replacement, 0, replacement->GetLength());
    if (res) {
	struct rectangle dummy,hit;
	lastStart->GetLogicalBounds(&dummy);
	lastStart->RecSrchExpose(dummy, hit);
	message::DisplayString(self, 0, "Replaced.");
    }

    return;
} 

