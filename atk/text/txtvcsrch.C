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
#include <ctype.h>
#define AUXMODULE 1
#include <textview.H>
#include <txtvcmds.h>

#include <text.H>
#include <im.H>
#include <message.H>
#include <search.H>
#include <environ.H>
#include <mark.H>
#include <txtproc.h>
#include <dictionary.H>
#include <viewref.H>
#include <fnote.H>
#include <environment.H>

/* Search command statics. */

#define SRCHSTRLEN 100
struct SearchPattern *lastPattern = NULL;
static char searchString[SRCHSTRLEN] = "";
boolean forwardSearch = TRUE; /* TRUE if last search was forward. */

void textview_SearchAgain(class textview  *self);
void textview_SearchAgainOppositeCmd(class textview  *self);
void textview_QueryReplaceCmd(class textview  *self);
long skipToNextBalanceSymbol(class text *doc, long pos, int direction);
void textview_BalanceCmd(class textview  *self);

    
extern long textview_ExposeRegion(class textview *tv, long pos1, long rlen, class view *inset, const struct rectangle &area, struct rectangle &hit, long &off, long extra);

int textview_SearchCmd(class textview  *self, char *arg)
{
    char defSrchString[SRCHSTRLEN], messageBuf[120];
    const char *tp;
    const char *prompt;
    int gf, ct;
    long pos;
    int j;
    class text *d;
    boolean defaultExists = FALSE;
   
    d = Text(self);
    ct = ((self)->GetIM())->Argument();

    if (*searchString != '\0'){
	defaultExists = TRUE;
	strcpy(defSrchString, searchString);
    }
    if (defaultExists) {
	sprintf(messageBuf, "Search for [%s] : ", defSrchString);
	prompt = messageBuf;
    }
    else
	prompt = "Search for: ";
    if ((unsigned long)arg < 256) {
	gf = message::AskForString(self, 0, prompt, NULL, searchString, sizeof(searchString));
	if (gf < 0) return(-1);
	if (defaultExists && *searchString == '\0')
	    strcpy(searchString, defSrchString);
    } else {
	/* Use string argument passed to proc. */
	strncpy(searchString, arg, SRCHSTRLEN);
    }
    pos= self->CollapseDot();
    if (!defaultExists || *searchString != '\0') {
	tp = search::CompilePattern(searchString, &lastPattern);
	if (tp) {
	    message::DisplayString(self, 0, tp);
	    return(-2);
	}
    }
    j = 0;
    while (j<ct) {
        pos = search::MatchPattern(d, pos, lastPattern);
	if (pos < 0) {
            (self)->SetDotLength(0);
            message::DisplayString(self, 0, "Search failed");
            return(-3);
        }
        (self)->SetDotPosition(pos);
        (self)->SetDotLength( search::GetMatchLength());
	pos+= search::GetMatchLength();
        j++;
    }
    (self)->FrameDot(pos - search::GetMatchLength());
    (self)->WantUpdate( self);
    forwardSearch = TRUE;
    return(0);
}

int textview_RSearchCmd(class textview  *self, char *arg)
{
    int ct, gf;
    long orgpos;
    long pos;
    int j;
    class text *d;
    char defSrchString[SRCHSTRLEN], messageBuf[130];
    const char *prompt;
    const char *tp;
    boolean defaultExists = FALSE;

    d = Text(self);
    pos= orgpos= (self)->GetDotPosition();
    ct = ((self)->GetIM())->Argument();
    if (*searchString != '\0'){
	defaultExists = TRUE;
	strcpy(defSrchString, searchString);
    }
    if (defaultExists) {
	sprintf(messageBuf, "Reverse search for [%s] : ", defSrchString);
	prompt = messageBuf;
    }
    else
	prompt = "Reverse search for: ";
    if ((unsigned long)arg < 256) {
	gf=message::AskForString(self, 0, prompt, NULL, searchString, sizeof(searchString));
	if (gf < 0)
	    return -1;
	if (defaultExists && *searchString == '\0')
	    strcpy(searchString, defSrchString);
    } else {
	/* Use string argument passed to proc. */
	strncpy(searchString, arg, SRCHSTRLEN);
    }
    (self)->SetDotLength(0);
    if (orgpos > 0) (self)->SetDotPosition(--pos);
    if (!defaultExists || *searchString != '\0') {
	tp = search::CompilePattern(searchString, &lastPattern);
	if (tp != 0) {
	    message::DisplayString(self, 0, tp);
	    return -2;
	}
    }
    j=0;
    while (j<ct) {
        pos = search::MatchPatternReverse(d, pos, lastPattern);
	if (pos < 0) {
            (self)->SetDotPosition(orgpos);
	    self->SetDotLength(0);
            message::DisplayString(self, 0, "Search failed");
            return -3;
        }
        (self)->SetDotPosition(pos);
        (self)->SetDotLength( search::GetMatchLength());
	--pos;
        j++;
    }
    (self)->FrameDot(pos + 1);
    (self)->WantUpdate( self);
    forwardSearch = FALSE;
    return 0;
}

