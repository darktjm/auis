/* File perltextview.C created by W R Nelson
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   perltextview, a view for editing perl code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <im.H>
#include <environment.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "srctext.H"
#include "perltext.H"
#include "perltextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *perl_Map;
static menulist *perl_Menus;

ATKdefineRegistry(perltextview, srctextview, perltextview::InitializeClass);
void styleStringDelim(perltextview *self, char key);
static void startLineComment(perltextview *self, char key, long before, long after);

static struct bind_Description perltextBindings[]={
    {"perltextview-string-delimiter","!",'!', NULL,0,0, (proctable_fptr)styleStringDelim, "Begins and ends strings with special characters for delimiters"},
    {"perltextview-string-delimiter","#",'#'},
    {"perltextview-string-delimiter",":",':'},
    {"perltextview-string-delimiter",">",'>'},
    {"perltextview-string-delimiter","<",'<'},
    {"perltextview-string-delimiter","$",'$'},
    {"perltextview-string-delimiter","%",'%'},
    {"perltextview-string-delimiter","&",'&'},
    {"perltextview-string-delimiter","*",'*'},
    {"perltextview-string-delimiter","+",'+'},
    {"perltextview-string-delimiter","-",'-'},
    {"perltextview-string-delimiter","/",'/'},
    {"perltextview-string-delimiter","=",'='},
    {"perltextview-string-delimiter","?",'?'},
    {"perltextview-string-delimiter","@",'@'},
    {"perltextview-string-delimiter","\\",'\\'},
    {"perltextview-string-delimiter","^",'^'},
    {"perltextview-string-delimiter","_",'_'},
    {"perltextview-string-delimiter","|",'|'},
    {"perltextview-string-delimiter","~",'~'},
    {"perltextview-string-delimiter","(",'('},
    {"perltextview-string-delimiter",",",','},
    {"perltextview-string-delimiter",".",'.'},
    {"perltextview-string-delimiter",";",';'},
    {"perltextview-string-delimiter","[",'['},
    {"perltextview-string-delimiter","{",'{'},
    {"perltextview-string-delimiter","\"",'"'},
    {"perltextview-string-delimiter","'",'\''},
    {"perltextview-string-delimiter","`",'`'},
    {"perltextview-string-delimiter",")",')'},
    {"perltextview-string-delimiter","]",']'},
    {"perltextview-string-delimiter","}",'}'},
    NULL
};

boolean perltextview::InitializeClass()
{
    perl_Menus = new menulist;
    perl_Map = new keymap;
    bind::BindList(perltextBindings, perl_Map, perl_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

perltextview::perltextview()
{
    ATKinit;
    this->perl_state = keystate::Create(this, perl_Map);
    this->perl_menus = (perl_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

perltextview::~perltextview()
{
    ATKinit;
    delete this->perl_state;
    delete this->perl_menus;
}

/* gets the next three characters.  This is used along with the proc beginningOfLongString to tell whether the string is of the form:  <<"blah"\nstuff stuff\nmorestuff\nblah. */
static void getNext3Chars(perltext *ct, long pos, char buff[])
{    
    long len = (ct)->GetLength();
    if ((pos+2)> len)
    {
	buff[0] = 0;
	return;
    }
    buff[0] = (ct)->GetChar(pos);
    buff[1] = (ct)->GetChar(pos+1);
    buff[2] = (ct)->GetChar(pos+2);
    buff[3] = 0;
}

/* this proc looks at the first three chars of buff to see if they are "<<\"", "<<'", or "<<`" */
static boolean beginningOfLongString(char buff[])
{
    return (buff[0] == '<' && buff[1] == '<' &&
	    (buff[2] == '"' || buff[2] == '\'' || buff[2] == '`'));
}

/* copies the string starting at pos into buff.	 First it takes the first char as a delimiter and copies until it sees that delimiter again.  This is only called from HandleEndOfLineStyle after it knows that it is in a string of the form: <<"blah" . . . */
static long copyString(perltext *self, long pos, char buff[])
{
    int delim, i=0;
    long len = (self)->GetLength();

    delim = (self)->GetChar(pos);
    while (pos < len)
	if ((buff[i++] = (self)->GetChar(++pos)) == delim)
	    break;
    buff[i-1] = 0;
    return pos;
}

