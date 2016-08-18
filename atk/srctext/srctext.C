/* File srctext.c created by R L Quinn
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


   srctext, an object for editing source code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/srctext.C,v 2.5 1995/02/24 00:11:42 rr2b Stab74 $";

#include <ctype.h>

#include <environment.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <attribs.h>
#include <environ.H>
#include <im.H> /* for CheckLineLengths' dialog box */
#include <message.H> /* for CheckLineLengths' dialog box */
#include <buffer.H> /* for CheckLineLengths' checkpoint detector */

#include <dataobject.H> /* for SrcInsets */
#include <viewref.H> /* for SrcInsets */
#include <sys/errno.h> /* for SrcInsets */
#include <filetype.H> /* for SrcInsets */
#include <dictionary.H> /* for SrcInsets */

#include "dogtags.h"
#include "compress.H"
#include "srctext.H"

#define TEXT_VIEWREFCHAR '\377'

/* these characters look like << and >> (in most fonts), and are used to delimit SrcInsets in the saved document */
#define SRCINSET_STARTCHAR '\253'
#define SRCINSET_ENDCHAR '\273'

static boolean PrettyPrint=FALSE, PPMANGLING=FALSE, BLANKCOL1=FALSE, DOBARS=FALSE; /*RSKprettyprint*/
static char PrettyOptions[10]; /*RSKprettyprint*/

ATKdefineRegistry(srctext, text, NULL);

/* GetEnvironment returns the environment enclosing pos, or the root environment if no style is there */
environment *srctext::GetEnvironment(long pos)
{
    environment *me=NULL, *root=this->text::rootEnvironment;
    if (root)
	me= (environment *)(root)->GetEnclosing(pos);
    return me;
}

/* nextInset returns the position of the first inset in the text after pos.  It searches a maximum of len chars after pos, and returns -1 if none were found */
static long nextInset(srctext *self, long pos, long len)
{
    long VRCpos;
    TryNextVRC: ; /* jump back here if the ViewRefChar wasn't REALLY an inset */
    VRCpos= (self)->Index(pos, TEXT_VIEWREFCHAR, len);
    if (VRCpos>=0)
	/* make sure it's not just a plain ol' \377 char */
	if (((environment *)(self->text::rootEnvironment)->GetInnerMost(VRCpos))->type == environment_View)
	    return VRCpos;
	else {
	    pos= VRCpos+1;
	    goto TryNextVRC;
	}
    return -1;
}

/* GetStyle returns the style enclosing pos, or NULL if no style is there */
style *srctext::GetStyle(long pos)
{
    environment *me=GetEnvironment(pos);
    return (me!=this->text::rootEnvironment) ? me->data.style : NULL;
}

/* srctext_WrapStyle calls environment_InsertStyle. It makes sure it doesn't wrap styles around insets, since we can't nest environments. This method should be called by RedoStyles, but interactive things like BackwardCheckWord should call WrapStyleNow instead. */
void srctext::WrapStyle(long pos, long len, style *style, boolean begflag, boolean endflag)
{
    environment *newenv;
    if (style!=NULL) {
	long nextinset=nextInset(this, pos,len);
	long distToNextEnv=(this->text::rootEnvironment)->GetNextChange(pos);
	/* only call environment_Remove if we suspect it's absolutely necessary, because it is S L O W */
	if (distToNextEnv<len || GetStyle(pos+1))
	    (this->text::rootEnvironment)->Remove(pos,len, environment_Style, TRUE);
	newenv=(this->text::rootEnvironment)->InsertStyle(pos, style, TRUE);
	if (nextinset < 0) { /* no insets in the way */
	    (newenv)->SetLength(len);
	    (newenv)->SetStyle(begflag,endflag);
	} else { /* dang, we can't nest environments so we have to jump past that inset */
	    (newenv)->SetLength(nextinset-pos);
	    (newenv)->SetStyle(begflag,TRUE);
	    len-= nextinset-pos+1;
	    pos= nextinset+1;
	    WrapStyle(pos,len, style, TRUE,endflag); /* recurse to style the rest */
	}
    }
}

/* srctext_WrapStyleNow() wraps a style, and ALSO flags the region as modified. This should be called for all *interactive* styling that results from user interaction in the view, such as BackwardCheckWord */
void srctext::WrapStyleNow(long posn, long len, style *style, boolean begflag, boolean endflag)
{
    if (style!=NULL) {
	WrapStyle(posn,len, style, begflag,endflag);
	RegionModified(posn,len);
    }
}

/* The actual work is done in a static function, so InitializeObject can call it.  The *method* is a dummy that calls this. */
static void SetupStyles(srctext *self)
{
    stylesheet *ss=(self)->GetStyleSheet();
    self->kindStyle[UPRCSE]= NULL;
    if ((self->comment_style= (ss)->Find("comment")) == NULL) {
	self->comment_style= new style;
	(self->comment_style)->SetName("comment");
	(self->comment_style)->AddNewFontFace(fontdesc_Italic);
	(ss)->Add(self->comment_style);
    }
    if ((self->linecomment_style= (ss)->Find("linecomment")) == NULL) {
	self->linecomment_style= new style;
	(self->comment_style)->Copy(self->linecomment_style);
	(self->linecomment_style)->SetName("linecomment");
	(ss)->Add(self->linecomment_style);
    }
    if ((self->function_style= (ss)->Find("function")) == NULL) {
	self->function_style= new style;
	(self->function_style)->SetName("function");
	(self->function_style)->AddNewFontFace(fontdesc_Bold);
	(self->function_style)->SetFontSize(style_PreviousFontSize, 2);
	(ss)->Add(self->function_style);
    }
    if ((self->label_style= (ss)->Find("label")) == NULL) {
	self->label_style= new style;
	(self->label_style)->SetName("label");
	(self->label_style)->AddNewFontFace(fontdesc_Bold | fontdesc_Italic);
	(ss)->Add(self->label_style);
    }
    if ((self->string_style= (ss)->Find("string")) == NULL) {
	self->string_style= new style;
	(self->string_style)->SetName("string");
	/*style_AddFlag(self->string_style, style_Underline); removed by popular demand -RSK*/
	(ss)->Add(self->string_style);
    }
    if ((self->kindStyle[USRDEF]= (ss)->Find("userdef")) == NULL) {
	self->kindStyle[USRDEF]= new style;
	(self->kindStyle[USRDEF])->SetName("userdef");
	(self->kindStyle[USRDEF])->AddNewFontFace(fontdesc_Bold);
	(ss)->Add(self->kindStyle[USRDEF]);
    }
    if ((ss)->Find("global") == NULL) {
	style *global=new style;
	(global)->SetName("global");
	(global)->SetFontFamily("andytype");
	(global)->AddNewFontFace(fontdesc_Fixed);
	(global)->AddFlag(style_ContinueIndent);
	(global)->AddFlag(style_TabsCharacters);
	(global)->SetNewLeftMargin(style_LeftEdge, 16, style_RawDots);
	(global)->SetNewIndentation(style_LeftMargin, -16, style_RawDots);
	(global)->SetJustification(style_LeftJustified);
	(ss)->Add(global);
    }
    (self)->SetGlobalStyle((ss)->Find("global"));
}

/* override this if the language needs additional styles */
void srctext::SetupStyles()
{
    ::SetupStyles(this);
}

/* redundant code, copied from eza.c, to determine checkpointing status */
static long CkpInterval; /* How often to run Checkpoint routine. */
#define DEFAULTCKPINTERVAL 30 /* Default for CkpInterval. */
long CkpLatency; /* The minimum amount of time to wait to checkpoint a buffer. */
#define DEFAULTCKPLATENCY 4 /* Default for CkpLatency. */
/**/

srctext::srctext()
{
    SetCopyAsText(TRUE); /* otherwise regions get pasted as INSETS */
    SetExportEnvironments(TRUE); /* keep styles intact on Copy/Cut */
    SetForceUpper(FALSE);
    this->useTabs=TRUE;
    this->reindentComments= environ::GetProfileSwitch("ReindentComments", TRUE);
    this->indentingEnabled= FALSE;
    this->words= NULL;
    this->copyright_key= NULL;
    this->OverstrikeMode= FALSE; /*RSK91overstrike*/

    /* Initialize Variables used to control the indenting style. */
    this->contIndent=2;
    this->commentIndent= 1;
    this->levelIndent=4;
    this->tabSize= 8;
    this->numTabStops= 0;
    this->maxlinelength=0;
    if ((CkpInterval = environ::GetProfileInt("CheckpointInterval", DEFAULTCKPINTERVAL)) != 0)
	CkpLatency = environ::GetProfileInt("CheckpointMinimum", DEFAULTCKPLATENCY * CkpInterval) / CkpInterval;

    /* Initialize variables for Esc-1 and Esc-2 comments */
    this->commentString= NULL;
    this->commentCol= 0;
    this->commentFixed= FALSE;
    this->linecommentString= NULL;
    this->linecommentCol= 0;
    this->linecommentFixed= FALSE;
    this->remarkPadding= 1;

    /* Initialize callback functions to NULL. */
    this->preWriteFctn= this->postWriteFctn= NULL;

    ::SetupStyles(this);
    { /*RSKprettyprint*/
	const char *envar=environ::Get("EZPRETTYPRINT");
	if (envar && strlen(envar)>0) {
	    PrettyPrint= TRUE;
	    strncpy(PrettyOptions, envar, 10);
	} else
	    PrettyOptions[0]= '\0';
    }
    THROWONFAILURE(TRUE);
}