void textview_SearchAgain(class textview  *self)
{
    class text *d = Text(self);
    long	savePos, pos;

    if (lastPattern != NULL) {
        savePos = pos = (self)->GetDotPosition();

	if (forwardSearch) {
	    pos = (self)->GetDotPosition() + (self)->GetDotLength();
            (self)->SetDotPosition( pos);
            (self)->SetDotLength( 0);
            pos = search::MatchPattern(d, pos, lastPattern);
        }
        else {
            (self)->SetDotLength( 0);
            if (pos > 0)
		(self)->SetDotPosition( --pos);
            pos = search::MatchPatternReverse(d, pos, lastPattern);
        }
        if (pos < 0)
	{
            message::DisplayString(self, 0, "Search failed.");
	    if ( !forwardSearch )
        	(self)->SetDotPosition( savePos);
	}
        else {
            (self)->SetDotPosition(pos);
            (self)->SetDotLength( search::GetMatchLength());
            (self)->FrameDot(pos);
            (self)->WantUpdate( self);
	    message::DisplayString(self, 0, "");	/* Clear any leftover message */
        }
    }
    else
        message::DisplayString(self, 0, "Must have searched at least once to search again.");
}

void textview_SearchAgainOppositeCmd(class textview  *self)
    {
    forwardSearch	^= TRUE;
    textview_SearchAgain(self);
}

