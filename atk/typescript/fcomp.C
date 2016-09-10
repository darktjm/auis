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

/* fcomp.c -- typescript package to perform filename completion
  * David Anderson
  * August 1988
  */

#include <andrewos.h>
ATK_IMPL("fcomp.H")
#include <ctype.h>

#include <text.H>
#include <mark.H>
#include <proctable.H>
#include <simpletext.H>
#include <dataobject.H>
#include <typescript.H>
#include <filetype.H>
#include <keystate.H>
#include <keymap.H>
#include <im.H>
#include <environ.H>
#include <environment.H>
#include <message.H>
#include <cursor.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <completion.H>
#include <search.H>

#include <fcomp.H>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

static class cursor *waitCursor;
static long searchCmd;
static long tokenSearchCmd;

static proctable_fptr typescript_GrabLastCmd;

#define Text(self) ((class text *) ((self)->dataobject))
#define MyEnvinfo(text,pos) (text->rootEnvironment)->GetInnerMost( pos)
#define min(x, y) (((x) < (y)) ? (x) : (y))

#define DEFAULTCHARWIDTH 12


ATKdefineRegistry(fcomp, ATK, fcomp::InitializeClass);
void CompleteFname(class typescript  *ts, long  key);
static void GatherStats(struct helpstat  *rock, enum message_HelpItem  itemtype, char  *item, long  dummy /* along for the ride */);
static void MakeReport(struct repCookie  *cookie, enum message_HelpItem  itemtype, char  *item, long  dummy /* along for the ride */);
static char * SaveLastCommand(class typescript  *td);
static int mystrcmp(char  **s1,char  **s2);
void PossibleCompletions(class typescript  *ts, long  key);
void CompleteTokenWork(class typescript  *ts, boolean  forward);
void CompleteTokenForward(class typescript  *ts, long  key);
void CompleteTokenBackward(class typescript  *ts, long  key);
void CompleteCmdWork(class typescript  *ts, boolean  forward);
void CompleteCmdForward(class typescript  *ts, long  key);
void CompleteCmdBackward(class typescript  *ts, long  key);