srctext::~srctext()
{
    if (this->copyright_key) free(this->copyright_key);
}

void srctext::HashInsert(Dict *hashTable[], Dict *word)
{
    Dict *bucket;
    int hashval;
    if (word!=NULL) {
	hashval= HASH(word->stng);
	bucket= (Dict *)malloc(sizeof(Dict));
	bucket->stng= (char *)malloc(strlen(word->stng)+1);
	strcpy(bucket->stng,word->stng);
	bucket->val= word->val;
	bucket->kind= word->kind;
	bucket->next= hashTable[hashval];
	hashTable[hashval]= bucket;
    }
}

static void PutPrefStringIntoHashTable(Dict *hashTable[], const char *st, int kind)
{
    static char prefword[255];
    static Dict prefdict[]= {{prefword,0,0}};
    char *stcp=prefword;
    if (!st) return;
    prefdict->kind= kind;
    do  {
	if (is_whitespace(*st) || *st==',' || *st=='\0') {
	    if (stcp>prefword) { /* we've got a word to insert */
		*stcp= '\0';
		stcp= prefword;
		if (kind==UPRCSE) makeupper(prefword);
		srctext::HashInsert(hashTable, prefdict);
	    }
	} else
	    *stcp++= *st;
    } while (*st++);
}

void srctext::BuildTable(char *classname, Dict *hashTable[], Dict wordlist[])
{
    int i;
    char profilename[256];
    const char *preflist;
    Dict *wordPtr;

    /* clear out the hash table */
    for (i=0; i<TABLESIZE; ++i)
	hashTable[i]= NULL;

    /* put the language's keyword set (if it HAS one) into hash table */
    wordPtr= wordlist;
    while (wordPtr!=NULL && wordPtr->stng!=NULL) {
	srctext::HashInsert(hashTable, wordPtr);
	++wordPtr;
    }
    /* add user-defined keywords (from "preferences" file) to hash table */
    sprintf(profilename,"%s_userdef\0", classname);
    preflist= environ::GetProfile(profilename);
    if (preflist && preflist[0])
	PutPrefStringIntoHashTable(hashTable, preflist, USRDEF);
    /* add uppercase-words (from "preferences" file) to hash table */
    sprintf(profilename,"%s_uppercase\0", classname);
    preflist= environ::GetProfile(profilename);
    if (preflist && preflist[0])
	PutPrefStringIntoHashTable(hashTable, preflist, UPRCSE);
}

/* srctext_Lookup does a case-sensitive hash table lookup */
Dict *srctext::Lookup(Dict *hashTable[], char *word) 
{
    Dict *bucket;
    static Dict miss={NULL,0,0};
    if (hashTable!=NULL && word!=NULL) {
	bucket= hashTable[HASH(word)];
	while (bucket!=NULL && (strcmp(bucket->stng,word))!=0)
	    bucket= bucket->next;
	if (bucket!=NULL) 
	    return bucket;
    }
    return &miss;
}

/* override */
void srctext::Clear()
{
    (this)->text::Clear(); /* This destroyes all styles in the stylesht. */
    SetupStyles();
}

/* addSortedTabStop adds a number to the appropriate location in the array of tab stops */
static void addSortedTabStop(srctext *self, int tabstop)
{
    int t=0;
    if (self->numTabStops >= MAX_TABSTOPS) return; /* ignore if too many */
    /* otherwise find the correct position in the array */
    while (t < self->numTabStops && tabstop > self->tabStop[t]) ++t;
    /* move larger tabs out of the way if need be */
    if (t < self->numTabStops) {
	int tm=self->numTabStops;
	while (tm>t) {
	    self->tabStop[tm]= self->tabStop[tm-1];
	    --tm;
	}
    }
    /* add the tab stop to the array and increment the tab stop count */
    self->tabStop[t]= tabstop;
    ++self->numTabStops;
}

/* setTabStops parses string of tab stops given in ezinit */
static void setTabStops(srctext *self, char *st)
{
    int tabstop=0;
    self->numTabStops= 0;
    if (!st) return; /* done if no tab stops specified */
    /* otherwise parse string and add tabstops to array */
    while (*st!='\0') {
	sscanf(st,"%d",&tabstop);
	addSortedTabStop(self,tabstop);
	while (isdigit(*st)) ++st; /* skip over already-read tab stop number */
	while (*st!='\0' && !isdigit(*st)) ++st; /* skip blanks, commas, etc, to next tab stop number */
    }
}

/* override */
void srctext::SetAttributes(struct attributes *atts)
{
    (this)->text::SetAttributes(atts);
    while (atts!=NULL) {
	char *key=atts->key;
	if (strcmp(key,"force-upper")==0)
	    SetForceUpper(atoi(atts->value.string));
	else if (strcmp(key,"use-tabs")==0)
	    this->useTabs=atoi(atts->value.string);
	else if (strcmp(key,"reindent-comments")==0)
	    this->reindentComments=atoi(atts->value.string);
	else if (strcmp(key,"comment-indent")==0)
	    this->commentIndent=atoi(atts->value.string);
	else if (strcmp(key,"cont-indent")==0)
	    this->contIndent=atoi(atts->value.string);
	else if (strcmp(key,"level-indent")==0)
	    this->levelIndent=atoi(atts->value.string);
	else if (strcmp(key,"comment-col")==0) {
	    this->commentCol=atoi(atts->value.string);
	    if (this->commentCol<0) {
		this->commentCol= -this->commentCol -1;
		this->commentFixed= TRUE;
	    } else
		--this->commentCol; /* convert to left-margin=0 */
	} else if (strcmp(key,"linecomment-col")==0) {
	    this->linecommentCol=atoi(atts->value.string);
	    if (this->linecommentCol<0) {
		this->linecommentCol= -this->linecommentCol -1;
		this->linecommentFixed= TRUE;
	    } else
		--this->linecommentCol; /* convert to left-margin=0 */
	} else if (strcmp(key,"remark-padding")==0)
	    this->remarkPadding=atoi(atts->value.string);
	else if (strcmp(key,"enable-indentation")==0)
	    this->indentingEnabled=atoi(atts->value.string);
	else if (strcmp(key,"tab-size")==0)
	    this->tabSize=atoi(atts->value.string);
	else if (strcmp(key,"tab-stops")==0)
	    setTabStops(this,atts->value.string);
	else if (strcmp(key,"max-length")==0)
	    SetMaxLineLength(atoi(atts->value.string));
	else if (strcmp(key,"overstrike")==0) /*RSK91overstrike*/
	    ChangeOverstrikeMode(atoi(atts->value.string));
	else if (strcmp(key,"write-as-text")==0)
	    SetWriteAsText(atoi(atts->value.string));
	else if (strcmp(key,"copyright-key")==0) {
	    int strln= strlen(atts->value.string);
	    if (this->copyright_key) free(this->copyright_key);
	    if (strln>0) {
		this->copyright_key= (char *)malloc(strln+1);
		strcpy(this->copyright_key, atts->value.string);
	    } else
		this->copyright_key= NULL;
	}
	atts=atts->next;
    }
}

/* CopyrightScramble does some trivial alphabet-shifting to make code unreadable, if "copyright-key" is set and doesn't match.  This same function also serves as a descrambler.  This function is a classprocedure so that generic text objects can use it too (via "srctextview-copyright-scramble" proc) */
void srctext::CopyrightScramble(text *txt, long pos, long len)
{
    char newst[2];
    newst[1]= '\0';
    while (len--) {
	int ch= txt->GetChar(pos);
	if (ch>='a' && ch<='z')
	    newst[0]= 'Z' - (ch-'a');
	else if (ch>='A' && ch<='Z')
	    newst[0]= 'z' - (ch-'A');
	else
	    newst[0]= '\0';
	if (newst[0])
	    txt->AlwaysReplaceCharacters(pos,1, newst,1);
	++pos;
    }
}