void textview_QueryReplaceCmd(class textview  *self)
{
    boolean defaultExists = FALSE;
    boolean keepAsking = TRUE;
    boolean keepRunning = TRUE;
    boolean returnPosition = TRUE;
    struct SearchPattern *lastPattern = NULL;
    char *prompt = NULL;
    const char *searchError = NULL;
    char casedString[SRCHSTRLEN];
    char promptBuf[SRCHSTRLEN +20];
    int numFound = 0;
    int numReplaced = 0;
    int replacementLen = 0;
    int reply = 0;
    long fencePos = 0;
    long originalLength = 0;
    long originalPos = 0;
    long pos = 0, lastpos = -1;
    long searchPos = 0;
    static char replacement[SRCHSTRLEN] = "";
    static char defSrchString[SRCHSTRLEN] = "";
    static int expertReplace = -999999;
    class mark *area = NULL;
    class text *d = NULL;

    if (ConfirmReadOnly(self))
	return;
    casedString[0] = '\0';
    promptBuf[0] = '\0';
    if (expertReplace == -999999)
	expertReplace = environ::GetProfileSwitch("ExpertMode", 0);

    d = Text(self);

    if (*searchString != '\0'){
	defaultExists = TRUE;
	sprintf(defSrchString, "%s", searchString);
    }
    if (defaultExists) {
	sprintf(promptBuf, "Replace [%s] : ", defSrchString);
	if (*searchString == '\0')
	    sprintf(searchString, "%s", defSrchString);
	prompt = promptBuf;
    }
    else
	prompt = "Replace: ";

    if (message::AskForString(self, 0, prompt, NULL, searchString, sizeof(searchString)) < 0)
	return;
    if (defaultExists && *searchString == '\0')
	sprintf(searchString, "%s", defSrchString);

    prompt = "New string: ";
    if (message::AskForString(self, 0, prompt, NULL, replacement, sizeof(replacement)) < 0)
	return;
    replacementLen = strlen(replacement);

    if (!defaultExists || *searchString != '\0')
	if ((searchError = search::CompilePattern(searchString, &lastPattern)) != NULL) {
	    message::DisplayString(self, 0, searchError);
	    return;
	}

    originalPos = searchPos = (self)->GetDotPosition();
    originalLength = (self)->GetDotLength();

    area = (originalLength != 0) ? (d)->CreateMark( originalPos, originalLength) : NULL;

    if (searchPos < (fencePos = (d)->GetFence())) searchPos = fencePos;

    while ((pos = search::MatchPattern(d, searchPos, lastPattern)) >= 0
	    && keepRunning
	    && (area == NULL || pos < (area)->GetPos() + (area)->GetLength()))  {

	long matchLen = search::GetMatchLength();
	/* length can change between matches on RE searches */

	if (pos != lastpos) ++numFound;
	lastpos = pos;

	(self)->SetDotPosition( pos);
	(self)->SetDotLength( matchLen);
	(self)->FrameDot( pos);

	if (keepAsking) {
	    (self)->Update();
	    reply = ((self)->GetIM())->GetCharacter();
	}
	else
	    reply = ' ';

	switch (reply) {
	    case 'q':
	    case '\007':
	    case EOF:		/* Interrupted im_GetChar */
		keepRunning = FALSE;
		break;
	    case '-': /* WHAT IS THIS?? -- GHOTI */
		{
		int i;

		if (isupper((d)->GetChar( pos))) {
		    for (i = 1; i < matchLen; i++)
			if (islower((d)->GetChar( pos + i))) break;
		    if (i >= matchLen) {
			for (i = 0; i < replacementLen; i++)
			    casedString[i] = (islower(replacement[i])) ? toupper(replacement[i]) : replacement[i];
			casedString[i] = '\0';
		    }
		    else { /* Upcase first letter. */
			for (i = 0; i < replacementLen; i++)
			    casedString[i] = replacement[i];
			casedString[i] = '\0';
			casedString[0] = (islower(replacement[0])) ? toupper(replacement[0]) : replacement[0];
		    }
		}
		else
		    strncpy(casedString, replacement, replacementLen);
		
		(d)->ReplaceCharacters( pos, matchLen, casedString, replacementLen);
		searchPos = pos + replacementLen;
		++numReplaced;
		}
		break;
	    case '!':
	    case '.':
	    case ',':
	    case '=':
		if (reply == '!')
		    keepAsking = FALSE;
		else {
		    keepRunning = FALSE;
		    if (reply == ',')
			returnPosition = FALSE;
		    else if (reply == '='){
			returnPosition = FALSE;
			searchPos = pos + matchLen;
			break;
		    }
		}
		/* Fall through. */
	    case ' ':
		(d)->ReplaceCharacters( pos, matchLen, replacement, replacementLen);
		searchPos = pos + replacementLen;
		++numReplaced;
		break;
	    case 'n':
		searchPos = pos + 1;
		break;
	    case 'r':
		if (expertReplace)  {
		    message::DisplayString(self, 0, "Recursive editing; ^C exits.");
		    im::KeyboardProcessor();
		    break;
		}
		/* Otherwise, fall through to the default */
	    default:
		message::DisplayString(self, 0, (char *)(expertReplace ?
				      "One of ' ', '.', 'n', 'r', '!', 'q' '-' ',' '='" :
				      "One of ' ', '.', 'n', '!', 'q' '-' ',' '='"));
		im::ForceUpdate();
		break;
	}

	if (keepAsking)
	    (self)->Update();
    }
    if (numFound > 0)  {
	char messageString[100];
	if (returnPosition){
	    if (area != NULL) {
		(self)->SetDotPosition( (area)->GetPos());
		(self)->SetDotLength( (area)->GetLength());
	    } else {
		(self)->SetDotPosition( originalPos);
		(self)->SetDotLength( originalLength);
	    }
	    (self)->FrameDot( originalPos);
	    (d)->NotifyObservers( observable_OBJECTCHANGED);
	}
	sprintf(messageString, "Found %d occurrences; replaced %d.",
		numFound, numReplaced);
	message::DisplayString(self, 0, messageString);
    }
    else
	message::DisplayString(self, 0, "No occurrences found.");

    if (area != NULL) {
	(d)->RemoveMark( area);
	delete area;
    }

    return;
}

struct paren_node {
    long type;
    struct paren_node *next;
};

long skipToNextBalanceSymbol(class text *doc, long pos, int direction)
{
    /* skip to next paren, bracket, or brace.
     * Search direction specified in "direction" */
    int thischar, nextChar;
    static const char opens[] = "({[", closes[] = ")}]";
    long docLength = (doc)->GetLength();
    long limit= (direction==FORWARD) ? docLength : 0;
    int increment= (direction==FORWARD) ? 1 : -1;

    if ( pos < 0  || pos >= docLength )
	return EOF;
    while ( pos != limit ) {
	pos += increment;
	thischar = (doc)->GetChar( pos);
	nextChar = (pos == limit) ? 0 : (doc)->GetChar( pos + increment);

	if (strchr(opens, thischar)!=NULL || strchr(closes, thischar)!=NULL)
	    return pos;
    }
    return EOF;
}

