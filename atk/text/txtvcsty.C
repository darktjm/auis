/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#define AUXMODULE 1
#include <textview.H>
#include <txtvcmds.h>

#include <text.H>
#include <im.H>
#include <message.H>
#include <view.H>
#include <mark.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <environ.H>
#include <completion.H>
#include <txtvinfo.h>
#include <txtproc.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>

#define DELETE_NOSTYLES 0
#define DELETE_LEFT 1
#define DELETE_RIGHT 2




extern long lcNewLine;
extern long lcInsertEnvironment;

static boolean useOldInsertionRules;
static long deletionDirection;
static long deletionPosition;
static long deletionCount;
static class environment **flipBegEnvs;
static class environment **flipEndEnvs;
static long flipBegCnt;
static long flipEndCnt;
static long flipBegMax = 0;
static long flipEndMax = 0;
static class mark *insertMark = NULL;


static class environment *AddNewEnvironment(class textview  *self, class stylesheet  *ss, class environment  *env, struct InsertStack  *insert);
static void PushFlipBegEnv(class environment  *te, boolean  value);
static void PushFlipEndEnv(class environment  *te, boolean  value);
static void SetBeginningDown(class textview  *self, class environment  *te, long  pos);
static void SetEndingDown(class textview  *self, class environment  *te, long  pos);
void textview_PlainerCmd(class textview  *self, char  *type);
void textview_PlainestCmd(class textview  *self);
void textview_ExposeStyleEditor(class textview  *self);
void textview_ShowStylesCmd(class textview  *self);
void textview_ChangeTemplate(class textview  *self);
void textview_ToggleExposeStyles(class textview  *self);
void textview_ToggleColorStyles(class textview  *self);
static void DoDisplayInsertEnvironment(class textview  *self);
void textview_DisplayInsertEnvironment(class textview  *self);
void textview_PlainInsertEnvCmd(class textview  *self);
void textview_UpInsertEnvironmentCmd(class textview  *self);
void textview_DownInsertEnvironmentCmd(class textview  *self);
void textview_LeftInsertEnvironmentCmd(class textview  *self);
void textview_RightInsertEnvCmd(class textview  *self);
static boolean StyleCompletionWork(class style  *style, struct result  *data);
static enum message_CompletionCode StyleComplete(char  *partial, long ss, char  *resultStr, int  resultSize);
static boolean StyleHelpWork(class style  *style, struct helpData  *helpData);
static void StyleHelp(char  *partial, long ss, message_workfptr helpTextFunction, long  helpTextRock);
void textview_InsertEnvironment(class textview  *self, const char  *sName);
void InitializeMod();


void textview::PrepareDeletion(long  pos, long  len)
{
    class environment *env;
    long envPos;
    long envLen;
    class environment *newEnv;
    long newEnvPos;
    long newEnvLen;
    long newPos;
    long checkPos;
    class text *d = Text(this);

    deletionCount = 0;
    deletionDirection = DELETE_NOSTYLES;

    if (useOldInsertionRules) {
	return;
    }

    env = (this)->GetInsertEnvironment( pos);
    (this)->ClearInsertStack();
    if (env != NULL) {
	if (len < 0) {
	    newPos = pos + len;
	    checkPos = newPos;
	    newEnv = (class environment *)(d->rootEnvironment)->GetInnerMost( checkPos);
	    deletionDirection = DELETE_LEFT;
	}
	else {
	    newPos = pos;
	    checkPos = pos + len;
	    newEnv = (class environment *)(d->rootEnvironment)->GetInnerMost( checkPos - 1);
	    deletionDirection = DELETE_RIGHT;
	}

	envPos = (env)->Eval();
	envLen = (env)->GetLength();
	newEnvPos = (newEnv)->Eval();
	newEnvLen = (newEnv)->GetLength();

	if (newEnv != env /* && newEnvPos + newEnvLen <= pos */) {
	    env = newEnv;
	    envPos = newEnvPos;
	    envLen = newEnvLen;
	}

	while ((len < 0 && (envPos == newPos && pos >= envPos + envLen))
	       || (len > 0 && (envPos + envLen == pos + len && pos <= envPos))) {
	    if (env->type == environment_Style) {
		class style *style = env->data.style;
		const char *styleName = (style)->GetName();

		if (styleName != NULL) {
		    struct InsertStack *p = (struct InsertStack *) malloc(sizeof(struct InsertStack));

		    p->name = strdup(styleName);
		    p->next = this->insertStack;
		    this->insertStack = p;
		}
	    }
	    env = (class environment *) (env)->GetParent();
	    envPos = (env)->Eval();
	    envLen = (env)->GetLength();
	}

	deletionPosition = newPos;

	if (len > 0 && checkPos == envPos + envLen) {
	    deletionDirection = DELETE_LEFT;
	}
	else if (len < 0 && checkPos == envPos) {
	    deletionDirection = DELETE_RIGHT;
	}

	for (deletionCount = 0; env != d->rootEnvironment; env = (class environment *) (env)->GetParent()) {
	    deletionCount++;
	}

    }
}