/* base64[] contains the character forms of all base64 digits, in order. (pray that none of these characters are ever used for comment delimiters in any language!) */
static char base64[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";

/* encode64 munches up str so that it's base-64 encoded using the above table. Make sure the string passed here has at least 4/3 its actual length allocated! */
static void encode64(char *str)
{
    int len, globs, temp0=0, temp1=0, temp2=0;
    char *src, *dst;
    len= strlen(str);
    globs= len/3;
    src= str+globs*3;
    dst= str+globs*4;
    /* first, clean up the ragged end and terminate the string that's going to be filled in */
    if (*src) {
	temp0= (*src & 0xFC) >> 2;
	temp1= (*src & 0x03) << 4;
	if (*(src+1)) {
	    temp1+= (*(src+1) & 0xF0) >> 4;
	    temp2= (*(src+1) & 0x0F) << 2;
	}
	*dst= base64[temp0];
	*(dst+1)= base64[temp1];
	*(dst+2)= base64[temp2];
	*(dst+3)= '\0';
    } else
	*dst= '\0';
    /* now, traverse the string backward and expand each 3-char glob to 4 base64 chars */
    if (globs>0)
	do  {
	    src-= 3;
	    dst-= 4;
	    *(dst+3)= base64[src[2] & 0x3F];
	    *(dst+2)= base64[((src[2] & 0xC0) >> 6) + ((src[1] & 0x0F) << 2)];
	    *(dst+1)= base64[((src[1] & 0xF0) >> 4) + ((src[0] & 0x03) << 4)];
	    *dst= base64[(src[0] & 0xFC) >> 2];
	} while (src>str);
}

/* base64value returns the actual value of a base64 digit that's in character form */
static int base64value(char ch)
{
    char *digit=strchr(base64,ch);
    if (digit)
	return (digit-base64);
    else
	return (-1);
}

/* decode64 turns the encoded str into readable format. This will shrink str's length to 3/4 its original size */
static boolean decode64(char *str)
{
    char *result=(char *)malloc((strlen(str)*4)/3 +1);
    char *src=str, *dst=result;
    int tmp;
    while (*src) {
	tmp= base64value(*src++);
	if (tmp<0) { free(result); return FALSE; }
	*dst= tmp << 2;
	if (!*src) goto EndString;
	tmp= base64value(*src++);
	if (tmp<0) { free(result); return FALSE; }
	*dst+= (tmp & 0x30) >> 4;
	*++dst= (tmp & 0x0F) << 4;
	if (!*src) goto EndString;
	tmp= base64value(*src++);
	if (tmp<0) { free(result); return FALSE; }
	*dst+= (tmp & 0x3C) >> 2;
	*++dst= (tmp & 0x03) << 6;
	if (!*src) goto EndString;
	tmp= base64value(*src++);
	if (tmp<0) { free(result); return FALSE; }
	*dst+= tmp;
	EndString: ; /* jump here if source string ends abruptly */
	*++dst= '\0';
    }
    strcpy(str, result);
    free(result);
    return TRUE;
}

/* SrcInset Rule:  SrcInsets should NOT have any newlines in them.  If there ARE any newlines, it means the SrcInset was tampered with by the user, so we poke those newline characters into the file right after the inset.  The *exception* to this rule is when the "srctext.BreakInsetsWithNewlinesLineNumbersBeDamned" preference is set to TRUE.  In this case, we added those newlines OURSELF, and we don't want to poke them back into the file because they'll be written back out again as part of the SrcInset when we _Write the file.  Obviously, the buffer with the translated inset is going to have LESS newline characters than the original file with the broken SrcInset, which makes the compiler's line numbers NOT match the buffer's. Yech. */

/* DiscardToEnddata is swiped directly from atk/text/text.c. It's used by InsertSrcInsetFile */
/* Assuming a \begindata has been read, discards up to and including a matching \enddata (discards internal levels of \begindata ... \enddata).  Returns FALSE if EOF is reached before the \enddata.  Something better needs to be done about this. */
static boolean DiscardToEnddata(FILE *file)
{
    int c, i;
    char buf[20];
    trymore: ;
    do  {
	if ((c= getc(file)) == EOF)
	    return FALSE;
    } while (c != '\\');
    haveback: ;
    i = 0;
    while (1) {     /* Read possible keyword */
	if ((c= getc(file)) == EOF)
	    return FALSE;
	if (i == 0 && (c == '\\' || c == '}' || c == '}'))
	    goto trymore;   /* Just a quoted char */
	if (c == '{')       /* End of keyword */
	    break;
	if (i < sizeof (buf) - 1)
	    buf[i++] = c;
    }
    buf[i] = '\0';
    do  {
	if ((c= getc(file)) == EOF)
	    return FALSE;
    } while (c != '}');
    /* If it's a begindata, recurse to discard subobject */
    if (strcmp(buf, "begindata") == 0) {
	if (DiscardToEnddata(file) == FALSE)
	    return FALSE;
	goto trymore;
    }
    if (strcmp(buf, "enddata") != 0)
	goto trymore;
    return TRUE;
}

/* InsertAFile is the guts of text_AlwaysInsertFile, but it will *unconditionally* create an INSET if MUSTinset is TRUE. */
static long InsertAFile(srctext *self, FILE *file, long position, char *objectName, long objectID, boolean MUSTinset)
{
    int length = 0;
    if (MUSTinset || (objectName!=NULL && objectID!=0 && !ATK::IsTypeByName(objectName, "text")))  {
	/* Either this is a NON-text datastream (we have to make an inset), or we WANT it to be an inset regardless or its type */
	dataobject *dat;
	if((self)->GetObjectInsertionFlag() == FALSE){
	    /* ignore non-text object */
	    char bb[512];
	    long ll;
	    fprintf(stderr, "Insertion of objects not allowed, ignoring %s!\n",objectName);
	    DiscardToEnddata(file);
	    sprintf(bb,"[A %s OBJECT WAS INSERTED HERE]",objectName);
	    ll = strlen(bb);
	    (self)->AlwaysInsertCharacters(position, bb, ll);
	    length = ll;
	} else {
	    dat = (dataobject *) ATK::NewObject(objectName);
	    if (dat == NULL) {
		fprintf(stderr, "Srctext: Can't find routines for object '%s'; ignoring!\n", objectName);
		DiscardToEnddata(file);
		length = 0;
	    } else {
		(dat)->Read(file, (dat)->UniqueID());
		dictionary::Insert(NULL, (char *) objectID, (char *) (dat)->UniqueID());
		(self)->AlwaysAddView(position, (dat)->ViewName(),    dat);
		length = 1;
	    }
	}
    } else {
	boolean wasReadOnly;
	long oldfence = (self)->GetFence();
	wasReadOnly = (self)->GetReadOnly();
	/* ReadSubString checks read-only, making this ugliness necessary. */
	if(wasReadOnly){
	    (self)->SetReadOnly(FALSE);
	    length = (self)->ReadSubString(position, file, objectID > 0);
	    (self)->SetReadOnly(wasReadOnly);
	} else if(position < oldfence){
	    /* reset the fence properly */
	    (self)->SetFence(0);
	    length = (self)->ReadSubString(position, file, objectID > 0);
	    (self)->SetFence(0);
	    (self)->SetFence(oldfence + length);
	} else 
	    length = (self)->ReadSubString(position, file, objectID > 0);
    }
    return length;
}

/* InsertSrcInsetFile is a wrapper for InsertAFile, that looks up the object's info and tells InsertAFile we MUST have an inset. */
static long InsertSrcInsetFile(srctext *self, FILE *file, char *filename /* not used */, long position)
{
    char *objectName;
    long objectID;
    objectName= filetype::Lookup(file, filename, &objectID, NULL);
    return InsertAFile(self, file, position, objectName, objectID, TRUE /* we absolutely positively MUST have an inset */);
}

/* override */
/* srctext_AlwaysInsertFile should be less cautious than text's, and only make the file be an inset if it MUST.  This gets called by Paste, too. */
long srctext::AlwaysInsertFile(FILE *file, char *filename, long position)
{
    char *objectName;
    long objectID;
    int myfile = 0;
    long length = 0;
    if (file == NULL) {
	if (filename != NULL && ((file= fopen(filename,"r")) != NULL))
	    ++myfile;
	else
	    return 0;
    }
    objectName = filetype::Lookup(file, filename, &objectID, NULL);
    length= InsertAFile(this, file, position, objectName, objectID, FALSE /* we probably DON'T want an inset */);
    dogtags_substituteregion(this, &position,&length);

    if (myfile)
	fclose(file);
    return length;
}

/* srctext_TranslateSrcInset turns a SrcInset into a REAL inset, and returns the length of what got inserted minus the length of the SrcInset that got replaced */
long srctext::TranslateSrcInset(long pos)
{
    long startinset=pos++, len=GetLength();
    int c, remembernewlines=0;
    char linebuffer[81], filename[MAXPATHLEN+1];
    FILE *decodedfile;
    long difflength=0;

    /* open decoded file */
    char *tfn=tmpnam(filename); /* randomize the filename */
    if(tfn==NULL) {
	fprintf(stderr, "srctext: Couldn't get a temporary filename.\n");
    }
    decodedfile= fopen(filename, "w+");
    do  {
	long chunklen;
	/* skip over chunk separator (spaces), and count doomed newlines */
	while (pos<len && (is_whitespace(c=GetChar(pos)) || c=='\n')) {
	    ++pos;
	    if (c=='\n') ++remembernewlines;
	}
	for (chunklen=0; chunklen<80 && !is_whitespace(c=GetChar(pos)) && c!='\n' && c!=EOF; ++pos)
	    linebuffer[chunklen++]= c;
	if (chunklen<1) break;
	linebuffer[chunklen]= '\0';
	if (linebuffer[0]!=SRCINSET_ENDCHAR) {
	    decode64(linebuffer);
	    fputs(linebuffer, decodedfile);
	}
    } while (pos<len && linebuffer[0]!=SRCINSET_ENDCHAR);
    if (linebuffer[0]!=SRCINSET_ENDCHAR) {
	/* bogus inset, not terminated */
	unlink(filename);
	fclose(decodedfile);
	return 0;
    }
    ++pos; /* skip over extra space in source code */

    /* insert newlines to compensate for the ones we're about to destroy */
    if (!environ::GetProfileSwitch("srctext.BreakInsetsWithNewlinesLineNumbersBeDamned", FALSE)) {
	difflength= remembernewlines;
	while (remembernewlines-- > 0)
	    AlwaysInsertCharacters(pos, "\n", 1);
    }

    /* insert decoded file into source */
    rewind(decodedfile);
    difflength+= InsertSrcInsetFile(this, decodedfile, filename, pos);
    unlink(filename); /* delete temporary decoded file */
    fclose(decodedfile);

    /* remove SrcInset data */
    difflength-= pos-startinset;
    AlwaysDeleteCharacters(startinset, pos-startinset);

    return difflength;
}

/* srctext_FindSrcInsets is called after a _Read is complete, but BEFORE the _RedoStyles happens. It hunts down SrcInset delimiters and calls _TranslateSrcInset to turn them into REAL insets. */
void srctext::FindSrcInsets()
{
    long pos=0, len=GetLength();
    while (pos<len) {
	pos= Index(pos, SRCINSET_STARTCHAR, len-pos);
	if (pos==EOF)
	    pos= len;
	else {
	    long difflen;
	    difflen= TranslateSrcInset(pos);
	    if (!difflen) ++pos; /* just in case the inset was bogus */
	    len+= difflen;
	}
    }
}

/* override */
long srctext::Read(FILE *file, long id)
{
    long tmpRetValue;
    tmpRetValue= (this)->text::Read(file, id);
    if (tmpRetValue == dataobject_NOREADERROR) {
	if (id==0) /* it better NOT be a datastream! */
	    FindSrcInsets();
	SetupStyles(); /* text_Read blows the stylesheet away, so we have to either find (or CREATE) all the styles AGAIN */
	dogtags_substitute(this);
	if (environ::GetProfileSwitch("InitialRedoStyles", TRUE))
	    RedoStyles();
	if (PrettyPrint && id==0 /*NOT a template*/) { /*RSKprettyprint*/
	    mark *wholefile=CreateMark(0, GetLength());
	    long fakeunmodified=GetModified();
	    char troffcommands[256];
	    troffcommands[0]='\0';
	    SetReadOnly(FALSE);
	    this->useTabs= FALSE; /* line numbers will mess up tabs */
	    if ((this->indentingEnabled || strchr(PrettyOptions,'i')) && !strchr(PrettyOptions,'I')) {
		if (strchr(PrettyOptions,'C'))
		    this->reindentComments= FALSE;
		if (strchr(PrettyOptions,'c'))
		    this->reindentComments= TRUE;
		/* reindent entire file */
		fprintf(stderr,"srctext: reindenting...\n");
		PPMANGLING= FALSE;
		Indent(wholefile);
	    }
	    if (strchr(PrettyOptions,'S'))
		RemoveStyles();
	    if (strchr(PrettyOptions,'b') && !strchr(PrettyOptions,'B')) {
		/* add do-bars without messing up actual indentation */
		DOBARS= TRUE;
		this->indentingEnabled= FALSE;
		fprintf(stderr,"srctext: adding do-bars...\n");
		PPMANGLING= TRUE;
		BLANKCOL1= (Indentation(0)==1); /* true for PL/x, which should have dobars starting in column 2 */
		Indent(wholefile);
		PPMANGLING= FALSE;
	    }
	    RemoveMark(wholefile);
	    delete wholefile;
	    if (strchr(PrettyOptions,'n') && !strchr(PrettyOptions,'N')) {
		/* add line numbers to the actual dataobject; troff doesn't know "paragraph" numbers */
		char linenum[10];
		long pos=0, lnumber=1;
		fprintf(stderr,"srctext: adding line numbers...\n");
		do  {
		    sprintf(linenum, "%5ld ", lnumber++);
		    JustInsertCharacters(pos, linenum,6);
		    pos= GetEndOfLine(pos) +1;
		} while (pos<GetLength());
		/* jam the left margin over to make more room for numbers */
		/*strcat(troffcommands, ".nr IN -1i\n"); / * doesn't work */
	    }
	    if (strchr(PrettyOptions,'W') && !strchr(PrettyOptions,'w'))
		/* fool troff into not line-wrapping */
		strcat(troffcommands, ".ll 36i\n"); /* pretend paper is 3 feet wide */
	    if (troffcommands[0]!='\0') {
		style *fmtnote;
		stylesheet *ss= GetStyleSheet();
		fmtnote= (ss)->Find("formatnote");
		if (!fmtnote) {
		    /* create a **formatnote** style ourself */
		    fmtnote= new style;
		    (fmtnote)->SetName("formatnote");
		    (fmtnote)->AddPassThru();
		    (fmtnote)->AddHidden();
		    (fmtnote)->AddNoFill();
		    (ss)->Add(fmtnote);
		}
		JustInsertCharacters(0, troffcommands,strlen(troffcommands));
		WrapStyleNow(0,strlen(troffcommands), fmtnote, FALSE,FALSE);
	    }
	    RestoreModified(fakeunmodified);
	    /* make the thing readonly!  Can't do it here, because bufferlist_GetBufferOnFile has its own local flag, and doesn't use it until AFTER the dataobject_Read is done.  Cripes. */
	}
    }
    return tmpRetValue;
}

/* override */
long srctext::ReadSubString(long pos, FILE *file, boolean quoteCharacters)
{
    long retval, slen;
    retval= this->text::ReadSubString(pos, file,quoteCharacters);
    if (this->copyright_key && Strncmp(pos, this->copyright_key, slen=strlen(this->copyright_key))==0) {
	DeleteCharacters(pos, slen);
	retval-= slen;
	srctext::CopyrightScramble((text *)this, pos, retval); /* descramble the code */
    }
    return retval;
}

/* override */
long srctext::ReadTemplate(char *templateName, boolean inserttemplatetext)
{
    long retval;
    retval= (this)->text::ReadTemplate(templateName,inserttemplatetext);
    if (retval >= 0)
	SetupStyles();
    return retval;
}

/* LinesTooLong goes through the entire file to see if there are any lines exceeding maxlen. Returns TRUE if there are. */
boolean srctext::LinesTooLong(int maxlen)
{
    long pos=0, len=GetLength();
    int c, linelen=0;
    while (pos<len) {
	c= GetChar(pos++);
	if (c=='\n') {
	    if (linelen>maxlen)
		return TRUE;
	    linelen= 0;
	} else if (c=='\t')
	    linelen= (linelen+8)&~7;
	else
	    ++linelen;
    }
    return FALSE;
}

/* CheckLineLengths displays a message in view if any lines are found that exceed maxlen. (if view is NULL, a guess is made about where to display the message) */
void srctext::CheckLineLengths(int maxlen, view *view)
{
    if (maxlen<1)
	/* nothing to check-- forget it */
	return;
    if (LinesTooLong(maxlen)) {
	char msg[512], *filename, *pathname;
	if (!view)
	    /* guess which window requested the length check */
	    view= (class view *)(im::GetLastUsed());
	pathname= (buffer::FindBufferByData(this))->GetFilename();
	filename= strrchr(pathname,'/');
	if (!filename)
	    filename= pathname; /* just in case pathname has no slashes */
	else ++filename; /* skip over that slash */
	im::SetProcessCursor(NULL); /* get rid of confusing wait cursor */
	sprintf(msg,"Warning: saving `%s' with lines exceeding %ld characters.", filename, GetMaxLineLength());
	message::DisplayString(view, 65 /*probably a dialog box*/, msg);
    }
}

/* simplewrite() is stolen from smpltext.c */
static boolean simplewrite(FILE *file, char *p, long len)
{
    /* Write out the literal contents of the character buffer.  write() is used because it is more efficient than fwrite.  if write() fails, attempt with fwrite (), if it succeeds then continue, if it fails, then when buffer calls ferror, it will notice that something has gone wrong */
    long wroteLen;
    while (len > 0)  {
	wroteLen = write(fileno(file), p, len);
	if (wroteLen == -1) {
	    if (errno == EINTR)  continue;
	    if((wroteLen = fwrite(p,1,len,file)) <= 0)	{
		fprintf(stderr, "Error while writing text object.\n");
		return FALSE;
	    }
	    fflush(file);
	}
	len -= wroteLen;
	p = &p[wroteLen];
    }
    return TRUE;
}

/* writesrc does the work for _Write, and can also be called for the stuff inside compress boxes */
static boolean writesrc(srctext *self, FILE *file, long writeID)
{
    boolean success=TRUE;
    long pos=0, len=(self)->GetLength();
    /* check line lengths if a max-length was set (unless checkpointing) */
    if ((self)->GetMaxLineLength()) {
	buffer *buff= buffer::FindBufferByData(self);
	if ((buff)->GetCkpClock()<=CkpLatency)
	    (self)->CheckLineLengths((self)->GetMaxLineLength(),NULL);
    }
    /* find ATK insets and turn them into SrcInsets, which have a format that won't make compilers explode (hopefully). */
    do  {
	long nextinset, actuallen;
	nextinset= nextInset(self, pos,len-pos);
	nextinset= (nextinset>=0) ? nextinset : len;
	/* flush out all the plain text up through 'nextinset' */
	do  {
	    char *plainbuffer;
	    plainbuffer= (self)->GetBuf(pos, nextinset-pos, &actuallen);
	    simplewrite(file, plainbuffer, actuallen);
	    pos+= actuallen;
	} while (pos<nextinset);
	pos= nextinset;
	if (nextinset<len) {
	    /* write out the inset, encode64 it, and shove it into the source file */
	    environment *env=(environment *)(self->text::rootEnvironment)->GetInnerMost(nextinset);
	    if (env->type == environment_View) { /* make extra sure it's a view */
		dataobject *inset= env->data.viewref->dataObject;
		success= success && (self)->OutputSrcInset(file,writeID, inset);
	    }
	    ++pos; /* skip over the \377 */
	}
    } while (pos<len && success);
    return success;
}

/* srctext_OutputSrcInset converts the datastream of the passed inset into base64-encoded chunks, and outputs those chunks to file. Retruns TRUE on success. */
boolean srctext::OutputSrcInset(FILE *file, long writeID, dataobject *inset)
{
    boolean success=TRUE;
    FILE *tempfile;
    char paddedbuffer[84], *linebuffer, filename[MAXPATHLEN+1];
    /* check for special cases (compress insets) */
    if (ATK::IsTypeByName((inset)->GetTypeName(), "compress")) {
	srctext *compresscontents= new srctext;
	(compresscontents)->AlwaysCopyText(0, (compress *)inset,0,((compress *)inset)->GetLength());
	success= writesrc(compresscontents,file, writeID);
	(compresscontents)->Destroy();
	return success;
    }
    /* set up padding to separate the encoded chunks */
    paddedbuffer[0]= ' ';
    if (GetMaxLineLength() && environ::GetProfileSwitch("srctext.BreakInsetsWithNewlinesLineNumbersBeDamned", FALSE))
	paddedbuffer[0]= '\n';
    linebuffer= paddedbuffer+1; /* don't mess with the separator char */

    /* open temp file and write inset out in normal format */
    char *tfn=tmpnam(filename);
    if(tfn==NULL) {
	fprintf(stderr, "srctext: Couldn't get a temporary filename.\n");
    }
    tempfile= fopen(filename, "w+");
    if (!tempfile) return FALSE;
    unlink(filename); /* this file is REALLY temporary */
    (inset)->Write(tempfile, writeID, 2 /* level 2, an inset */);
    /* read the temp file back in, encode64 it, and send result to source file */
    rewind(tempfile);
    sprintf(linebuffer, "%c", SRCINSET_STARTCHAR);
    success= success && simplewrite(file, linebuffer, 1);
    while (!feof(tempfile)) {
	int count=0;
	while (!feof(tempfile) && count<60)
	    linebuffer[count++]= fgetc(tempfile);
	linebuffer[count]= '\0';
	encode64(linebuffer);
	success= success && simplewrite(file, paddedbuffer, strlen(paddedbuffer));
    }
    sprintf(linebuffer, " %c ", SRCINSET_ENDCHAR);
    success= success && simplewrite(file, linebuffer, 3);
    fclose(tempfile);
    return success;
}

/* override */
long srctext::Write(FILE *file, long writeID, int level)
{
    long len=GetLength(), retval;

    /* terminate file with newline if not already (and it's not an inset) */
    if (level==0 && len>0 && GetChar(len-1) != '\n')
	InsertCharacters(len++, "\n", 1);

    if (this->preWriteFctn)
	(*(this->preWriteFctn))(this,file,writeID,level);

    /* see if we should format it or let the text object handle things */
    if (level==0 && GetWriteStyle()!=text_DataStream) {
	if (!writesrc(this, file, writeID))
	    fprintf(stderr, "srctext error: File could not be written properly. (Saving insets requires free space in /tmp.  If this file has insets, clean out /tmp and try again).\n");
	retval= this->dataobject::id;
    } else /* this thing is an inset (or else user set "datastream=yes"), just belch it out as a datastream */
	retval= (this)->text::Write(file, writeID, level);

    if (this->postWriteFctn)
	(*(this->postWriteFctn))(this,file,writeID,level);

    return retval;
}

/* override */
void srctext::WriteSubString(long pos, long len, FILE *file, boolean quoteCharacters)
{
    if (this->copyright_key) {
	text *tmptxt;
	if (quoteCharacters) {
	    char *key= this->copyright_key;
	    while (*key) {
		switch (*key) {
		    case '\n':
			fprintf(file, "\n\n"); break;
		    case '\\':
		    case '{': case '}':
			fprintf(file, "\\%c", *key); break;
		    default:
			fprintf(file, "%c", *key);
		}
		++key;
	    }
	} else
	    fprintf(file, "%s", this->copyright_key);
	fflush(file);
	tmptxt= new text;
	tmptxt->AlwaysCopyTextExactly(0, this,pos,len);
	srctext::CopyrightScramble(tmptxt, 0,len); /* scramble the code */
	tmptxt->WriteSubString(0,len, file,quoteCharacters);
	tmptxt->Destroy();
    } else
	this->text::WriteSubString(pos,len, file,quoteCharacters);
}

/* SetWriteCallbacks can be used to set up a "remove nasties" and "restore nasties" fuction, useful for things like EZ-integrated debuggers that insert special breakpoint characters and such into the actual dataobject, but DON'T want them written to the file. */
void srctext::SetWriteCallbacks(srctext_writefptr pre, srctext_writefptr post)
{
    this->preWriteFctn= pre;
    this->postWriteFctn= post;
}

/* override */
/* srctext_GetModified checks for modified insets in a sneaky way. The method inherited from text would be ridiculously slow because it checks ALL *environments*, and there are a LOT of them in a huge source file. Instead, this hunts down TEXT_VIEWREFCHAR's only, thus ignoring *style* environments. */
/* the base ATK version was fixed somewhere around version 5.1.something.  When that's been installed, this override can Go Away */
long srctext::GetModified()
{
    long pos=0, len=GetLength();
    long mod, maxmod=this->dataobject::modified;
    while ((pos=nextInset(this, pos,len-pos)) >= 0 && pos<len) {
	environment *env=(environment *)(this->text::rootEnvironment)->GetInnerMost(pos);
	if (env->type == environment_View) {
	    dataobject *inset= env->data.viewref->dataObject;
	    mod= (inset)->GetModified();
	    if (mod > maxmod)
		maxmod= mod;
	}
	++pos;
    }
    return maxmod;
}

/* static data cached by GetLineForPos and GetPosForLine */
static long prev_line=1, prev_mod=-1;
static srctext *prev_self=NULL;
static mark *prev_pos=NULL;

/* COUNT_LINES is a pseudofunction that takes a srctext object, position, and the name of a variable to modify.  It counts the number of lines represented at position p (1 if there's a newline there, ??? if it's a compress inset, otherwise lin is unchanged). MAKE SURE PARMS HAVE NO SIDE EFFECTS! */
#define COUNT_LINES(st,p,lin) \
  { \
    int ch=((st))->GetChar((p)); \
    if (ch=='\012') \
      ++lin; \
    else if (ch==TEXT_VIEWREFCHAR) { \
      environment *env=(environment *)((st)->text::rootEnvironment)->GetInnerMost((p)); \
      if (env->type == environment_View) { \
        dataobject *inset= env->data.viewref->dataObject; \
        if (ATK::IsTypeByName((inset)->GetTypeName(), "compress")) \
          lin+= ((compress *)inset)->GetLines()-1; /* -1 to compensate for \n after box being counted */ \
      } \
    } \
  }

/* override */
/* GetPosForLine "tricks" everyone who calls it by ALSO counting newlines inside compress objects */
long srctext::GetPosForLine(long line)
{
    register long base_line=1, pos=0;
    long len=GetLength(), mod=this->dataobject::modified;

    if (this==prev_self && mod==prev_mod && prev_pos && !(prev_pos)->ObjectFree()) {
	if (line>=prev_line) {
	    /* save a lot of counting by starting at the cache */
	    base_line= prev_line;
	    pos= (prev_pos)->GetPos();
	    if (line==prev_line)
		pos= GetBeginningOfLine(pos);
	} else if (line>(prev_line/2)) {
	    /* save a lot of counting by going backward from the cache */
	    base_line= line - prev_line;
	    pos= (prev_pos)->GetPos();
	    while (base_line <= 0) {
		if (pos<0) {
		    pos= -2;
		    break;
		}
		--pos;
		COUNT_LINES(this,pos,base_line);
	    }
	    ++pos;
	    base_line= line;
	    goto GotPosForLine; /* skip normal forward count */
	}
    }
    while (base_line < line) {
	if (pos >= len)
	    break;
	COUNT_LINES(this,pos,base_line);
	++pos;
    }
    GotPosForLine: ; /* jump here if we did a backward count, to skip the normal forward count */
    line= base_line; /* just in case the requested line was purely fictitious-- make sure we cache something real */
    if (this==prev_self) {
	/* we're interested in somewhere else now, so "cache" the new PosForLine */
	prev_line= line;
	prev_mod= mod;
	if (prev_pos==NULL)
	    prev_pos= CreateMark(pos,0);
	else
	    (prev_pos)->SetPos(pos);
    } else {
	/* we're interested in a whole different dataobject, so make a new "cache" */
	if (prev_pos) {
	    if (!(prev_pos)->ObjectFree())
		(prev_self)->RemoveMark(prev_pos);
	    delete prev_pos;
	}
	prev_pos= CreateMark(pos,0);
	prev_line= line;
	prev_mod= mod;
	prev_self= this;
    }
    return pos;
}

/* override */
/* GetLineForPos "tricks" everyone who calls it by ALSO counting newlines inside compress objects */
long srctext::GetLineForPos(long pos)
{
    register long base_pos=0, line=1;
    long mod=this->dataobject::modified;
    if (this==prev_self && mod==prev_mod && prev_pos && !(prev_pos)->ObjectFree()) {
	long prvpos=(prev_pos)->GetPos();
	if (pos>=prvpos) {
	    /* save a lot of counting by starting at the cache */
	    base_pos= prvpos;
	    line= prev_line;
	} else if (pos>(prvpos/2)) {
	    /* save a lot of counting by going backward from the cache */
	    register long neglines=0;
	    base_pos= prvpos;
	    while (--base_pos >= pos)
		COUNT_LINES(this,base_pos,neglines);
	    line= prev_line - neglines;
	    goto GotLineForPos; /* skip normal forward count */
	}
    }
    while (base_pos < pos) {
	COUNT_LINES(this,base_pos,line);
	++base_pos;
    }
    GotLineForPos: ; /* jump here if we did a backward count, to skip the normal forward count */
    if (this==prev_self) {
	/* we're interested in somewhere else now, so "cache" the new LineForPos */
	prev_line= line;
	prev_mod= mod;
	if (prev_pos==NULL)
	    prev_pos= CreateMark(pos,0);
	else
	    (prev_pos)->SetPos(pos);
    } else {
	/* we're interested in a whole different dataobject, so make a new "cache" */
	if (prev_pos) {
	    if (!(prev_pos)->ObjectFree())
		(prev_self)->RemoveMark(prev_pos);
	    delete prev_pos;
	}
	prev_pos= CreateMark(pos,0);
	prev_line= line;
	prev_mod= mod;
	prev_self= this;
    }
    return line;
}

/* override this if the language needs to check for procedure names and such */
void srctext::BackwardCheckWord(long from, long to)
{
    Dict *word;
    char buff[256];
    long j=from;
    if (IsTokenChar(GetChar(from)) && !GetStyle(from) && !InString(from)) {
	j= BackwardCopyWord(from,to,buff);
	Keywordify(buff,TRUE);
	if ((word= srctext::Lookup(this->words,buff))->stng != NULL) {
	    int l= strlen(buff);
	    if (GetForceUpper())
		ReplaceCharacters(j+1,l, word->stng,l);
	    WrapStyleNow(j+1,l, this->kindStyle[word->kind], FALSE,FALSE);
	}
	else if (GetForceUpper()) {
	    makeupper(buff); /* just in case Keywordify didn't do this (ctext) */
	    if ((word= srctext::Lookup(this->words,buff))->stng != NULL) {
		int l= strlen(buff);
		if (word->kind==UPRCSE)
		    ReplaceCharacters(j+1,l, word->stng,l);
	    }
	}
    }
    return /*j*/;
}

/* CheckComment will wrap the comment style around slash-star comments. (it's not used HERE, but is inherited by C, PL/x, and some other languages, and overridden by others.)  'start' is the position of slash (not asterisk) that starts the comment. Returns position of ending slash. */
long srctext::CheckComment(long start)
{
    int prev, c=0;
    long end=start+1, len=GetLength();
    while (++end<len) {
	prev= c;
	c= GetChar(end);
	if (c=='/' && prev=='*')
	    break;
    }
    WrapStyle(start,end-start+1, this->comment_style, FALSE,FALSE);
    return end;
}

/* CheckLinecomment will wrap the linecomment style to the end of the line. (it's not used HERE, but is universally useful for any languages with bang comments.) */
long srctext::CheckLinecomment(long start)
{
    long end=GetEndOfLine(start);
    WrapStyle(start,end-start, this->linecomment_style, FALSE,TRUE);
    return end-1;
}

/* CheckString will wrap the string style around strings and character constants. No need to override this, since only RedoStyles will call it, and it already knows when it's appropriate or not. */
long srctext::CheckString(long start)
{
    long end=start, len=GetLength();
    int delim=GetChar(start);
    while (end<len)
	if (GetChar(++end)==delim && !Quoted(end))
	    break;
    WrapStyle(start,end-start+1, this->string_style, FALSE,FALSE);
    return end;
}

/* override this if the language needs to check for procedure names and such */
long srctext::CheckWord(long i, long end)
{
    long j;
    Dict *word;
    char buff[256];
    j= CopyWord(i,end,buff);
    Keywordify(buff,FALSE);
    if ((word= srctext::Lookup(this->words,buff))->stng != NULL)
	WrapStyle(i,strlen(word->stng), this->kindStyle[word->kind], FALSE,FALSE);
    return j;
}

/* BackwardCheckLabel styles a label if it's the first thing on a line. 'pos' is the position of the character after the label (usually a colon). This is called by both RedoStyles (data object), and StyleLabel (view object) */
void srctext::BackwardCheckLabel(long pos)
{
    char buff[256];
    long startlabel;
    while (is_whitespace(GetChar(--pos))) ;
    startlabel= BackwardCopyWord(pos,0,buff);
    Keywordify(buff,FALSE);
    if (startlabel<pos && srctext::Lookup(this->words, buff)->stng == NULL) {
	/* the word before the ':' is not a keyword */
	long startline=startlabel;
	while (is_whitespace(GetChar(startline))) --startline;
	if (startline<0 || GetChar(startline)=='\n')
	    /* and it's the first thing on line */
	    WrapStyleNow(startlabel+1,pos-startlabel, this->label_style, TRUE,TRUE);
    }
}

/* override this if there's a different way to identify comments in that language */
/* InCommentStart returns the starting position of the comment, if pos is inside one, otherwise 0 is returned */
long srctext::InCommentStart(long pos)
{
    style *sty= GetStyle(pos);
    if (sty==this->linecomment_style || sty==this->comment_style) {
	long start= (GetEnvironment(pos))->Eval();
	if (start<1)
	    start= 1; /* make sure it's "TRUE" even if start position is zero. */
	else if (GetChar(start-1) == TEXT_VIEWREFCHAR) {
	    /* oops, an inset. jump it and recurse to keep looking */
	    long continued=InCommentStart(start-1);
	    if (continued) return continued;
	}
	return start;
    } else
	return 0;
}

/* override this for any source views that have indentation schemes */
/* Indentation returns the proper indentation of the line that starts at pos */
int srctext::Indentation(long pos)
{
    /* leave it where it is; there's no "generic" indentation scheme */
    return CurrentIndent(pos);
}

/* override this if non-alphanumeric characters are tokens in that language */
boolean srctext::IsTokenChar(char ch)
{
    return isalnum(ch);
}

/* override this if the language is more or less picky about what's recognized */
/* This function "keywordifies" a string. "Keywordify" means "make the word findable in the hash table". */
char *srctext::Keywordify(char *buff, boolean checkforceupper)
{
    return buff;
}

/* srctext_RemoveStyles gets called by all RedoStyles methods to first remove the old styles */
void srctext::RemoveStyles()
{
    environment *root= this->text::rootEnvironment;
    /* JUST remove the STYLES, don't go blowing away embedded views! */
    (root)->Remove(0,GetLength(),environment_Style,TRUE);
    return;
}

/* override this if the language has comments or keywords */
void srctext::RedoStyles()
{
    /* first, remove the old styles */
    RemoveStyles();
    /* then, put the new styles in (if there WERE any-- srctext is non-language-specific) */
    return;
}

long srctext::SkipWhitespace(long pos, long end)
{
    while (pos<end && is_whitespace(GetChar(pos))) ++pos;
    return pos;
}

/* assumes that pos points to 1 BEFORE ending delimiter */
long srctext::BackwardSkipString(long pos, char delim)
{
    while (pos>=0 && (GetChar(pos)!=delim || Quoted(pos)))
	--pos;
    return pos-1;
}

boolean srctext::InString(long pos)
{
    int c, lastquote=0;
    long commentskip;
    boolean quotes=FALSE;
    --pos;
    while (pos>=0 && ((c=GetChar(pos))!='\n' || Quoted(pos))) {
	if ((c=='\'' || c=='\"') && !Quoted(pos)) {
	    if (c==lastquote) {
		quotes=FALSE;
		lastquote=0;
	    } else if (!quotes) {
		quotes=TRUE;
		lastquote=c;
	    }
	} else if ((commentskip=InCommentStart(pos)) > 0) {
	    /* skip over comments; quotes inside them are irrelevant */
	    while (pos>commentskip && GetChar(pos-1)!='\n')
		--pos;
	}
	--pos;
    }
    return quotes;
}

/* srctext_CopyWord copies a word from 'pos' to 'end' into 'buffer'. It returns the position of the last character in the word. */
long srctext::CopyWord(long pos, long end, char buf[])
{
    int ch, count=0;
    while (count<255 && pos<end && IsTokenChar(ch= GetChar(pos))) {
	buf[count++]=ch;
	++pos;
    }
    buf[count]=0;
    return pos-1;
}

long srctext::BackwardCopyWord(long from, long to, char buf[])
{
    long count=0,i=0,j;
    while (from-i >= to && IsTokenChar(GetChar(from-i))) ++i;
    j= from-i;
    if (from-i >= to-1)
	for (; i>0 && count<255; --i,++count)
	    buf[count]= GetChar(from-i+1);
    buf[count]= '\0';
    return j;
}

/* srctext_CurrentIndent returns the indentation of the current line */
int srctext::CurrentIndent(long pos)
{
    register int ind=0;

    pos= GetBeginningOfLine(pos);
    while (1)
	switch (GetChar(pos++)) {
	    case '\t':
		ind=(ind+8)&~7;
		break;
	    case ' ':
		++ind;
		break;
	    case '|'/*246*/: /*RSKprettyprint*/
		if (PrettyPrint) ++ind;
		else return ind;
		break;
	    default:
		return ind;
	}
}

/* Return the current "column" (chars from left margin), compensating for tab characters */
int srctext::CurrentColumn(long pos)
{
    int ind=0;
    long oldPos=pos; /* save the current position */

    /* backup to the beginning of the line */
    pos= GetBeginningOfLine(pos);

    /* count the number of columns to the current position */
    while (pos<oldPos)
	if (GetChar(pos++)=='\t')
	    ind= (ind+8)&~7;
	else
	    ++ind;
    return ind;
}

/* srctext_NextTabStop returns the column that the next Tab Stop (or multiple of tabSize, whichever comes first) from curcol. A zero is returned if we have to move to the next line */
int srctext::NextTabStop(int curcol)
{
    int nextMultiple=((int)(curcol/this->tabSize)+1) * this->tabSize;
    int tabnum=0;
    if (this->numTabStops < 1)
	/* there ARE no tab stops */
	if (this->tabSize>0)
	    return nextMultiple;
	else
	    /* tabSize was *disabled*, so just move 1 space */
	    return curcol+1;
    /* find next tab stop */
    while (tabnum < this->numTabStops && this->tabStop[tabnum] <= curcol)
	++tabnum;
    if (tabnum >= this->numTabStops)
	/* reached the end of the tab stops */
	if (this->tabSize > 0)
	    return nextMultiple;
	else
	    /* tabSize was *disabled*, so jump to next line */
	    return 0;
    /* return next multiple of tabSize, or next tab stop, whichever is closest */
    if (this->tabStop[tabnum] <= nextMultiple || this->tabSize < 1)
	return this->tabStop[tabnum];
    else
	return nextMultiple;
}

/* ExtendToOutdent modifies the pos and len parameters to appropriately point to the full indentation level that pos is inside of.  It returns TRUE if it thinks the region has a "significant" size (3 lines or more). */
boolean srctext::ExtendToOutdent(int indent, long *pos, long *len)
{
    register long start=*pos, end=*pos, temp=GetBeginningOfLine(start);
    long lines=0, txtlen=GetLength();
    /* find beginning */
    do  {
	start= temp;
	temp= GetBeginningOfLine(temp-1);
	++lines;
    } while (temp>0 && (CurrentIndent(temp)>=indent || GetChar(SkipWhitespace(temp,txtlen))=='\n'));
    *pos= SkipWhitespace(start, txtlen);
    /* find end */
    end= GetEndOfLine(end);
    while (end<txtlen && (CurrentIndent(end+1)>=indent || GetChar(SkipWhitespace(end+1,txtlen))=='\n')) {
	end= GetEndOfLine(end+1);
	++lines;
    }
    *len= end - *pos +1; /* compress will strip off the last newline, but grab it anyway in case the last line was totally blank */
    return (lines>2);
}

/* returns true iff the character at pos is quoted (ie. preceded by "\"). Override this is you want to take into account the backslash being quoted. (ie "\\", for ctext and idltext). */
boolean srctext::Quoted(long pos)
{
    return FALSE;
}

long srctext::ReverseBalance(long pos)
{
    int thischar;
    char *parentype;
    struct paren_node {
	long type;
	struct paren_node *next;
    } *parenstack=NULL, *temp;
    static char *opens="({[", *closes=")}]";

    while (pos>0) {
	thischar= GetChar(--pos);

	if ((parentype= strchr(opens,thischar)) != NULL) {
	    if (InCommentStart(pos) || InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    if (parenstack==NULL || parenstack->type != (parentype-opens))
		/* empty stack or wrong paren type. Abort! */
		break;
	    /* pop close-paren off stack */
	    temp= parenstack;
	    parenstack= parenstack->next;
	    free(temp);
	    if (parenstack==NULL)
		return pos;
	} else if ((parentype= strchr(closes, thischar)) != NULL) {
	    if (InCommentStart(pos) || InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    /* push close-paren onto stack */
	    temp= (struct paren_node *)malloc(sizeof(struct paren_node));
	    temp->type= parentype - closes;
	    temp->next= parenstack;
	    parenstack= temp;
	}
    }
    /* free stack if we found a bogus paren or made it to start of file */
    while (parenstack) {
	temp= parenstack;
	parenstack= parenstack->next;
	free(temp);
    }
    return EOF;
}

boolean srctext::DoMatch(long pos, char *str, int len)
{
    while(len>0 && GetChar(pos++)==*str++)
	--len;
    return len==0;
}

static char *spaces="        ";

/*RSK91overstrike: changed srctext_InsertCharacters calls to srctext_ JustInsertCharacters calls*/
#define TABOVER(self,oldpos,pos,oldcol,col) \
  if(col>oldcol){\
    (self)->DeleteCharacters(oldpos,pos-oldpos);\
    pos=oldpos;\
    if (PPMANGLING && oldcol==0) { int count=(col-oldcol)/self->levelIndent; while (count--) { if (BLANKCOL1) (self)->JustInsertCharacters(pos++, " ",1); (self)->JustInsertCharacters(pos++, (char *)(DOBARS?"|"/*246*/:" "),1); (self)->JustInsertCharacters(pos, spaces,self->levelIndent-1-BLANKCOL1); pos+= self->levelIndent-1-BLANKCOL1; oldcol+=self->levelIndent; } } else /*RSKprettyprint*/ \
    if(self->useTabs){\
      int nc;\
      while(col>=(nc=(oldcol+8)&~7)){\
        oldcol=nc;\
        (self)->JustInsertCharacters(pos++,"\t",1);\
      }\
    }else{\
      int count=(col-oldcol)/8;\
      while(count--){\
        (self)->JustInsertCharacters(pos,spaces,8);\
        oldcol+=8;\
        pos+=8;\
      }\
    }\
    (self)->JustInsertCharacters(pos,spaces,col-oldcol);\
    pos+=(col-oldcol);\
  }

/*RSK91overstrike: this is a duplicate of the original simpletext_InsertCharacters; it's used by the TABOVER macro to ignore overstrike mode*/
boolean srctext::JustInsertCharacters(long pos, const char *str, long len)
{
    if (pos >= GetFence()) {
	AlwaysInsertCharacters(pos, str, len);
	return TRUE;
    } else
	return FALSE;
}

long srctext::TabAndOptimizeWS(long pos, int target)
{
    long home=0, oldPos, col=0, oldCol=0;
    /* find start of line */
    while (pos>0 && GetChar(pos-1)!='\n') {
	++home;
	--pos;
    }
    oldPos= pos;
    /* optimize all preceding whitespace on line */
    while (home--) {
	switch (GetChar(pos)) {
	    case '\t':
		col= (col+8)&~7;
		break;
	    case ' ':
		++col;
		break;
	    default:
		if(col>oldCol+1)
		    TABOVER(this,oldPos,pos,oldCol,col);
		oldPos= pos+1;
		oldCol= ++col;
	}
	++pos;
    }
    /* skip over existing whitespace */
    while (col<target) {
	int nc;
	int c=GetChar(pos);
	if (c==' ') {
	    ++col;
	    ++pos;
	} else if (c=='\t' && (nc=(col+8)&~7)<=target) {
	    col=nc;
	    ++pos;
	} else
	    break;
    }
    /* add new whitespace */
    TABOVER(this,oldPos,pos,oldCol,target);
    return pos;
}

/* COMMENTSTYLED is a macro that determines whether the character at pos is in a comment */
#define COMMENTSTYLED(self,pos) ((((self))->GetStyle((pos))==(self)->comment_style) || (((self))->GetStyle((pos))==(self)->linecomment_style))

/* Indent will leave existing comments where they are unless reindent-comments is on. ReindentLine is used by the ^J key, but [Tab] still calls this routine. */
long srctext::Indent(mark *mark)
{
    boolean blankline;
    int ind, c=0;
    long end, pos=(mark)->GetPos();

    pos= GetBeginningOfLine(pos);
    do  {
	end= SkipWhitespace(pos,GetLength());
	blankline= (GetChar(end)=='\n' || end>=GetLength());
	if (!this->indentingEnabled) /* unlikely; Tab key wouldn't have CALLED us */
	    ind= CurrentIndent(pos);
	else if (blankline || !COMMENTSTYLED(this,end+1)) {
	    /* either it's a blank line (so guess), */
	    /* or it's code (use correct indentation) */
	    ind= Indentation(pos);
	    if (blankline && (mark)->GetLength() && !PrettyPrint/*RSKprettyprint*/)
		/* a blank line in a selected region shouldn't waste chars */
		ind= 0;
	} else
	    /* it's a non-blank comment line (leave it where it is unless reindent-comments is on) */
	    if (this->reindentComments)
		ind= Indentation(pos);
	    else
		ind= CurrentIndent(pos);
	/* optimize whitespace and scrap extraneous afterwards */
	pos= TabAndOptimizeWS(pos,ind);
	for(end=pos; is_whitespace(GetChar(end)); ++end) ;
	if(end>pos)
	    DeleteCharacters(pos,end-pos);
	end=(mark)->GetEndPos();
	while(pos<end && (c=GetChar(pos++))!=EOF && c!='\n') ;
    } while(pos<end && c!=EOF);
    return pos;
}

/* ReindentLine, which is called when ^J is hit, reindents properly, and in a comment will guess how the next line should be indented. */
void srctext::ReindentLine(long pos)
{
    long end, skipws;
    int c;
    boolean blankline;

    pos= GetBeginningOfLine(pos);
    skipws= SkipWhitespace(pos,GetLength());
    c= GetChar(skipws);
    blankline= ((c=='\n') || (c==EOF));
    if (blankline || !InCommentStart(skipws+1))
	/* code, or empty comment line; use proper indentation */
	pos= TabAndOptimizeWS(pos,Indentation(pos));
    else
	/* don't mess up continued comments unless reindent-comments is on */
	if (this->reindentComments)
	    pos= TabAndOptimizeWS(pos,Indentation(pos));
	else
	    pos= TabAndOptimizeWS(pos,CurrentIndent(pos));
    for(end=pos; (c=GetChar(end))==' ' || c=='\t'; ++end);
    if(end>pos)
	DeleteCharacters(pos,end-pos);
}

/* srctext_ReflowComment will check to see if pos is inside a comment. If the comment is continued from a previous line, the carriage return and extra whitespace will be removed to "flow" the comments together. */
boolean srctext::ReflowComment(long pos)
{
    long startcomment=InCommentStart(pos);
    if (startcomment) {
	long startline=GetBeginningOfLine(pos);
	if (startline>startcomment) {
	    /* comment is broken by a carriage return inside it-- flow back together if second line isn't blank */
	    long skipws= SkipWhitespace(startline, GetLength());
	    if (GetChar(skipws) != '\n') {
		ReplaceCharacters(startline-1, skipws-startline+1, " ",1);
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* Breakable returns TRUE if pos is after whitespace or a comma, or the start of a change flag, which means the line can be broken at pos. */
boolean srctext::Breakable(long pos)
{
    int pch, ch=GetChar(pos);
    boolean instring=InString(pos);
    if (is_whitespace(ch) && !instring)
	return TRUE;
    pch= GetChar(pos-1);
    if ((is_whitespace(pch) || pch==',') && !instring)
	return TRUE;
    if (ch=='@' && InCommentStart(pos))
	return TRUE;
    return FALSE;
}

/* BreakLine breaks up a line so it fits within the specified max-length. endofline is a mark (so it stays put during reindenting) pointing to the newline at the end of the line to be broken. */
void srctext::BreakLine(mark *endofline)
{
    long end;
    int c;
    int ccol= CurrentColumn((endofline)->GetPos()-1);
    int prevlen= ccol+1; /* add 1 to ensure at least 1 iteration of while loop */
    /* ccol & prevlen keep track of how long the remainder of the line IS and WAS */
    /* this prevents infinite loops when a 'word' is too long to break up right */
    while (ccol>=GetMaxLineLength() && ccol<prevlen) {
	/* line too long; fragment it */
	long stopline= (endofline)->GetPos();
	int curcol= ccol+1;
	do  {
	    --stopline;
	    c= GetChar(stopline);
	    if (c=='\t') /* recalculate */
		curcol= CurrentColumn(stopline);
	    else /* decrement (it's a LOT quicker!) */
		--curcol;
	} while (c!='\n' && stopline>0 && (!Breakable(stopline) || curcol>GetMaxLineLength()));
	if (stopline<1 || c=='\n')
	    return; /* never mind */
	if (GetStyle(stopline)==this->linecomment_style)
	    /* we can't break bang comments! */
	    stopline= (GetEnvironment(stopline))->Eval();
	end= stopline;
	while (stopline>0 && is_whitespace(GetChar(stopline-1)))
	    --stopline;
	if (stopline<end)
	    DeleteCharacters(stopline,end-stopline);
	/* pretend there's a blank line so it uses 'desired' _Indentation */
	InsertCharacters(stopline,"\n\n",2);
	ReindentLine(stopline+1);
	stopline= SkipWhitespace(stopline+1, GetLength());
	DeleteCharacters(stopline, 1); /* remove extra newline */
	/* reindent *again* in case we have to right-justify an end-of-comment */
	ReindentLine(stopline);
	prevlen= ccol;
	ccol= CurrentColumn((endofline)->GetPos()-1);
    }
}

/*RSK91overstrike: mostly snagged from Patch10's ConfirmViewDeletion in atk/text/txtvcmod.c*/
static boolean MakeSureNotOverstrikingView(srctext *d, long pos, long len)
{
    boolean hasViews;
    environment *env;

    for (hasViews = FALSE; len--; ++pos)
	if ((d)->GetChar(pos) == TEXT_VIEWREFCHAR) {
	    env= (environment *)(d->text::rootEnvironment)->GetInnerMost(pos);
	    if (env->type == environment_View) {
		hasViews = TRUE;
		break;
	    }
	}
    if (! hasViews)
	return TRUE;
    return FALSE; /*can't type over insets*/
}

/*RSK91overstrike: this routine was based on Patch10's textview_ViDeleteCmd (atk/text/txtvcmod.c).*/
void srctext::OverstrikeAChar(long pos)
{
    long dsize;
    int c;

    if (GetChar(pos - 1) == '\n' && GetChar(pos) == '\n')
	return;
    dsize = GetLength();
    if (MakeSureNotOverstrikingView(this, pos, 1))
	if ((c=GetChar(pos))!='\n' && c!='\t' && pos<dsize)
	    DeleteCharacters(pos, 1);
}

/*RSK91overstrike: override simpletext's normal character insertion*/
boolean srctext::InsertCharacters(long pos, const char *str, long len)
{
    if (IsInOverstrikeMode()) {
	int i= 0;
	do  {
	    if (*(str+i)!='\n' && *(str+i)!='\t')
		OverstrikeAChar(pos+i);
	    ++i;
	} while (i<len);
    }
    return (this)->text::InsertCharacters(pos,str,len);
}

/* Address of pos and address of len are passed in. Sets length to zero if no valid tokens are found. */
/* (this is used by other packages, but not the source views themselves) */
void srctext::GetToken(long *pos, long *len)
{
    long revpos = (*pos);
    long forpos = (*pos)+(*len);
    long endtxt = GetLength();

    if (IsTokenChar(GetChar(revpos)))
	while (revpos>=0 && IsTokenChar(GetChar(revpos-1))) --revpos;
    (*pos) = revpos;
    while (forpos<endtxt && IsTokenChar(GetChar(forpos))) ++forpos;
    (*len) = forpos-(*pos);         
    return;
}
