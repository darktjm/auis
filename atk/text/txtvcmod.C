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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/txtvcmod.C,v 3.6 1996/10/19 14:28:02 robr Exp $";
#endif



 

#include <andrewos.h>

#include <ctype.h>
#define AUXMODULE 1
#include <textview.H>
#include <txtvcmds.h>
#include <txtvinfo.h>

#include <text.H>
#include <im.H>
#include <message.H>
#include <environment.H>
#include <viewref.H>
#include <completion.H>
#include <environ.H>
#include <proctable.H>

#include <sys/param.h>
#include <sys/stat.h>

#include <txtproc.h>

/* AutoCut means that backspace cuts the selected
 * region (if a region is selected) and self-insert cuts
 * the selected region (if any) when new chars are inserted.
 * It is not exactly motif--but pretty close.
 *
 * NOTE:  This could be textview instance data if it makes
 *        sense to have one view behave different from another.
 *	  It may be necessary someday to have textview::autocut_mode, though.
 */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

int textview_GetNextNonSpacePos(class textview  *self, int  pos);
void textview_AddSpaces(class textview  *self, long  pos, long  startPos, long  len);
boolean ConfirmViewDeletion(class textview  *self, long  pos , long  len);
boolean ConfirmReadOnly(class textview  *self);
void textview_SelfInsertCmd(register class textview  *self, char  a);
void textview_DeleteWordCmd (register class textview  *self);
void textview_ForeKillWordCmd (register class textview  *self);
void textview_OpenLineCmd(register class textview  *self);
void textview_JoinCmd(register class textview  *self);
static void yankKillLine (register class textview  *self, int		 action);
void textview_YankLineCmd(register class textview  *self);
boolean textview_objecttest(register class textview  *self,char  *name,char  *desiredname);
void textview_InsertInsetCmd (register class textview  *self, long  rock);
void textview_InsertFile(class textview  *self);
void textview_YankCmd(register class textview  *self);
static void textview_DoRotatePaste(register class textview  *self, int  count);
void textview_PutAfterCmd(register class textview  *self);
void textview_PutBeforeCmd(register class textview  *self);
void textview_BackwardsRotatePasteCmd(class textview  *self);
void textview_RotatePasteCmd(class textview  *self);
void textview_InsertNLCmd(register class textview  *self);
static int stringmatch(register class text  *d,register long  pos,register char  *c);
void textview_ZapRegionCmd(register class textview  *self);
void textview_KillLineCmd(register class textview  *self);
void textview_MITKillLineCmd(register class textview  *self);
void textview_AppendNextCut(class textview  *self);
void textview_CopyRegionCmd (register class textview  *self);
void textview_GetToCol (register class textview  *self, register int  col);
void textview_InsertSoftNewLineCmd(register class textview  *self);
void textview_MyLfCmd(register class textview  *self);
void textview_MySoftLfCmd(class textview  *self);
void textview_DeleteCmd (register class textview  *self);
void textview_ViDeleteCmd (register class textview  *self);
void textview_KillWhiteSpaceCmd(register class textview  *self);
int textview_GetSpace(class textview  *self, register int  pos);
static void AdjustIndentation(class textview  *self, int amount);
void textview_UnindentCmd(class textview  *self);
void textview_IndentCmd(class textview  *self);
void textview_ExchCmd(register class textview  *self);
void textview_RuboutCmd (register class textview  *self);
void textview_TwiddleCmd (register class textview  *self);
void textview_RuboutWordCmd (register class textview  *self);
void textview_BackKillWordCmd (register class textview  *self);
static void yankDeleteWord (register class textview  *self, int		 action, proctable_fptr moveFunction);
void textview_YankWordCmd(register class textview  *self);
void textview_DeleteEndOfWordCmd(register class textview  *self);
void textview_YankEndOfWordCmd(register class textview  *self);
void textview_DeleteBackwardWordCmd(register class textview  *self);
void textview_YankBackwardWordCmd(register class textview  *self);
void textview_DeleteWSWordCmd(register class textview  *self);
void textview_YankWSWordCmd(register class textview  *self);
void textview_DeleteBackwardWSWordCmd(register class textview  *self);
void textview_YankBackwardWSWordCmd(register class textview  *self);
void textview_DeleteEndOfWSWordCmd(register class textview  *self);
void textview_YankEndOfWSWordCmd(register class textview  *self);
static void viYankDeleteLine(class textview  *self, int		 action);
void textview_ViDeleteLineCmd(class textview  *self);
void textview_ViYankLineCmd(class textview  *self);
void textview_OpenLineBeforeCmd(class textview  *self);
void textview_OpenLineAfterCmd(class textview  *self);
void textview_InsertAtBeginningCmd(class textview  *self);
void textview_InsertAtEndCmd(class textview  *self);
void textview_ChangeRestOfLineCmd(class textview  *self);
void textview_ChangeLineCmd(class textview  *self);
void textview_ChangeWordCmd(class textview  *self);
void textview_ChangeSelectionCmd(class textview  *self);
void textview_ReplaceCharCmd(class textview  *self);
void textview_SubstituteCharCmd(class textview  *self);
void textview_DigitCmd(class textview  *self, register char  c);
static void AdjustCase(class textview  *self, boolean  upper , boolean  firstOnly);
void textview_UppercaseWord(class textview  *self, long  key);
void textview_LowercaseWord(class textview  *self, long  key);
void textview_CapitalizeWord(class textview  *self, long  key);
void textview_ToggleCase(class textview  *self, long  key);
void textview_QuoteCmd(register class textview  *self);


int textview_GetNextNonSpacePos(class textview  *self, int  pos)
{
    class text *d = Text(self);
    long len = (d)->GetLength();

    while (pos < len)  {
	long tc = 0xff & (d)->GetChar(pos);
	if (tc != ' ' && tc != '\t') {
	    return pos;
	}
	pos++;
    }
    return pos;
}
    
void textview_AddSpaces(class textview  *self, long  pos, long  startPos, long  len)
{
    class text *d = Text(self);

    if (len > 0) {
	char *buf = (char *) malloc(len + 1);

	(d)->CopySubString( startPos, len, buf, FALSE);
	(d)->InsertCharacters( pos, buf, len);
	free(buf);
    }
}
	