void textview::FinishDeletion()
{
    class text *d = Text(this);
    class environment *env;
    class environment *newEnv;
    long pos;

    if (deletionDirection == DELETE_LEFT) {
	pos = deletionPosition - 1;
    }
    else {
	pos = deletionPosition;
    }

    env = d->rootEnvironment;

    while (deletionCount-- != 0) {
	newEnv = (class environment *)(env)->GetChild( pos);
	if (newEnv != NULL) {
	    env = newEnv;
	}
    }
	
    this->insertEnvironment = env;
    (this->insertEnvMark)->SetPos( deletionPosition);
    (this->insertEnvMark)->SetModified( FALSE);
}

void textview::DeleteCharacters(long  pos, long  len)
{
    if (len != 0) {
	(this)->PrepareDeletion( pos, len);
	if (len < 0) {
	    (Text(this))->DeleteCharacters( pos + len, -len);
	}
	else {
	    (Text(this))->DeleteCharacters( pos, len);
	}
	(this)->FinishDeletion();
	(Text(this))->NotifyObservers( observable::OBJECTCHANGED);
    }
}

static class environment *AddNewEnvironment(class textview  *self, class stylesheet  *ss, class environment  *env, struct InsertStack  *insert)
{
    class style *style = NULL;
    class environment *newEnv = NULL;

    if (insert-> next != NULL) {
	env = AddNewEnvironment(self, ss, env, insert->next);
    }

    if (ss != NULL) {
	style = (ss)->Find( insert->name);
	if (style == NULL) {
	    char message[100];

	    sprintf(message, "Could not find style %s", insert->name);
	    message::DisplayString(self, 0, message);
	}
    }

    if (style != NULL) {
	newEnv = (env)->WrapStyle( (insertMark)->GetPos(), (insertMark)->GetLength(), style);
    }

    return newEnv;
}

void textview::FinishInsertion()
{
    long i;
    long pos;

    if (insertMark != NULL) {
	if (this->insertEnvironment != NULL) {
	    this->insertEnvironment = AddNewEnvironment(this, (Text(this))->GetStyleSheet(), this->insertEnvironment, this->insertStack);
	}
	(this)->ClearInsertStack();

	(Text(this))->RemoveMark( insertMark);
	delete insertMark;
	insertMark = NULL;
    }

    if (flipBegCnt > 0 || flipEndCnt > 0) {
	for (i = flipBegCnt - 1; i >= 0; i--) {
	    flipBegEnvs[i]->includeBeginning = ! flipBegEnvs[i]->includeBeginning;
	}

	for (i = flipEndCnt - 1; i >= 0; i--) {
	    flipEndEnvs[i]->includeEnding = ! flipEndEnvs[i]->includeEnding;
	}

	pos = (this->insertEnvMark)->GetEndPos();
	(this->insertEnvMark)->SetPos( pos);
	(this->insertEnvMark)->SetLength( 0);
	(this->insertEnvMark)->SetModified( FALSE);
	(this->insertEnvMark)->SetStyle( FALSE, FALSE);
    }
    
}

static void PushFlipBegEnv(class environment  *te, boolean  value)
{
    if (flipBegCnt >= flipBegMax) {
	    flipBegMax = flipBegCnt * 2;
	    flipBegEnvs = (class environment **) realloc(flipBegEnvs, sizeof(class environment *) * flipBegMax);
    }
    flipBegEnvs[flipBegCnt++] = te;
    te->includeBeginning = value;
}

static void PushFlipEndEnv(class environment  *te, boolean  value)
{
    if (flipEndCnt >= flipEndMax) {
	    flipEndMax = flipEndCnt * 2;
	    flipEndEnvs = (class environment **) realloc(flipEndEnvs, sizeof(class environment *) * flipEndMax);
    }
    flipEndEnvs[flipEndCnt++] = te;
    te->includeEnding = value;
}

