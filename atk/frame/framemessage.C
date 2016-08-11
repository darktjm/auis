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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/frame/RCS/framemessage.C,v 3.8 1996/03/13 16:57:17 robr Stab74 $";
#endif


 

/* framemsg.c
 * Provides functions for frame's message line.
  */

#include <andrewos.h>
ATK_IMPL("framemessage.H")

#include <im.H>
#include <keystate.H>
#include <view.H>
#include <environ.H>
#include <event.H>
#include <msghandler.H>
#include <frame.H>
#include <text.H>
#include <textview.H>
#include <frameview.H>
#include <style.H>
#include <framemessage.H>
#include <owatch.H>

#define DEFAULTMESSAGETIMEOUT 15 /* Default timeout in seconds. */
static int  DIALOGPRIORITY ;
#define DEFAULTDIALOGPRIORITY 50
#define USEDIALOG(priority) (priority > DIALOGPRIORITY)
static long messageTimeout;

#ifdef RCH_ENV
#include <application.H>

static boolean got_copyrightnotice = FALSE;
static char copyrightnotice[100] = AUIS_SHORT_COPYRIGHT;
#else
static char copyrightnotice[]=AUIS_SHORT_COPYRIGHT;
#endif


ATKdefineRegistry(framemessage, msghandler, framemessage::InitializeClass);
#ifndef NORCSID
#endif
static void EraseDisplayedMessage(class framemessage  *self);
static void QueueErasure(class framemessage  *self, long  length /* Length of text to erase. */);
static void BuildPrompt(class framemessage  *self, char  *prompt, char  *defaultString);
static void Process(class framemessage  *self, char  *buffer, int  bufferSize);
#ifdef THISCODEWORKS
static char *KludgePrompt(char  *prompt, long  defaultChoice, char  *choices[], char  *abbrevKeys);
#endif /* THISCODEWORKS */
void CancelNoExit(class framemessage  *self);


framemessage::framemessage()
{
    ATKinit;


    this->frame = NULL;
    this->erasureEvent=NULL;
    this->messageText = new text;
    (this->messageText)->SetObjectInsertionFlag(FALSE);
#ifdef RCH_ENV
    if (!got_copyrightnotice) {
	char *atkvers;
	FILE *fp;
	char *fname;
	char msg[60];

	atkvers = application::GetATKVersion();
	fname = environ::AndrewDir("/lib/atkstartup.msg");
	if (fname != NULL) {
	    if ((fp = fopen(fname, "r")) != NULL) {
		if (fgets(msg, sizeof(msg), fp) != NULL) {
		    sprintf(copyrightnotice, "ATK %s %s", atkvers, msg);
		}
		fclose(fp);
	    }
	}
	got_copyrightnotice = TRUE;
    }
#endif
    (this->messageText)->InsertCharacters( 0, copyrightnotice, strlen(copyrightnotice));
    this->messageView = frameview::Create(this);
    (this->messageView)->SetDataObject( this->messageText);
    this->oldInputFocus = NULL;
    this->asking = FALSE;
    this->punt = FALSE;
    this->completionProc = NULL;
    this->helpProc = NULL;
    this->flags = 0;
    this->textBuffer = NULL;
    this->maxTextSize = 0;
    this->keystatep = NULL;
    this->messageLen = 0;
    DIALOGPRIORITY = environ::GetProfileInt( "DialogPriority", DEFAULTDIALOGPRIORITY);
    if(DIALOGPRIORITY > 99 ) DIALOGPRIORITY = 99;
    if(DIALOGPRIORITY < 1 ) DIALOGPRIORITY = 1;
    this->companion = NULL;
    THROWONFAILURE( TRUE);
}

framemessage::~framemessage()
{
    ATKinit;

    if (this->messageView != NULL)
        (this->messageView)->Destroy();
    if (this->messageText)
        (this->messageText)->Destroy();

}

class framemessage *framemessage::Create(class frame  *frame)
        {
	ATKinit;


    class framemessage *self;

    self = new framemessage;
    self->frame = frame;
    return self;
}

static void EraseDisplayedMessage(class framemessage  *self)
    {
    if (self->messageLen > 0) {
        (self->messageText)->AlwaysDeleteCharacters( 0, self->messageLen);
	self->messageLen = 0;
	(self->messageView)->SetWantedLines( 1);
        (self->messageView)->WantUpdate( self->messageView);
    }
    self->erasureEvent=NULL;
}