boolean ConfirmViewDeletion(class textview  *self, long  pos , long  len)
{
    class text *d;
    boolean hasViews;
    static char *yesOrNo[] = {"Yes", "No", NULL};
    long answer;
    class environment *env;

    d = Text(self);

    for (hasViews = FALSE; len--; pos++)
	if ((d)->GetChar( pos) == TEXT_VIEWREFCHAR) {
	    env = (self)->GetStyleInformation( NULL, pos, NULL);
	    if (env->type == environment_View) {
		hasViews = TRUE;
		(self)->ReleaseStyleInformation( env);
		break;
	    }
	    (self)->ReleaseStyleInformation( env);
	}

    if (! hasViews)
        return TRUE;

    if (message::MultipleChoiceQuestion(self, 80,
         "Really delete inset(s)?", 1, &answer,
          yesOrNo, NULL) < 0 || answer != 0) {
        message::DisplayString(self, 0, "Cancelled.\n");
        return FALSE;
    }

    return TRUE;
}

/* Added friendly read-only behavior 04/27/89 --cm26 */

boolean ConfirmReadOnly(class textview  *self)
{
    if ((Text(self))->GetReadOnly()) {
        message::DisplayString(self, 0,
          "Document is read only.");
        return TRUE;
    } else
        return FALSE;
}