static long balance(class text *doc, long pos)
{
    /*
     * Returns the pos of the balancing symbol to the one
     * pointed to by the passed value of pos
     */
    const char *parentype;
    struct paren_node *parenstack = NULL;
    static const char opens[] = "({[", closes[] = ")}]";
    const char *closeTable, *openTable;

    if ( pos <= 0  || pos >= (doc)->GetLength() )
	return EOF;
    if ( strchr(opens, (doc)->GetChar( pos)) != NULL ) {
	/* searching forward */
	openTable = opens;
	closeTable = closes;
    } else if ( strchr(closes, (doc)->GetChar( pos)) != NULL ) { 
	/* searching backward */
	openTable = closes;
	closeTable = opens;
    }
    do  {
	if ( (parentype = strchr(closeTable, (doc)->GetChar( pos))) != NULL ) {
	    if ( parenstack == NULL || parenstack->type != (parentype - closeTable) )
		break;
	    else {
		struct paren_node *temp = parenstack;

		parenstack = parenstack->next;
		free(temp);
		if (parenstack == NULL)
		    return pos; /* found matching symbol */
	    }
	} else if ((parentype = strchr(openTable, (doc)->GetChar( pos))) != NULL ) {
	    struct paren_node *temp = NEW(struct paren_node);

	    temp->type = parentype - openTable;
	    temp->next = parenstack;
	    parenstack = temp;
	}
    } while (parenstack != NULL &&
	     (pos= skipToNextBalanceSymbol(doc, pos, openTable == opens ? FORWARD : BACKWARD)) != EOF );

    return EOF;
}

void textview_BalanceCmd(class textview  *self)
{
    class text	*doc;
    long	pos, docLength;
    long	leftBalancedPos = EOF, rightBalancedPos = EOF;
    static 	const char	balanceSymbols[] = "[{()}]";

    doc	= Text(self);
    docLength	= (doc)->GetLength();
    pos = (self)->GetDotPosition();
    while ( pos < docLength)
    {
	if ( (pos =  skipToNextBalanceSymbol(doc, pos - 1, FORWARD)) == EOF )
	    break;
	if ( strchr(balanceSymbols, (doc)->GetChar( pos)) - balanceSymbols > 2 )
	{
	    leftBalancedPos	= balance(doc, pos);
	    rightBalancedPos	= pos;
	    break;
	}
	else
	    if ( (pos = balance(doc, pos)) != EOF )
		pos++;
	    else
		break;
    }
    if ( leftBalancedPos == EOF || rightBalancedPos == EOF )
    {
	message::DisplayString(self, 0, "Sorry, could not balance.");
	return;
    }
    (self)->SetDotPosition( leftBalancedPos);
    (self)->SetDotLength( rightBalancedPos - leftBalancedPos + 1);
    (self)->FrameDot( leftBalancedPos);
    (self)->WantUpdate( self);
}

struct textv_recfind {
    class view *self;
    class view *v;
    long pos;
};
static struct textv_recfind rfound;

static boolean FindNextViewSplot(long rock, class text *self, long pos, class environment *curenv)
{
    class viewref *vr;
    class view *v;

    if (curenv->type==environment_View) {
	vr = curenv->data.viewref;
	v = (class view *)dictionary::LookUp(rfound.self, (char *)vr);
	if (!v) {
	    return FALSE;
	}
	rfound.pos = pos;
	rfound.v = v;
	return TRUE;
    }
    else {
	return FALSE;
    }
}