/* Enqueue erasure event. */
static void QueueErasure(class framemessage  *self, long  length /* Length of text to erase. */)
{
   if(!(self->messageView)->GetIM()) return;
   if(self->erasureEvent) {
       ((self->messageView)->GetIM())->CancelInteractionEvent(  self->erasureEvent);
       self->erasureEvent=NULL;
   }
    self->messageLen = length;
    if(length>0)
	self->erasureEvent = ((self->messageView)->GetIM())->SetInteractionEvent( (im_interactionfptr) EraseDisplayedMessage,
			       (long) self,
			       event_SECtoTU(messageTimeout));
}

int framemessage::DisplayString(int  priority, char  *string)
{

    int len = strlen(string);

    if (USEDIALOG(priority) ) {
	return (this->frame)->DisplayString( priority, string);
    }

    if ((this)->Asking()) {
	if(len>0){
	    if (this->messageLen != 0)
		(this->messageText)->AlwaysDeleteCharacters(
					    2,this->messageLen-5);
	    else
		(this->messageText)->AlwaysInsertCharacters(
					    0, "[  ] ", 5);
	    (this->messageText)->AlwaysInsertCharacters( 2, string, len);
	    len += 5;
	    if((this->messageText)->GetFence() == 0)
		(this->messageText)->SetFence(len);
	}else
	    (this->messageText)->AlwaysDeleteCharacters(
					0,this->messageLen);
    } else {
	(this->messageView)->SetWantedLines( 1);
        (this->messageText)->Clear();
	if(len>0){
	    (this->messageText)->AlwaysInsertCharacters( 0, string, len);
	    /* Omitted setdotpos because 1) it should be at 0, 2) if it doesn't have the input focus, the dot will not be displayed. */
	    (this->messageView)->SetTopPosition( 0);
	}
    }

    QueueErasure(this, len);
    (this->messageView)->WantUpdate( this->messageView);
    return 0;
}

#define ERROR -1 /* For lack of a better value. */

/* Really does everything common to all "AskFor..." calls.
 * Should either be renamed, or broken into two routines. 
 */
static void BuildPrompt(class framemessage  *self, char  *prompt, char  *defaultString)
            {

    int len;

    (self->messageText)->Clear();
    self->messageLen = 0;
    if((self->messageView)->GetIM())  if(self->erasureEvent) {
       ((self->messageView)->GetIM())->CancelInteractionEvent(  self->erasureEvent);
       self->erasureEvent=NULL;
   }
    (self->messageText)->AlwaysInsertCharacters( 0, prompt, len = strlen(prompt));
    (self->messageText)->SetFence( len);
    if (!(self->flags & message_NoInitialString) && (defaultString != NULL))
        (self->messageText)->AlwaysInsertCharacters( (self->messageText)->GetLength(), defaultString, strlen(defaultString));
    (self->messageView)->SetDotPosition( (self->messageText)->GetLength());
    (self->messageView)->SetDotLength( 0);
}



static void Process(class framemessage  *self, char  *bufout, int  bufoutSize)
{
    char *qanswer=NULL;
    qanswer=im::GetAnswer();
    if(qanswer==NULL) {
	if(!im::AnswerWasCancel()) im::KeyboardProcessor();
	else CancelNoExit(self);
	if (!self->punt) {
	    (self)->GetCurrentString( bufout, bufoutSize);
	    im::RecordAnswer(bufout);
	} else im::RecordCancellation();
    } else {
	strncpy(bufout, qanswer, bufoutSize-1);
	bufout[bufoutSize-1]='\0';
    }
}

int framemessage::AskForString(int  priority, char  *prompt , char  *defaultString , char  *bufout, int  bufoutSize /* Is actual sizeof bufout including NUL. */)
                {
    char *qanswer=NULL;
    struct owatch_data *checkit=NULL;
    if ((this)->Asking())
        return ERROR;
    asking=TRUE;
    this->flags = 0;
    this->hasDefault = (defaultString != NULL);
    
    if ( ((class textview *) this->messageView)->editor == VI &&
	 ((class textview *) this->messageView)->viMode == COMMAND )
	(this->messageView)->ToggleVIMode();

    if (USEDIALOG(priority) ) {
	int ret=(this->frame)->AskForString( priority, prompt, defaultString, bufout, bufoutSize);
	asking=FALSE;
	return ret;
    }
    BuildPrompt(this, prompt, defaultString);

    this->oldInputFocus = ((this->messageView)->GetIM())->GetInputFocus();
    checkit=owatch::Create(this->oldInputFocus);
    
    (this->messageView)->WantInputFocus( this->messageView);

    (this->messageView)->SetWantedLines( 1);
    Process(this, bufout, bufoutSize);
    
    this->asking = FALSE;

    if (owatch::CheckAndDelete(checkit))
        (this->oldInputFocus)->WantInputFocus( this->oldInputFocus);

    if (this->punt) {
        this->punt = FALSE;
        return ERROR;
    }

    QueueErasure(this, (this->messageText)->GetLength());

    return 0;
}