void textview_SelfInsertCmd(register class textview  *self, char  a)
{
    register int ct, i, pos;
    register class text *d;

    if (ConfirmReadOnly(self))
        return;

    d = Text(self);
    ct=((self)->GetIM())->Argument();
    if (IsAutoCutMode() && (self->GetDotLength() > 0))
	textview_ZapRegionCmd(self);
    pos = (self)->CollapseDot();
    (self)->PrepareInsertion( FALSE);
    for (i = 0; i < ct; i++) {
    	(d)->InsertCharacters(pos++,&a,1);
    }
    (self)->FinishInsertion();
    (self)->SetDotPosition(pos);
    (self)->FrameDot( pos);
    (d)->NotifyObservers( observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

void textview_DeleteWordCmd (register class textview  *self)
{
    register int count, pos, len;


    if ( self->editor == VI )
	yankDeleteWord(self, DELETE, (proctable_fptr)textview_ForwardWordCmd);
    else {
	if (ConfirmReadOnly(self))
	    return;

	pos = (self)->CollapseDot();

	count = ((self)->GetIM())->Argument();
	((self)->GetIM())->ClearArg();   /* Don't want Fwd Word to use it! */

	while (count--)
	    textview_ForwardWordCmd(self);

	len = (self)->GetDotPosition() - pos;

	if (ConfirmViewDeletion(self, pos, len)) {
	    (self)->DeleteCharacters( pos, len);
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	    (self)->FrameDot( pos);
	}
    }
}

void textview_ForeKillWordCmd (register class textview  *self)
{
    register int count, pos, len;
    register class text *d;

    if ( self->editor == VI )
	yankDeleteWord(self, DELETE, (proctable_fptr)textview_ForwardWordCmd);
    else {
	if (ConfirmReadOnly(self))
	    return;

	d = Text(self);

	pos = (self)->CollapseDot();

	count = ((self)->GetIM())->Argument();
	((self)->GetIM())->ClearArg();   /* Don't want Fwd Word to use it! */

	while (count--)
	    textview_ForwardWordCmd(self);

	len = (self)->GetDotPosition() - pos;

	if (ConfirmViewDeletion(self, pos, len)) {
	    if (((self)->GetIM())->GetLastCmd() == lcKill) {
		FILE *pasteFile;
		pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
		len += (d)->InsertFile( pasteFile, NULL, pos);
		((self)->GetIM())->CloseFromCutBuffer( pasteFile);
		/* hack! Back up the ring so we overwrite old with new. */
		((self)->GetIM())->RotateCutBuffers( 1);
	    }

	    (self)->DoCopyRegion( pos, len, FALSE, d->CopyAsText);
	    (self)->DeleteCharacters( pos, len);
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	    (self)->FrameDot( pos);
	    ((self)->GetIM())->SetLastCmd( lcKill);
	}
    }
}

void textview_OpenLineCmd(register class textview  *self)
{
    register int i, pos, ct;
    char tc;
    class text *d;

    if (ConfirmReadOnly(self))
        return;
    pos = (self)->CollapseDot();
    d=Text(self);
    tc = '\012';
    i = 0;
    ct = ((self)->GetIM())->Argument();
    (self)->PrepareInsertion( TRUE);
    for (i = 0; i < ct; i++) {
	(d)->InsertCharacters(pos,&tc,1);
    }
    (self)->FinishInsertion();

    if ( self->editor == VI )
	textview_NextLineCmd(self);

    (d)->NotifyObservers( observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

void textview_JoinCmd(register class textview  *self)
    {
    class text	*text;
    register int	i, ct;
    long pos;
    char	blank = ' ';

    if (ConfirmReadOnly(self))
        return;
    ct = (self->imPtr)->Argument();
    (self->imPtr)->ClearArg();
    text = Text(self);
    for (i = 0; i < ct; i++)
    {
	textview_EndOfLineCmd(self);
	pos = (self)->GetDotPosition();
	if (pos < (text)->GetLength() )
	{
	    if ( (text)->GetChar( pos) == '\n' )
	    {
		(text)->ReplaceCharacters( pos, 1,  &blank, 1);
		(self)->SetDotPosition( ++pos);
	    }
	    if ( charType((text)->GetChar( pos)) == WHITESPACE )
		textview_DeleteWordCmd(self);
	}
    }
}

/* In order to append to ATK datastream in the cutbuffer,
  we must yank it into text, and then cut the larger datastream
  back out in place of the old. Otherwise we might get two nested
  text obejcts instead of one long text object. */

static void yankKillLine (register class textview  *self, int		 action)
        {
    register int count, pos, endpos, lastpos, numNLs, applen = 0;
    register class text *d;

    endpos = pos = (self)->CollapseDot();
    d = Text(self);
    if ( (d)->GetChar(pos) == '\012')
	return;
    count = (self->imPtr)->Argument();
    lastpos = (d)->GetLength();

    /* find end of line range to be deleted/yanked */
    for (numNLs = 0; endpos < lastpos && numNLs < count; endpos++)
	if ((d)->GetChar( endpos) == '\012')
	    numNLs++;

    if (((self)->GetIM())->GetLastCmd() == lcKill) {
	FILE *pasteFile;
	pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
	applen = (d)->InsertFile( pasteFile, NULL, pos);
	((self)->GetIM())->CloseFromCutBuffer( pasteFile);
	/* hack! back up the ring so we overwrite old with new. */
	((self)->GetIM())->RotateCutBuffers( 1);
    }

    (self)->DoCopyRegion( pos, endpos + applen - pos, FALSE, d->CopyAsText);

    /* If this is not a deletion operation, we still have to
      remove the chars we inserted for our append operation */
    if ( action == DELETE )
	(self)->DeleteCharacters( pos, endpos + applen - pos);
    else
	(self)->DeleteCharacters( pos, applen);
	
    (self->imPtr)->SetLastCmd( lcKill);	/* mark us as a text killing command */
    (d)->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_YankLineCmd(register class textview  *self)
    {
    yankKillLine(self, YANK);
}

boolean textview_objecttest(register class textview  *self,char  *name,char  *desiredname)
{
    if(ATK::LoadClass(name) == NULL){
        char foo[640];
        sprintf(foo,"Can't load %s",name);
         message::DisplayString(self, 0, foo);
        return(FALSE);
    }
    if(! ATK::IsTypeByName(name,desiredname)){
        char foo[640];
        sprintf(foo,"%s is not a %s",name,desiredname);
         message::DisplayString(self, 0, foo);
        return(FALSE);
    }
    return(TRUE);
}

/*
  Prompt user for dataobject to insert, unless dataobject name is passed
  as (char *) in rock. If ArgProvided (^U) then also ask for view name.
*/

void textview_InsertInsetCmd (register class textview  *self, long  rock)
{
    char iname[100];
    char viewname[200];
    long pf;
    boolean promptforname = ((self)->GetIM())->ArgProvided();

    ((self)->GetIM())->ClearArg();    
    viewname[0] = '\0';
    if (ConfirmReadOnly(self))
        return;
    if((Text(self))->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }

    if (rock >= 0 && rock < 256) {
        pf = message::AskForString(self, 0, "Data object to insert here: ", 0, iname, sizeof(iname));
        if (pf < 0){
            message::DisplayString(self, 0, "Punt!");
            return;
        }
        if(strlen(iname)==0){
            message::DisplayString(self, 0, "No name specified");
            return;
        }
    }
    else {
        strncpy(iname, (char *) rock, sizeof(iname));
    }

    if(textview_objecttest(self,iname,"dataobject") == FALSE) return;
    if(promptforname){
        if( message::AskForString (self, 0, "View to place here:", 0, viewname, 200) < 0) return;
        if(textview_objecttest(self,viewname,"view") == FALSE) return;
    }
    (self)->PrepareInsertion( FALSE);
    self->currentViewreference = (Text(self))->InsertObject( (self)->GetDotPosition(), iname,viewname);
    (self)->FinishInsertion();
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
    im::ForceUpdate();
}

void textview_InsertFile(class textview  *self)
{
    char filename[MAXPATHLEN], *basename, *nmend;
    FILE *inputFile;
    long initialPos;
    long pos;
    struct stat buf;
    
    if (ConfirmReadOnly(self))
        return;
    // get buffer's filename
    basename = self->WantInformation("filename");  
    if (basename) {
	strcpy(filename, basename);
	char *slash = strrchr(filename, '/');
	if (slash) *slash = '\0';
    }
    else if ( ! im::GetDirectory(filename)) 	// or get cwd
	strcpy(filename, "/");			// or use "/"
    nmend = filename+strlen(filename);
    if (nmend[-1] != '/') strcpy(nmend, "/");	// ensure trailing /

    if (completion::GetFilename(self, "Insert file: ", filename, 
		filename, sizeof(filename), FALSE, TRUE) == -1 )
        return;
    stat(filename,&buf);
    if(buf.st_mode & S_IFDIR) {
	message::DisplayString(self, 0, "Can't insert a directory"); 
	return;
    }
    if ((inputFile = fopen(filename, "r")) == NULL) {
        message::DisplayString(self, 0, 
			"Could not open requested file.");
        return;
    }

    (self)->CollapseDot();

    if ( self->editor == VI )
    {
	textview_EndOfLineCmd(self);
	textview_OpenLineCmd(self);
    }

    initialPos = pos = (self)->GetDotPosition();

    (self)->PrepareInsertion( FALSE);
    pos+= (Text(self))->InsertFile( inputFile,filename, pos); /*added "pos+=" so insertion would be highlighted. RSK*/

    fclose(inputFile);

    (self)->SetDotPosition(initialPos);
    (self)->SetDotLength( pos - initialPos);
/*  im_SetLastCmd(textview_GetIM(self), lcYank); */
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
}

static void YankCmd(register class textview  *self, boolean onlycut)
{
    long ct;
    long initialPos;
    long pos;
    class text *d;
    FILE *pasteFile;

    if (ConfirmReadOnly(self))
        return;
    ct = ((self)->GetIM())->Argument();
    if (ct > 100) {
        message::DisplayString(self, 0, "Yank argument limit: 100");
        return;
    }

    d = Text(self);

    initialPos = pos = (self)->CollapseDot();

    (self)->PrepareInsertion( FALSE);
    while (ct--) {
	pasteFile = onlycut?((self)->GetIM())->OnlyFromCutBuffer():((self)->GetIM())->FromCutBuffer();
        pos += (d)->InsertFile( pasteFile, NULL, pos);
	((self)->GetIM())->CloseFromCutBuffer( pasteFile);
    }
    (self)->FinishInsertion();

    (self)->SetDotPosition(initialPos);
    (self)->SetDotLength( pos - initialPos);
    ((self)->GetIM())->SetLastCmd( lcYank);
    (d)->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_YankCmd(class textview *self)
{
    YankCmd(self, FALSE);
}

static void textview_DoRotatePaste(register class textview  *self, int  count)
{
    register class text *d = Text(self);

    ((self)->GetIM())->ClearArg();        /* Make it safe to call yank. */

    if (ConfirmReadOnly(self))
        return;
    (d)->DeleteCharacters(		/* Get rid of what is there now. */
	(self)->GetDotPosition(), (self)->GetDotLength());
    if (((self)->GetIM())->GetLastCmd() != lcYank)    /* If not following yank. */
        count--;    /* Make it get top thing off of ring, instead of one down on ring. */
    ((self)->GetIM())->RotateCutBuffers( count);    /* Put the ring in the right place. */
    YankCmd(self, TRUE);    /* Snag it in off the ring. */
}



void textview_PutAfterCmd(register class textview  *self)
    {
    FILE *pasteFile;
    int cutChar;
    long	oldPos;

    oldPos = (self)->CollapseDot();

    pasteFile = ((self)->GetIM())->FromCutBuffer();
    while ( (cutChar = getc(pasteFile)) != EOF && cutChar != '\n' );
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
    if ( cutChar == '\n' )
    {
	/* lines are being pasted - insert text after CURRENT LINE */
	textview_NextLineCmd(self);
	textview_BeginningOfLineCmd(self);
	if ( (self)->GetDotPosition() <= oldPos )
	{
	    /* at end of file and no NL - add one */
	    textview_EndOfLineCmd(self);
	    textview_OpenLineCmd(self);
	}
    }
    /* otherwise insert at current cursor position */
    textview_YankCmd(self);
}

void textview_PutBeforeCmd(register class textview  *self)
    {
    FILE *pasteFile;
    int cutChar;

    (self)->CollapseDot();

    pasteFile = ((self)->GetIM())->FromCutBuffer();
    while ( (cutChar = getc(pasteFile)) != EOF && cutChar != '\n' );
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
    if ( cutChar == '\n' )
	/* lines are being pasted - insert text before CURRENT LINE */
	textview_BeginningOfLineCmd(self);
    /* otherwise insert at current cursor position */
    textview_YankCmd(self);
}

void textview_BackwardsRotatePasteCmd(class textview  *self)
{
    textview_DoRotatePaste(self, - ((self)->GetIM())->Argument());
}

void textview_RotatePasteCmd(class textview  *self)
{
    textview_DoRotatePaste(self, ((self)->GetIM())->Argument());
}

void textview_InsertNLCmd(register class textview  *self)
{
    textview_SelfInsertCmd(self,'\012');
    if (((self)->GetIM())->GetLastCmd() != lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcNewLine);
    }
}

static int stringmatch(register class text  *d,register long  pos,register char  *c)
{
    /* Tests if the text begins with the given string */
    while(*c != '\0') {
        if((d)->GetChar(pos) != *c) return FALSE;
        pos++; c++;
    }
    return TRUE;
}

/* NOTE! If you call this routine with the appendFlag set,
  if what is already in the cutFile is ATK datastream,
  and if what is in the region is formatted as ATK datastream,
  the cutFile will contain TWO nested datastreams.

  You must yank the old into the region and cut the larger region
  if you want one datastream.
*/

void textview::DoCopyRegion(long  pos , long  len, boolean  appendFlag, boolean copyAsText)
{
    class text *d;
    register long nextChange;
    FILE *cutFile;
    int UseDataStream;

    d = Text(this);
    (d->rootEnvironment)->GetInnerMost( pos);
    nextChange = (d->rootEnvironment)->GetNextChange( pos);

    cutFile = ((this)->GetIM())->ToCutBuffer();
    if (UseDataStream = ((nextChange <= len|| stringmatch(d,pos,"\\begindata")) && (d)->GetExportEnvironments()))
	fprintf(cutFile, "\\begindata{%s, %d}\n",
		copyAsText ? "text": (d)->GetTypeName(),
		/* d->header.dataobject.id */ 999999);
    d->writeID = im::GetWriteID();
    (d)->WriteSubString( pos, len, cutFile, UseDataStream);
    
    if (UseDataStream)
	fprintf(cutFile, "\\enddata{%s,%d}\n", 
		copyAsText ? "text": (d)->GetTypeName(), /* d->header.dataobject.id */ 999999);

    if (appendFlag)
	((this)->GetIM())->AppendToCutBuffer( cutFile);
    else
	((this)->GetIM())->CloseToCutBuffer( cutFile);
}

void textview_ZapRegionCmd(register class textview  *self)
{
    long pos, len;
    class text *d = Text(self);

    if (ConfirmReadOnly(self))
        return;
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    (self)->DoCopyRegion( pos, len, FALSE, d->CopyAsText);
    (self)->DeleteCharacters( pos, len);
    (self)->SetDotLength( 0);
    ((self)->GetIM())->SetLastCmd( lcKill);
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
}

/* In order to safely allow killing multiple lines of text, each possibly */
/* including ATK objects, the cutbuffer contents are pasted back and then */
/* a larger area is re-cut.  This isn't visible and wouldn't be so bad were */
/* it not for the extreme inefficiency of cutbuffer transfers. */

void textview_KillLineCmd(register class textview  *self)
{
    register int count, pos, endpos, lastpos;
    register class text *d;


    if ( self->editor == VI )
    	yankKillLine(self, DELETE);
    else {
	if (ConfirmReadOnly(self))
	    return;
	d = Text(self);
	count = ((self)->GetIM())->Argument();
	endpos = pos = (self)->CollapseDot();
	lastpos = (d)->GetLength();

	while (count-- > 0 && endpos < lastpos) {
	    if ((d)->GetChar( endpos) == '\012')
		endpos++;
	    else
		while ((d)->GetChar( endpos) != '\012' && endpos < lastpos)
		    endpos++;
	}

	if (((self)->GetIM())->GetLastCmd() == lcKill) {
	    FILE *pasteFile;
	    pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
	    endpos += (d)->InsertFile( pasteFile, NULL, pos);
	    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
	}

	if (endpos > pos) {
	    (self)->DoCopyRegion( pos, endpos - pos, FALSE, d->CopyAsText);
	    (self)->DeleteCharacters( pos, endpos - pos);
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	    (self)->SetDotLength( 0);
	    ((self)->GetIM())->SetLastCmd( lcKill);
	}
    }
}

/* At MIT They think of subsequent kills as part of one
  user command.
  Multiple kills will overwrite one cut buffer element.
  Multiple kills interrupted by a cut in another
  window will result in the latter cut being appended to
  the other window's cut. */

void textview_MITKillLineCmd(register class textview  *self)
{
    register int count, pos, endpos, lastpos;
    register class text *d;


    if ( self->editor == VI )
    	yankKillLine(self, DELETE);
    else {
	if (ConfirmReadOnly(self))
	    return;
	d = Text(self);
	count = ((self)->GetIM())->Argument();
	endpos = pos = (self)->CollapseDot();
	lastpos = (d)->GetLength();

	while (count-- > 0 && endpos < lastpos) {
	    if ((d)->GetChar( endpos) == '\012')
		endpos++;
	    else
		while ((d)->GetChar( endpos) != '\012' && endpos < lastpos)
		    endpos++;
	}

	if (((self)->GetIM())->GetLastCmd() == lcKill) {
	    FILE *pasteFile;
	    pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
	    endpos += (d)->InsertFile( pasteFile, NULL, pos);
	    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
	    /* hack! back up the ring so we overwrite old with new. */
	    ((self)->GetIM())->RotateCutBuffers( 1);
	}

	if (endpos > pos) {
	    (self)->DoCopyRegion( pos, endpos - pos, FALSE, d->CopyAsText);
	    (self)->DeleteCharacters( pos, endpos - pos);
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	    (self)->SetDotLength( 0);
	    ((self)->GetIM())->SetLastCmd( lcKill);
	}
    }
}

/* This routine will, if immediately followed by a text cutting commnd, cause
 * the cut to append to the current buffer instead of placing it in a new
 * buffer. */

void textview_AppendNextCut(class textview  *self)
{
    ((self)->GetIM())->SetLastCmd( lcKill);	/* mark us as a text killing command */
}

void textview_CopyRegionCmd (register class textview  *self)
{
    class text *d = Text(self);

    (self)->DoCopyRegion(
		  (self)->GetDotPosition(),
		  (self)->GetDotLength(), FALSE, d->CopyAsText);
}

void textview_GetToCol (register class textview  *self, register int  col)
{
    register class text *d;
    register int pos;

    d = Text(self);
    pos = (self)->GetDotPosition ();
    (self)->PrepareInsertion( FALSE);
    while (col > 0)  {
	if (col >= 8) {
	    col -= 8;
	    (d)->InsertCharacters (pos,"\011",1);
	}
	else  {
	    col--;
	    (d)->InsertCharacters(pos," ",1);
	}
	pos++;
    }
    (self)->FinishInsertion();
    (self)->SetDotPosition (pos);
    (d)->NotifyObservers( observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

void textview_InsertSoftNewLineCmd(register class textview  *self)
{
    textview_SelfInsertCmd(self,'\r');
}

void textview_MyLfCmd(register class textview  *self)
{
    register class text *d = Text(self);
    register int pos, startPos, endPos;

    if (ConfirmReadOnly(self))
	return;
    pos = (self)->CollapseDot();
    startPos = (d)->GetBeginningOfLine( pos);
    endPos = textview_GetNextNonSpacePos(self, startPos);
    pos = (d)->GetEndOfLine( pos);
    (self)->SetDotPosition( pos);
    textview_InsertNLCmd(self);
    textview_AddSpaces(self, (self)->GetDotPosition(), startPos, endPos - startPos);
    (self)->SetDotPosition( (self)->GetDotPosition() + endPos - startPos);
}

void textview_MySoftLfCmd(class textview  *self)
{
    register class text *d;
    register int pos, len, endPos, startPos;
    long c;

   if (ConfirmReadOnly(self))
        return;

    d = Text(self);
    pos = (self)->CollapseDot();
    for (startPos = pos; startPos > 0 && (c = (d)->GetChar( startPos - 1)) != '\r' && c != '\n'; startPos--) {
    }

    endPos = textview_GetNextNonSpacePos(self, startPos);
    len = (d)->GetLength();
    while (pos < len && (c = (d)->GetChar( pos)) != '\r' && c != '\n') {
	pos++;
    }
    
    (self)->SetDotPosition( pos);
    textview_InsertSoftNewLineCmd(self);
    textview_AddSpaces(self, (self)->GetDotPosition(), startPos, endPos - startPos);
    (self)->SetDotPosition( (self)->GetDotPosition() + endPos - startPos);
}

void textview_DeleteCmd (register class textview  *self)
{
    register int pos, len;

    if (ConfirmReadOnly(self))
        return;
    if (IsAutoCutMode() && (self->GetDotLength() > 0)) {
	textview_ZapRegionCmd(self);
	return;
    }
    pos = (self)->CollapseDot();

    len = ((self)->GetIM())->Argument();

    if (ConfirmViewDeletion(self, pos, len)) {
	(self)->DeleteCharacters( pos, len);
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	(self)->SetDotPosition(pos); /* this is USUALLY unnecessary, but... -RSK*/
        (self)->FrameDot( pos);
    }
}

void textview_ViDeleteCmd (register class textview  *self)
{
    register class text *d;
    register int pos, len, j;
    int		dsize;

    if (ConfirmReadOnly(self))
        return;
    d = Text(self);
    pos = (self)->CollapseDot();

    if ( (d)->GetChar( pos - 1) == '\n' && (d)->GetChar( pos) == '\n' )
	return;
	
    dsize = (d)->GetLength();
    len = (self->imPtr)->Argument();

    if (ConfirmViewDeletion(self, pos, len)) {
	for (j = 0; j < len; j++)
	{
	    if ( (d)->GetChar( pos) != '\n' && pos < dsize)
	    {
		(self)->DeleteCharacters( pos, 1);
	    }
	    else
		if ( pos-- >= 0 )
		{
		    (self)->DeleteCharacters( pos, 1);
		}
		else
		{
		    pos	= 0;
		    break;
		}
	    dsize--;
	}
	(self)->FrameDot( pos);
	(d)->NotifyObservers( observable_OBJECTCHANGED);
    }
}

void textview_KillWhiteSpaceCmd(register class textview  *self)
{
    register class text *d;
    register int p, tc, ep;

    /* First move back until no longer looking at whitespace */
    /* Then delete forward white space. */

    if (ConfirmReadOnly(self))
        return;
    d = Text(self);
    p = (self)->CollapseDot();
    while (p > 0)  {
	tc = 0xff & (d)->GetChar (p-1);
	if (tc == 9 || tc == 32) 
	    p--;
	else
	    break;
    }
    (self)->SetDotPosition(p);
    for (ep = p;
       ((tc = (d)->GetChar (ep)) != EOF) && (tc =='\t' || tc == ' '); ep++)
	;
    (self)->DeleteCharacters( p, ep - p);
    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
}

int textview_GetSpace(class textview  *self, register int  pos)
{
    register int rval;
    register int tc;
    register class text *d;
    register long len;
    
    d = Text(self);
    rval = 0;
    len = (d)->GetLength();
    while (pos < len)  {
	tc = 0xff & (d)->GetChar(pos);
	if (tc == 32) rval++;
	else if (tc == 9)
	    rval += 8;
	else
	    return rval;
	pos++;
    }
    return rval;
}

static void AdjustIndentation(class textview  *self, int amount)
{
    int indentation;

    if (ConfirmReadOnly(self))
        return;

    textview_StartOfParaCmd(self);
    indentation = textview_GetSpace(self,
             (self)->GetDotPosition());
    textview_KillWhiteSpaceCmd(self);
    indentation += amount;
    textview_GetToCol(self, indentation);
    textview_EndOfParaCmd(self);
}

void textview_UnindentCmd(class textview  *self)
{
    AdjustIndentation(self, -4);
}

void textview_IndentCmd(class textview  *self)
{
    AdjustIndentation(self, 4);
}

void textview_ExchCmd(register class textview  *self)
{
    register long p;
    register long len;

    p = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotPosition( (self->atMarker)->GetPos());
    (self)->SetDotLength( (self->atMarker)->GetLength());
    (self->atMarker)->SetPos(p);
    (self->atMarker)->SetLength( len);
    (self)->FrameDot( (self)->GetDotPosition());
    (self)->WantUpdate( self);
}

void textview_RuboutCmd (register class textview  *self)
{
    register long endpos, len;
    class text *d = Text(self);

    if (ConfirmReadOnly(self))
        return;

    if (IsAutoCutMode() && (self->GetDotLength() > 0)) {
	textview_ZapRegionCmd(self);
	return;
    }

    endpos = (self)->CollapseDot();

    len = ((self)->GetIM())->Argument();

    if (endpos == 0)
        return;

    if (endpos - len < 0)
	len = endpos;

    if (ConfirmViewDeletion(self, endpos - len, len)) {
	if ( self->editor == VI ) {
	    long lookback=0; /*RSK*/
	    while (endpos-lookback>0 && lookback<len && d->GetChar(endpos-lookback-1)!='\n')
		++lookback;
	    len= lookback;
	}
	self->DeleteCharacters(endpos -= len, len);
        (self)->SetDotPosition( endpos);
        (self)->FrameDot( endpos);
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

/* Switches the two characters before the dot. */

void textview_TwiddleCmd (register class textview  *self)
{
    long pos;
    register class text *text;
    char char1, char2;
    class environment *env1, *env2;
    class viewref *vr1, *vr2;

    if (ConfirmReadOnly(self))
        return;

    pos = (self)->CollapseDot();
    text = Text(self);

    if (pos < 2 || (text)->GetReadOnly() == TRUE)
        return;

    char1 = (text)->GetChar( pos - 2);
    char2 = (text)->GetChar( pos - 1);

    (text)->ReplaceCharacters( pos - 2, 1, &char2, 1);
    (text)->ReplaceCharacters( pos - 1, 1, &char1, 1);

    /* Code to deal with VIEWREFCHARS  -cm26 */
    /* If a view must be moved, it's deleted then reinserted */
    /* (environment_Update does not seem to work) */
    /* The order of the following is kind of important */

    env1 = (class environment *)(text->rootEnvironment)->GetInnerMost( pos - 2);
    if (env1 != NULL)
        if (env1->type != environment_View)
            env1 = NULL;
        else {
            vr1 = env1->data.viewref;
            env1->data.viewref = NULL;  /* Protect viewref from Delete */
            (env1)->Delete();
        }

    env2 = (class environment *)(text->rootEnvironment)->GetInnerMost( pos - 1);
    if (env2 != NULL)
        if (env2->type != environment_View)
            env2 = NULL;
        else {
            vr2 = env2->data.viewref;
            env2->data.viewref = NULL;  /* Protect viewref from Delete */
            (env2)->Delete();
        }

    if (env1 != NULL)
        (text->rootEnvironment)->WrapView(
            pos - 1, 1, vr1);

    if (env2 != NULL)
        (text->rootEnvironment)->WrapView(
            pos - 2, 1, vr2);

    (self)->FrameDot( pos);
    (text)->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_RuboutWordCmd (register class textview  *self)
{
    register int count, endpos, len;

    if (ConfirmReadOnly(self))
        return;

    endpos = (self)->CollapseDot();

    count = ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();   /* Don't want Bkwd Word to use it! */

    while (count--)
	textview_BackwardWordCmd(self);

    len = endpos - (self)->GetDotPosition();

    if (ConfirmViewDeletion(self, endpos - len, len)) {
	(self)->DeleteCharacters( endpos, -len);
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
        (self)->FrameDot( endpos - len);
    }
}

/* The safest way to prepend 'back killed' text is to yank it and kill the larger region. */

void textview_BackKillWordCmd (register class textview  *self)
{
    register int count, endpos, pos, startpos;
    register class text *d;

    if (ConfirmReadOnly(self))
        return;

    d = Text(self);

    endpos = pos = (self)->CollapseDot();

    count = ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();   /* Don't want Bkwd Word to use it! */

    while (count--)
	textview_BackwardWordCmd(self);

    startpos = (self)->GetDotPosition();

    if (ConfirmViewDeletion(self, startpos, endpos - startpos)) {
	if ((((self)->GetIM())->GetLastCmd() == lcKill)) {
	    FILE *pasteFile;
	    pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
	    endpos += (d)->InsertFile( pasteFile, NULL, pos);
	    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
	    /* hack! Back up the ring so we overwrite old with new. */
	    ((self)->GetIM())->RotateCutBuffers( 1);
	}
	(self)->DoCopyRegion( startpos, endpos - startpos, FALSE, d->CopyAsText);
	(self)->DeleteCharacters( startpos, endpos - startpos);
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
        (self)->FrameDot( startpos);
	((self)->GetIM())->SetLastCmd( lcKill);
    }
}

static void yankDeleteWord (register class textview  *self, int		 action, proctable_fptr moveFunction)
            { 
    register int i, ct, pos, npos, cutpos;
    boolean	backward = FALSE;
    FILE	*cutFile;
    register class text *text;

    if (ConfirmReadOnly(self))
        return;

    text = Text(self);
    (self)->CollapseDot();
    i = 0;
    ct = ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();
    cutFile = ((self)->GetIM())->ToCutBuffer();
    while (i<ct)  {
	pos = (self)->GetDotPosition();
	(*moveFunction)(self, 0);
	npos=(self)->GetDotPosition();
	if ( npos < pos )
	{
	    /* backward move */
	    backward	= TRUE;
	    cutpos = pos;
	    pos = npos;
	    npos = cutpos;
	}
	for (cutpos = pos; cutpos < npos; cutpos++)
	    putc((text)->GetChar( cutpos), cutFile);
	if ( action == DELETE && ConfirmViewDeletion(self, pos, npos - pos) )
	    (self)->DeleteCharacters( pos, npos - pos);
	if ( backward && action == YANK )
	    pos	= npos;
	(self)->SetDotPosition( pos);
	i++;
    }
    ((self)->GetIM())->CloseToCutBuffer( cutFile);
    (self)->FrameDot( pos);
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_YankWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_ForwardWordCmd);
}

void textview_DeleteEndOfWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, DELETE, (proctable_fptr)textview_EndOfWordCmd);
}

void textview_YankEndOfWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_EndOfWordCmd);
}

void textview_DeleteBackwardWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, DELETE, (proctable_fptr)textview_BackwardWordCmd);
}

void textview_YankBackwardWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_BackwardWordCmd);
}

void textview_DeleteWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, DELETE, (proctable_fptr)textview_ForwardWSWordCmd);
}