void perltextview::HandleEndOfLineStyle(long pos)
{
    perltext *ct = (perltext *)this->view::dataobject;
    char buff[256];

    /* terminate any bang-comments that might be there */
    (this)->srctextview::HandleEndOfLineStyle(pos);
    /* terminate strings of the form: <<"blah"\nthis is the string\nblah\n */
    if ((ct)->GetStyle(pos)==ct->srctext::string_style) {
	long start=pos, delim_start, string_start;
	while ((ct)->GetChar(start-1)=='\n' && !(ct)->Quoted(start-1)) --start;
	delim_start= (ct)->GetBeginningOfLine(start);
	string_start= ((ct)->GetEnvironment(pos))->Eval();
	if (string_start >= delim_start)
	    return;
	getNext3Chars(ct, string_start, buff);
	if (beginningOfLongString(buff)) {
	    copyString(ct,string_start+2,buff);
	    if (!(ct)->Strncmp(delim_start, buff, strlen(buff)))
		(ct->text::rootEnvironment)->Remove(start,pos-start+1, environment_Style, FALSE);
	}
    }
}

static long backwardSkipWhitespace(perltext *ct, long pos)
{
    int c;
    while ((pos > 0) && (is_whitespace(c=(ct)->GetChar(pos)) || c == '\n')) --pos;
    return pos;
}

/* begins or ends a string that is delimited by a '"', '\'', or '`' provided the string starts with one of those delimiters. */
static void styleString(perltextview *self, char key, long pos)
{
    perltext *ct=(perltext *)self->view::dataobject;

    if ((ct)->Quoted(pos))
	return;
    if ((ct)->InString(pos)) {
	/* we're in a string now, let's see if it needs terminating */
	long start=(ct)->BackwardSkipString(pos,key)+1, newpos;
	environment *existingstyle= (ct)->GetEnvironment(pos);

	newpos = (existingstyle)->Eval();
	if ((start<=0) || (key != (ct)->GetChar(newpos))) 
	    return;
	if (existingstyle == ct->text::rootEnvironment)
	    /* need to wrap whole string */
	    (ct)->WrapStyleNow(start,pos-start+1, ct->srctext::string_style, FALSE,FALSE);
	else if (newpos == start)
	    /* only need to terminate existing style */
	    (existingstyle)->SetStyle(FALSE,FALSE);
    } else if (!(ct)->GetStyle(pos) && (ct)->GetChar(pos-1) != '$')	
	/* we're not already IN a string; let's START one */
	(ct)->WrapStyleNow(pos,(self)->GetDotPosition()-pos, ct->srctext::string_style, FALSE,TRUE);
}

static void reindentLine(perltextview *self, char key)
{
    perltext *ct=(perltext *)self->view::dataobject;
    long pos;
    pos= (self)->GetDotPosition();
    if ((ct)->IndentingEnabled() && !(ct)->InCommentStart(pos)) 
	(ct)->ReindentLine(pos);
    (ct)->NotifyObservers(0);
}

static void reindent(perltextview *self, char key /* must be char for "&" to work. */) 
{
    perltext *ct=(perltext *)self->view::dataobject;

    if (key == '}')
	reindentLine(self,key);
    if (!(ct)->InCommentStart((self)->GetDotPosition()))
	(self)->MatchParens(key);
}

/* puts the style on a label. */
static void styleLabel(perltextview *self, char key, long pos)
{
    perltext *ct=(perltext *)self->view::dataobject;
    if (pos && !(ct)->InCommentStart(pos) && !(ct)->InString(pos)) {
	mark *colonpos=(ct)->CreateMark(pos,0);
	reindentLine(self,key); /* do this first, so the inserted char won't be part of the label style */
	(ct)->BackwardCheckLabel((colonpos)->GetPos());
	(ct)->RemoveMark(colonpos);
	delete colonpos;
    }
}


static void check_sub(perltextview *self, long pos)
{
    perltext *ct=(perltext *)self->view::dataobject;
    char buff[256], c;
    long start, end;
    Dict *word;

    end = backwardSkipWhitespace(ct, pos -1);
    start = (ct)->BackwardCopyWord(end, 0, buff)+1;
    pos = backwardSkipWhitespace(ct, start-1);
    pos = (ct)->BackwardCopyWord(pos, 0, buff);
    c = (ct)->GetChar(pos);
    if (strchr(VARIABLE_START, c) && c != 0) return;
    word= srctext::Lookup(ct->srctext::words,buff);
    if (word->val & SUB_BIT) {
	(ct)->WrapStyleNow(start, end-start+1, ct->srctext::function_style, FALSE, FALSE);
    }
}