void CompleteFname(class typescript  *ts, long  key)
{
    long pos;
    long fname;
    class text *theText = Text(ts);
    char pathname[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    char canonical[MAXPATHLEN];
    char *insert;
    const char *msg = NULL;
    long c;
    long tlen = (theText)->GetLength();
    long cStart = (ts->cmdStart)->GetPos();
    long endpos;

    if (ts->pipescript) return;

    pos = (ts)->GetDotPosition();
    if (pos < cStart) {
	return;
    }
    for (endpos = pos; endpos < tlen && ! isspace((theText)->GetChar( endpos)); endpos++) {
    }
    for (fname = pos - 1; fname >= cStart && (c = (theText)->GetChar( fname)) != '>'
	   && c != '<' && ! isspace(c); fname--) {
    }
    fname++;
    (theText)->CopySubString( fname, endpos-fname, pathname, FALSE);
    pathname[endpos-fname] = 0;
    switch (completion::FileComplete(pathname, FALSE, buffer, MAXPATHLEN)) {
	case message_Complete:
	    msg = "[Unique]";
	    break;
	case message_CompleteValid:
	    msg = "[Others]";
	    break;
	case message_Valid:
	    msg = "[Incomplete]";
	    break;
	case message_Invalid:
	    message::DisplayString(ts, 0, "[No Match]");
	    return;	/* Don't attempt to insert a name. */
    }
    filetype::CanonicalizeFilename(canonical, pathname, MAXPATHLEN);
    if (strlen(canonical) > strlen(buffer)) {
	/* the path the user typed is LONGER than the path completion found; delete the end off it -RSK*/
	long diff= strlen(canonical)-strlen(buffer);
	theText->DeleteCharacters(endpos-diff, diff);
	ts->SetDotPosition(endpos-diff);
	msg= "[Truncated]";
    } else {
	insert = buffer + strlen(canonical);
	(theText)->InsertCharacters( endpos, insert, strlen(insert));
	(ts)->SetDotPosition( endpos+strlen(insert));
    }
    message::DisplayString(ts, 0, msg);
    (ts)->SetDotLength( 0);
    (theText)->NotifyObservers( 0);
}


struct helpstat {
    boolean refusedot;
    long itemcnt;
    long maxitemlen;
    long errorcnt;
};

#define acceptitem(rock, item, itemtype) \
  (itemtype == message_HelpListItem) && \
  strcmp(item, "./") && strcmp(item, "../") && \
  (item[0] != '.' || !rock->refusedot)

static void GatherStats(struct helpstat  *rock, enum message_HelpItem  itemtype, char  *item, long  dummy /* along for the ride */)
{
    if (acceptitem(rock, item, itemtype)) {
        long itemlen = strlen(item);

        rock->itemcnt++;
        if (itemlen > rock->maxitemlen) {
            rock->maxitemlen = itemlen;
        }
    }
    else if (itemtype != message_HelpListItem) rock->errorcnt++;
}


struct repCookie {
    boolean refusedot;
    char **report;
    long columns;
    long colwidth;
    long rows;
    long rowlen;
    long repcolwidth;
    long reprowlen;
    long widthinchar;
    long maxwidthinpix;
    long count;
    class graphic *ts;
    class fontdesc *font;
};

#define COLSPACE    1

static void MakeReport(struct repCookie  *cookie, enum message_HelpItem  itemtype, char  *item, long  dummy /* along for the ride */)
{
    long widthinpix;

    if (acceptitem(cookie, item, itemtype)) {
	cookie->report[cookie->count++] = strdup(item);
	widthinpix = (cookie->font)->StringSize( (class graphic*) cookie->ts, item, NULL, NULL);
	if(widthinpix > cookie->maxwidthinpix) {
	    cookie->maxwidthinpix = widthinpix;
	}
    }
}

static char *
SaveLastCommand(class typescript  *td)
{
    long len, spos;
    class text *mydoc;
    char *textBuf;
    char *cmdStr, *p;
    long retLen;

    mydoc =  Text(td);
    spos = (td->cmdStart)->GetPos();
    len = (mydoc)->GetLength();

    if (spos == len)
	return NULL;

    cmdStr = (char *) malloc(len - spos + 1);
    *cmdStr = '\0';

    p = cmdStr;
    while(spos < len) {
	textBuf = (mydoc)->GetBuf( spos, len - spos, &retLen);
	if(textBuf != NULL) {
	    strncpy(p,textBuf,retLen);
	    p[retLen] = '\0';
	    p += retLen;
	    spos += retLen;
	}
	else
	    break;
    }
    return cmdStr;
}

static int
mystrcmp(char  **s1,char  **s2)
{
  if(s1 && *s1 && s2 && *s2)
      return(strcmp(*s1,*s2));
  else return(0);
}

void PossibleCompletions(class typescript  *ts, long  key)
{
    long pos;
    class text *theText = Text(ts);
    long c;
    long tlen = (theText)->GetLength();
    long cStart = (ts->cmdStart)->GetPos();
    long endpos;
    long fname;
    char pathname[MAXPATHLEN];
    long pathlen;
    char *cmdStr;
    struct helpstat rock;
    long cmdOffset;

    if (ts->pipescript) return;

    pos = (ts)->GetDotPosition();
    if (pos < cStart) {
	return;
    }
    for (endpos = pos; endpos < tlen && ! isspace((theText)->GetChar( endpos)); endpos++) {
    }
    cmdOffset = endpos - cStart;
    for (fname = pos - 1; fname >= cStart && (c = (theText)->GetChar( fname)) != '>'
      && c != '<' && ! isspace(c); fname--) {
    }
    fname++;
    cmdStr = SaveLastCommand(ts);
    (theText)->CopySubString( fname, endpos-fname, pathname, FALSE);
    pathname[endpos-fname] = 0;

    (theText)->InsertCharacters( tlen++, "\n", 1);

    rock.itemcnt = 0;
    rock.maxitemlen = 0;
    rock.errorcnt = 0;
    pathlen = strlen(pathname);
    rock.refusedot = FALSE;
    if(pathlen == 0) rock.refusedot = TRUE;
    else {
	if(pathname[pathlen-1] == '/') rock.refusedot = TRUE;
    }
    completion::FileHelp((*pathname) ? pathname : "./", 0, (message_workfptr)GatherStats, (long) &rock);

    if(rock.errorcnt || rock.itemcnt == 0) {
	message::DisplayString(ts, 0, "[No Match]");
    }
    else {
	static class style *tsStyle = NULL;
	static class fontdesc *tsfont;
	static long charwidth = DEFAULTCHARWIDTH;
	enum style_FontSize dummy;
	char ffamily[MAXPATHLEN];
	long tssize;
	char spaces[MAXPATHLEN];
	long windowwidth, windowheight;
	struct repCookie cookie;
	long row, col;
	char **files, *thisfile;
	int len, count;


	if(!tsStyle && (tsStyle = (ts)->GetDefaultStyle())) {
	    (tsStyle)->GetFontSize( &dummy, &tssize);
	    (tsStyle)->GetFontFamily( ffamily, MAXPATHLEN);
	    tsfont = fontdesc::Create(ffamily, fontdesc_Fixed, tssize);
	    charwidth = (tsfont)->StringSize( (ts)->GetDrawable(), " ", NULL, NULL);
	}

	(ts)->GetTextSize(&windowwidth, &windowheight);
	cookie.ts = (ts)->GetDrawable();
	cookie.font = tsfont;
	cookie.refusedot = rock.refusedot;
	cookie.count = 0;
	cookie.maxwidthinpix = 0;
	cookie.report = (char**) calloc(rock.itemcnt, sizeof(char*));

	completion::FileHelp((*pathname) ? pathname : "./", 0, (message_workfptr)MakeReport, (long)&cookie);

	cookie.colwidth = rock.maxitemlen + COLSPACE;
	cookie.columns = windowwidth / (cookie.maxwidthinpix + charwidth);
	if (cookie.columns == 0) cookie.columns = 1;
	cookie.rows = rock.itemcnt / cookie.columns;
	if(rock.itemcnt % cookie.columns) cookie.rows++;
	cookie.repcolwidth = rock.maxitemlen + 1;
	cookie.reprowlen = cookie.repcolwidth * cookie.columns;

	qsort(cookie.report, rock.itemcnt, sizeof(char*), QSORTFUNC(mystrcmp));

	for(count = 0; count < cookie.colwidth; count++) 
	    spaces[count] = ' ';

	files = cookie.report;
	for(count = 0,row = 0; row < cookie.rows; row++) {
	    for(col = 0; col < cookie.columns; count++, col++)
		if (count < cookie.count) {
		    thisfile = *files++;
		    len = strlen(thisfile);
		    (theText)->InsertCharacters( tlen, thisfile, len);
		    tlen += len;
		    (theText)->InsertCharacters( tlen, spaces, cookie.colwidth - len);
		    tlen += cookie.colwidth - len;
		    free(thisfile);
		}
	    (theText)->InsertCharacters( tlen++, "\n", 1);
	}
	free(cookie.report);
    }

    ts->readOnlyLen = -1;
    ts->lastCmdPos = (ts->cmdText)->GetLength();
    (ts->cmdStart)->SetPos( tlen);
    (ts->cmdStart)->SetLength( 0);
    (theText)->SetFence( tlen);
    (ts)->FrameDot( tlen);

    write(ts->SubChannel, "\n", 1);
    if (cmdStr != NULL) {
	(theText)->InsertCharacters( tlen, cmdStr, strlen(cmdStr));
	free(cmdStr);
    }
    (ts)->SetDotPosition( tlen + cmdOffset);
    (ts)->SetDotLength( 0);
    (ts)->FrameDot( tlen + cmdOffset);

    (theText)->NotifyObservers( 0);
}

static struct SearchPattern *pattern = 0;
static char lastcmd[MAXPATHLEN];
static long lastmatch;
static long beginToken;
static long endToken;

void CompleteTokenWork(class typescript  *ts, boolean  forward)
{
    long pos;
    class text *theText = Text(ts);
    long begincmd;
    long lastEventCmd;
    char cmd[MAXPATHLEN];
    long match;
    const char *patcode;
    long tlen;
    long clen;
    long c;
    long tokenLen;

    if (ts->pipescript) return;

    tlen = (theText)->GetLength();
    clen = (ts->cmdText)->GetLength();

    lastEventCmd = ((ts)->GetIM())->GetLastCmd();

    pos = (ts)->GetDotPosition();
    begincmd = (ts->cmdStart)->GetPos();

    if (pos < begincmd) {
	pos = tlen;
    }

    if (lastEventCmd == tokenSearchCmd) {
	match = lastmatch;
	strcpy(cmd, lastcmd);
    }
    else {
	char unquotedCmd[MAXPATHLEN];

	for (beginToken = pos - 1; beginToken >= begincmd && (c = (theText)->GetChar( beginToken)) != '<' && c != '>' && ! isspace(c); beginToken--) {
	}
	beginToken++;

	for (endToken = pos; endToken < clen && (c = (theText)->GetChar( beginToken)) != '<' && c != '>' && ! isspace(c); endToken++) {
	}


	(theText)->CopySubString( beginToken, endToken - beginToken, unquotedCmd, FALSE);
	search::GetQuotedSearchString(unquotedCmd, cmd, sizeof(cmd));
	match = (ts->cmdText)->GetLength();
    }

    if (strlen(cmd) != 0) {
        if ((patcode = search::CompilePattern(cmd, &pattern))) {
	    message::DisplayString(ts, 0, patcode);
	    return;
        }
	else {
	    while (TRUE) {
		if (forward) {
		    match = search::MatchPattern(ts->cmdText, match+1, pattern);
		}
		else {
		    match = search::MatchPatternReverse(ts->cmdText, match-1, pattern);
		}
		if (match <= 0 || (c = (ts->cmdText)->GetChar( match - 1)) == '\n' || c == '<' || c == '>' || isspace(c)) {
		    long end;

		    for (end = match; end < clen && (c = (ts->cmdText)->GetChar( end)) != '<' && c != '>' && c != '\n' && ! isspace(c); end++) {
		    }
		    
		    tokenLen = end - match;
		    break;
		}
	    }
	}
    }
    else {
	if (forward) {
	    long end;

	    while (match < clen && (c = (ts->cmdText)->GetChar( match)) != '<' && c != '>' && c != '\n' && ! isspace(c)) {
		match++;
	    }
	    while (match < clen && ((c = (ts->cmdText)->GetChar( match)) == '<' || c == '>' || c == '\n' || isspace(c))) {
		match++;
	    }
	    for (end = match + 1; end < clen && (c = (ts->cmdText)->GetChar( end)) != '<' && c != '>' && c != '\n' && ! isspace(c); end++) {
	    }
	    tokenLen = end - match;
	    
	    if (match >= tlen) {
		match = -1;
	    }
	}
	else {
	    long end;
	    for (end = match - 1; end >= 0 && ((c = (ts->cmdText)->GetChar( end)) == '<' || c == '>' || c == '\n' || isspace(c)); end--) {
	    }
	    if (end >= 0) {
		for (match = end - 1;  match >= 0 && ((c = (ts->cmdText)->GetChar( match)) != '<' && c != '>' && c != '\n' && ! isspace(c)); match--) {
		}
		tokenLen = end - match;
		match++;
	    }
	    
	    else {
		match = -1;
	    }
	}
    }

    if (match >= 0 && match != clen) {
	lastmatch = match;
	strcpy(lastcmd, cmd);
	(theText)->DeleteCharacters( beginToken, endToken - beginToken);
	pos = beginToken;
	(ts->cmdText)->CopySubString( match, tokenLen, cmd, FALSE);
	(theText)->InsertCharacters( beginToken, cmd, tokenLen);
	endToken = beginToken + tokenLen;
	(ts)->SetDotPosition( endToken);
	(ts)->SetDotLength( 0);
	(ts)->FrameDot( endToken);
	(theText)->NotifyObservers( 0);
    }
    else message::DisplayString(ts, 0, "[No matching command]");

    ((ts)->GetIM())->SetLastCmd( tokenSearchCmd);
}

void CompleteTokenForward(class typescript  *ts, long  key)
{
    CompleteTokenWork(ts, TRUE);
}

void CompleteTokenBackward(class typescript  *ts, long  key)
{
    CompleteTokenWork(ts, FALSE);
}


void CompleteCmdWork(class typescript  *ts, boolean  forward)
{
    long pos;
    class text *theText;
    long begincmd;
    long lastEventCmd;
    char cmd[MAXPATHLEN];
    long match;
    const char *patcode;

    if (ts->pipescript) return;

    lastEventCmd = ((ts)->GetIM())->GetLastCmd();
    theText = Text(ts);

    pos = (ts)->GetDotPosition();
    begincmd = (ts->cmdStart)->GetPos();

    if (lastEventCmd == searchCmd) {
	match = lastmatch;
	strcpy(cmd, lastcmd);
    }
    else {
	char unquotedCmd[MAXPATHLEN];
	unsigned long len = (ts->cmdStart)->GetLength();

	if (len >= sizeof(unquotedCmd) - 1) {
	    len = sizeof(unquotedCmd) -1;
	}
	    
	(theText)->CopySubString( begincmd, len, unquotedCmd, FALSE);
	search::GetQuotedSearchString(unquotedCmd, cmd, sizeof(cmd));
	match = (ts->cmdText)->GetLength();
    }

    if (strlen(cmd) != 0) {
        if ((patcode = search::CompilePattern(cmd, &pattern))) {
	    message::DisplayString(ts, 0, patcode);
	    return;
        }
	else {
	    while (TRUE) {
		if (forward) {
		    match = search::MatchPattern(ts->cmdText, match+1, pattern);
		}
		else {
		    match = search::MatchPatternReverse(ts->cmdText, match-1, pattern);
		}
		if (match <= 0 || (ts->cmdText)->GetChar( match - 1) == '\n') {
		    break;
		}
	    }
	}
    }
    else {
	if (forward) {
	    if (match < (ts->cmdText)->GetLength()) {
		match = (ts->cmdText)->GetEndOfLine( match) + 1;
	    }
	    else {
		match = -1;
	    }
	}
	else {
	    if (match > 0) {
		match = (ts->cmdText)->GetBeginningOfLine( match - 1);
	    }
	    else {
		match = -1;
	    }
	}
    }

    if (match >= 0 && match != (ts->cmdText)->GetLength()) {
	long endmatch;

	lastmatch = match;
	strcpy(lastcmd, cmd);
	(theText)->DeleteCharacters( begincmd, (ts->cmdStart)->GetLength());
	pos = begincmd;
	endmatch = (ts->cmdText)->GetEndOfLine( match);
	(ts->cmdText)->CopySubString( match, endmatch-match, cmd, FALSE);
	(theText)->InsertCharacters( pos, cmd, endmatch-match);
	pos += endmatch-match;
	(ts)->SetDotPosition( pos);
	(ts)->SetDotLength( 0);
	(ts)->FrameDot( pos);
	(theText)->NotifyObservers( 0);
    }
    else message::DisplayString(ts, 0, "[No matching command]");

    ((ts)->GetIM())->SetLastCmd( searchCmd);
}

void CompleteCmdForward(class typescript  *ts, long  key)
{
    CompleteCmdWork(ts, TRUE);
}

void CompleteCmdBackward(class typescript  *ts, long  key)
{
    CompleteCmdWork(ts, FALSE);
}

boolean fcomp::InitializeClass()
{
    struct ATKregistryEntry  *imc;

    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

    searchCmd = im::AllocLastCmd();
    tokenSearchCmd = im::AllocLastCmd();

    lastcmd[0] = 0;
    ATK::LoadClass("search");

    ATK::LoadClass("completion");

    imc = ATK::LoadClass("typescript");

    proctable::DefineProc("fcomp-complete-filename", (proctable_fptr)CompleteFname, imc, NULL, "Filename completion (typescript)");
    proctable::DefineProc("fcomp-possible-completions", (proctable_fptr)PossibleCompletions, imc, NULL, "Display possible filename completions (typescript)");

    proctable::DefineProc("fcomp-complete-command-backward", (proctable_fptr)CompleteCmdBackward, imc, NULL, "Complete partial command - searching backwards (typescript)");
    proctable::DefineProc("fcomp-complete-command-forward", (proctable_fptr)CompleteCmdForward, imc, NULL, "Complete partial command - searching forwards (typescript)");
    proctable::DefineProc("fcomp-complete-token-backward", (proctable_fptr)CompleteTokenBackward, imc, NULL, "Complete partial token - searching backwards (typescript)");
    proctable::DefineProc("fcomp-complete-token-forward", (proctable_fptr)CompleteTokenForward, imc, NULL, "Complete partial token - searching forwards (typescript)");

    typescript_GrabLastCmd = proctable::GetFunction(proctable::Lookup("typescript-Grab-Last-Cmd"));

    return TRUE;
}