void textview_YankWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_ForwardWSWordCmd);
}

void textview_DeleteBackwardWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, DELETE, (proctable_fptr)textview_BackwardWSWordCmd);
}

void textview_YankBackwardWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_BackwardWSWordCmd);
}

void textview_DeleteEndOfWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, DELETE, (proctable_fptr)textview_EndOfWSWordCmd);
}

void textview_YankEndOfWSWordCmd(register class textview  *self)
    {
    yankDeleteWord(self, YANK, (proctable_fptr)textview_EndOfWSWordCmd);
}

static void viYankDeleteLine(class textview  *self, int		 action)
        {
    register int  count, pos, endpos, lastpos, numNLs, applen = 0;
    register class text *d;
 
    pos = (self)->CollapseDot();
    d = Text(self);
    count = ((self)->GetIM())->Argument();
    lastpos = (d)->GetLength();

    /* find beginning of line */
    while (pos > 0 && (d)->GetChar( pos - 1) != '\012')
	pos--;

    endpos = pos;

    /* find end of line range to be deleted/yanked */
    for (numNLs = 0; endpos < lastpos && numNLs < count; endpos++)
	if ((d)->GetChar( endpos) == '\012')
	    numNLs++;

    if (((self)->GetIM())->GetLastCmd() == lcKill) {
	FILE *pasteFile;
	pasteFile = ((self)->GetIM())->OnlyFromCutBuffer();
	applen = (d)->InsertFile( pasteFile, NULL, pos);
	((self)->GetIM())->CloseFromCutBuffer( pasteFile);
	/* hack! back up the ring so we overwrite old with new. */
	((self)->GetIM())->RotateCutBuffers( 1);
    }

    (self)->DoCopyRegion( pos, endpos + applen - pos, FALSE, d->CopyAsText);

    /* If this is not a deletion operation, we still have to
      remove the chars we inserted for our append operation */
    if ( action == DELETE ) {
	(self)->DeleteCharacters( pos, endpos + applen - pos);
	if (pos == (d)->GetLength() && pos > 0 )
	{
	    (self)->SetDotPosition( pos - 1);
	    (self->imPtr)->ClearArg();
	    textview_BeginningOfFirstWordCmd(self);
	    pos = (self)->GetDotPosition();
	}
	(d)->NotifyObservers( observable_OBJECTCHANGED);
	(self)->SetDotPosition( pos);
	(self)->FrameDot( pos);
	(self)->WantUpdate( self);
    }
    else
	(self)->DeleteCharacters( pos, applen);
	
    (self->imPtr)->SetLastCmd( lcKill);	/* mark us as a text killing command */
}