void styleStringDelim(perltextview *self, char key)
{
    perltext *ct=(perltext *)self->view::dataobject;
    long pos, newpos, startpos, stringpos=0, beforeInsert, afterInsert, len; 
    char buff[256], c=0;
    int delim_count = 1;
    boolean endstyle = FALSE, string = FALSE;
    Dict *word;

    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */

    beforeInsert= pos= (self)->CollapseDot();
    (self)->SelfInsert(key);
    len = (ct)->GetLength();
    afterInsert= (self)->GetDotPosition();
    --pos;

    /* looking for weird strings  */
    if ((ct)->InString(beforeInsert)) {
	stringpos = ((ct)->GetEnvironment(beforeInsert))->Eval();   /* get begining of string */
	pos = (ct)->CopyWord(stringpos, len, buff);			   /* look @ word there */

	/* if key is not the delim then return */
	if (key != (ct)->GetChar(++pos))
	    return;

	word= srctext::Lookup(ct->srctext::words,buff);
	if (word->val & TWO_DELIM_BIT) {
	    while (++pos < len && delim_count < 2)
		if (key == (ct)->GetChar(pos) && !(ct)->Quoted(pos))
		    ++delim_count;
	    if (delim_count == 2)
		endstyle = TRUE;
	    string = TRUE;
	} else if (word->val & THREE_DELIM_BIT) {
	    while (++pos < len && delim_count < 3)
		if (key == (ct)->GetChar(pos) && !(ct)->Quoted(pos))
		    ++delim_count;
	    if (delim_count == 3)
		endstyle = TRUE;
	    string = TRUE;
	}
    } else if (!(ct)->GetStyle(pos)) {
	startpos = (ct)->BackwardCopyWord(pos,0,buff);

	if (startpos >= 0) c = (ct)->GetChar(startpos);   /* otherwise,  c=0 */
	if (!strchr(VARIABLE_START, c) || c==0) {
	    word= srctext::Lookup(ct->srctext::words,buff);
	    if (word->val & (TWO_DELIM_BIT | THREE_DELIM_BIT)) {
		(ct)->WrapStyleNow(startpos+1, pos-startpos+1, ct->srctext::string_style, FALSE, TRUE);
		string = TRUE;
	    }
	}
    }
    if (endstyle) {
	/* only need to terminate existing style */
	environment *existingstyle= (ct)->GetEnvironment(pos);
	(existingstyle)->SetStyle(FALSE,FALSE);
	return;
    }

    /* didn't find a special string (eg. tr/blah/halb/) so do what you used to do */
    if (!string)
	switch (key)
	{
	    case '/':
		newpos = backwardSkipWhitespace(ct, pos -1);
		c = (ct)->GetChar(newpos);
		switch (c)
		{
		    case '=' : 
		    case '(' :
		    case ',' :
		    case ';' :
		    case '}' :
		    case '{' :
		    case '~' :
		    case '&' :
		    case '|' :
			styleString(self, key, beforeInsert);
			break;
		    default:
			(ct)->BackwardCopyWord(newpos, 0, buff);
			word=srctext::Lookup(ct->srctext::words, buff);
			if (word->stng && (word->val & (STR_FOLLOW_KEYWD | COMPARE_STR_BIT))) {
			    /*if (word->stng && (word->val & COMPARE_STR_BIT) ||  (!word->val && word->stng))  */
			    styleString(self, key, beforeInsert);
			}
		}
		break;
	    case '"':
	    case '\'':
	    case '`':
		if (beforeInsert-1 >= 0)
		    styleString(self, key, beforeInsert);
		break;
	    case '}':
	    case ']':
	    case ')':
		if (!stringpos)
		    reindent(self, key);
		break;
	    case '{':
		check_sub(self, beforeInsert);
		break;
	    case ':':
		styleLabel(self,key,beforeInsert);
		break;
	    case '#':
		startLineComment(self,key,beforeInsert, afterInsert);
		break;
	    default:
		break;
	}
}

void perltextview::PostMenus(menulist *menulist)
{
    (this->perl_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->perl_menus);
}

class keystate *perltextview::PrependKeyState()
{
    this->perl_state->next= NULL;
    return (this->perl_state)->AddBefore((this)->srctextview::PrependKeyState());
}

static void startLineComment(perltextview *self, char key, long before, long after)
{
    perltext *ct=(perltext *)self->view::dataobject;
    if (!(ct)->GetStyle(before)) {
	if ('$'!= (ct)->GetChar(before-1))
	    (ct)->WrapStyleNow(before,after-before, ct->srctext::linecomment_style, FALSE,TRUE);
	else
	    if (before+1 < after)
		(ct)->WrapStyleNow(before+1,after-before, ct->srctext::linecomment_style, FALSE,TRUE);
    }
    (self)->SetDotPosition(after);
    (self)->FrameDot(after);
    (ct)->NotifyObservers(0);
}

void perltextview::Paren(char key)
{
    perltext *ct = (perltext *)this->view::dataobject;
    long pos=GetDotPosition();
    if (key=='}')
	SelfInsertReindent(key);
    else
	SelfInsert(key);
    if (!(ct)->InCommentStart(pos) && !(ct)->InString(pos))
	MatchParens(key);
}