int framemessage::AskForPasswd(int  priority, char  *prompt , char  *defaultString , char  *bufout, int  bufoutSize /* Is actual sizeof bufout including NUL. */)
                {
    static class style *passwdStyle = NULL;
    struct owatch_data *checkit=NULL;
    if (passwdStyle == NULL) {
            passwdStyle = new style;
	    (passwdStyle)->AddHidden();
    }
    this->flags = 0;
    this->hasDefault = (defaultString != NULL);

    if ((this)->Asking())
        return ERROR;
    asking=TRUE;
    if (USEDIALOG(priority) ) {
	int ret=(this->frame)->AskForPasswd( priority, prompt, defaultString, bufout, bufoutSize);
	asking=FALSE;
	return ret;
    }
    BuildPrompt(this, prompt, defaultString);

    this->oldInputFocus = ((this->messageView)->GetIM())->GetInputFocus();
    checkit=owatch::Create(this->oldInputFocus);
    

    (this->messageText)->AlwaysAddStyle(
	(this->messageText)->GetLength()-1, 1, passwdStyle);

    (this->messageView)->WantInputFocus( this->messageView);


    (this->messageView)->SetWantedLines( 1);
    Process(this, bufout, bufoutSize);
    
    this->asking = FALSE;

    if (owatch::CheckAndDelete(checkit))
        (this->oldInputFocus)->WantInputFocus( this->oldInputFocus);

    if (this->punt) {
        this->punt = FALSE;
        return ERROR;
    }

    QueueErasure(this, (this->messageText)->GetLength());

    return 0;
}