/* finds the first match, in the text or any child, starting at pos. */
static boolean textview_RecSearchLoop(class textview *self, long pos, struct SearchPattern *pat)
{
    class text *d;
    long len, newpos, newlen;
    long textlen;
    class environment *nextenv;

    d = Text(self);
    textlen = d->GetLength();

    rfound.self = self;
    rfound.pos = (-1);
    rfound.v = NULL;
    nextenv = d->EnumerateEnvironments(pos, textlen-pos, (text_eefptr)FindNextViewSplot, 0);

    newpos = search::MatchPattern(d, pos, pat);
    newlen = search::GetMatchLength();

    /* now loop until BOTH found positions are negative (not found) */
    while (rfound.pos >= 0 || newpos >= 0) {
	if (rfound.pos < 0 || (newpos >= 0 && newpos < rfound.pos)) {
	    /* newpos wins */

	    pos = newpos;
	    len = newlen;
	    self->recsearchchild = NULL;
	    self->recsearchpos->SetPos(pos);
	    self->recsearchpos->SetLength(len);
	    self->recsearchvalid = TRUE;
	    return TRUE;
	}
	else {
	    /* rfound.pos wins */
	    /* we have to copy stuff out of rfound, because it can be overwritten during the child RecSearch() call */
	    class view *v = rfound.v;
	    long tmppos = rfound.pos;
	    long tmplen = nextenv->GetLength();

	    if (v->RecSearch(pat, FALSE)) {
		self->recsearchchild = v;
		self->recsearchpos->SetPos(tmppos);
		self->recsearchpos->SetLength(tmplen);
		self->recsearchvalid = TRUE;
		return TRUE;
	    }

	    rfound.self = self;
	    rfound.pos = (-1);
	    rfound.v = NULL;
	    pos = tmppos+tmplen;
	    d->EnumerateEnvironments(pos, textlen-pos, (text_eefptr)FindNextViewSplot, 0);
	}
    }

    self->recsearchvalid = FALSE;
    self->recsearchpos->SetPos(0);
    self->recsearchpos->SetLength(0);
    self->recsearchchild = NULL;
    return FALSE;
}

boolean textview::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    long pos;
    class text *d;

    this->InitChildren();
    d = Text(this);
    fnote::OpenAll(d);

    if (!toplevel) {
	pos = 0;
    }
    else {
	pos = this->GetDotPosition();
    }

    return textview_RecSearchLoop(this, pos, pat);
}

boolean textview::RecSrchResume(struct SearchPattern *pat)
{
    long pos, len;

    if (!this->recsearchvalid)
	return FALSE;

    pos = this->recsearchpos->GetPos();
    len = this->recsearchpos->GetLength();

    if (this->recsearchchild) {
	/* recsearchchild is the child that got the last success */
	if (this->recsearchchild->RecSrchResume(pat)) {
	    /* pos and child stay the same */
	    this->recsearchvalid = TRUE;
	    return TRUE;
	}

	/* ok, that child ran out of matches. restart right after it. */
	pos += len;
    }
    else {
	/* last success was a string. restart right after it */
	pos += len;
    }
    
    return textview_RecSearchLoop(this, pos, pat);
}

boolean textview::RecSrchReplace(class dataobject *srctext, long srcpos, long srclen)
{
    long pos, len;
    boolean res;
    class text *d;

    if (!this->recsearchvalid)
	return FALSE;

    if (this->recsearchchild) {
	/* hand it off to the child instead */
	return this->recsearchchild->RecSrchReplace(srctext, srcpos, srclen);
    }

    d = Text(this);

    if (!srctext->IsType("simpletext")) {
	message::DisplayString(this, 0, "Replacement object is not text.");
	return FALSE;
    }

    pos = this->recsearchpos->GetPos();
    len = this->recsearchpos->GetLength();

    res = d->DeleteCharacters(pos, len);
    if (!res) {
	message::DisplayString(this, 0, "Document is read-only.");
	return FALSE;
    }

    d->AlwaysCopyText(pos, (class simpletext *)srctext, srcpos, srclen);
    this->recsearchpos->SetPos(pos);
    this->recsearchpos->SetLength(srclen);

    return TRUE;
}
void textview::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {

    long pos = recsearchpos->GetPos();
    long len = recsearchpos->GetLength();
    SetDotPosition(pos);
    if(recsearchchild) SetDotLength(0);
    else SetDotLength(len);

    long off=0;
    long extra=logical.height/3;
    long p1=textview_ExposeRegion(this, pos, len, recsearchchild, logical, hit, off, extra);
    if(recsearchchild) {
	struct rectangle insetrect=hit;
	if(!recsearchchild->GetIM()) recsearchchild->LinkTree(this);
	recsearchchild->RecSrchExpose(insetrect,hit);
	/* now use the hit area to compute an adjustment so that the hit will appear at hit.left,extra */
	p1=textview_ExposeRegion(this, pos, 1, recsearchchild, logical, hit, off, extra-(hit.top-insetrect.top));
	insetrect=hit;
	recsearchchild->RecSrchExpose(insetrect,hit);
    } else {
	WantInputFocus(this);
    }
    SetTopOffTop(p1, off);
    WantUpdate(this);
}


