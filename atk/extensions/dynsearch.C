/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
static class search lastPattern;
static int lastPatternValid = 0;
static int lcSearch = 0;
static int failures = 0;
static int lastDirection = FORWARD;




ATKdefineRegistry(dynsearch, ATK, dynsearch::InitializeClass);
static void dynsearch_SearchForward(class textview  *view);
static void dynsearch_SearchReverse(class textview  *view);
static int GetPattern(class textview  *view		/* textview we're using */, class text  *text		/* and doc */, int  direction			/* direction of the search */);
static void dynsearch_SearchAgain(class textview  *self);
static void dynsearch_SearchAgainOpposite(class textview  *self);


static void dynsearch_SearchForward(class textview  *view)
    {

    int pos = 0, argument;
    int count;
    class text *text;

    lastDirection = FORWARD;
    text = (class text *) view->GetDataObject();
    argument = ((view)->GetIM())->Argument();
    if (GetPattern(view, text, FORWARD) < 0)
	return;
    (view)->SetDotPosition( (view)->GetDotPosition() + (view)->GetDotLength());
    for (count = 0; count < argument; count++) {
	pos = lastPattern.MatchPattern (text, (view)->GetDotPosition());
	if (pos < 0) {
	    (view)->SetDotLength(0);
	    message::DisplayString(view, 0, "Search failed.");
	    failures |= FORWARD;
	    (view)->WantUpdate( view);
	    return;
	}
	failures = 0;
	(view)->SetDotPosition( pos);
	(view)->SetDotLength( lastPattern.GetMatchLength());
    }
    (view)->FrameDot( pos);
    (view)->WantUpdate( view);
    message::DisplayString(view, 0, "Done.");
    return;
}

static void dynsearch_SearchReverse(class textview  *view)
    {

    int argument, originalPos, pos = 0;
    int count;
    class text *text;

    lastDirection = REVERSE;
    text = (class text *) view->GetDataObject();
    originalPos = (view)->GetDotPosition();
    argument = ((view)->GetIM())->Argument();
    if (GetPattern(view, text, REVERSE) < 0)
	return;
    (view)->SetDotLength( 0);
    if (originalPos > 0)
        (view)->SetDotPosition( originalPos - 1);
    for (count = 0; count < argument; count++) {
	pos = lastPattern.MatchPatternReverse(text, (view)->GetDotPosition());
	if (pos < 0) {
	    (view)->SetDotPosition( originalPos);
            message::DisplayString(view, 0, "Reverse search failed.");
	    failures |= REVERSE;
	    (view)->WantUpdate( view);
	    return;
	}
	failures = 0;
	(view)->SetDotPosition( pos);
	(view)->SetDotLength( lastPattern.GetMatchLength());
    }
    (view)->FrameDot( pos);
    (view)->WantUpdate( view);
    message::DisplayString(view, 0, "Done.");
    return;
}


static int GetPattern(class textview  *view		/* textview we're using */, class text  *text		/* and doc */, int  direction			/* direction of the search */)
            {

    char string[MAXSTRING];
    unsigned int pos, len;
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
	errorMessage = lastPattern.CompilePattern(string);
	if (errorMessage != NULL) {
	    message::DisplayString(view, 0, errorMessage);
	    return -1;
	}
	lastPatternValid = 1;
	strcpy(lastString, string);
    }
    ((view)->GetIM())->SetLastCmd( lcSearch);
    return 0;
}

static void dynsearch_SearchAgain(class textview  *self)
{
    class text *d = (class text *)(self)->GetDataObject();
    long	savePos, pos;

    if (lastPatternValid) {
        savePos = pos = (self)->GetDotPosition();

	if (lastDirection == FORWARD) {
            pos = (self)->CollapseDot();
            pos = lastPattern.MatchPattern(d, pos);
        }
        else {
            (self)->SetDotLength( 0);
            if (pos > 0)
		(self)->SetDotPosition( --pos);
            pos = lastPattern.MatchPatternReverse(d, pos);
        }
        if (pos < 0)
	{
            message::DisplayString(self, 0, "Search failed.");
	    if ( !lastDirection )
        	(self)->SetDotPosition( savePos);
	}
        else {
            (self)->SetDotPosition(pos);
            (self)->SetDotLength( lastPattern.GetMatchLength());
            (self)->FrameDot(pos);
            (self)->WantUpdate( self);
        }
    }
    else
        message::DisplayString(self, 0, "Must have searched at least once to search again.");
}

static void dynsearch_SearchAgainOpposite(class textview  *self)
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