static void SetBeginningDown(class textview  *self, class environment  *te, long  pos)
{
    while ((te = (class environment *)(te)->GetChild( pos)) != NULL && te->type == environment_Style) {
	if (te->includeBeginning && (te)->Eval() == pos) {
	    PushFlipBegEnv(te, FALSE);
	}
    }
}

static void SetEndingDown(class textview  *self, class environment  *te, long  pos)
{
    long endPos;

    if (pos <= 0) {
	return;
    }

    while ((te = (class environment *)(te)->GetChild( pos - 1)) != NULL && te->type == environment_Style && (endPos = ((te)->Eval() + (te)->GetLength())) >= pos) {
	if (endPos == pos && te->includeEnding) {
	    PushFlipEndEnv(te, FALSE);
	}
    }
}

void textview::PrepareInsertion(boolean  insertingNewLine)
{
    long pos = (this)->GetDotPosition() + (this)->GetDotLength();
    long ePos;
    class environment *te;
    class text *d = Text(this);
    long lastCmd;

    flipBegCnt = 0;
    flipEndCnt = 0;

    if (useOldInsertionRules) {
	return;
    }

    if ((lastCmd = (this->imPtr)->GetLastCmd()) == lcInsertEnvironment && this->insertEnvironment != NULL && ! (this->insertEnvMark)->GetModified()) {
	if (this->insertStack != NULL) {
	    insertMark = (d)->CreateMark( pos, 0);
	    (insertMark)->SetStyle( FALSE, TRUE);
	}

	if ((ePos = (this->insertEnvironment)->Eval()) == pos) {
	    SetBeginningDown(this, this->insertEnvironment, pos);
	    for (te = this->insertEnvironment;
		 te != d->rootEnvironment && (te)->Eval() == pos;
		 te = (class environment *) (te)->GetParent()) {
		if (! te->includeBeginning) {
		    PushFlipBegEnv(te, TRUE);
		}
	    }
	    SetEndingDown(this, te, pos);
	}
	else if (ePos + (this->insertEnvironment)->GetLength() == pos) {
	    SetEndingDown(this, this->insertEnvironment, pos);
	    for (te = this->insertEnvironment;
		 te != d->rootEnvironment && (te)->Eval() + (te)->GetLength() != pos;
		 te = (class environment *) (te)->GetParent()) {
		if (! te->includeEnding) {
		    PushFlipEndEnv(te, TRUE);
		}
	    }
	    SetBeginningDown(this, te, pos);
	}
	else {
	    te = (class environment *)(this->insertEnvironment)->GetChild( pos);
	    if (te != NULL && (ePos = (te)->Eval()) != pos) {
		(te)->Split( pos - ePos);
	    }
	    SetEndingDown(this, this->insertEnvironment, pos);
	    SetBeginningDown(this, this->insertEnvironment, pos);
	}

	if (flipBegCnt > 0 || flipEndCnt > 0) {
	    (this->insertEnvMark)->SetStyle( FALSE, TRUE);
	}
    }
#ifdef IGNORE_STYLEFLAGS_AT_STARTOFPARAGRAPH
    else if (lastCmd != lcNewLine) {
	if (! insertingNewLine && (pos == 0 || (d)->GetChar( pos - 1) == '\n')) {
	    /*
	     Need to turn off the endings of any environments that
	     stop at pos and turn on the beginnings of any environments
	     that estart at pos
	     */
	    class environment *le;
	    class environment *pe;

	    te = d->rootEnvironment;
	    le = te;
	    while ((pe = (class environment *)(te)->GetChild( pos)) != NULL && pe->type == environment_Style) {
		if ((pe)->Eval() == pos) {
		    if (! pe->includeBeginning) {
			PushFlipBegEnv(pe, TRUE);
		    }
		    le = te;
		}
		te = pe;
	    }
	    if (pos > 0) {
		SetEndingDown(this, le, pos);
	    }
	}
    }
#endif /*IGNORE_STYLEFLAGS_AT_STARTOFPARAGRAPH*/
}

class environment *textview::GetEnclosingEnvironment(long  pos)
{
    class environment *te;
#ifdef IGNORE_STYLEFLAGS_AT_STARTOFPARAGRAPH
    long lastCmd = (this->imPtr)->GetLastCmd();
    class text *d = Text(this);

