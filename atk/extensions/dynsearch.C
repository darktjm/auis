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

#include <andrewos.h>
ATK_IMPL("dynsearch.H")
#include <text.H>
#include <textview.H>
#include <im.H>
#include <search.H>
#include <message.H>
#include <proctable.H>

#include <dynsearch.H>

#define MAXSTRING	256
/* Describe how the current seach string has failed. */
#define FORWARD		1
#define REVERSE		2

static char lastString[MAXSTRING] = "";
static char *lastPattern;
static int lcSearch = 0;
static int failures = 0;
static int lastDirection = FORWARD;




ATKdefineRegistry(dynsearch, ATK, dynsearch::InitializeClass);
static void dynsearch_SearchForward(register class textview  *view);
static void dynsearch_SearchReverse(register class textview  *view);
static int GetPattern(class textview  *view		/* textview we're using */, class text  *text		/* and doc */, int  direction			/* direction of the search */);
void dynsearch_SearchAgain(class textview  *self);
void dynsearch_SearchAgainOpposite(class textview  *self);


static void dynsearch_SearchForward(register class textview  *view)
    {

    int pos = 0, argument;
    register int count;
    register class text *text;

    lastDirection = FORWARD;
    text = (class text *) view->dataobject;
    argument = ((view)->GetIM())->Argument();
    if (GetPattern(view, text, FORWARD) < 0)
	return;
    (view)->SetDotPosition( (view)->GetDotPosition() + (view)->GetDotLength());
    for (count = 0; count < argument; count++) {
	pos = search::MatchPattern (text, (view)->GetDotPosition(), (struct SearchPattern *)lastPattern);
	if (pos < 0) {
	    (view)->SetDotLength(0);
	    message::DisplayString(view, 0, "Search failed.");
	    failures |= FORWARD;
	    (view)->WantUpdate( view);
	    return;
	}
	failures = 0;
	(view)->SetDotPosition( pos);
	(view)->SetDotLength( search::GetMatchLength());
    }
    (view)->FrameDot( pos);
    (view)->WantUpdate( view);
    message::DisplayString(view, 0, "Done.");
    return;
}

static void dynsearch_SearchReverse(register class textview  *view)
    {

    int argument, originalPos, pos = 0;
    register int count;
    register class text *text;

    lastDirection = REVERSE;
    text = (class text *) view->dataobject;
    originalPos = (view)->GetDotPosition();
    argument = ((view)->GetIM())->Argument();
    if (GetPattern(view, text, REVERSE) < 0)
	return;
    (view)->SetDotLength( 0);
    if (originalPos > 0)
        (view)->SetDotPosition( originalPos - 1);
    for (count = 0; count < argument; count++) {
	pos = search::MatchPatternReverse(text, (view)->GetDotPosition(), (struct SearchPattern *)lastPattern);
	if (pos < 0) {
	    (view)->SetDotPosition( originalPos);
            message::DisplayString(view, 0, "Reverse search failed.");
	    failures |= REVERSE;
	    (view)->WantUpdate( view);
	    return;
	}
	failures = 0;
	(view)->SetDotPosition( pos);
	(view)->SetDotLength( search::GetMatchLength());
    }
    (view)->FrameDot( pos);
    (view)->WantUpdate( view);
    message::DisplayString(view, 0, "Done.");
    return;
}