/* Hairy function... */
int framemessage::AskForStringCompleted(int  priority, char  *prompt , char  *defaultString , char  *bufout, int  bufoutSize /* Is actual sizeof bufout including NUL. */, class keystate  *keystatep, message_completionfptr completionProc, message_helpfptr helpProc, long  completionData, int  flags)
                                    {

/* The order things happen in this routine is very important. */
    char *qanswer=NULL;
    struct owatch_data *checkit=NULL;
    
    if ((this)->Asking())
	return ERROR;
    asking=TRUE;
    if (USEDIALOG(priority) ) {
	int ret=(this->frame)->AskForStringCompleted( priority, prompt, defaultString, bufout, bufoutSize, keystatep, completionProc, helpProc, completionData, flags);
	asking=FALSE;
	return ret;
    }
    this->flags = flags;
    this->hasDefault = (defaultString != NULL);

    if ( ((class textview *) this->messageView)->editor == VI &&
	 ((class textview *) this->messageView)->viMode == COMMAND )
	(this->messageView)->ToggleVIMode();

    BuildPrompt(this, prompt, defaultString);
    this->textBuffer = bufout;
    this->maxTextSize = bufoutSize;

    if (keystatep != NULL) {
        (keystatep)->SetObject( this->messageView);
        this->keystatep = keystatep;
    }

    this->completionProc = completionProc;
    this->helpProc = helpProc;
    this->completionData = completionData;

    if((this->messageView)->GetIM())
	this->oldInputFocus=((this->messageView)->GetIM())->GetInputFocus();
    else this->oldInputFocus=NULL;
    checkit=owatch::Create(this->oldInputFocus);
    
    (this->messageView)->WantInputFocus( this->messageView);

    (this->messageView)->SetWantedLines( 1);
    qanswer=im::GetAnswer();
    if(qanswer==NULL) {
	if(!im::AnswerWasCancel()) im::KeyboardProcessor();
	else CancelNoExit(this);
	if (!this->punt) {
	    (this)->GetCurrentString( bufout, bufoutSize);
	    if ((this->flags & message_NoInitialString) && *bufout == '\0' && defaultString != NULL)
		strncpy(bufout, defaultString, bufoutSize);
	    im::RecordAnswer(bufout);
	} else im::RecordCancellation();
    } else {
	strncpy(bufout, qanswer, bufoutSize-1);
	bufout[bufoutSize-1]='\0';
    }
    this->completionProc = NULL;
    this->helpProc = NULL;

    this->textBuffer = NULL;
    this->maxTextSize = 0;

    this->asking = FALSE;
    
    if(this->frame) (this->frame)->RemoveHelp();
    
    if (owatch::CheckAndDelete(checkit))
        (this->oldInputFocus)->WantInputFocus( this->oldInputFocus);

    this->keystatep = NULL;
    if (this->punt) {
        this->punt = FALSE;
        return ERROR;
    }

    QueueErasure(this, (this->messageText)->GetLength());

    return 0;
}
#ifdef THISCODEWORKS
/* This routine kludges together a linear prompt for a multiple choice question. */
static char *KludgePrompt(char  *prompt, long  defaultChoice, char  *choices[], char  *abbrevKeys)
                {

    int i;
    long size = strlen(prompt) + 1;
    char *multipleChoicePrompt, *promptBuffer;

    for (i = 0; choices[i] != NULL; i++) {

        long len = strlen(choices[i]);

        if (len == 0)
            return NULL;
        size += len + 2; /* 2 for ", " */
        if (abbrevKeys != NULL) {
            if (abbrevKeys[i] == '\0')
                return NULL;
            if (abbrevKeys[i] == *choices[i]) /* Key equals first char of choice. */
                size += 2; /* 2 for "()" arround first letter. */
            else
                size += 3; /* for "(x)" before choice... */
        }
    }
    if (i == 0 || defaultChoice > i) /* Trap some bogus cases. */
        return NULL;

    if (defaultChoice != -1)
        if (abbrevKeys != NULL)
            size += 4; /* for "[x]?" at end. */
        else
            size += 3 + strlen(choices[defaultChoice]); /* for "[choice]?" at end. */

    promptBuffer = multipleChoicePrompt = (char *) malloc(size + 1);
    strcpy(promptBuffer, prompt);
    promptBuffer += strlen(multipleChoicePrompt);

    /* This crap fails if a choice is "". I trap this above. */
    for (i = 0; choices[i] != NULL; i++) {
        if (abbrevKeys != NULL) {
            *promptBuffer++ = '(';
            *promptBuffer++ = abbrevKeys[i];
            *promptBuffer++ = ')';
            strcat(promptBuffer, choices[i] + (abbrevKeys[i] == *choices[i]) ? 1 : 0);
        }
        else
            strcat(promptBuffer, choices[i]);
        strcat(promptBuffer, ", ");
        promptBuffer += strlen(promptBuffer);
    }

    if (defaultChoice != -1) {
        *promptBuffer++ = '[';
        if (abbrevKeys != NULL)
            *promptBuffer++ = abbrevKeys[i];
        else
            strcat(promptBuffer, choices[i]);
        *promptBuffer++ = ']';
        *promptBuffer++ = '?';
    }
    else { /* Mungify the ", " after the last item. */
        *--promptBuffer = '?';
        *--promptBuffer = ' ';
    }
}
#endif /* THISCODEWORKS */

int framemessage::MultipleChoiceQuestion(int  priority, char  *prompt, long  defaultChoice, long  *result, char  **choices, char  *abbrevKeys)
                            {
    struct owatch_data *checkit=NULL;
#ifdef THISCODEWORKS /* This is saracastic I assume (i.e. really should be #if 0). */
    char *tempPrompt;
#endif /* THISCODEWORKS  */

    this->flags = 0;

    if ((this)->Asking())
	return ERROR;
    asking=TRUE;
#ifdef THISCODEWORKS
    if (USEDIALOG(priority) ) {
	messageText->Clear();
	messageLen = 0;
	int ret=(this->frame)->MultipleChoiceQuestion( priority, prompt, defaultChoice, result, choices, abbrevKeys);
	asking=FALSE;
	return ret;
    }

    if ((tempPrompt = KludgePrompt(prompt, defaultChoice, choices, abbrevKeys)) == NULL) {
	asking=FALSE;
	return ERROR;
    }

    /* if VI user, put them in input mode if necessary */
    if ( (this->messageView)->editor == VI && (this->messageView)->viMode == COMMAND )
	(this->messageView)->ToggleVIMode();

    BuildPrompt(this, tempPrompt, "");

    this->oldInputFocus = ((this->messageView)->GetIM())->GetInputFocus();
    checkit=owatch::Create(this->oldInputFocus);

    (this->messageView)->WantInputFocus( this->messageView);
    
    /* if this code is to be used it should do something about the answer queue in im */
    im::KeyboardProcessor();

    free(tempPrompt);

    if (!this->punt)
        ; /* Should assign result with appropriate thing. */

    this->asking = FALSE;

    if (owatch::CheckAndDelete(checkit))
        (this->oldInputFocus)->WantInputFocus( this->oldInputFocus);

    if (this->punt) {
        this->punt = FALSE;
        return ERROR;
    }

    QueueErasure(this, (this->messageText)->GetLength());

    return 0;
#else /* THISCODEWORKS */
    messageText->Clear();
    messageLen = 0;
    int ret=(this->frame)->MultipleChoiceQuestion( priority, prompt, defaultChoice, result, choices, abbrevKeys);
    asking=FALSE;
    return ret;
#endif /* THISCODEWORKS */
}