    if (lastCmd != lcNewLine
	 && (pos == 0
	     || (d)->GetChar( pos - 1) == '\n')) {
	class environment *pe;

	te = d->rootEnvironment;
	while ((pe = (class environment *)(te)->GetChild( pos)) != NULL && pe->type == environment_Style) {
	    te = pe;
	}
	return te;
    }
#endif /*IGNORE_STYLEFLAGS_AT_STARTOFPARAGRAPH*/
    te = (this)->GetEnclosedStyleInformation( pos, NULL);
    while (te != NULL && te->type != environment_Style) {
	te = (class environment *) (te)->GetParent();
    }
    return te;
}

class environment *textview::GetInsertEnvironment(long  pos)
{
    class environment *te;
    long lastCmd = (this->imPtr)->GetLastCmd();
    
    if (lastCmd != lcInsertEnvironment || (this->insertEnvMark)->GetModified()) {
	te = (this)->GetEnclosingEnvironment( pos);
	(this)->ClearInsertStack();
	(this->insertEnvMark)->SetPos( pos);
	(this->insertEnvMark)->SetModified( FALSE);
    }
    else {
	te = this->insertEnvironment;
    }

    return te;
}

void textview::ClearInsertStack()
{
    while (this->insertStack != NULL) {
	struct InsertStack *p;

	p = this->insertStack;
	this->insertStack = p->next;

	free(p->name);
	free(p);
    }
}

void textview::PopInsertStack()
{
    if (this->insertStack != NULL) {
	struct InsertStack *p;

	p = this->insertStack;
	this->insertStack = p->next;

	free(p->name);
	free(p);
    }
}

void textview::AddStyleToInsertStack(char  *styleName)
{
    struct InsertStack *p;

    p = (struct InsertStack *) malloc(sizeof(struct InsertStack));
    if (p != NULL) {
	p->name = strdup(styleName);
	if (p->name == NULL) {
	    free(p);
	    return;
	}
	p->next = this->insertStack;
	this->insertStack = p;
    }
}

void textview::PlainInsertEnvironment()
{
    class text *d = Text(this);
    long pos;
    
    pos = (this)->GetDotPosition();

    /* this call also resets insertStack if we have not previously been dealing with styles */

    this->insertEnvironment = (this)->GetInsertEnvironment( pos);

    if (this->insertStack != NULL) {
	(this)->ClearInsertStack();
    }

    this->insertEnvironment = d->rootEnvironment;

}


void textview::UpInsertEnvironment()
{
    class text *d = Text(this);
    long pos;
    
    pos = (this)->GetDotPosition();

    /* this call also resets insertStack if we have not previously been dealing with styles */

    this->insertEnvironment = (this)->GetInsertEnvironment( pos);

    if (this->insertStack != NULL) {
	(this)->PopInsertStack();
    }
    else {
	if (this->insertEnvironment != NULL && this->insertEnvironment !=d->rootEnvironment) {
	    this->insertEnvironment = (class environment *) (this->insertEnvironment)->GetParent();
	}
    }
}

void textview::DownInsertEnvironment()
{
    class environment *pe;
    class environment *te;
    class environment *ce;
    long pos;

    pos = (this)->GetDotPosition();

    this->insertEnvironment = (this)->GetInsertEnvironment( pos);

    if (this->insertStack == NULL && this->insertEnvironment != NULL) {
	pe = (this)->GetEnclosedStyleInformation( pos, NULL);
	ce = (class environment *)(this->insertEnvironment)->GetCommonParent( pe);
	if (ce == this->insertEnvironment && pe != this->insertEnvironment) {
	    do {
		te = pe;
		pe = (class environment *) (te)->GetParent();
	    } while (pe != this->insertEnvironment);

	    this->insertEnvironment = te;
	}
	else {
	    if (pos > 0
		&& (te = (class environment *)(this->insertEnvironment)->GetChild( pos - 1)) != NULL
		&& (te)->Eval() + (te)->GetLength() == pos) {
		this->insertEnvironment = te;
	    }
	    else if ((te = (class environment *)(this->insertEnvironment)->GetChild( pos)) != NULL) {
		this->insertEnvironment = te;
	    }
	}
    }
}