static int GetPattern(class textview  *view		/* textview we're using */, class text  *text		/* and doc */, int  direction			/* direction of the search */)
            {

    char string[MAXSTRING];
    int pos, len;
    int useLast = FALSE;
    int lastCmdWasSearch;
    int lastSearchFailed = FALSE;

    pos = (view)->GetDotPosition();
    len = (view)->GetDotLength();
    lastCmdWasSearch = ((view)->GetIM())->GetLastCmd() == lcSearch;
    if (lastCmdWasSearch)
	lastSearchFailed = (failures & direction);
    /* Now figure out what pattern to use. */
    if (lastCmdWasSearch && !lastSearchFailed)
	useLast = TRUE;
    else if (lastCmdWasSearch || len == 0) {

        char prompt[MAXSTRING + sizeof("Reverse search for : ")];

        if (*lastString != '\0')
            sprintf(prompt, "%s for [%s] : ", (direction == FORWARD) ? "Search" : "Reverse search", lastString);
        else
            sprintf(prompt, "%s for: ", (direction == FORWARD) ? "Search" : "Reverse search");
	if (message::AskForString(view, 0, prompt, NULL, string, 100) < 0)
	    return -1;
	if (string[0] == 0)
	    useLast = TRUE;
    }
    else {
	/* Use the current selection. */

	char unquotedString[MAXSTRING];

	if (len >= sizeof(unquotedString) - 1) {
	    message::DisplayString(view, 0, "Search string too long - continuing with truncating string.");
	    len = sizeof(unquotedString) -1;
	}
	    
	(text)->CopySubString( pos, len, unquotedString, FALSE);
	if (search::GetQuotedSearchString(unquotedString, string, sizeof(string)) == NULL) {
	    message::DisplayString(view, 0, "Search string too long - continuing with truncating string.");
	}
	    
	if (strcmp(string, lastString) == 0)
	    useLast = TRUE;
    }
    if (!useLast) {

        const char *errorMessage;

	failures = 0;
	errorMessage = search::CompilePattern(string, (struct SearchPattern **)&lastPattern);
	if (errorMessage != NULL) {
	    message::DisplayString(view, 0, errorMessage);
	    return -1;
	}
	strcpy(lastString, string);
    }
    ((view)->GetIM())->SetLastCmd( lcSearch);
    return 0;
}

void dynsearch_SearchAgain(class textview  *self)
{
    class text *d = (class text *)(self)->GetDataObject();
    long	savePos, pos;

    if (lastPattern != NULL) {
        savePos = pos = (self)->GetDotPosition();

	if (lastDirection == FORWARD) {
            pos = (self)->CollapseDot();
            pos = search::MatchPattern(d, pos, (struct SearchPattern *)lastPattern);
        }
        else {
            (self)->SetDotLength( 0);
            if (pos > 0)
		(self)->SetDotPosition( --pos);
            pos = search::MatchPatternReverse(d, pos, (struct SearchPattern *)lastPattern);
        }
        if (pos < 0)
	{
            message::DisplayString(self, 0, "Search failed.");
	    if ( !lastDirection )
        	(self)->SetDotPosition( savePos);
	}
        else {
            (self)->SetDotPosition(pos);
            (self)->SetDotLength( search::GetMatchLength());
            (self)->FrameDot(pos);
            (self)->WantUpdate( self);
        }
    }
    else
        message::DisplayString(self, 0, "Must have searched at least once to search again.");
}

void dynsearch_SearchAgainOpposite(class textview  *self)
{
    if (lastDirection == FORWARD) {
	lastDirection = REVERSE;
    }
    else {
	lastDirection = FORWARD;
    }
    dynsearch_SearchAgain(self);
}

boolean dynsearch::InitializeClass()
    {
    struct ATKregistryEntry  *textviewClassinfo;

    lcSearch = im::AllocLastCmd();
    lastPattern = NULL;
    lastString[0] = 0;

    textviewClassinfo = ATK::LoadClass("textview");
    proctable::DefineProc("dynsearch-search-forward", (proctable_fptr) dynsearch_SearchForward, textviewClassinfo, NULL,
                         "Search forward for a pattern; uses selection.");
    proctable::DefineProc("dynsearch-search-reverse", (proctable_fptr) dynsearch_SearchReverse, textviewClassinfo, NULL,
                         "Search backwards for a pattern; uses selection.");
    proctable::DefineProc("dynsearch-search-again", (proctable_fptr) dynsearch_SearchAgain, textviewClassinfo, NULL,
                         "Search again in the same direction, using the last search pattern.");
    proctable::DefineProc("dynsearch-search-again-opposite", (proctable_fptr) dynsearch_SearchAgainOpposite, textviewClassinfo, NULL,
                         "Search again in the opposite direction, using the last search pattern.");
    return TRUE;
}