void CancelNoExit(class framemessage  *self)
{
    if ((self)->Asking()) {
	self->punt = TRUE;
	self->asking = FALSE; /* In case we get a display string before the interact falls through. */
    }
    else if(self->companion) (self->companion)->CancelQuestion();
}

void framemessage::CancelQuestion()
    {

    if ((this)->Asking()) {
        this->punt = TRUE;
        this->asking = FALSE; /* In case we get a display string before the interact falls through. */
        im::KeyboardExit();
    }
    else if(this->companion) (this->companion)->CancelQuestion();
}

int framemessage::GetCurrentString(char  *bufout, int  bufoutSize /* Is actual sizeof bufout including NUL. */)
            {

    int pos, length;

    if (!(this)->Asking()){
	if(this->companion) 
	    return (this->companion)->GetCurrentString( bufout, bufoutSize);
	return 0;
    }
    pos = (this->messageText)->GetFence();
    length = (this->messageText)->GetLength();
    if (bufoutSize <= (length - pos))
        length = pos + bufoutSize - 1;
    while (pos < length)
        *bufout++ = (this->messageText)->GetChar( pos++);
    *bufout = '\0';
    return (this->messageView)->GetDotPosition() - pos;
}

int framemessage::InsertCharacters(int  pos, char  *string, int  len)
                {
    if (!(this)->Asking()){
	if(this->companion) 
	    return (this->companion)->InsertCharacters( pos, string, len);
        return ERROR;
    }
    pos += (this->messageText)->GetFence();
    (this->messageText)->InsertCharacters( pos, string, len);
    (this->messageText)->NotifyObservers( 0);
    return 0;
}

int framemessage::DeleteCharacters(int  pos, int  len)
            {

    if (!(this)->Asking()){
	if(this->companion) 
	    return (this->companion)->DeleteCharacters( pos, len);
        return ERROR;
    }

    (this->messageView)->SetWantedLines( 1);
    pos += (this->messageText)->GetFence();
    (this->messageText)->DeleteCharacters( pos, len);
    (this->messageText)->NotifyObservers( 0);
    return 0;
}

int framemessage::GetCursorPos()
    {
    if (!(this)->Asking()){
	if(this->companion) 
	    return (this->companion)->GetCursorPos();
        return ERROR;
    }

    return (this->messageView)->GetDotPosition() - (this->messageText)->GetFence();
}

int framemessage::SetCursorPos(int  pos)
        {

    if (!(this)->Asking()){
	if(this->companion) 
	    return (this->companion)->SetCursorPos(pos );
        return ERROR;
    }

    pos += (this->messageText)->GetFence();
    (this->messageView)->SetDotPosition( pos);
    (this->messageView)->SetDotLength( 0);
    (this->messageView)->WantUpdate( this->messageView);
    return 0;
}

boolean framemessage::Asking()
    {

    return this->asking;
}
void framemessage::SetCompanion(class msghandler  *companion)
{
    this->companion = companion;
}

boolean framemessage::InitializeClass()
    {

    messageTimeout = environ::GetProfileInt("MessageTimeout", DEFAULTMESSAGETIMEOUT); /* Expressed in seconds. */
    return TRUE;
}

void framemessage::Advice(enum message_Preference  pp)
{
    (this->frame)->Advice(pp);
}