void textview::LeftInsertEnvironment()
{
    class environment *te;
    class environment *pe;
    long ePos;
    class text *d = Text(this);
    long pos;
    
    pos = (this)->GetDotPosition();

    te = (this)->GetInsertEnvironment( pos);

    if (te != NULL && this->insertStack == NULL) {
	if ((ePos = (te)->Eval()) == pos) {
	    te = (class environment *) (te)->GetParent();
	}
	else if (ePos + (te)->GetLength() == pos) {
	    if (pos != 0 && (pe = (class environment *)(te)->GetChild( pos - 1)) != NULL && pe->type == environment_Style) {
		te = pe;
	    }
	}
	else {
	    if (pos != 0) {
		pe = (class environment *)(te)->GetChild( pos - 1);
		if (pe != NULL && pe != d->rootEnvironment && (pe)->Eval() + (pe)->GetLength() == pos) {
		    te = pe;
		}
	    }
	}
    }

    this->insertEnvironment = te;
}

void textview::RightInsertEnvironment()
{
    class environment *te;
    class environment *pe;
    long ePos;
    long pos;
    
    pos = (this)->GetDotPosition();

    te = (this)->GetInsertEnvironment( pos);

    if (te != NULL && this->insertStack == NULL) {
	if ((ePos = (te)->Eval()) == pos) {
	    pe = (class environment *)(te)->GetChild( pos);
	    if (pe != NULL) {
		te = pe;
	    }
	}
	else if (ePos + (te)->GetLength() == pos) {
	    te = (class environment *) (te)->GetParent();
	}
	else {
	    if (pos != 0) {
		pe = (class environment *)(te)->GetChild( pos);
		if (pe != NULL && pe->type == environment_Style) {
		    te = pe;
		}
	    }
	}
    }

    this->insertEnvironment = te;
}

long textview_LookCmd(ATK *selfa, long c)
{
    textview *self=(textview *)selfa;
    class text *d;
    class stylesheet *ss;
    class style *styleptr;
    const char *name;	

    if (ConfirmReadOnly(self))
        return 0;

    d = Text(self);
    ss = (d)->GetStyleSheet();

    if (ss != NULL) {
	styleptr = ss->Find((char *)c);
	if ((name = (styleptr)->GetName()) != NULL) {
	    textview_InsertEnvironment(self, name);
	    return 0;
	}
    }

    message::DisplayString(self, 0, "Sorry; can't add style.");
    return 0;
}

/* This is no longer a command you can type;
 * it is now called from a menu item */

void textview::LookCmd(int look)
{
    textview_LookCmd(this, look);
}

void textview_PlainerCmd(class textview  *self, char  *type)
{
    class text *d;
    class environment *env;
    int pos, len;

    if (ConfirmReadOnly(self))
        return;

    d = Text(self);
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (((self)->GetIM())->GetLastCmd() == lcDisplayEnvironment && (env = self->displayEnvironment) != NULL && env != d->rootEnvironment)  {
	pos = (env)->Eval();
	len = env->length;
	(env)->Delete();
	(d)->SetModified();
	((self)->GetIM())->SetLastCmd( lcDisplayEnvironment);
	self->displayEnvironment = NULL;
	message::DisplayString(self, 0, "");
    }
    else if (len == 0) {
	if (strcmp(type, "old") == 0) {
	    env = (class environment *)(d->rootEnvironment)->GetInnerMost( pos);
	    if (env == NULL  || env == d->rootEnvironment) return;
	    pos = (env)->Eval();
	    len = env->length;
	    (env)->Delete();
	    (d)->SetModified();
	}
	else {
	    (self)->UpInsertEnvironment();
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	    DoDisplayInsertEnvironment(self);
	    return;
	}
    }
    else {
	if ((d->rootEnvironment)->Remove( pos, len, environment_Style, FALSE))
	    (d)->SetModified();
    }
    (d)->RegionModified( pos, len);
    (d)->NotifyObservers( observable::OBJECTCHANGED);
}

void textview_PlainestCmd(class textview  *self)
{
    class text *d;
    int pos, len;

    if (ConfirmReadOnly(self))
        return;

    d = Text(self);
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len == 0) {
	(self)->PlainInsertEnvironment();
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
	return;
    }
    else if ((d->rootEnvironment)->Remove(
       pos, len, environment_Style, TRUE))
	(d)->SetModified();
    (d)->RegionModified( pos, len);
    (d)->NotifyObservers( observable::OBJECTCHANGED);
}

/* Inserts a lookz view in front of the current paragraph. */