void textview_ViDeleteLineCmd(class textview  *self)
    {
    viYankDeleteLine(self, DELETE);
}

void textview_ViYankLineCmd(class textview  *self)
    {
    viYankDeleteLine(self, YANK);
}

void textview_OpenLineBeforeCmd(class textview  *self)
    {
    long	oldPos;
    char	nlChar	=	'\n';

    oldPos	= (self)->GetDotPosition();
    textview_PreviousLineCmd(self);
    ((self)->GetIM())->ClearArg(); 
    if ( oldPos == (self)->GetDotPosition() )
    {
	(self)->PrepareInsertion( TRUE);
	(Text(self))->InsertCharacters( 0, &nlChar, 1);
	(self)->FinishInsertion();
	(self)->SetDotPosition( 0);
    }
    else
    {
	textview_EndOfLineCmd(self);
	textview_OpenLineCmd(self);
    }
    (self)->ToggleVIMode();
}

void textview_OpenLineAfterCmd(class textview  *self)
    {
    (self->imPtr)->ClearArg(); 
    textview_EndOfLineCmd(self);
    textview_OpenLineCmd(self);
    (self)->ToggleVIMode();
}

void textview_InsertAtBeginningCmd(class textview  *self)
    {
    textview_BeginningOfLineCmd(self);
    (self)->ToggleVIMode();
}

void textview_InsertAtEndCmd(class textview  *self)
    {
    textview_EndOfLineCmd(self);
    (self)->ToggleVIMode();
}

void textview_ChangeRestOfLineCmd(class textview  *self)
    {
    (self->imPtr)->ClearArg(); 
    textview_KillLineCmd(self);
    (self)->ToggleVIMode();
}

void textview_ChangeLineCmd(class textview  *self)
    {
    int		dsize;
    class text	*text;
    
    text	= Text(self);
    dsize	= (text)->GetLength();
    
    textview_ViDeleteLineCmd(self);
    if ( (text)->GetEndOfLine( (self)->GetDotPosition()) == dsize )
    	textview_OpenLineAfterCmd(self);
    else
	textview_OpenLineBeforeCmd(self);
}

void textview_ChangeWordCmd(class textview  *self)
    {
    textview_DeleteEndOfWordCmd(self);
    (self)->ToggleVIMode();
}

void textview_ChangeSelectionCmd(class textview  *self)
    {
    textview_ZapRegionCmd(self);
    (self)->ToggleVIMode();
}

void textview_ReplaceCharCmd(class textview  *self)
    {
    char 	tc;
    long	pos;
    

    (self->imPtr)->ClearArg(); 
    pos	= (self)->GetDotPosition();
    tc = ((self)->GetIM())->GetCharacter();
    (Text(self))->ReplaceCharacters( pos, 1, &tc, 1);
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_SubstituteCharCmd(class textview  *self)
    {

    textview_ViDeleteCmd(self);
    (self)->ToggleVIMode();
}

void textview_DigitCmd(class textview  *self, register char  c)
{
    class im *im = (self)->GetIM();

    if ( self->editor == VI && self->viMode == COMMAND )
    {
	if ((im)->ArgProvided() || c != '0') {
	    (im)->BumpArg( c-'0');
	    (im)->DisplayArg();
	}
	else {
		/* kludge to handle "0" vi command */
		textview_BeginningOfLineCmd(self);
	}
	return;
    }
    textview_SelfInsertCmd(self, c);
}

static void AdjustCase(class textview  *self, boolean  upper , boolean  firstOnly)
{
    long pos, len, count, i;
    boolean capitalize;

    if (ConfirmReadOnly(self))
        return;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    count = len;

    if (len == 0)
        while (pos > 0 && isalnum((Text(self))->GetChar( pos - 1)))
            pos--;

    capitalize = TRUE;

    while (pos < (Text(self))->GetLength()) {
        i = (Text(self))->GetChar( pos);
        if (len == 0) {
            if (! isalnum(i))
                break;
        } else {
            if (--count < 0)
                break;
        }
        if (upper) {
            if ((capitalize || ! firstOnly) && islower(i)) {
                char new_c = toupper(i);
                (Text(self))->ReplaceCharacters( pos, 1, &new_c, 1);
            }
	    else if(!capitalize && firstOnly && isupper(i)){
		char new_c = tolower(i);
		(Text(self))->ReplaceCharacters( pos, 1, &new_c, 1);
	    }
        } else if (isupper(i)) {
            char new_c = tolower(i);
            (Text(self))->ReplaceCharacters( pos, 1, &new_c, 1);
        }
        capitalize = isspace(i);
        pos++;
    }

    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);
}