void textview_ExposeStyleEditor(class textview  *self)
{
    int pos;
    class text *d;

    if(textview_objecttest(self, "lookzview", "view") == FALSE)
        return;
    if(textview_objecttest(self, "lookz", "dataobject") == FALSE)
        return;
    if((Text(self))->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }

    d = Text(self);
    pos = (d)->GetBeginningOfLine( (self)->GetDotPosition() + (self)->GetDotLength());

    /* If we can insert it between two carriage returns (or at the beginning of the
         * document) do so, otherwise insert it at the start of this paragraph. */
    if (pos > 0 && (d)->GetChar( pos - 1) != '\n')
	pos += 1;

    (self)->PrepareInsertion( FALSE);
    self->currentViewreference =
          (Text(self))->InsertObject( pos, "lookz", "lookzview");
    (self)->FinishInsertion();
    (self)->FrameDot( pos);
    (d)->NotifyObservers( observable::OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

#define SHOWSIZE 250

void textview_ShowStylesCmd(class textview  *self)
{
    char tbuf[SHOWSIZE];
    const char *tp;
    class environment *te;
    class environment *de = NULL;
    class text *d;
    int curLen;
    boolean flag= FALSE;
    long pos;

    if (((self)->GetIM())->GetLastCmd() == lcDisplayEnvironment) {
	de = self->displayEnvironment;
	pos = self->displayEnvironmentPosition;
    }
    else {
	(self)->ReleaseStyleInformation( self->displayEnvironment);
	self->displayEnvironment = NULL;
	pos = (self)->CollapseDot();
    }

    strcpy(tbuf, "Styles: ");
    curLen= strlen(tbuf) +5;	/* account for the prompt, the highlight **s, and the \0 char */
    d = Text(self);
    te = (self)->GetEnclosedStyleInformation( pos, NULL);
    if (te) {
	if (te == d->rootEnvironment) {
	    strcat(tbuf, "None");
	}
	else {
	    while (te != d->rootEnvironment) {
		tp = te->data.style->name;
		
		if(tp == NULL) tp = "(Unnamed Style)";
		if (flag)
		    strcat(tbuf, " <- ");
		else
		    flag= TRUE;
		if (de == NULL)
		    de = te;
		else if (de == te)
		    de = NULL;

		curLen += strlen(tp) +4;	/* account for the style name and the following " <- " */
		if (curLen >= SHOWSIZE) break;
		if (de == te)
		    strcat(tbuf, "**");
		strcat(tbuf,tp);
		if (de == te)  {
		    strcat(tbuf, "**");
		    (self)->SetDotPosition( (te)->Eval());
		    (self)->SetDotLength( (te)->GetLength());
		}
		te = (class environment *) te->parent;
	    }
	}
	if (de == NULL)  {
	    (self)->SetDotPosition( pos);
	    (self)->SetDotLength( 0);
	}
	message::DisplayString(self, 0, tbuf);
	((self)->GetIM())->SetLastCmd( lcDisplayEnvironment);
	self->displayEnvironment = de;
	self->displayEnvironmentPosition = pos;
    }
    else message::DisplayString(self, 0, "Error");
    (self)->WantUpdate( self);
}


void textview_ChangeTemplate(class textview  *self)
{
    char tname[150];
    class text *d;
    int gf;

    if (ConfirmReadOnly(self))
        return;

    d = Text(self);
    gf = message::AskForString(self, 0,
      "Add styles from template [default]: ", "", tname, 100);
    if (gf < 0)
        return;
    if (tname[0] == '\0')  {   /* no name specified -- use default */
	strcpy(tname, "default");
    }
    if ((d)->ReadTemplate( tname, (d)->GetLength() == 0) < 0)
        message::DisplayString(self, 100, "Could not read template file.");
    else {
	(d)->RegionModified( 0, (d)->GetLength());
	(d)->NotifyObservers( observable::OBJECTCHANGED);
        message::DisplayString(self, 0, "Done.");
    }
}

#define BADCURPOS -1

void textview_ToggleExposeStyles(class textview  *self)
{
    self->exposeStyles = ! self->exposeStyles;
    self->force = TRUE;
    self->csxPos = BADCURPOS;		/* Indication that cursor is not visible */
    self->cexPos = BADCURPOS;
    (self)->WantUpdate( self);
}

void textview_ToggleColorStyles(class textview  *self)
{
    self->showColorStyles = ! self->showColorStyles;
    message::DisplayString(self, 0, (char *)(self->showColorStyles ?
			   "Color styles will be displayed." :
			   "Color styles will be ignored."));
    self->force = TRUE;
    (self)->WantUpdate( self);
}

static void DoDisplayInsertEnvironment(class textview  *self)
{
    class text *d = Text(self);
    const char *cp;
    char *p = NULL;
    char outstr[1000];

    if (self->insertEnvironment == d->rootEnvironment && self->insertStack == NULL) {
	cp = "Current Insertion Style: default";
    }
    else {
	long count = 0;
	long len;
	struct InsertStack *st;
	class environment *env;
	const char *name;

	p = &outstr[999];
	*p = '\0';


	for (st = self->insertStack; st != NULL; st = st->next) {
	    name = st->name;
	    if (name == NULL) {
		name = "unknown";
	    }
	    p -= 6;
	    strncpy(p, " (new)", 6);
	    len = strlen(name);
	    p -= len;
	    strncpy(p, name, len);
	    p -= 2;
	    strncpy(p, ", ", 2);
	    count++;
	}
	for (env = self->insertEnvironment; env != d->rootEnvironment; env = (class environment *) (env)->GetParent()) {

	    if (env->type != environment_Style) {
		name = "inset";
	    }
	    else if ((name = env->data.style->name) == NULL) {
		name = "unknown";
	    }

	    len = strlen(name);
	    p -= len;
	    strncpy(p, name, len);
	    p -= 2;
	    strncpy(p, ", ", 2);
	    count++;
	}
	*p = ':';
	if (count > 1) {
	    *--p = 's';
	}
	p -= 23;
	strncpy(p, "Current Insertion Style", 23);
	cp = p;
    }
    message::DisplayString(self, 0, cp);

#if 0
    if (self->insertStack != NULL) {
	char outstr[1000];
	char *name;

	name = self->insertStack->name;
	if (name == NULL) {
	    name = "unknown";
	}

	sprintf(outstr, "Current Insertion Style: %s (new)", name);
	message::DisplayString(self, 0, outstr);
    }
    else if (self->insertEnvironment == NULL) {
	message::DisplayString(self, 0, "Current Insertion Style: none");
    }
    else if (self->insertEnvironment == d->rootEnvironment) {
	message::DisplayString(self, 0, "Current Insertion Style: default");
    }
    else {
	char outstr[1000];
	char *name;

	if (self->insertEnvironment->type != environment_Style) {
	    name = "inset - need to fix this";
	}
	else if ((name = self->insertEnvironment->data.style->name) == NULL) {
	    name = "unknown";
	}
	    
	sprintf(outstr, "Current Insertion Style: %s", name);
	message::DisplayString(self, 0, outstr);
    }
#endif
}

void textview_DisplayInsertEnvironment(class textview  *self)
{
    long pos;

    if (ConfirmReadOnly(self))
        return;

    pos = (self)->CollapseDot();

    self->insertEnvironment = (self)->GetInsertEnvironment( pos);

    DoDisplayInsertEnvironment(self);
	
    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
}


void textview_PlainInsertEnvCmd(class textview  *self)
{
    if (ConfirmReadOnly(self))
        return;

    (self)->CollapseDot();

    (self)->PlainInsertEnvironment();

    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

    DoDisplayInsertEnvironment(self);
}

void textview_UpInsertEnvironmentCmd(class textview  *self)
{
    if (ConfirmReadOnly(self))
        return;

    (self)->CollapseDot();

    (self)->UpInsertEnvironment();

    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

    DoDisplayInsertEnvironment(self);
}

void textview_DownInsertEnvironmentCmd(class textview  *self)
{
    if (ConfirmReadOnly(self))
        return;

    (self)->CollapseDot();

    (self)->DownInsertEnvironment();

    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

    DoDisplayInsertEnvironment(self);
}

void textview_LeftInsertEnvironmentCmd(class textview  *self)
{
    if (ConfirmReadOnly(self))
        return;

    (self)->CollapseDot();

    (self)->LeftInsertEnvironment();

    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

    DoDisplayInsertEnvironment(self);
}

void textview_RightInsertEnvCmd(class textview  *self)
{
    if (ConfirmReadOnly(self))
        return;

    (self)->CollapseDot();

    (self)->RightInsertEnvironment();
    
    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

    DoDisplayInsertEnvironment(self);
}

static boolean StyleCompletionWork(class style  *style, struct result  *data)
{
    completion::CompletionWork((style)->GetName(), data);
    return FALSE;
}

static enum message_CompletionCode StyleComplete(char  *partial, long ss, char  *resultStr, int  resultSize)
{
    class stylesheet  *styleSheet=(class stylesheet *)ss;
    struct result result;
    char textBuffer[100];

    *textBuffer = '\0';
    result.partial = partial;
    result.partialLen = strlen(partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textBuffer;
    result.max = sizeof(textBuffer) - 1; /* Leave extra room for a NUL. */

    (styleSheet)->EnumerateStyles( (stylesheet_efptr) StyleCompletionWork, (long) &result);

    strncpy(resultStr, result.best, resultSize);
    if (result.bestLen == resultSize) /* Now make sure buffer ends in a NUL. */
        resultStr[result.bestLen] = '\0';

    return result.code;
}

struct helpData {
    char *partial;
    message_workfptr textFunction;
    long textRock;
};

static boolean StyleHelpWork(class style  *style, struct helpData  *helpData)
{
    char infoBuffer[1024];
    char strippedMenuName[1000];
    const char *p;
    const char *q;
    char *r;

    if (completion::FindCommon(helpData->partial,
       (style)->GetName()) == (int)strlen(helpData->partial)) {
        const char *menuName;
	
	strippedMenuName[0] = '\0';
	r = strippedMenuName;
	if ((menuName = (style)->GetMenuName()) != NULL) {
	    q = menuName;
	    while ((p = strchr(q, '~')) != NULL) {
		while (q != p) {
		    *r++ = *q++;
		}
		while (*++p != ',' && *p != '\0') {
		}
		q = p;
	    }
	    *r = '\0';
	}

        sprintf(infoBuffer, "%-16s %s", (style)->GetName(), strippedMenuName);

        (*helpData->textFunction)(helpData->textRock,
            message_HelpListItem, infoBuffer, NULL);
    }
    return FALSE; /* Keep on enumerating. */
}

static void StyleHelp(char  *partial, long ss, message_workfptr helpTextFunction, long  helpTextRock)
{
    class stylesheet  *styleSheet=(class stylesheet *)ss;
    struct helpData helpData;

    helpData.partial = partial;
    helpData.textFunction = helpTextFunction;
    helpData.textRock = helpTextRock;

    (styleSheet)->EnumerateStyles( (stylesheet_efptr)StyleHelpWork, (long) &helpData);
}

void textview_InsertEnvironment(class textview  *self, const char  *sName)
{
    class text *d = Text(self);
    class stylesheet *ss = (d)->GetStyleSheet();
    char styleName[100];
    class style *style;
    long pos = (self)->GetDotPosition();
    long len = (self)->GetDotLength();
    long lastCmd;

    if (ConfirmReadOnly(self))
        return;

    lastCmd = ((self)->GetIM())->GetLastCmd();
    self->insertEnvironment = (self)->GetInsertEnvironment( pos + len);

    if (sName == NULL || sName[0] == '\0') {
	if (message::AskForStringCompleted(self, 0, "Insert style: ",
					  0, styleName,
					   sizeof(styleName), 0,  StyleComplete,
					    StyleHelp, (long) ss,
					  message_MustMatch |
					  message_NoInitialString) == -1) {
	    return;
	}
    }
    else {
	strcpy(styleName, sName);
    }

    style = (ss)->Find( styleName);

    if (style != NULL) {

	if (lastCmd != lcInsertEnvironment && len != 0) {
	    /* We want to wrap the environment here and now */
	    if ((d)->AddStyle( pos, len, style) == NULL) {
		message::DisplayString(self, 0,
				      "Sorry; can't add style.");
	    }
	    else {
		char message[200];

		sprintf(message, "Added the style %s", styleName);
		message::DisplayString(self, 0, message);
		(d)->NotifyObservers( observable::OBJECTCHANGED);
	    }
	}
	else {
	    (self)->AddStyleToInsertStack( styleName);
	    ((self)->GetIM())->SetLastCmd( lcInsertEnvironment);

	    DoDisplayInsertEnvironment(self);
	}
    }
    else {
	char message[200];

	sprintf(message, "Could not find the style %s", styleName);
	message::DisplayString(self, 0, message);
    }
}
void InitializeMod()
{
    flipEndMax = 4;
    flipEndEnvs = (class environment **) malloc(sizeof(class environment *) * flipEndMax);

    flipBegMax = 4;
    flipBegEnvs = (class environment **) malloc(sizeof(class environment *) * flipBegMax);

    useOldInsertionRules = environ::GetProfileSwitch("UseOldInsertionRules", FALSE);

}