void textview_UppercaseWord(class textview  *self, long  key)
{
    AdjustCase(self, TRUE, FALSE);
}

void textview_LowercaseWord(class textview  *self, long  key)
{
    AdjustCase(self, FALSE, FALSE);
}

void textview_CapitalizeWord(class textview  *self, long  key)
{
    AdjustCase(self, TRUE, TRUE);
}

void textview_ToggleCase(class textview  *self, long  key)
{
    long pos = (self)->CollapseDot();
    long len = (Text(self))->GetLength();
    int i;
    char new_c;

    if (ConfirmReadOnly(self) || pos==len)
	return;

    i = (Text(self))->GetChar( pos);

    if (isupper(i))
        new_c = tolower(i);
    else if (islower(i))
        new_c = toupper(i);
    else
        new_c = i;

    (Text(self))->ReplaceCharacters( pos, 1, &new_c, 1);
    (Text(self))->NotifyObservers( observable_OBJECTCHANGED);

    (self)->SetDotPosition( pos + 1);
}

void textview_QuoteCmd(register class textview  *self)
{
    register long i;
    long count;
    register class text *d;
    char tc;
    long where;

    if (ConfirmReadOnly(self))
        return;
    count = ((self)->GetIM())->Argument();
    d = Text(self);
    tc = ((self)->GetIM())->GetCharacter();
    if (tc >= '0' && tc <= '7') {
        char c1 = ((self)->GetIM())->GetCharacter() - '0',
             c2 = ((self)->GetIM())->GetCharacter() - '0';
        tc = ((tc - '0' << 3) + c1 << 3) + c2;
    }
    where = (self)->GetDotPosition();
    (self)->PrepareInsertion( tc == '\n');
    for (i = 0; i < count; i++) {
	(d)->InsertCharacters( where+i, &tc, 1);
    }
    (self)->FinishInsertion();
    (self)->SetDotPosition( where+count);
    (d)->NotifyObservers( observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

