/* File htmltext.C created by Ward Nelson.  Revised by WJHansen
  (c) Copyright IBM Corp 1995.  All rights reserved.
  Copyright Carnegie Mellon University, 1996. All rights reserved.

    htmltext
	an object for reading and writing HTML+ formatted files.
*/

#include <andrewos.h>
ATK_IMPL("htmltext.H")
#include <util.h>
#include <im.H>
#include <dictionary.H>
#include <buffer.H>
#include <dataobject.H>
#include <environ.H>
#include <environment.H>
#include <print.H>
#include <fnote.H>
#include <fontdesc.H>
#include <image.H>
#include <imageio.H>
#include <style.H>
#include <stylesheet.H>
#include <table.H>
#include <text.H>
#include <link.H>
#include <urlbutton.H>
#include <viewref.H>
#include <mflex.H>
#include <awidget.H>
#include <search.H>
#include <print.H>

#include <attlist.H>
#include <htmltext.H>
#include <htmlenv.H>
#include <hidden.H>
#include <htmlform.H>

#include "web.h"

#define saferealloc(A,B) ((A==NULL) ? malloc(B) : realloc(A,B))

// #define PRINTXXX 0
#ifndef PRINTXXX
#define PRINTXXX 1
#endif

static char *tag;		/* used for "efficiency". */
static long taglen;

struct stylemap {
    const char *ident;
    long type;
};

struct symbolmap {
    const char *code;
    char symbol;
    const char *stylename;
};


ATKdefineRegistry(htmltext, text, htmltext::InitializeClass);


// CONSTRUCTORS - DESTRUCTORS
static boolean ReadingATemplate;	 /* used by ReadTemplate
				to hack ReadSubString behavior */

	boolean
htmltext::InitializeClass()  {
	tag = NULL;
	taglen = 0;
	ReadingATemplate= FALSE;
	if (environ::GetProfileSwitch( "ExposeStylesLikeSGML", TRUE)) {
	    /* user didn't set it to FALSE */
	    environ::AddStringProfile( "ExposeStylesLikeSGML: yes");
	}
	return TRUE;
}

htmltext::htmltext()  {
	ATKinit;
	SetCopyAsText(TRUE);
	delete rootEnvironment;
	this->text::rootEnvironment =
			(environment *)
			htmlenv::GetTextRootEnvironment(this);

#if 0
	/* Reading the template here is icky but it Cures
		the problem of insets in tables and users who
		don't map a template 	to the html extension */
	ReadTemplate("html", FALSE);
#endif
        SetBaseTemplateName("html");
	this->readonly = GetReadOnly();
	title=NULL;
	//    ToolCount("EditViews-htmltext",NULL);
	THROWONFAILURE(TRUE);
}

htmltext::~htmltext()  {
    ATKinit;
    /* text object (superclass) will be freeing its root environment for us */
    if(title) free(title);
    return;
}


/* ==========================================

	READ AND WRITE SUBSTRINGS

	The definitions and functions in this section are used for
	sending to and reading from the cut buffer.

*/


/* ReadTemplate gets overridden as a cheesy hack to tell ReadSubString that it has to copy the style definitions from its dummy dataobject into the actual text object. */
long htmltext::ReadTemplate(const char *templateName, boolean inserttemplatetext)
{
    long retval;
    ReadingATemplate= TRUE;
    retval= this->text::ReadTemplate(templateName, inserttemplatetext);
    ReadingATemplate= FALSE;
    return retval;
}


/* OverrideStyles is redundant code from a text static function.  It's used by ReadSubString to copy the stylesheet from the temporary holder object into the target.  This is necessary because ReadSubString is called when reading a template. */
static void htmltext_OverrideStyles(class stylesheet *ssptr, class stylesheet *templateptr)
{
    int i;
    class style **styles, *overridestyle;

    for (i = 0, styles = ssptr->styles; i < ssptr->nstyles; i++, styles++)
	(*styles)->template_c = 0;
    for (i = 0, styles = templateptr->styles; i < templateptr->nstyles; i++, styles++) {
	if ((overridestyle = (ssptr)->Find( (*styles)->name)))
	     ((*styles))->Copy( overridestyle);
	else {
	    overridestyle = new style;
	    ((*styles))->Copy( overridestyle);
	    (ssptr)->Add( overridestyle);
	}
    }
    (ssptr)->SetTemplateName( templateptr->templateName);
    ssptr->version++;
}

/*XXX- This is a cheesy set of functions to convert back and forth
	between real htmlenv attributes, and the styles we covertly
	put into the conventional datastream to allow them to
	be retained through Copy/Paste operations. */
#define COVERTSTYLENAME "HTMLatts"
struct envlist {
	struct htmlenv *env;
	struct envlist *next;
};
static struct envlist *attenvs;

/* rememberCovertStyles is called for all environments.
	If it sees a covertly-created style,
	it remembers it in a global linked list.
*/
	static boolean
rememberCovertStyles(long dummy, text *txt, long pos,
		environment *env)  {
	if (env->type==environment_Style
			&& strcmp((env->data.style)->GetName(),
				COVERTSTYLENAME)==0) {
		struct envlist *elist = (struct envlist *)
				malloc(sizeof(struct envlist));
		elist->env = (htmlenv *)env;
		elist->next = attenvs;
		attenvs = elist;
	}
	return FALSE; /* keep enumerating */
}

	static void
chgCovertStylesToAttributes(htmltext *txt)  {
	attenvs= NULL;
	txt->EnumerateEnvironments(0, txt->GetLength(),
			(text_eefptr)rememberCovertStyles, 0);
	while (attenvs) {
		struct envlist *listhead= attenvs;
		htmlenv *cur= (htmlenv *)listhead->env,
				*parent= (htmlenv *)cur->GetParent();
		if (!parent || parent==txt->text::rootEnvironment
					|| parent->Eval()!=cur->Eval()) {
		    /* not nested in a style; look for an inset immediately afterward */
		    if (parent) parent= (htmlenv *)parent->GetNextChild(cur, cur->Eval()+cur->GetLength());
		    if (parent && parent->type!=environment_View) parent= NULL;
		}
		if (parent) {
			char *attbuf, *attstr;
			attlist *atts;
			long actuallen;
			attbuf= txt->GetBuf(cur->Eval()+1,
					cur->GetLength()-2, &actuallen);
			attstr= (char *)malloc(cur->GetLength()-2+1);
			strncpy(attstr, attbuf, actuallen);
			attstr[actuallen]= '\0';
			if (actuallen < cur->GetLength()-2) {
				/* text's "Gap" got in the way;
					grab the rest of the string */
				long remainderlen;
				attbuf = txt->GetBuf(cur->Eval()+
						1 +actuallen,
					cur->GetLength()-2 -actuallen,
					&remainderlen);
				strncat(attstr, attbuf,
					cur->GetLength()-2 -actuallen);
			}
			atts= new attlist;
			atts->MakeFromString(attstr);
			parent->SetAttribs(atts);
			// discard the original reference, the environment
			// has its own reference on the attributes now.
			atts->Destroy();
		} else
		    printf("htmltext: covert styles found in incorrect location; discarded.\n");
		/* delete, remove from list,
				and move on to next environment */
		txt->AlwaysDeleteCharacters(cur->Eval(),
				cur->GetLength());
		attenvs= listhead->next;
		free(listhead);
	}
}

	static boolean
rememberAttributes(long dummy, text *txt, long pos,
			environment *env)  {
	attlist *attl= ((htmlenv *)env)->GetAttribs();
	if (attl->Size()>0) {
		struct envlist *elist= (struct envlist *)
				malloc(sizeof(struct envlist));
		elist->env= (htmlenv *)env;
		elist->next= attenvs;
		attenvs= elist;
	}
	return FALSE; /* keep enumerating */
}

	static void
chgAttributesToCovertStyles(htmltext *txt)  {
	stylesheet *ss= txt->GetStyleSheet();
	style *covertstyle= ss->Find(COVERTSTYLENAME);
	if (!covertstyle) { /* CREATE covert style */
		covertstyle= new style;
		covertstyle->SetName(COVERTSTYLENAME);
		ss->Add(covertstyle);
	}
	attenvs= NULL;
	txt->EnumerateEnvironments(0, txt->GetLength(),
			(text_eefptr)rememberAttributes, 0);
	while (attenvs) {
		struct envlist *listhead= attenvs;
		htmlenv *env= (htmlenv *)listhead->env;
		long pos= env->Eval();
		attlist *attl= ((htmlenv *)env)->GetAttribs();
		char *attstr= attl->MakeIntoString();
		long attstrlen= strlen(attstr);
		/* make sure the covert-styled text
				ends up inside the correct style */
		environment *child= (environment *)env, *parent= (environment *)env, *prevsibling;
		do  {
		    boolean begflag= (child==env && child->type==environment_Style);
		    child->SetStyle(begflag, child->includeEnding);
		    child= (environment *)child->GetChild(pos);
		} while (child);
		while ((parent= (environment *)parent->GetParent()) != NULL)
		    if (parent->type==environment_Style)
			parent->SetStyle(TRUE, parent->includeEnding);
		if ((prevsibling= (environment *)(env->GetParent())->GetPreviousChild(env, pos)) != NULL)

			prevsibling->SetStyle(prevsibling->
					includeBeginning, FALSE);
		/* insert attributes and their values as covert text,
			and wrap with special covert style */
		txt->AlwaysInsertCharacters(pos, "<>", 2);
		txt->AlwaysInsertCharacters(pos+1, attstr, attstrlen);
		txt->AlwaysAddStyle(pos, attstrlen+2, covertstyle);
		/* remove from list and move on to next environment */
		attenvs = listhead->next;
		free(listhead);
	}
}


#ifndef DONT_NEED_ENVIRONMENT_COPY_HACK

/*XXX- CopyText is too stupid to make the new environments
 similar to the ones it's copying. The nestedmark class should have a
 nestedmark::MakeSimilar() member function,
 nestedmark::NewButSimilar() should call it, text.WrapStyle()
 should call it, and environment and htmlenv should both override it.
 Then, we can
	#define DONT_NEED_ENVIRONMENT_COPY_HACK
*/

static htmltext *targettxt=NULL;

	static boolean
xferAtts(long rock, text *self, long pos, environment *env)  {
	htmlenv *targetenv = (htmlenv *)
			(targettxt->text::rootEnvironment)->
				GetChild(rock+pos);
	/*XXX- this doesn't account for nesting levels, it just makes a
	 GUESS about the corresponding environment, based on type
	 and length and style name. */
	while (targetenv && (targetenv->type!=env->type
			|| targetenv->GetLength()!=env->GetLength()
			|| (env->type==environment_Style &&
			strcmp((targetenv->environment::data.style)->
			GetName(), (env->data.style)->GetName())!=0)))
		targetenv= (htmlenv *)targetenv->GetChild(rock+pos);
			/* hopefully it's nested in there somewhere;
				keep looking */
	if (targetenv)
		targetenv->attribs = (((htmlenv *)env)->attribs)->
				CopyAttributes();
	else
		printf("htmltext: environment_copy_hack failed to copy attributes, there's no corresponding environment at %ld+%ld!\n",
				rock, pos);

	return FALSE;  /* keep going */
}
#endif /*DONT_NEED_ENVIRONMENT_COPY_HACK*/


/* ReadSubString() is a hack to restore the attributes of
		environments that WriteSubString retained (covertly).
*/
	long
htmltext::ReadSubString(long pos, FILE *file,
			boolean quoteCharacters)  {
	if (quoteCharacters) {
		htmltext *holder= new htmltext;
		long addedlen;
		holder->text::ReadSubString(0, file, quoteCharacters);
		chgCovertStylesToAttributes(holder);
		addedlen= holder->GetLength();
		if (ReadingATemplate) { /* stupid text::Read smashed our htmlenv; smash its environment and put an htmlenv back */
		    if (this->text::rootEnvironment) (this->text::rootEnvironment)->FreeTree(); /* free the bogus rootenvironment previously allocated, first */
		    this->text::rootEnvironment= (environment *)htmlenv::GetTextRootEnvironment(this);
		}
		CopyText(pos, holder,0,addedlen);

#ifndef DONT_NEED_ENVIRONMENT_COPY_HACK
		targettxt= this;
		holder->EnumerateEnvironments(0, addedlen, xferAtts, pos);
#endif /*DONT_NEED_ENVIRONMENT_COPY_HACK*/
		if (ReadingATemplate) /* grab the style definitions out of holder before we destroy it! */
		    htmltext_OverrideStyles(this->text::styleSheet, holder->text::styleSheet);
		holder->Destroy();
		return addedlen;
	} else
		return
			this->text::ReadSubString(pos, file,
					quoteCharacters);
}

/* WriteSubString() is a hack to make sure the attributes
	of environments are retained (covertly) when text is Copied.
*/
	void
htmltext::WriteSubString(long pos, long len, FILE *file,
			boolean quoteCharacters)  {
	    if (quoteCharacters) {
		htmltext *holder= new htmltext;
		holder->SetWriteID(im::GetWriteID());
		holder->CopyText(0, this,pos,len);

#ifndef DONT_NEED_ENVIRONMENT_COPY_HACK
		targettxt= holder;
		EnumerateEnvironments(pos, len, xferAtts, -pos);
#endif /*DONT_NEED_ENVIRONMENT_COPY_HACK*/

		chgAttributesToCovertStyles(holder);
		holder->text::WriteSubString(0, holder->GetLength(),
				file, quoteCharacters);
		holder->Destroy();
	} else
		this->text::WriteSubString(pos, len, file,
				quoteCharacters);
}


/* ==========================================

	READ AND WRITE HTML

	The definitions and functions in this section are used for
	BOTH reading and writing files in HTML format.
*/


// the WNL_ flags control what is sent to the file
// for newlines in the text
//
#define WNL_Normal 0		// \n\n ==> <p>,  \n ==> <br>
#define WNL_Literal (1<<0) 	// output as-is
#define WNL_brATnl (1<<1)	// every \n ==> <br>
		// brATnl is used where HTML implies extra newlines
#define WNL_ParaIndent (1<<2)	// both \n\n & \n ==> <p>
//	(1<<3) is a spare
#define WNL_FLAGS ((1<<4)-1)	// get all WNL_ flags


static const struct symbolmap symboltable[] = {
    {"AElig",	'\306',	""},
    {"Aacute",	'\301',	""},
    {"Acirc",	'\302',	""},
    {"Agrave",	'\300',	""},
    {"Aring",	'\305',	""},
    {"Atilde",	'\303',	""},
    {"Auml",	'\304',	""},
    {"Ccedil",	'\307',	""},
    {"ETH",	'\320',	""},
    {"Eacute",	'\311',	""},
    {"Ecirc",	'\312',	""},
    {"Egrave",	'\310',	""},
    {"Euml",	'\313',	""},
    {"Iacute",	'\315',	""},
    {"Icirc", 	'\316',	""},
    {"Igrave",	'\314',	""},
    {"Iuml",	'\317',	""},
    {"Ntilde",	'\321',	""},
    {"Oacute",	'\323',	""},
    {"Ocirc",	'\324',	""},
    {"Ograve",	'\322',	""},
    {"Oslash",	'\330',	""},
    {"Otilde",	'\325',	""},
    {"Ouml",	'\326',	""},
    {"THORN",	'\336',	""},
    {"Uacute",	'\332',	""},
    {"Ucirc",	'\333',	""},
    {"Ugrave",	'\331',	""},
    {"Uuml",	'\334',	""},
    {"Yacute",	'\335',	""},
    {"aacute",	'\341',	""},
    {"acirc",	'\342',	""},
    {"acute",	'\264',	""},	/* not in Mosaic */
    {"aelig",	'\346',	""},
    {"agrave",	'\340',	""},
    {"amp", 	'&',   	""},
    {"aring",	'\345',	""},
    {"atilde",	'\343',	""},
    {"auml",	'\344',	""},
    {"brvbar",	'\246',	""},	/* not in Mosaic */
    {"ccedil",	'\347',	""},
    {"cedilla",	'\270',	""},	/* not in Mosaic */
    {"cent",  	'\242',	""},	/* not in Mosaic */
    {"copy",	'\251',	""},	/* not in Mosaic */
    {"curren",	'\244',	""},	/* not in Mosaic */
    {"deg",  	'\260',	""},	/* not in Mosaic */
    {"die",    	'\250',	""},	/* not in Mosaic */
    {"divide",	'\367',	""},	/* not in Mosaic */
    {"eacute",	'\351',	""},
    {"ecirc",	'\352',	""},
    {"egrave",	'\350',	""},
    {"eth",    	'\360',	""},
    {"euml",	'\353',	""},
    {"frac12",	'\275',	""},	/* not in Mosaic */
    {"frac14",	'\274',	""},	/* not in Mosaic */
    {"frac34",	'\276',	""},	/* not in Mosaic */
    {"gt",    	'>',    	""},
    {"iacute",	'\355',	""},
    {"icirc",  	'\356',	""},
    {"iexcl",	'\241',	""},	/* not in Mosaic */
    {"igrave",	'\354',	""},
    {"iquest",	'\277',	""},	/* not in Mosaic */
    {"iuml",	'\357',	""},
    {"laquo",	'\253',	""},	/* not in Mosaic */
    {"lt",     	'<',    	""},
    {"macron",	'\257',	""},	/* not in Mosaic */
    {"micro",	'\265',	""},	/* not in Mosaic */
    {"middot",	'\267',	""},	/* not in Mosaic */
    {"nbsp",	' ' /* was \240, should be a non-breaking space*/,	""},	/* not in Mosaic */
    {"not",   	'\254',	""},	/* not in Mosaic */
    {"ntilde",	'\361',	""},
    {"oacute",	'\363',	""},
    {"ocirc",	'\364',	""},
    {"ograve",	'\362',	""},
    {"ordf",  	'\252',	""},	/* not in Mosaic */
    {"ordm",	'\272',	""},	/* not in Mosaic */
    {"oslash",	'\370',	""},
    {"otilde",	'\365',	""},
    {"ouml",	'\366',	""},
    {"para",  	'\266',	""},	/* not in Mosaic */
    {"plusmn",	'\261',	""},	/* not in Mosaic */
    {"pound",	'\243',	""},	/* not in Mosaic */
    {"quot",  	'"',    	""},
    {"raquo",	'\273',	""},	/* not in Mosaic */
    {"reg",   	'\256',	""},	/* not in Mosaic */
    {"sect",  	'\247',	""},	/* not in Mosaic */
    {"shy",  	'\255',	""},	/* not in Mosaic */
    {"sup1",	'\271',	""},	/* not in Mosaic */
    {"sup2",	'\262',	""},	/* not in Mosaic */
    {"sup3",	'\263',	""},	/* not in Mosaic */
    {"szlig", 	'\337',	""},
    {"thorn",	'\376',	""},
    {"times",	'\327',	""},	/* not in Mosaic */
    {"uacute",	'\372',	""},
    {"ucirc",	'\373',	""},
    {"ugrave",	'\371',	""},
    {"uuml",	'\374',	""},
    {"yacute",	'\375',	""},
    {"yen",  	'\245',	""},	/* not in Mosaic */
    {"yuml",	'\377',	""},
    // XXX- add all the junk in fonts andysymbol & andysymbola
    {NULL, 0, NULL}
};

// SYMBOL TABLE LOOKUP functions

	static const struct symbolmap *
LookupNameBySymbol(char symbol, const char *style) {
    static struct symbolmap temp; /* temporary-use #ascii symbol return struct */
    static char symbolname[10]; /* temporary-use #ascii symbol symbol name */
	long i =0;
	while (symboltable[i].code) {
		if (symboltable[i].symbol ==symbol &&
				strcmp(symboltable[i].stylename, style) ==0)
			return &symboltable[i];
		++i;
	}
	if (isprint(symbol))
	    return NULL;
	else if (style[0]=='\0') { /* no style, and not printable; use #ascii value as a label name */
	    sprintf(symbolname, "#%1d", symbol);
	    temp.code= symbolname;
	    return &temp;
	}
	return NULL;
}


/* gets the value of the attribute named id */
	static const char *
getatt(attlist *atts, const char *id)  {
	struct htmlatts *temp = (atts)->GetAttribute(id);
	return (temp) ? temp->value : NULL;
}


/* Create a value in the global `tag'.
	The return value is a pointer to a static char *
	and should be used or copied immediately.
	DO NOT retain a pointer to it.
	returns NULL if an empty string was passed for name
		(which means the style name could not be identified)
*/
	static const char *
build_first_tag(attlist *atts, const char *name)  {
	char *attributes;
	long len;

	if (!name && !atts) {
		/* used for initialization and/or clean up */
		if (tag) free(tag);
		tag = NULL;
		taglen = 0;
		return NULL;
	}
	if (*name == '\0')
		/* empty string, so we don't actually want a tag */
		return NULL;

	/* find out how much space is needed */
	attributes = (atts)->MakeIntoString();
	len = strlen(name) + strlen(attributes) + 4;
			/* +4  for <>, possible space, and EOS */

	/* make sure there is enough space */
	if (taglen < len) {
		taglen = 32 + len;		/* add a little extra */
		tag = (char *)saferealloc(tag, taglen);
	}
	/* put the info in the string */
	sprintf(tag, "<%s%s%s>", name,
			attributes[0]?" ":"", attributes);
	return tag;
}

/* "render"ing styles
	If a style with name stylenm does not have an entry in
	the WStyle table below, then it is rendered in HTML as a
	sequence of html tags that approximate the meaning of the style.
	The starttag for the first contains the attribute "atkstyle"
	with a value composed of the style name, a colon, and a
	sequence of code letters giving the details of the style definition.
	Subsequent start tags are given the attribute "atknop".

	The full rendering is then

		<x ATKSTYLE=stylename:codeletters><y ATKNOP>...\n
		...
		...</y></x>\n

A `codeletters' string is constructed to described the
desired attributes of a style, so it can be reconstructed.
The codes, meanings, and html tags are
	C - color  		<font color=#dddddd>  (Cdddddd;)
	F - bigger font		<font size="+1">
	H - helvetica (sans serif)
	L - left indent		<dl><dt> ... </dl>
	R - right indent
	S - superscript		<sup>
	T - typewriter (& fixed width)  <tt>
	X - indent para  (Xiii, where iii is indent in points)
	b - bold		<b>
	c - center		<center>
	d - dotted box		<u>
	f - smaller font	<font size="-1">
	h - hidden		<ignore><!-- ... --></ignore>
	i - italic		<i>
	k - strike thru		<strike>
	n - format note		<ignore><!-- ... --></ignore>
	o - overbar		<u>
	p - end_paragraph	<p>
	r - right justify
	s - subscript		<sub>
	u - underline		<u>
	x - exdent para  (xiii, where iii is indent in points)
*/



/* AppendIndentCode
	convert val to pts
	output code followed by number of points
	(see atk/lookz/lookzview.C for value translation)
*/
	static void
AppendIndentCode(char **pcodex, char code, long val, enum style_Unit Unit) {
	switch (Unit) {
	case style_RawDots:	break;
	case style_Points:	break;
	case style_Inches:	val =  (long)(val * .00109863);  break;
	case style_CM:		val =  (long)(val * .000432533);  break;
	case style_Ems:  	val *= 12;  // BOGUS
	case style_Lines:  	val *= 12;  // BOGUS
	}
	sprintf(*pcodex, "%c%ld", code, val);
	(*pcodex) += strlen(*pcodex);
}

	static char *
StyleToCodes(style *sty) {
	char codes[300], *codex = codes;
	char fambuf[300];
	enum style_FontSize sizebasis;
	enum style_ScriptMovement scrbasis;
	enum style_Unit units;
	style_Justification just;
	long val;
	enum style_MarginValue Basis;
	enum style_Unit Unit;

	// font size
	(sty)->GetFontSize(&sizebasis, &val);
	if (sizebasis == style_PreviousFontSize) {
		// relative font size
		if (val > 0) *codex++ = 'F';
		else if (val < 0) *codex = 'f';
	}
	// check for script/baseline movement
	(sty)->GetFontScript(&scrbasis, &val, &units);
	if (val<0) *codex = 'S';	// superscript
	else if (val>0) *codex++= 's';	// subscript
	// check font faces
	if ((sty)->IsAnyAddedFontFace(fontdesc_Bold))
		*codex++ = 'b';
	if ((sty)->IsAnyAddedFontFace(fontdesc_Italic))
		*codex++ = 'i';
	if ((sty)->IsAnyAddedFontFace(fontdesc_Fixed))
		*codex++ = 'T';
	(sty)->GetFontFamily(fambuf, sizeof(fambuf)-1);
	if (strcmp(fambuf, "AndySans") == 0
			|| strcmp(fambuf, "Helvetica") == 0)
		*codex++ = 'H';

	// justification
	just = (sty)->GetJustification();
	if (just == style_RightJustified)
		*codex++ = 'r';
	if (just == style_Centered)
		*codex++ = 'c';

	// check flags
	if ((sty)->IsHiddenAdded())
		*codex++ = 'h';
	if ((sty)->IsPassThruAdded())
		*codex++ = 'n';
	if ((sty)->IsUnderlineAdded())
		*codex++ = 'u';
	if ((sty)->IsDottedBoxAdded())
		*codex++ = 'd';
	if ((sty)->IsStrikeThroughAdded())
		*codex++ = 'k';
	if ((sty)->IsOverBarAdded())
		*codex++ = 'o';
	if ((sty)->IsAttribute("end_paragraph"))
		*codex++ = 'p';
	char *colorname;
	colorname = (sty)->GetAttribute("color");
        if (colorname != NULL) {
		double r, g, b;
		print::LookUpColor(colorname, &r, &g, &b);
		*codex++ = 'C';
		sprintf(codex, "#%02d%02d%02d;", (int)(r*255), (int)(g*255), (int)(b*255));
		codex += 7;
	}

	// indentation
	(sty)->GetNewIndentation(&Basis, &val, &Unit);
		if (val > 0 && Basis == style_LeftMargin)
			AppendIndentCode(&codex, 'X', val, Unit);
		if (val < 0 && Basis == style_LeftMargin)
			AppendIndentCode(&codex, 'x', -val, Unit);
	(sty)->GetNewLeftMargin(&Basis, &val, &Unit);
		if (val > 0 && Basis == style_LeftMargin)
			AppendIndentCode(&codex, 'L', val, Unit);
	(sty)->GetNewRightMargin(&Basis, &val, &Unit);
		if (val > 0 && Basis == style_RightMargin)
			AppendIndentCode(&codex, 'R', val, Unit);

	*codex++ = '\0';
	return strdup(codes);
}

/* CodesToTags
	converts a string of code letters to HTML tag sequence
	if `starttag' is FALSE, generate </...> tags in reverse order
	return value is valid only until the next call of CodesToTags
	OR into *pmode the appropriate bits
*/
	static char *
CodesToTags(const char *codes, const char *stylename, boolean starttag,
			long *pmode) {
	const char *codex, *codeend, *taggle;
	int delta;
	char atkattr[300];
	const char *styleAttr = "";
	static char tag[300], *tagx;
	char colortag[20];

	if (starttag) {
		styleAttr = atkattr;
		sprintf(atkattr, " atkstyle=%s:%s", stylename, codes);
		codex = codes;
		codeend = codes+strlen(codes);
		delta = 1;
	}
	else {
		codex = codes+strlen(codes)-1;
		codeend = codes-1;
		delta = -1;
	}
	tagx = tag;
	*tagx = '\0';

	while (codex != codeend) {
		switch(*codex) {
		case '0':  case '1':  case '2':  case '3':  case '4':
		case '5':  case '6':  case '8':  case '7':  case '9':
			codex += delta;			// skip digit in a points value
			continue;				// no taggle
		case ';':  // color code 'C' from reverse direction
			codex += 8*delta;	// presumably -8
			continue;

		case 'L': taggle = "dl";  break;	// special case below
		case 'S': taggle = "sup";  break;
		case 'T': taggle = "tt";  *pmode |= WNL_brATnl;  break;
		case 'b': taggle = "b";  break;
		case 'c': taggle = "center";  break;
		case 'd': taggle = "u";  *pmode |= WNL_brATnl;  break;
		case 'h': taggle = "ignore";  break;	// special case below
		case 'i': taggle = "i";  break;
		case 'k': taggle = "strike";  break;
		case 'n': taggle = "ignore";  break;	// special case below
		case 'o': taggle = "u";  break;
		case 's': taggle = "sub";  break;
		case 'u': taggle = "u";  break;

		case 'p': taggle = (starttag) ? "p" : NULL;  break;	//???
		case 'F': taggle = (starttag) ? "font size=\"+1\"" : "font";  break;
		case 'f': taggle = (starttag) ? "font size=\"-1\"" : "font";  break;

		// It is BOGUS to put attrs directly in the begin_tag
		// The correct approach is to add atts to the htmlenv.

		case 'C': 
			if (starttag) {
				sprintf(colortag, "font color=%.7s", codex+1);
				taggle = colortag;
				codex += 8*delta;	// presumably +8
			}
			else taggle = "font";
			break;

		case 'X':
		case 'x':
			*pmode |= WNL_ParaIndent;  taggle = NULL;  break;

		default:  // code with no tag
			taggle = NULL;  break;
		} // end switch (*codex)

		if (starttag) {
			if (taggle) {
				sprintf(tagx, "<%s%s>", taggle, styleAttr);
				tagx += strlen(tagx);
			}
			switch (*codex) {
			case 'L':  sprintf(tagx, "<dt>");  break;
			case 'n':	case 'h':
				sprintf(tagx, "<!-- %s", styleAttr);
				break;
			}
			if (*tagx) tagx += strlen(tagx);
		}
		else {
			if (*codex == 'n' || *codex == 'h') {
				sprintf(tagx, "-->");
				tagx += strlen(tagx);
			}
			if (taggle) {
				sprintf(tagx, "</%s>",  taggle);
				tagx += strlen(tagx);
			}
		}

		styleAttr = " atknop";
		codex += delta;
	}
	return (*tag) ? tag : NULL;
}

// GetPts - parse digits at *(codex+1) et sequens
// return the value found
	static long
GetPts(char **codex) {
	long pts = 0;
	while (isdigit((*codex)[1])) {
		pts = 10 * pts + ((*codex)[1] - '0');
		(*codex)++;
	}
	return pts;
}

/* CodesToStyle
	converts a codes string to a style
	caller is responsible for freeing the returned style

	caller must 	(newstyle)->SetName(tag);
	and 		(ss)->Add(newstyle);
*/
	static style *
CodesToStyle(char *codex) {
	style *newstyle = new style;
	long leftindent = 0, rightindent = 0, indentpara = 0;
	char colornm[8];

	for ( ; *codex; codex++) {
		switch (*codex) {
		case 'C': // color attribute
			strncpy(colornm, codex+1, 7);
			colornm[7] = '\0';
			(newstyle)->AddAttribute( "color", colornm);
			codex += 8;
			break;
		case 'F': // bigger font
			(newstyle)->SetFontSize(style_PreviousFontSize, +2);
			break;
		case 'H': // helvetica (sans serif)
			(newstyle)->SetFontFamily("AndySans");
			break;
		case 'L': // left indent
			leftindent += GetPts(&codex);   break;
		case 'R': // right indent
			rightindent += GetPts(&codex);  break;
		case 'S': // superscript
			(newstyle)->SetFontScript(style_PreviousScriptMovement,
					-6, style_Points);
			(newstyle)->SetFontSize(style_PreviousFontSize, -2);
			break;
		case 'T': // typewriter (& fixed width)
			(newstyle)->SetFontFamily("AndyType");
			(newstyle)->AddNewFontFace(fontdesc_Fixed);
			break;
		case 'X': // indent para
			indentpara += GetPts(&codex);
			break;
		case 'b': // bold
			(newstyle)->AddNewFontFace(fontdesc_Bold);
			break;
		case 'c': // center
			(newstyle)->SetJustification(style_Centered);
			break;
		case 'd': // dotted box
			(newstyle)->AddDottedBox();
			break;
		case 'f': // smaller font
			(newstyle)->SetFontSize(style_PreviousFontSize, -2);
			break;
		case 'h': // hidden
			(newstyle)->AddHidden();
			break;
		case 'i': // italic
			(newstyle)->AddNewFontFace(fontdesc_Italic);
			break;
		case 'k': // strike thru
			(newstyle)->AddStrikeThrough();
			break;
		case 'n': // format note
			(newstyle)->AddPassThru();
			break;
		case 'o': // overbar
			(newstyle)->AddOverBar();
			break;
		case 'p': // end_paragraph
			(newstyle)->AddAttribute("end_paragraph", "");
			break;
		case 'r': // right justify
			(newstyle)->SetJustification(style_RightJustified);
			break;
		case 's': // subscript
			(newstyle)->SetFontScript(style_PreviousScriptMovement,
					2, style_Points);
			(newstyle)->SetFontSize(style_PreviousFontSize, -2);
			break;
		case 'u': // underline
			(newstyle)->AddUnderline();
			break;
		case 'x': // exdent para
			indentpara -= GetPts(&codex);
			break;
		}
	}
	// process indents
	if (leftindent)
		(newstyle)->SetNewLeftMargin(style_LeftMargin,
			leftindent, style_Points);
	if (rightindent)
		(newstyle)->SetNewRightMargin(style_RightMargin,
			rightindent, style_Points);
	if (indentpara)
		(newstyle)->SetNewIndentation(style_LeftMargin,
			indentpara, style_Points);
	return newstyle;
}


/* ==========================================

	READ HTML

	The definitions and functions in this section are used for
	reading files in HTML format.
*/

ASlot_NAME(parentform);

/* these are the mode bits (for reading) */
#define STOP			(1<<0)
#define LITERAL		(1<<1)
#define TABLE			(1<<2)
#define DONE			(1<<3)
#define SPACE_NEEDED	 (1<<4)
#define RAW_TEXT		(1<<5)
#define IGNORE_SPACE (1<<6)
#define CATASTROPHE		-1	/* fallback return code for failures */

typedef long tagfunc(htmltext **ptxtobj,
		struct htmltaginfo *taginfo, long *ppos, long *pmode);

static tagfunc space_func;
static tagfunc font_func;
static tagfunc footnote_func;
static tagfunc header_func;
static tagfunc rawinset_func;
static tagfunc titleinset_func;
static tagfunc img_func;
static tagfunc lit_func;
static tagfunc list_func;
static tagfunc quote_func;
static tagfunc ignore_func;
static tagfunc table_func;

static tagfunc form_func;
static tagfunc input_func;
static tagfunc select_func;
static tagfunc option_func;
static tagfunc textarea_func;

static long do_read(htmltext *txtobj, FILE *file, char *termstr,
		long *pmode);
static hidden *insert_hidden(htmltext *self, long *ppos);

#define INCREMENTKEYWORDSIZE 32
#define GETANDTEST(C,file) ((C = getc(file)) != EOF)

enum DelimiterType { LONE, PAIR };

struct tagmap {
	const char *id;
	const char *stylename;
	enum DelimiterType endtag;
		/* LONE if it's just a standalone <tag>,
		PAIR if it has a matching </tag> */
	long (*function)(htmltext **, htmltaginfo *, long *, long *);
};

/* this table is sorted alphabetically for ease of searching */
	static const struct tagmap
tagtable[] = {
{ "a",  	  	"anchor", PAIR,   NULL },
{ "abbrev", 	"abbreviation", PAIR,   NULL },
//{ "abstract",  	"abstract",  PAIR,  NULL },  // not done
{ "acronym", 	"acronym", PAIR,   NULL },
{ "added",     	"added", PAIR,   NULL },
//{ "address", 	"address", PAIR,   NULL }, // not done
{ "arg",     	"argument", PAIR,   NULL },
{ "b", 	  	"bold", PAIR,   NULL },
//{ "base", 	"base", PAIR,   NULL }, // not done
{ "blockquote", "quoteparagraph", PAIR,   quote_func },
//{ "body", 	"body", PAIR,   NULL }, // not done
//{ "box",     	"box", PAIR,   NULL }, // not done
{ "br",    	 	NULL, LONE,   space_func },
//{ "byline", 	"byline", PAIR,   NULL }, // not done
//{ "caption", 	"caption", PAIR,   NULL }, // not done
{ "center",		"center",	PAIR,  NULL },
{ "changed", 	"changebar", PAIR,   NULL },
{ "cite",     	"citation", PAIR,   NULL },
{ "cmd",     	"command", PAIR,   NULL },
{ "code",     	"code", PAIR,   NULL },
{ "dd",     		"anchortarget", LONE,   list_func },
{ "dfn",     	"define", PAIR,   NULL },
//{ "dir", 		"dir", PAIR,   NULL }, // not done
{ "dl", 		"definition", PAIR,   list_func },
{ "dt", 		"term", LONE,   list_func },
{ "em", 		"emphasize", PAIR,   NULL },
//{ "fig", 		"figure", PAIR,   NULL }, // not done
{ "font",               NULL, PAIR, font_func },
{ "footnote", 	NULL, PAIR,   footnote_func },
{ "form", 		"form", PAIR,   form_func },
{ "h1", 		"header1", PAIR,   header_func },
{ "h2", 		"header2", PAIR,   header_func },
{ "h3", 		"header3", PAIR,   header_func },
{ "h4", 		"header4", PAIR,   header_func },
{ "h5", 		"header5", PAIR,   header_func },
{ "h6", 		"header6", PAIR,   header_func },
{ "head", 		NULL, PAIR,   rawinset_func },
		// XXX-replace "head" with special parser someday
		// (check for <TITLE> inside it,  too)
{ "hr", 		NULL, LONE,   space_func },
//{ "htmlplus", 	NULL, LONE,   NULL }, // not done
{ "i", 		"italic", PAIR,   NULL },
{ "ignore",  	NULL, PAIR, ignore_func },
{ "image", 	NULL, PAIR,   NULL },
{ "img", 	NULL, LONE,   img_func },
{ "input", 		"input", LONE,   input_func },
{ "kbd", 		"keyboard", PAIR,   NULL },
//{ "l", 		NULL, LONE,   space_func }, // not done
{ "li",	 	NULL, LONE,   list_func },
{ "lit", 		"literal", PAIR,   lit_func },
{ "margin", 	"margin", PAIR,   NULL },
//{ "math", 	"math", PAIR,   NULL }, // not done
//{ "menu", 	"menu", PAIR,   NULL }, // not done
//{ "note", 		"note", PAIR,   NULL }, // not done
{ "ol", 		"ordered", PAIR,   list_func },
{ "option", 	"option", LONE,   option_func },
//{ "over", 		"over", PAIR,   NULL }, // not done
{ "p", 		NULL, LONE,   space_func },
{ "person", 	"person", PAIR,   NULL },
{ "pre", 		"preformatted", PAIR,   lit_func },
{ "q", 		"inlinequote", PAIR,   NULL },
{ "quote",		"quoteparagraph",PAIR, quote_func },	/* <quote> is obsolete, Write operation will change it to <blockquote> per HTML 3.2 */
{ "removed", 	"removed", PAIR,   NULL },
{ "samp", 		"sample", PAIR,   lit_func },
{ "select", 		NULL, PAIR,   select_func },
{ "strike",		"strikethrough", PAIR,  NULL },
{ "strong", 	"strong", PAIR,   NULL },
{ "sub", 		"subscript", PAIR,   NULL },
{ "sup", 		"superscript", PAIR,   NULL },
//{ "tab", 		"tab", LONE,   NULL }, // not done
{ "table", 		NULL , PAIR,   table_func },
{ "td", 		NULL, LONE,   table_func },
{ "textarea", 	NULL, PAIR,   textarea_func },
{ "th", 		NULL, LONE,   table_func },
{ "title", 		NULL, PAIR,   titleinset_func },
		//XXX-need a function to display title in window
{ "tr", 		NULL, LONE,   table_func },
{ "tt", 		"typewriter", PAIR,   NULL },
{ "u", 		"underline", PAIR,   NULL },
{ "ul", 		"unordered", PAIR,   list_func },
{ "var", 		"variable", PAIR,   NULL },
{ NULL, 		NULL, LONE,   NULL }
};


static char *skiptokenchars(const char *s)
{
    while (*s && !isspace(*s) && *s != '=') ++s;
    return (char *)s;
}

//  TAGINFO and ATTLIST  functions

/* makes a copy of the htmltaginfo struct passed in and returns it */
	static struct htmltaginfo *
copy_taginfo(struct htmltaginfo *info)  {
	struct htmltaginfo *newinfo =
		(struct htmltaginfo *)malloc(sizeof(struct htmltaginfo));
	newinfo->thestyle = info->thestyle;
	newinfo->startpos = info->startpos;
	newinfo->anchorcode = info->anchorcode;
	newinfo->tagid = strdup(info->tagid);
	newinfo->atts = (info->atts)->CopyAttributes();
	return newinfo;
}

	static attlist *
parsetaginfo(struct htmltaginfo *node, char *taginfo)  {
	char *p = skiptokenchars(taginfo);
	long len = p - taginfo +1;

	node->tagid = (char *)malloc(len);
	strncpy(node->tagid, taginfo, len);
	node->tagid[len-1] = '\0';
	return attlist::ParseAttributes(p);
}


	static void
end_this_style(htmltext *self, struct htmltaginfo *node,
				long *ppos)  {
	htmlenv *env =NULL;
	long start, end;

	/* XXX - should not do this if this is for a literal tag */
	start = node->startpos;
	if ((self)->GetChar(start)  == ' ')  start ++;
	end = *ppos;
	if ((self)->GetChar(end-1) == ' ') end --;

	// I don't know what this test was for.  It is always TRUE -wjh
	//if (node->anchorcode != Anchor
	//		|| (node->atts)->GetAttribute("href")) {

	if ( ! node->thestyle) {  /* no style needed */ }
	else if ((end-start) > 0) {
		// need style for text of non-zero length
		env = (htmlenv *)(self)->AddStyle(start,
						  end-start, node->thestyle);
		env->attribs->Destroy();
		env->attribs = node->atts;
		if(env->attribs) env->attribs->Reference();
		if (strcmp(node->tagid, "ol") == 0) {
			(self)->RenumberList(0,
					listtype_ORDERED, env);
			*ppos = (env)->GetLength()
				+ (env)->Eval() + *ppos - end;
		}
		(env)->SetStyle(FALSE, FALSE);
	}
	else {
		/* has a style, but zero length.  Hmm.  Rather than
				discarding, make a hidden inset */
		hidden *hiddenobj;
		const char *tagstring= build_first_tag(node->atts,
						       node->tagid);
		hiddenobj= insert_hidden(self, ppos);
		hiddenobj->InsertCharacters(0, "</>", 3);
		hiddenobj->InsertCharacters(2, node->tagid,
				strlen(node->tagid));
		hiddenobj->InsertCharacters(0, tagstring,
				strlen(tagstring));
	}
	// }
}


// makeTagNode
//	construct an htmltagnode for an instance of a tag
//	parse the tag info andbuild attributes list
//	get thestyle from stylename or ATKSTYLE
//	set thestyle to NULL for ATKNOP
//
	static struct htmltaginfo *
makeTagNode(htmltext *self, const char *style_name, long pos,
				char *tag)  {
	struct htmltaginfo *newnode = (struct htmltaginfo *)
			malloc (sizeof(struct htmltaginfo));
	if (!newnode) return NULL;
	style *s = 	NULL;

	newnode->atts = parsetaginfo(newnode, tag);

	if (strcmp(newnode->tagid, "a") != 0)
		newnode->anchorcode = NotAnchor;
	else if ((newnode->atts)->GetAttribute("href"))
		newnode->anchorcode = Anchor;
	else if ((newnode->atts)->GetAttribute("name")) {
		newnode->anchorcode = AnchorTarget;
		style_name = "anchortarget";
	}
	else newnode->anchorcode = IglAnch;

	struct htmlatts *val;
	if ((newnode->atts)->GetAttribute("atknop"))
		s = NULL;
	else if ((val=(newnode->atts)->GetAttribute("atkstyle"))) {
		// build style from codes in the attribute value
		char *stylenm = strdup(val->value);
		char *codes = strchr(stylenm, ':');
		if (codes) {*codes = '\0';  codes++;}

		s = ((self)->GetStyleSheet())->Find(stylenm);
		if (!s && codes) {
			s = CodesToStyle(codes);
			if (!s) { free(stylenm); return NULL; }
			s->SetName(stylenm);
			(self->GetStyleSheet())->Add(s);
		}
		free(stylenm);

	}
	else if (style_name) {
		s = ((self)->GetStyleSheet())->Find(style_name);
		if (!s) {
			// whoah! template is missing a style,
			//   let's CREATE one to avoid losing the markup!
			s = new style;
			if (!s) return NULL;
			s->SetName(style_name);
			(self->GetStyleSheet())->Add(s);
		}
	}
	newnode->thestyle = s;
	newnode->startpos = pos;
	return newnode;
}

	static void
deleteTag(struct htmltaginfo *node)  {
	if ( ! node) return;
	free((node)->tagid);
	    //  The attributes lists are now refcounted.
	    // The attribute list will only be destroyed here
	    // if no environment uses it.
	if((node)->atts) (node)->atts->Destroy();
	free(node);
}


// TEXT BUILDING functions  (for reading HTML)

	static long
end_paragraph(htmltext *self, long pos)  {
	if (pos > 0) {
		if ((self)->GetChar(pos-1)!='\n') {
			(self)->InsertCharacters(pos, "\n\n", 2);
			pos += 2;
		} else if ((self)->GetChar(pos-2)!='\n') {
			(self)->InsertCharacters(pos++, "\n", 1);
		}
	}
	return pos;
}

	static long
end_line(htmltext *self, long pos)  {
	if (pos > 0) {
		if ((self)->GetChar(pos-1)!='\n')
			(self)->InsertCharacters(pos++, "\n", 1);
	}
	return pos;
}

void htmltext::ImbedTextInTable(table *t, chunk *c) {
    t->Imbed((char *)GetTypeName(), c);
}

/* Add a cell to a table.
	column might change depending on if this cell
	is really part of another cell
*/
	static htmltext *
add_cell(htmltext *txt, table *table, short row, short *column,
			struct htmltaginfo *taginfo, long border)  {
	htmltext *txtobj;
	struct cell *cll;
	struct chunk chk;

	const char *rowstr = NULL, *colstr = NULL;

	while ((*column) <= (table)->NumberOfColumns() &&
			(table)->IsJoinedToAnother(row-1, *column-1))
		++(*column);

	if ((*column) > (table)->NumberOfColumns())
		(table)->ChangeSize((table)->NumberOfRows(),
				*column);

	chk.LeftCol = chk.RightCol = (*column)-1;
	chk.TopRow = chk.BotRow  = row-1;

	//	(table)->Imbed((char *)txt->GetTypeName(), &chk);
	txt->ImbedTextInTable(table, &chk);
	cll = (table)->GetCell(chk.TopRow,chk.LeftCol);
	txtobj = (htmltext *) cll->interior.ImbeddedObject.data;
	txtobj->readonly=txt->readonly;

	{ /* make sure the inset has a template and a global style */
	    style *globalstyle= txtobj->GetGlobalStyle();
	    if (!globalstyle && !txtobj->text::templateName) {
		txtobj->text::templateName= (char *)malloc(5);
		strcpy(txtobj->text::templateName, "html");
		txtobj->ReadTemplate("html", FALSE);
		globalstyle= txtobj->GetGlobalStyle();
	    }
	    if (!globalstyle) {
		globalstyle= new style;
		globalstyle->SetName("global");
		(txtobj->GetStyleSheet())->Add(globalstyle);
		txtobj->SetGlobalStyle(globalstyle);
	    }
	}

	/* increase the number of rows if the cell spans rows  */
	if ((rowstr = getatt(taginfo->atts, "rowspan"))) {
		row += atoi(rowstr)-1;
		if (row > (table)->NumberOfRows())
			(table)->ChangeSize(row,
					(table)->NumberOfColumns());
		chk.BotRow = row-1;
	}

	/* increase the number of rows if the cell spans columns*/
	if ((colstr = getatt(taginfo->atts, "colspan"))) {
		*column = *column + atoi(colstr) -1;
		if (*column > (table)->NumberOfColumns())
			(table)->ChangeSize((table)->NumberOfRows(),
					*column);
		chk.RightCol = *column-1;
	}

	(table)->SetInterior(&chk, JOINED);
	if (border)
		(table)->SetBoundary(&chk, BLACK);

	return txtobj;
}

	static table *
insert_table(htmltext *self, long *ppos)  {
	viewref *vr =NULL;
	table *newtable =NULL;

	*ppos = end_line(self, *ppos); /* make sure we're on our own line */
	vr = (self)->InsertObject((*ppos)++, "table", "htablev");
	if (!vr) { /* no table.dll? */
	    --(*ppos);
	    fprintf(stderr, "htmltext: table inset could not be created. Is table.dll missing?\n"); fflush(stderr);
	    return NULL;
	}
	newtable = (table *)vr->dataObject;
	(newtable)->SetName("THIS IS A TEST"); /*XXX- what is this, and why is it here? */
	(newtable)->ChangeSize(1,1);

	*ppos = end_line(self, *ppos);
	return newtable;
}

	static long
insert_hr(htmltext *self, long pos)  {
	pos = end_line(self, pos); /* make sure we're on our own line */
	(self)->InsertObject(pos++, "hr", "hrv");
	pos = end_paragraph(self, pos);
	return pos;
}

	static hidden *
insert_hidden(htmltext *self, long *ppos)  {
	viewref *vref =(self)->InsertObject((*ppos)++, "hidden",
			"hiddenview");
	hidden *hiddenobj = NULL;
	if (vref) {
		hiddenobj = (hidden *)vref->dataObject;
		/* make the hidden object so that it is
			invisible if the file is readonly */
		hiddenobj->Visibility = (self->readonly)
				? hidden_HIDDEN : hidden_EXPOSED;
	}
	return hiddenobj;
}



/* br, p, l, hr specific stuff */
	static long
space_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
			long *ppos, long *pmode)  {
	switch (taginfo->tagid[0]) {
		case 'b':		/* tag is br */
			(*ptxtobj)->InsertCharacters((*ppos)++, "\n", 1);
			break;
		case 'h':		/* tag is hr */
			*ppos = insert_hr(*ptxtobj, *ppos);
			break;
		case 'l':		/* tag is l */
			if (!(*pmode & STOP)) {

/*				if (taginfo->atts)
					unmatched_L = copy_taginfo(taginfo);  */
				*ppos = end_paragraph(*ptxtobj, *ppos);
			}
			break;
		case 'p':		/*tag is p */
			if (!(*pmode & STOP)) {

/*				if (taginfo->atts)
					unmatched_P = copy_taginfo(taginfo);
*/
				*ppos = end_paragraph(*ptxtobj, *ppos);
			}
			break;
	}
	return 0;
}
	class NO_DLL_EXPORT htmlfontstack;
        DEFINE_MFLEX_CLASS(htmlfontstack,style *,5);
        static htmlfontstack fontstack;
static long font_func(htmltext **ptxtobj, struct htmltaginfo *taginfo, long *ppos, long *pmode) {
    if(*pmode & STOP) {
        size_t top=fontstack.GetN();
        if(top==0) return 0;
        top--;
        taginfo->thestyle=fontstack[top];
        fontstack.Remove(top, (size_t)1);
    } else {
        style **ns=fontstack.Append();
        const char *sizestr=NULL;
        const char *colorstr=NULL;
        htmlatts *sizeatt=(taginfo->atts)->
          GetAttribute("size");
        if(sizeatt) sizestr=sizeatt->value;
        htmlatts *coloratt=(taginfo->atts)->
          GetAttribute("color");
        if(coloratt) colorstr=coloratt->value;
        static char stylebuf[100];
        stylebuf[0]='\0';
        int size=5;
        if(sizestr!=NULL) {
            switch(sizestr[0]) {
                case '+':
                    sizestr++;
                    // fall through
                case '-':
                    size+=atoi(sizestr);
                    break;
                default:
                    size=atoi(sizestr);
                    break;
            }
            if(size<1) size=1;
            if(size>7) size=7;
            strcat(stylebuf, "SizeX");
            stylebuf[4]=size+'0';
        }
        if(colorstr) {
            if(stylebuf[0]!='\0') strcat(stylebuf, "-");
            strcat(stylebuf, "Color=");
            strncat(stylebuf, colorstr, sizeof(stylebuf)-1-strlen(stylebuf));
            stylebuf[sizeof(stylebuf)-1]='\0';
        }
        class style *s = ((*ptxtobj)->GetStyleSheet())->Find(stylebuf);
        if (!s) {
            // whoah! template is missing a style,
            //   let's CREATE one to avoid losing the markup!
            s = new style;
            if (s) {
                s->SetName(stylebuf);
                if(sizestr) s->SetFontSize(style_ConstantFontSize, 4+(size-1)*2);
                else s->SetFontSize(style_PreviousFontSize, 0);
                if(colorstr) s->AddAttribute("color", colorstr);
                ((*ptxtobj)->GetStyleSheet())->Add(s);
            }
        }
        *ns=s;
    }
    return 0;
}

/* specifics for footnote tag */
	static long
footnote_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	long temppos = taginfo->startpos;
	if (*pmode & STOP) {
		fnote *fn = new fnote;
		if (' ' == (*ptxtobj)->GetChar(temppos)) ++temppos;
		if (*ppos - temppos > 0) {	// can't be too cautious
			(*ptxtobj)->AddView(temppos,"fnotev",fn);
			(fn)->addenv(*ptxtobj,temppos);   // inserts a char
			((htmlenv *)fn->env)->Remove(temppos, 1,
					environment_Style, FALSE);
			fn->env = (environment *)
					((htmlenv *)
					((*ptxtobj)->text::rootEnvironment))
					->WrapStyle(temppos,
						*ppos-temppos +1, fn->vstyle);
			((htmlenv *)(fn->env))->
					SetStyle(FALSE, FALSE);
			*ppos = temppos+1;
			(fn)->Close(*ptxtobj);
		}
	}
	return 0;
}

/* specific stuff for header tag to do */
	static long
header_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	if (!(*pmode & (LITERAL|STOP))) {
		*ppos = end_line(*ptxtobj, *ppos);
		taginfo->startpos = *ppos;
	}
	if (*pmode & STOP && !(*pmode & LITERAL))
		*ppos = end_paragraph(*ptxtobj, *ppos)-1;
	return 0;
}

/* titleinset_func is used for <HEAD>, <TITLE>,  etc,
	which don't yet have native support, but are recognizable
*/
	static long
titleinset_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	static htmltext *oldtxtobj =NULL;
	char tag[32];
	int taglen;

	if (!(*pmode & STOP)) {
		oldtxtobj = *ptxtobj;
		*ptxtobj = insert_hidden(oldtxtobj, ppos);
		sprintf(tag, "<%s>", taginfo->tagid);
		taglen = strlen(tag);
		((hidden *)*ptxtobj)->InsertCharacters(0, tag, taglen);
		*ppos = taglen;
		return RAW_TEXT;
	} else {
	    hidden *ht=(hidden *)*ptxtobj;
	    sprintf(tag, "</%s>", taginfo->tagid);
	    const long taglen=7; // <title>
	    long len=ht->GetLength()-taglen;
	    ht->InsertCharacters(*ppos, tag,   strlen(tag));
	    char *t=ht->GetBuf(taglen, len , &len);
	    if(oldtxtobj->title) free(oldtxtobj->title);
	    oldtxtobj->title=(char *)malloc(len+1);
	    if(oldtxtobj->title) {
		strncpy(oldtxtobj->title, t, len);
		oldtxtobj->title[len]='\0';
	    }
	    *ptxtobj = oldtxtobj;
	}
	return 0;
}
/* rawinset_func is used for <HEAD>, <TITLE>,  etc,
	which don't yet have native support, but are recognizable
*/
	static long
rawinset_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	static htmltext *oldtxtobj =NULL;
	char tag[32];
	int taglen;

	if (!(*pmode & STOP)) {
		oldtxtobj = *ptxtobj;
		*ptxtobj = insert_hidden(oldtxtobj, ppos);
		sprintf(tag, "<%s>", taginfo->tagid);
		taglen = strlen(tag);
		((hidden *)*ptxtobj)->InsertCharacters(0, tag, taglen);
		*ppos = taglen;
		return RAW_TEXT;
	} else {
	    long p=strlen(taginfo->tagid)+2;
	    struct SearchPattern *pat=NULL;
	    if(pat==NULL) search::CompilePattern( "<title>.*</title>", &pat);
	    int result= search::MatchPattern((hidden *)*ptxtobj, p, pat);
	    if(result>=0) {
		long matchlen= search::GetMatchLength()-7-8;
		if(oldtxtobj->title) free(oldtxtobj->title);
		oldtxtobj->title=(char *)malloc(matchlen+1);
		((hidden *)*ptxtobj)->CopySubString(result+7, matchlen, oldtxtobj->title, FALSE);
	    }
	    sprintf(tag, "</%s>", taginfo->tagid);
		((hidden *)*ptxtobj)->InsertCharacters(*ppos, tag,
				strlen(tag));
		*ptxtobj = oldtxtobj;
	}
	return 0;
}

	static long
img_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	const char *src;
	htmlenv *env;
	attlist *atts;

	atts = taginfo->atts; /* parseparms(tag+3); */
	src = getatt(atts, "src");
	if (src)
		env = (*ptxtobj)->AddImage(*ppos, src, atts);
	else
		env = NULL;
	if (env) {
		(env)->SetAttribs(atts);
		++(*ppos);
	} else {
		/* couldn't create an image inset,
				put it in a hidden inset instead */
		hidden *hiddenobj = insert_hidden(*ptxtobj, ppos);
		if (hiddenobj) {
			const char *tag = build_first_tag(taginfo->atts,
					taginfo->tagid);
			(hiddenobj)->InsertCharacters(0, tag, strlen(tag));
		}
	}
	return 0;
}

	static long
lit_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	long retval = 0;
	if ( ! (*pmode & STOP)) retval = LITERAL;
	return retval;
}

	static long
list_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode)  {
	htmlenv *env = NULL;
	boolean handled_dt = FALSE;
	static struct htmltaginfo *pendingDx =NULL;

	switch (taginfo->tagid[0]) {
		case 'd':		/* tag for dl, dt and dd */
			if (pendingDx) {
				end_this_style(*ptxtobj, pendingDx, ppos);
				if (strcmp(pendingDx->tagid, "dt") ==0) {
					/* (*ptxtobj)->InsertCharacters((*ppos)++,
							"\t", 1);    */
					handled_dt = TRUE;
				}
				deleteTag(pendingDx);
				pendingDx = NULL;
			}
			if (taginfo->tagid[1] == 't') {		/* dt tag */
				*ppos = end_line(*ptxtobj, *ppos);
				taginfo->startpos = *ppos;
				pendingDx = copy_taginfo(taginfo);
				break;
			}
			else if (taginfo->tagid[1] == 'l') {	 /* dl tag */
				*ppos = end_line(*ptxtobj, *ppos);
			}
			else if (taginfo->tagid[1] == 'd') {	/* dd tag */
				if (!handled_dt) {
					*ppos = end_line(*ptxtobj, *ppos);
			}
				if ((taginfo->atts)->Size() > 0) {
					/* there's an ID="_" attribute
					  (or something) on this <DD> tag,
					  so slap it into an **anchortarget** */
					pendingDx = copy_taginfo(taginfo);
				}
			}
			break;
		case 'o':		/* tag for ordered */
		case 'u':		/* tag for unordered */
			*ppos = end_line(*ptxtobj, *ppos);
			if (!(*pmode & STOP))
				taginfo->startpos = *ppos;
			break;
		case 'l':		/* tag for li */
			*ppos = end_line(*ptxtobj, *ppos);
			env = (*ptxtobj)->AddDingbat(*ppos,
					listtype_UNORDERED, NULL);
			*ppos += (env)->GetLength();
			break;
	}
	return 0;
}

	static long
quote_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
			long *ppos, long *pmode)  {
	*ppos = end_paragraph(*ptxtobj, *ppos);
	return 0;
}

	static long
ignore_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
			long *ppos, long *pmode)  {
	static long startloc;
	long retval = 0;
	if (*pmode & STOP) {
		htmltext *t = *ptxtobj;
		// excise the <!-- and --> already inserted in *ptxtobj
		if (t->GetChar(startloc) == '<'
				&& t->GetChar(startloc+1) == '!'
				&& t->GetChar(startloc+2) == '-'
				&& t->GetChar(startloc+3) == '-')
			t->AlwaysDeleteCharacters(startloc, 4);
		if (t->GetChar(*ppos-3) == '-'
				&& t->GetChar(*ppos-2) == '-'
				&& t->GetChar(*ppos-1) == '>')
			t->AlwaysDeleteCharacters(*ppos-3, 3);
	}
	else {
		startloc = *ppos;
		retval = LITERAL;
	}
	return retval;
}
        
static struct table_rock {
    short newrow, newcolumn;
    boolean border;
    table *tbl;
    htmltext *oldtxtobj;
    table_rock *next;
} *table_rocks=NULL;     

static void FixupTable(table *t) {
    int r,c;
    for(r=0;r<t->NumberOfRows();r++) {
        for(c=0;c<t->NumberOfColumns();c++) {
            cell *tc=t->GetCell(r,c);
            if(tc->celltype!=table_ImbeddedObject) continue;
            dataobject *obj=tc->interior.ImbeddedObject.data;
            if(obj==NULL) continue;
#if 1
            ATK_CLASS(htmltext);
            if(!obj->IsType(class_htmltext)) continue;
            htmltext *ht=(htmltext *)obj;
            if(ht->GetLength()!=1) continue;
            environment *env=(environment *)ht->rootEnvironment->GetInnerMost(0);
            if(env->type!=environment_View) continue;
            viewref *vr=env->data.viewref;
            if(vr==NULL) continue;
            dataobject *eobj=vr->dataObject;
            if(eobj==NULL) continue;
            eobj->Reference();
            obj->Destroy();
            tc->interior.ImbeddedObject.data=eobj;
            eobj->AddObserver(t);
#else
            dataobject *eobj=NewFixupText(obj);
            if(obj!=eobj) {
                obj->Destroy();
                tc->interior.ImbeddedObject.data=eobj;
                eobj->AddObserver(t);
            }
#endif
        }
    }
}


	static long
table_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
			long *ppos, long *pmode)  {
	long retval = 0;
	style *globalstyle = NULL;
	const char *alignment =NULL;
        
	switch (taginfo->tagid[1]) {
	case 'a': /* tag for table */
            if (!(*pmode & STOP)) {
                table_rock *nr=new table_rock;
                nr->oldtxtobj = *ptxtobj;
                retval = TABLE;
                nr->tbl = insert_table(*ptxtobj, ppos);
                nr->newrow = 1;
                nr->newcolumn = 0;
                htmlatts *borderatt=(taginfo->atts)->
                  GetAttribute("border");
                nr->border =  (borderatt != NULL) ? atoi(borderatt->value):0;
                nr->next=table_rocks;
                table_rocks=nr;
            } else {
                FixupTable(table_rocks->tbl);
                if(table_rocks->oldtxtobj) *ptxtobj = table_rocks->oldtxtobj;
                table_rock *lr=table_rocks;
                if(lr) table_rocks=lr->next;
                delete lr;
            }
		break;
	case 'h': /* tag for th */
	case 'd': /* tag for td */
            if (!(*pmode & TABLE) || table_rocks==NULL) return CATASTROPHE;
		/* add a column if needed and insert the stuff */
		if (++(table_rocks->newcolumn) > (table_rocks->tbl)->NumberOfColumns())
			(table_rocks->tbl)->ChangeSize((table_rocks->tbl)->NumberOfRows(),
					table_rocks->newcolumn);
		*ptxtobj = add_cell(*ptxtobj, table_rocks->tbl, table_rocks->newrow, &table_rocks->newcolumn,
				taginfo, table_rocks->border);
		*pmode &= ~SPACE_NEEDED;
		/* modify the global style */
		globalstyle = (*ptxtobj)->GetGlobalStyle();
		if (taginfo->tagid[1] == 'h')
			(globalstyle)->AddNewFontFace(fontdesc_Bold);

		alignment = getatt(taginfo->atts, "align");
		if (!alignment) alignment= "unspecified";
		switch (tolower(alignment[0])) {
		case 'c':
		    if (cistrcmp(alignment, "center") == 0)
			globalstyle->SetJustification(style_Centered);
  		    break;
		case 'j':
			if (cistrcmp(alignment, "justified") == 0)
				(globalstyle)->SetJustification(
						style_LeftAndRightJustified);
			break;
		case 'l':
			if (cistrcmp(alignment, "left") == 0)
				(globalstyle)->SetJustification(
						style_LeftJustified);
			break;
		case 'r':
			if (cistrcmp(alignment, "right") == 0)
				(globalstyle)->SetJustification(
						style_RightJustified);
			break;
		default:
			break;
		}
		break;
	    case 'r': /* tag for tr */
		if (!(*pmode & TABLE) || table_rocks==NULL) return CATASTROPHE;
		/* add a row if needed */
		if (table_rocks->newcolumn<1)
		    break; /* ignore presence of empty rows (especially if it's the first thing in the table) */
		if (++table_rocks->newrow > (table_rocks->tbl)->NumberOfRows())
			(table_rocks->tbl)->ChangeSize(table_rocks->newrow,
					(table_rocks->tbl)->NumberOfColumns());
		table_rocks->newcolumn = 0;
		break;
	    default: /* uh-oh, there must be something in the tagtable that didn't get implemented here yet */
		return CATASTROPHE;
		break;
	}
	return retval;
}

static htmlform *currentform = NULL;

#if 0 /* use commented out below */
// global variables for <SELECT> and <OPTION>
static htmltext *selectattrs = NULL;
#endif

// <form> creates htmlform object
// </form> Completes() htmlform
// both insert 'hidden' inset for the tag
//
	static long
form_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode) {
	char buff[300];

	if ( ! (*pmode & STOP)) {
		if (currentform != NULL) {
			// XXX ERROR - form within form
		}
		currentform = new htmlform(*ptxtobj, taginfo->atts);
		/* build_first_tag assigns to tag, so no need to assign again */
		/* tag = */ build_first_tag(taginfo->atts, taginfo->tagid);
	}
	else {
		currentform->Completed();
		currentform = NULL;
		sprintf(buff, "</%s>", taginfo->tagid);
	}

//	// insert hidden form of tag into text
// can't do this because the "form" style also carries the attibutes
//		XXX need user interface to insert forms
//		XXX need to eliminate either hidden blobs
//		xxx	or eliminate storing attributes in styles
//	hidden *hiddenobj = insert_hidden(*ptxtobj, ppos);
//	const char *tag = buff;
//	if (hiddenobj)
//		(hiddenobj)->InsertCharacters(0, tag, strlen(tag));

	return 0;
}

// <INPUT> calls AddInputNode
// inserts resulting widget into the text
//
	static long
input_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode) {
	dataobject *newobj = currentform
			->AddInputNode(taginfo->atts);
	(*ptxtobj)->AlwaysAddView(*ppos,
			newobj->ViewName(), newobj);
	return 0;
}


static attlist *selatts=NULL;
static AWidget *selwgt=NULL;
// <SELECT> starts collecting list of options
// </SELECT> calls AddSelectNode
//
	static long
select_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
                      long *ppos, long *pmode) {
        static htmltext *selectbase = NULL;
	static long selectpos;
        if (*pmode & SPACE_NEEDED
            && !(*pmode & (RAW_TEXT))) {
            char c= (*ptxtobj)->GetChar(*ppos-1);
            if ( ! isspace(c))
                (*ptxtobj)->InsertCharacters((*ppos)++, " ", 1);
            *pmode &= ~SPACE_NEEDED;
        }
	if ( ! (*pmode & STOP)) {
		// <SELECT>
		if (selectbase != NULL) {
			// XXX ERROR, select within select
                }
                if(selwgt) selwgt->Destroy();
		selectbase = *ptxtobj;
		selectpos = *ppos;
                selwgt=currentform
				->AddSelectNode(taginfo->atts, ptxtobj);
                *ppos = 0;
                *pmode&=~SPACE_NEEDED;
                *pmode|=IGNORE_SPACE;
	}
	else {
            //  </SELECT>
            if(selatts) {
                currentform->ApplyOptionAttrs(selatts);
                selatts->Destroy();
                selatts=NULL;
            }
            if(selwgt) {
                currentform->FinishSelectNode();
                selectbase->AlwaysAddView(selectpos,
                                          selwgt->ViewName(),
                                          selwgt);
                selwgt=NULL;
            }
            *ptxtobj = selectbase;
            *ppos=selectpos;
            selectbase = NULL;
            *pmode&=~IGNORE_SPACE;
            *pmode&=~SPACE_NEEDED;
	}
        return 0;
}


//<OPTION> start saving option text in next optionslist entry
	static long
option_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
            long *ppos, long *pmode) {
#if 0
	char *atts = taginfo->atts->MakeIntoString();
	int attrslen = selectattrs->GetLength();
	selectattrs->AlwaysInsertCharacters(attrslen, ":", 1);
	selectattrs->AlwaysInsertCharacters(attrslen+1, atts,
                                            strlen(atts));
#endif
        if(selatts) {
            (*ptxtobj)->AlwaysInsertCharacters(*ppos, "\n", 1);
            (*ppos)++;
            currentform->ApplyOptionAttrs(selatts);
            selatts->Destroy();
        }
        selatts=taginfo->atts;
        selatts->Reference();
        *pmode&=~SPACE_NEEDED;
        *pmode|=IGNORE_SPACE;
	return 0;
}

// <TEXTAREA>  sets up text to rcv initial value
// </TEXTAREA> calls AddTextareaNode
//  (similar to rawinset_func)
//  XXX should retain original initial value ???
//
	static long
textarea_func(htmltext **ptxtobj, struct htmltaginfo *taginfo,
		long *ppos, long *pmode) {
	static htmltext *basetxtobj = NULL;
	static long basepos;
        static AWidget *wgt=NULL;
	if ( ! (*pmode & STOP)) {
		// <TEXTAREA>
		if (basetxtobj != NULL) {
			// XXX ERROR, textarea within textarea
		}
                if(wgt) wgt->Destroy();
		basetxtobj = *ptxtobj;
		basepos = *ppos;
		*ppos = 0;
                wgt=currentform -> AddTextareaNode(taginfo->atts, ptxtobj);
		// XXX ??? *pmode
		// XXX ??? styles for textarea contents
	}
	else {
            //  </TEXTAREA>
            if(wgt) {
                currentform->FinishNode();
		basetxtobj->AlwaysAddView(basepos, wgt->ViewName(), wgt);
                wgt=NULL;
            }
            *ptxtobj = basetxtobj;
            *ppos=basepos;
            basetxtobj = NULL;
	}
	return 0;
}

// amp_str
//	read token following '&' and process it
//	return a (static) string containing either
//		the translated character or the original string.
//                      The maximum recognized string is 201 bytes including the '&' and NUL.
//	*plen is set to length of returned string
	static char *
amp_str(FILE *file, int *plen) {
            
	static char sym[201], *symx;
	int c;
	unsigned int len=0;

	symx = sym;
        *symx++ = '&';
        *symx='\0';
        *plen=1;
        
        if(!GETANDTEST(c,file)) return sym;
            
        if(c=='#') {
            *symx++='#';
            *symx='\0';
            *plen=2;
            if(!GETANDTEST(c,file)) return sym;
            if(!isdigit(c)) {
                ungetc(c, file);
                return sym;
            }
            do {
                *symx++=c;
            } while(GETANDTEST(c,file) && isdigit(c));
            if(c!=EOF && c!=';') ungetc(c,file);
            sym[0]=atoi(sym+2);
            sym[1]='\0';
            *plen=1;
            return sym;
        }
        if(!isalpha(c)) {
            ungetc(c, file);
            return sym;
        }
        // now find the first matching symbol name
        // XXX should this look for the *LONGEST* matching symbol name?
        // (as written that could be achieved by proper ordering of the symboltable,
        // but this should also be fixed to avoid a linear lookup...) -robr
        int i=-1;
        unsigned int clen=0;
        int n=sizeof(symboltable)/sizeof(struct symbolmap);
        do {
            *symx=c;
            len++;
            // this could be vastly improved, but at the moment I'm too lazy
            // to implement an incremental binary search. -robr
            // see if any symbol matches the name so far.
            for(i=0;i<n;i++) {
                if(strncmp(sym+1, symboltable[i].code, len)==0) break;
            }
            if(i>=n) break; // nope no match
            // on to the next character
            symx++;
            // here we know the first len characters match with the entry at [i]
            // so if the strlen of that entry equals len then we are done.
            clen=strlen(symboltable[i].code);
        } while(len>=sizeof(sym)-2 || (len!=clen && GETANDTEST(c,file) && isalpha(c)));
        *symx='\0';
        // found an exact match, return the symbol code.
        if(i<(int)n && len==clen) {
            sym[0]=symboltable[i].symbol;
            sym[1]='\0';
            if(GETANDTEST(c,file) && c!=';') ungetc(c, file);
            *plen=1;
            return sym;
        }
        // no match
        *plen=strlen(sym);
        // put back the last character (the one which convinced us there was no possible match)
        if(c!=EOF && c!=';') ungetc(c, file);
        // return the sequence verbatim
        return sym;
}



/* The memory leak that used to be here has been fixed by requiring
	the caller to free the storage returned.
*/
	static char *
read_tag(FILE *file)  {
	char *tag =NULL;
	long tagpos =0, taglength =0; //Current length of the tag buffer
	int c;
	/* read in the "tag" */
	tagpos = 0;
	while (GETANDTEST(c,file)) {
		if (tagpos >= taglength-2) {
			taglength += INCREMENTKEYWORDSIZE;
			tag = (char *)saferealloc(tag, taglength);
		}
		if (c == '&') {
			int len;
			char *s = amp_str(file, &len);
			if (tagpos + len > taglength-1) {
				taglength += len +
					INCREMENTKEYWORDSIZE;
				tag = (char *)saferealloc(tag, taglength);
			}
			strcpy(tag+tagpos, s);
			tagpos += len;
		}
		else if (c != '>')
			tag[tagpos++]= c;
		else {  // '>'
			if (strncmp(tag, "!--", 3) == 0
					&& strncmp(tag+tagpos-2, "--", 2) != 0)
				// not "-->", so not end of comment
				tag[tagpos++]= c;
			else break;		// end of tag
		}
	}
	tag[tagpos] = '\0';
	if (strncmp(tag, "!--", 3) != 0) {
		// Convert tags other than comments to all lowercase
		char *s;
		for (s=tag; *s && *s > ' '; s++)
			if (isalpha(*s))  *s = tolower(*s);
	}
	return tag;
}

	static int
tagCompare(const void *t1, const void *t2)  {
	return strcmp(((struct tagmap *)t1)->id,
				((struct tagmap *)t2)->id);
}

	static struct tagmap *
LookupTagById(char *name)  {
	struct tagmap temp;
	temp.id = name;
	return (struct tagmap *)bsearch(&temp,
			tagtable, sizeof(tagtable)/sizeof(struct tagmap),
			sizeof(struct tagmap), tagCompare);
}


/* handle_tag is called by do_read for an incoming htmltag.
	If the tag has a match (eg. <b> </b>) do_read will be called
	and it will return when the match for the tag is found.
*/
	static long
handle_tag(htmltext **ptxtobj, FILE *file, char *term, long *ppos,
			char *tag, long *pmode)  {
	int stop = 0;
	char *key =NULL, justtag[36], *ptr = NULL;
	long addmode =0;
	struct tagmap *tm = NULL;
	struct htmltaginfo *newnode = NULL;

	int taglen = strlen(tag);

	stop = (tag[0] == '/');
	key = &tag[stop ? 1 : 0];

	ptr = skiptokenchars(key);
	if (ptr-key > (int)sizeof(justtag)-1) /* probably a long SGML
			comment with no spaces; prevent overruns */
	    ptr = key + sizeof(justtag)-1;
	strncpy(justtag, key, ptr - key);
	justtag[ptr-key] = '\0';

	// at this point
	//	justtag is the lowercase ascii of the first word in tag
	//		(it was lowercased at the end or read_tag)
	//	stop is TRUE iff the tag text begins with /

	if (*pmode & RAW_TEXT) {
		if (stop && term && strcmp(term, justtag) ==0) {
			/* mode |= STOP; */
			return DONE;
		}
		(*ptxtobj)->InsertCharacters(*ppos, "<>", 2);
		(*ptxtobj)->InsertCharacters(*ppos+1,
				tag, taglen);
		*ppos += (2+taglen);
		return 0;
	}

	if (stop) {
		if (term && strcmp(term, justtag) == 0) {
			// found the end of a style opened earlier
			return DONE;
		}
		else goto CouldntHandleTag;
	}

	tm=LookupTagById(justtag);

	if ( ! tm)		// unknown tag
		goto CouldntHandleTag;

	newnode = makeTagNode(*ptxtobj, tm->stylename?tm->stylename:justtag, *ppos, key);
	if (! newnode)
		// malloc failed
		goto CouldntHandleTag;

	if (tm->function) {
		addmode = tm->function(ptxtobj, newnode,
				ppos, pmode);
		if (addmode==CATASTROPHE) {
			deleteTag(newnode);
			goto CouldntHandleTag;
		}
		*pmode |= addmode;	// SET added mode bits
	}

	if (tm->endtag == PAIR) {
		*ppos = do_read(*ptxtobj, file, justtag, pmode);
		*pmode &= ~addmode; //reset added mode bits
		/* NOTE:  SPACE_NEEDED could have
		   been set in the recursive call to do_read(); */
		if (tm->function) {
			long temp;
		 	*pmode |= STOP;
			temp = tm->function(ptxtobj, newnode,
				ppos, pmode);
			*pmode &= ~STOP;
			if (temp==CATASTROPHE) {
				deleteTag(newnode);
				goto CouldntHandleTag;
		    	}
		}
		end_this_style(*ptxtobj, newnode, ppos);
		if (newnode->thestyle && (newnode->thestyle)->
					IsAttribute("end_paragraph"))  {
			// "end_paragraph" is set for `center' in html.tpl
			*ppos = end_paragraph(*ptxtobj, *ppos);
		}
	}
	deleteTag(newnode);
	return 0;

CouldntHandleTag: ; /* jump here if function met with
		some catastrophe (like a <tr> not inside a <table>) */

	hidden *hiddenobj;
	hiddenobj = insert_hidden(*ptxtobj, ppos);
	hiddenobj->InsertCharacters(0,"<>", 2);
	hiddenobj->InsertCharacters(1,tag, strlen(tag));
	return 0;
}


	static long
do_read(htmltext *txtobj, FILE *file, char *termstr, long *pmode)  {
	char *tag=NULL, temp=0;
	int c=0;
	long pos=(txtobj)->GetLength();
	long newmode=0;

	while (GETANDTEST(c,file))  {
            if(c!=' ' && c!='\t' && c!='\n') *pmode&=~IGNORE_SPACE;
		switch (c) {
		case '<':
			/* read the stuff inside of the <>  */
			if (GETANDTEST(c,file) &&
					(isspace(c) || c == '>' || c == '<')) {
				(txtobj)->InsertCharacters(pos++, "<", 1);
				ungetc(c,file);
				continue;
			}
			ungetc(c,file);
			tag= read_tag(file);

			newmode= handle_tag(&txtobj, file, termstr,
				&pos, tag, pmode);
			if(tag) free(tag);
			/* might have inserted some characters so.... */
			pos=(txtobj)->GetLength();

			if (termstr && newmode & DONE) return pos;
			continue;

		case '&':
			if (*pmode & RAW_TEXT) break;

			/* the semantics have changed slightly:
			  the sequence ampersand space will now
			  be preceded by a space if SPACE_NEEDED
			  was set. */

			// put in a space, if needed

                        if (*pmode & SPACE_NEEDED)
                            if ( ! isspace((txtobj)-> GetChar(pos-1)))  (txtobj)->InsertCharacters(pos++, " ", 1);
                        *pmode &= ~SPACE_NEEDED;
			{
				int len;
				char *sym = amp_str(file, &len);
				(txtobj)->InsertCharacters(pos, sym, len);
				pos += len;
			} continue;

		case ' ':
		case '\t':
		case '\n':
                    if(*pmode & IGNORE_SPACE) continue;
                    if (*pmode & (LITERAL|RAW_TEXT)) break;
                    if (!(*pmode & SPACE_NEEDED))
                        *pmode |= SPACE_NEEDED;
                    continue;
		}
		if (*pmode & SPACE_NEEDED
                && !(*pmode & (RAW_TEXT))) {
			char c= (txtobj)->GetChar(pos-1);
			if ( ! isspace(c))
                            (txtobj)->InsertCharacters(pos++, " ", 1);
                        *pmode &= ~SPACE_NEEDED;
		}

		temp= (char) c;
		(txtobj)->InsertCharacters(pos++, &temp, 1);
	}
	return pos;
}



#define MAXENVSTACK 100

	long
htmltext::Read(FILE *file, long id)  {
	struct envelt {
		htmlenv *environment;
		long pos;
	};
	long retval;

	this->readonly= GetReadOnly();
	SetReadOnly(FALSE);

	if (id == 0 && fileno(file) >= 0) {
		/* html file */
		struct envelt envStack[MAXENVSTACK],
				*envptr;
		htmlenv *rootenv;
		long len, mode= 0;

#if 0 /* old robr memory leakage fix. */
		delete rootEnvironment;
		this->text::rootEnvironment= (environment *)
				htmlenv::GetTextRootEnvironment(this);
#else
			this->Clear(); /* text::Read calls ClearStyles, so... */
#endif
		if (this->text::templateName != NULL)
			ReadTemplate(this->text::templateName, FALSE);

		envptr= envStack;

		rootenv= (htmlenv *)
				((htmlenv *)this->text::rootEnvironment)->
							GetEnclosing(0);
		envptr->environment= rootenv;
		envptr->pos= (rootenv)->Eval();

		len= do_read(this, file, NULL, &mode);

		if (envptr != envStack)  {
			fprintf(stderr,
				"Not all environments were closed. - Closing them by default\n");

			while (envptr != envStack)  {
				(envptr->environment)->
						SetLength(len - envptr->pos);
				--envptr;
			}
		}

#ifdef SMARTSTYLES
		/* the following code is for smart styles*/
		if (rootenv)
			htmlenv_RegionModified(rootenv,
				environment_ReadChange, 0, len);

		htmlenv_RegionModified(
			(htmlenv *)this->text::rootEnvironment,
			environment_ReadFileChange, 0, GetLength());
#endif /*SMARTSTYLES*/

		retval= dataobject::NOREADERROR;
			/*XXX-this is a dumb assumption. We should've
				been checking this in do_read somehow.
				But all we get back is "len", and being
				zero isn't necessarily a failure */
	}
	else {
		/* atk data stream */
		retval= (this)->text::Read(file, id);
		/* stupid text::Read smashed our htmlenv and replaced it with an environment.  We can't fix it HERE or we lose all the example styles that were in the template (if that's what we're reading).  Added a hack to htmltext::ReadSubString */
	}
	SetReadOnly(this->readonly);
	return retval;
}
#undef MAXENVSTACK




/* WRITE HTML

==========================================

	The definitions and functions in this section are used for
	writing files in HTML format.
*/

static int linelength;  // output line length (ignore recursion)
		// truncate lines by inserting newline for
		// first space after 65 characters.

static long dowrite(htmltext *self, FILE *file, htmlenv *parenv,
		long parpos, long parlen, long nlflags);
static void write_table(htmltext *self, table *table, FILE *file);


	static void
putnl (FILE *file) {
	putc('\n', file);
	linelength = 0;
}


// XXX- should be in color.H/C
	static void
ColorName(char *name, char *givenname, unsigned char rgb[3]) {
	if (givenname)
		strcpy(name, givenname);
	else
		sprintf(name, "#%02x%02x%02x",
				rgb[0], rgb[1], rgb[2]);
}

	static long
write_inset(htmltext *txtobj, dataobject *inset, FILE *file,
		long pos, long nlflags, htmlenv *env)  {
	const char *begin_tag;
	char end_tag[36];
	htmlform *form;

	if (inset->IsType("hr")) {
		fprintf(file, "\n<hr>");
		if (txtobj)
			/* skip past inset and possible nls */
			pos+= 1 + ((txtobj)->GetChar(pos+1)=='\n')
				+ ((txtobj)->GetChar(pos+2)=='\n');
	}
	else if (inset->IsType("imageio")) {
		if (env) {
			begin_tag = build_first_tag(env->attribs, "img");
			fprintf(file, "%s", begin_tag);
		} else fprintf(file, "Image inset was here.");
		if (txtobj) ++pos;
	}
	else if (inset->IsType("AWidget") &&
			(form=(htmlform *)(void *)*(((AWidget *)inset)
				->Get(slot_parentform)))) {
		// it is part of a form iff `parentform' attribute is present
		if (form)
			form->WriteTaggedText(file, (AWidget *)inset);
		if (txtobj) ++pos;
	}
	else if (inset->IsType("img")) {
		if (!env) /*XXX- why this check??? */
			fprintf(file, "Image inset was here.\n");
		if (txtobj) ++pos;
	}
	else if (inset->IsType("bp")) {
		fprintf(file, "\n<hr>\n\n");
		if (txtobj) ++pos;
	}
	else if (inset->IsType("hidden")) {
		// originally, putnl's were skipped for non-normal nlflags -wjh
		if (linelength > 0) putnl(file);
		((hidden *)inset)->WriteSubString(0,
			((hidden *)inset)->GetLength(), file, FALSE);
		putnl(file);
		if (txtobj) ++pos;
	}
	else if (inset->IsType("fnote")) {
		if (!((fnote *)inset)->IsOpen()) {
			/* copy the fnote(text) inset's contents into an 				htmltext object and write that out */
			htmltext *htxt= new htmltext;
			((fnote *)inset)->SetCopyAsText(TRUE);
			((fnote *)inset)->SetExportEnvironments(TRUE);
			(htxt)->AlwaysCopyTextExactly(0, (fnote *)inset,
				0, ((fnote *)inset)->GetLength());
			dowrite(htxt, file,
				(htmlenv *)htxt->text::rootEnvironment, 0,
				(htxt)->GetLength(), WNL_Normal);
			(htxt)->Destroy();
		}
		if (txtobj) ++pos;
	}
	else if (inset->IsType("table")) {
		int border= 0;
		strcpy(end_tag, "</table");

		/* check to see if there is a border */
		border= ((table *)inset)->ColorAbove(0, 0) == BLACK;

		fprintf(file, "<%s%s>", end_tag+2, (border)?" border":"");
		strcat(end_tag, ">");
		write_table(txtobj, (table *)inset, file);
		fprintf(file, "%s", end_tag);
		if (txtobj && '\n'==(txtobj)->GetChar(++pos)) ++pos;
	}
	else if (inset->IsType("urlbutton")) {
		const char *urlt= ((urlbutton *)inset)->GetURLLabel(),
			*urlb= ((urlbutton *)inset)->GetURL();
		if (urlt==NULL || urlt[0]=='\0')
			urlt= urlb;
		if (txtobj) ++pos;
		fprintf(file, "<a href=\"%s\">%s</a>", urlb, urlt);
	}
	else if (inset->IsType("link")) {
		link *lk = (link *)inset;
		char extraargs[200], FGColor[200], BGColor[200],
			*tcolornm;
		unsigned char colorRGB[3];
		tcolornm = lk->GetFGColor(colorRGB);
		ColorName(FGColor, tcolornm, colorRGB);
		tcolornm = lk->GetBGColor(colorRGB);
		ColorName(BGColor, tcolornm, colorRGB);

		sprintf(extraargs,
			" FGColor=%s  BGColor=%s  Style=%d",
			FGColor, BGColor, lk->GetStyle());
		char *dest = lk->GetRawLink();
		if (strncmp(dest, "href=", 5)==0) dest +=5;
		if (*dest == '"') dest++;
		char *last = dest+strlen(dest)-1;
		if (*last == '"') last--;
		fprintf(file, "<a href=\"%.*s\"%s>%s</a>",
					(int)(last-dest+1), dest, extraargs,
					lk->GetSafeText());
		if (txtobj) ++pos;
	}
	else { /* I don't recognize the inset so....  skip it */
		fprintf(file, "[A %s INSET WAS HERE]\n",
					inset->GetTypeName());
		if (txtobj) ++pos;
	}
	return pos;
}

#define TAGINCREMENT 32
#define ENSURESIZE(x) { \
	if (!tag || x > tag_len) { \
		tag_len= (x+TAGINCREMENT); \
		tag= (char *)saferealloc(tag, tag_len); \
	} }
#define ENSUREADDLSIZE(x) {\
	if (!tag || x+1+strlen(tag)+1 > tag_len) {\
		tag_len+= (x+1+TAGINCREMENT); \
		tag= (char *)saferealloc(tag, tag_len); \
	} }

	static void
write_table(htmltext *self, table *table, FILE *file)  {
	short row= 0, column= 0, rowspan= 0, colspan= 0;
	short numrows= (table)->NumberOfRows();
	short numcolumns= (table)->NumberOfColumns();
	struct cell *cell;
	char *tag=NULL;
	unsigned long tag_len= 0;

	for (row=0; row<numrows; ++row) {
	    if (row != 0) fprintf(file, "<tr>\n");
	    for (column=0; column<numcolumns; ++column) {
		if ((table)->IsJoinedToAnother(row, column)) continue;
		/* find out if we are "spanning" cells */
		rowspan=1;
		while (row+rowspan<numrows &&
					(table)->IsJoinedAbove(row+rowspan,
						 column))
			++rowspan;
		colspan=1;
		while (column+colspan<numcolumns &&
					(table)->IsJoinedToLeft(row,
						column+colspan))
			++colspan;

		/* start building the tag */
		ENSURESIZE(3);
		strcpy(tag, "td");
		if (rowspan > 1) {
			ENSUREADDLSIZE(strlen(" rowspan=")+9);
				/* we have enough room for the rowspan to be
				999,999,999.  I sure hope that is enough. */
			sprintf(tag, "%s rowspan=%d", tag, rowspan);
		}
		if (colspan > 1) {
			ENSUREADDLSIZE(strlen(" colspan=")+9);
				/* we have enough room for the colspan to be
				999,999,999.  I sure hope that is enough. */
			sprintf(tag, "%s colspan=%d", tag, colspan);
		}
		cell= (table)->GetCell(row, column);

		switch (cell->celltype) {
		case table_EmptyCell:
			/* write out tag */
			fprintf(file, "<%s>\n", tag);
			break;
		case table_TextCell: {
			char *celltext = cell->interior.TextCell.textstring;
			if (celltext && strlen(celltext) > 0) {
				/* print the string between literal tags */
				const char *align = "";
				switch(*celltext++) {
				case '\"': align = " align=right";  break;
				case '\'': align = " align=left";  break;
				case '^': align = " align=center";  break;
				default : celltext--;	break;	// no align flag
				}
				fprintf(file, "<%s%s>%s\n", tag, align, celltext);
			} else
				fprintf(file, "<%s>\n", tag);
			} break;
		case table_ValCell: {
			ENSUREADDLSIZE(strlen(" align=right"));
			strcat(tag, " align=right");

			/* if the cell formula looks like a number then just 				output the formula (which is a string)
			   otherwise grab the extended_value.standard 				(which is the formula evaluated) and output 				that (it happens to be a double, so convert it to 				a string before you output it) */


			if (isdigit(cell->interior.ValCell.formula[0])
					|| '-' ==
					   cell->interior.ValCell.formula[0]) {
				fprintf(file, "<%s>%s\n", tag,
						cell->interior.ValCell.formula);
			} else
			{
				char buff[64];
				int i;
				sprintf(buff, "%f", cell->interior.ValCell
						.value.extended_value.standard);
				i= strlen(buff);
				while (i-1>0 && buff[i-1] == '0')
					--i;   /* get rid of all those zeros */
				if (buff[i-1] == '.')
					/* if decimal is left get rid of it too */
					--i;
				buff[i]= '\0';
				fprintf(file, "<%s>%s\n", tag, buff);
			}
			} break;
		case table_ImbeddedObject: {
			dataobject *dobj =
					cell->interior.ImbeddedObject.data;
			if ( ! file) {
				/* there is no file */
				/* NOT Sure what to do here */
				/* print an error message or something ? */
				break;
			}
			if ( ! (dobj)) {
				fprintf(file,
					"<%s>MISSING DATA OBJECT!",
					tag);
				break;
			}
			if ( ! dobj->ATK::IsType("text")) {
				// Embedded object is not text
				fprintf(file, "<td>");
				/* call write_inset with NULL text object,
					0 for position, normal write nlflags,
					and NULL for environment */
				write_inset(NULL, dobj, file, 0,
						WNL_Normal, NULL);
				putnl(file);
				break;
			}
			// Embedded object is a text

			text *txt= (text *) dobj;
			htmltext *htxt=NULL;
			environment *env=NULL;
			enum style_Justification justify
				= style_PreviousJustification;
			boolean bold= FALSE;

			int htmltextobj =
				dobj->IsType("htmltext");

			if ( ! htmltextobj){
				/* NOT  */
				int iscode= dobj->IsType("srctext");
				htxt= new htmltext;
				/* copy the text and styles into the htmltext
					object, and let it _Write itself */
				(txt)->SetCopyAsText(TRUE);
				(txt)->SetExportEnvironments(TRUE);
				(htxt)->AlwaysCopyTextExactly(0, txt, 0,
						(txt)->GetLength());
				if (iscode) {
					/* source code; make sure the <CODE>
							tag gets put around it */
					stylesheet *ss= (htxt)->GetStyleSheet();
					style *sty;
					sty= (ss)->Find("code");
					if (!sty) {
						/* _create_ one; we can't live
							without a code style */
						sty= new style;
						(sty)->SetName("code");
						(ss)->Add(sty);
					}
					(htxt)->AddStyle(0,
						(htxt)->GetLength(), sty);
				}
				txt=(text *)htxt;
			}
			env= (environment *)
					((environment *)txt->rootEnvironment)
						->GetInnerMost(1);
			while (env/* != txt->rootEnvironment*/) {
				if (justify == style_PreviousJustification)
					justify = (env->data.style)->
						GetJustification();
				if (htmltextobj &&
						(env->data.style)->
						IsAnyAddedFontFace
								(fontdesc_Bold))
					bold= TRUE;
				env= (environment *)(env)->GetParent();
			}

			/* add the align attribute to the tag if necessary */
			switch (justify) {
			case style_LeftJustified:
				ENSUREADDLSIZE(strlen(" align=left"));
				strcat(tag, " align=left");
				break;
			case style_RightJustified:
				ENSUREADDLSIZE(strlen(" align=right"));
				strcat(tag, " align=right");
			case style_Centered:
			    ENSUREADDLSIZE(strlen(" align=center") +1);
			    strcat(tag, " align=center");
			    break;
			default:
				break;
			}

			if (bold) tag[1]= 'h'; /* change the 'td' tag to 'th' */
			fprintf(file, "<%s>", tag);

			((htmltext *)txt)->Write (file,
						(self)->GetWriteID(), 0);
			if (!htmltextobj)
				(htxt)->Destroy();

			} break;	// end case table_ImbeddedObject
		}  // end switch (celltype)
	    }  // end col
	}  // end row

	if (tag) free(tag);
}
#undef TAGINCREMENT
#undef ENSURESIZE
#undef ENSUREADDLSIZE

/* writesymbols is called to output the contents of
	a **symbol** or **symbola** style
*/
	static long
writesymbols(FILE *file, htmltext *self, long pos, long len,
		const char *stylename)  {
	long i;
	int ch;
	const struct symbolmap *sym;
	for (i=0; i<len; ++i,++pos) {
		ch = (self)->GetChar(pos);
		switch (ch) {
		case ' ': case '\t': case '\n':
			break;
		case '7':
			if (stylename[6]=='a') {
				// symbola
				fprintf(file, "<li>");
					/*XXX- this should be "&#149;",
					but Mosaic doesn't support that.
					It does display <LI> tags as bullets,
					even outside of lists
					Sigh. Netscape, too.  -wjh */
				linelength += 4;
				break;
			}
			/* else fall through */
		default:
			sym= LookupNameBySymbol(ch, stylename);
			if (sym)
				fprintf(file, "&%s;", sym->code);
			else
				fprintf(file, "&#%d;", ch);	// might be right ?!
			linelength += 6;	// an estimate
		}
	}
	return pos;
}


// /////////////////////
// WStyle
//	Control table for writing styled text as HTML

// Write-newline flags 	(defined above)
//	WNL_Normal == 0
//	WNL_Literal == (1<<0)
//	WNL_brATnl == (1<<1)
//	WNL_ParaIndent == (1<<2)

// header flag

#define WF_HEADER	(1<<4)	/* it is a header */

// newline control flags

#define WF_BegintagNL	(1<<5)	/* \n before start tag */
#define WF_EndtagNL	(1<<6)	/* \n after end tag */
#define WF_EndtagPreNL	(1<<7)	/* \n before end tag */

#define WF_AlltagNL (WF_BegintagNL | WF_EndtagNL | WF_EndtagPreNL)

// action flags

#define WA_NOP		(1<<8)	// no output for this style 
#define WA_DD		(1<<9)	// dt style -> <dt>...<dd> 
#define WA_SYMLI		(1<<10)	// symbol to <li> 
#define WA_SYMW		(1<<11)	// call write_symbol 
#define WA_INDENT	(1<<12)	// surround:  <dl><dd>... </dl>
#define WA_ADDDT	(1<<13)	// add <dt> after begin tag 
#define WA_USECODE	(1<<14)	// use CodesToStyles 

struct write_tbl {
	const char *stylename;
	const char *tag;
	int flags;
};

// organized by stylename.
//	But stylenames are always lower case???!

static const struct write_tbl WStyle[] = {
	{"Annotation", NULL, WA_NOP},
	{"Bold", "b", 0},
	{"ChapterHeading", "h2", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"ContParagraph", NULL, WA_NOP},
	{"Box", "samp", WNL_Literal }, // not  literal in HTML+ spec
	{"DefinitionDescription", NULL, WA_NOP},
	{"DefinitionList", "dl", WF_AlltagNL | WNL_brATnl},
	{"DefinitionTerm", "dt", WA_DD | WF_EndtagNL},  // endtag is <dd>
	{"DingBat",0, WA_SYMLI},
	{"FootNote", "footnote", 0},
	{"Italic", "i", 0},
	{"Lines", "pre", WNL_Literal},
	{"ListItem", NULL, WA_NOP},
	{"LongQuote", "blockquote", 0},
	{"MajorHeading", "h1", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"NewParagraph", NULL, WA_NOP},
	{"OrderedList", "ol", WF_AlltagNL | WNL_brATnl},
	{"ParagraphHeading", "h5", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"ParameterDescription", NULL, WA_NOP},
	{"ParameterList", "dl", WF_AlltagNL | WNL_brATnl},
	{"ParameterTerm", "dt", WA_DD | WF_EndtagNL},  // endtag is <dd>
	{"Plain", NULL, WA_NOP},
	{"Quote", "q", 0},
	{"Screen", "pre", WNL_Literal},
	{"SectionHeading", "h3", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"SimpleList", "ul", WF_AlltagNL | WNL_brATnl},
		// XXX should add PLAIN attribute to env to suppress
		// bullets, in case someone writes an HTML 3.0 browser
	{"SubChapterHeading", "h2", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"SubParagraphHeading", "h6", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"SubScript", "sub", 0},
	{"SubSectionHeading", "h4", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"SuperScript","sup", 0},
	{"Symbol", "", WA_SYMW},
	{"Symbola", "", WA_SYMW},
	{"UnOrderedList", "ul", WF_AlltagNL | WNL_brATnl},
	{"abbreviation", "abbrev", 0},
	{"acronym", "acronym", 0},
	{"added", "added", 0},
	{"anchor", "a", 0},
	{"anchortarget", "a", 0},
	{"argument", "arg", 0},
	{"bigger", "font size=\"+1\"", 0},
	{"black", "font color=#000000", 0},
	{"blue", "font color=#0000ff", 0},
	{"bold", "b", 0},
	{"center", "center", 0},
	{"changebar", "changed", 0},
	{"chapter", "h2", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"citation", "cite", 0},
	{"code", "code", WA_INDENT},
	{"command", "cmd", 0},
	{"cyan", "font color=#00ffff", 0},
	{"define", "dfn", 0},
	{"definition", "dl", WF_AlltagNL | WNL_brATnl},
	{"description", "dl", WA_ADDDT | WF_BegintagNL | WF_EndtagNL},
	{"dingbat", "", WA_SYMLI},
	{"emphasize", "em", 0},
	{"example", "pre", WNL_Literal | WA_INDENT},
	{"footnote", "footnote", 0},
	{"form", "form", 0},
	{"global", NULL, WA_NOP},
	{"green", "font color=#00ff00", 0},
	{"header1", "h1", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"header2", "h2", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"header3", "h3", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"header4", "h4", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"header5", "h5", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"header6", "h6", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"heading", "h5", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"helptopic", "a" , 0},
	{"indent", "", WA_INDENT | WF_BegintagNL},
	{"inlinequote", "q", 0},
	{"italic", "i", 0},
	{"keyboard", "kbd", 0},
	{"leftindent", "", WA_INDENT | WF_BegintagNL},
	{"literal", "pre", WA_INDENT | WNL_Literal},
	{"magenta", "font color=#ff00ff", 0},
	{"majorheading", "h1", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"margin", "margin", 0},
	{"ordered", "ol", WF_AlltagNL | WNL_brATnl},
	{"person", "person", 0},
	{"preformatted", "pre", WA_INDENT | WNL_Literal},
	{"programexample", "pre" , WA_INDENT | WNL_Literal},
	{"quotation", "blockquote", 0},
	{"quoteparagraph", "blockquote", 0},
	{"red", "font color=#ff0000", 0},
	{"removed", "removed", 0},
	{"sample", "samp", WA_INDENT | WNL_Literal},
	{"section", "h3", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"smaller", "font size=\"-1\"", 0},
	{"strikethrough", "strike", 0},
	{"strong", "strong", 0},
	{"subheading", "h6", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"subscript", "sub", 0},
	{"subsection", "h4", WF_HEADER | WF_EndtagNL | WNL_brATnl},
	{"superscript", "sup", 0},
	{"symbol", "", WA_SYMW},
	{"symbola", "", WA_SYMW},
	{"term", "dt", WA_DD | WF_EndtagNL}, // end is <dd>
	{"title", "title" , 0},
		/*XXX- Hmm, this would be bad there was an actual <TITLE> tag
		  already in the document somewhere */
	{"typewriter", "tt", 0},
	{"underline", "u", 0},
	{"unordered", "ul", WF_AlltagNL | WNL_brATnl},
	{"variable", "var", 0},
	{"verbatim", "pre", WNL_Literal},
	{"yellow", "font color=#ffff00", 0}
};
/* unsupported
	Example HELPTOPICANCHORSUBST NoteParagraph
	answer caption comment display enumerate flushleft flushright
	function helvetica hidden identifier index indexi invisible
	keyword label note paragraph pragma realtopic sansserif

(The following are deleted in ez2htmlapp.C:
	formatnote helptopic topic
	Annotation FormatNote ListItem Plain InvisibleIndex Index
)
*/

/*
	write parlen characters, starting at parpos, to file
	the style is in parenv
*/
	static long
dowrite(htmltext *self, FILE *file, htmlenv *parenv,
		long parpos, long parlen, long nlflags)  {
	long pos, len, endpos= parpos+parlen;
	htmlenv *env= NULL;
	const char *begin_tag;
	char c;
	const struct symbolmap *entry;
	dataobject *inset;

	pos= parpos;
	while (pos < endpos) {
	    if (parenv && (env=(htmlenv*)parenv->
				GetChild(pos)) != NULL)   {

		// begin processing environment: style or nested object

		len= (env)->GetLength()-(pos-(env)->Eval());

		if (env && env->type == environment_View) {
			// embedded object
			inset = env->data.viewref->dataObject;
			pos = write_inset(self, inset, file, pos,
					WNL_Normal, env);
			putnl(file);
			continue;
		}
		if (env && env->type != environment_Style) {
			// unknown environment, skip it
			fprintf(file, "\n[Document section of %s%ld]",
 				"unknown type skipped here.  Length = ",
				len);
			putnl(file);
			pos += len;
			continue;
		}

		// begin processing styled text

		// get style name
		style *sty = env->environment::data.style;
		const char *name = (sty)->GetName();

		const struct write_tbl *WS = NULL;	   // pt to entry or tstyle
		struct write_tbl tstyle;  // for style w/o entry
		int leftx, rightx, midx;
			// possibles are in range [leftx...rightx], inclusive

		// look up stylename in WStyle
		leftx = 0;
		rightx = sizeof(WStyle) / sizeof(*WStyle) - 1;
		while (leftx <= rightx) {
			midx = (leftx+rightx)/2;
			int dir = strcmp(WStyle[midx].stylename, name);
			if (dir > 0) rightx = midx-1;
			else if (dir == 0) {
				WS = &WStyle[midx];
				break;
			}
			else leftx = midx+1;
		}
		if ( ! WS) {
			// style not in table
			tstyle.stylename = name;
			tstyle.flags = WA_USECODE
					| WF_EndtagNL;
			tstyle.tag =  sty->GetAttribute("codeletters");
			if ( ! tstyle.tag) {
				tstyle.tag = StyleToCodes(sty);
				sty->AddAttribute("codeletters", tstyle.tag);
			}
			WS = &tstyle;
		}

		// check for symbol
		if (WS->flags & (WA_SYMLI | WA_SYMW)) {
			if (WS->flags & (WA_SYMLI)) {
				// the `dingbat' style
				fprintf(file, "<li>");
				linelength += 4;
				pos += len;
			}
			else
				// symbol or symbola
				pos = writesymbols(file, self, pos, len, name);
			continue;
		}

		// determine preliminary mode for recursion
		long tnlflags =
			((WS->flags | nlflags) & WNL_Literal)
			| (WS->flags & WNL_brATnl)
			| ((WS->flags | nlflags) & WNL_ParaIndent);

		// output begin tag
		if (WS->flags & WA_NOP)
			begin_tag = NULL;
		else if (WS->flags & WA_USECODE)
			begin_tag = CodesToTags(WS->tag, name,
					TRUE, &tnlflags);
		else
			begin_tag = build_first_tag(env->attribs,
						WS->tag);
		if (begin_tag)
			fprintf(file, "%s", begin_tag);

		// add additional tags for definitions
		if (WS->flags & WA_ADDDT)
			fprintf(file, "<dt>");
		if (WS->flags & WA_INDENT)
			fprintf(file, "<dl><dd>");

		// newline after begintag
		if (WS->flags & WF_BegintagNL)
			putnl(file);
		else if (begin_tag)
			linelength += strlen(begin_tag);


		// recurse to output contents of environment
		pos= dowrite(self, file, env, pos, len, tnlflags);


		// write corresponding end tag & newlines
		if (WS->flags & WA_INDENT) {
			fprintf(file, "</dl>");
			putnl(file);
		}
		if (begin_tag) {
			int nchars;
			if ((WS->flags & WF_EndtagPreNL) 
					&& ! (nlflags & WNL_Literal) 
					&& ! ispunct((self)->GetChar(pos))) {
				// newline before endtag, 
				//    unless next char is punctuation
				putnl(file);
			}

			if (WS->flags & WA_DD) {
				fprintf(file, "<dd>");
				nchars = 4;
			}
			else if (WS->flags & WA_USECODE) {
				char *codes = CodesToTags(WS->tag,
					WS->stylename, FALSE, &tnlflags);
				fprintf(file, "%s", codes);
				nchars = strlen(codes);
			}
                        else {
                            const char *space = strchr(WS->tag, ' ');
                            int len = (space) ? space - WS->tag : 100;
                            fprintf(file, "</%.*s>", len, WS->tag);
                            nchars = 3 + strlen(WS->tag);
                        }

			if ((WS->flags & WF_EndtagNL) 
					&& ! (nlflags & WNL_Literal) 
					&& ! ispunct((self)->GetChar(pos))) {
				// newline after endtag, 
				//    unless next char is punctuation
				putnl(file);
			}
			else linelength += nchars;
		}

		// headers imply blank lines, so we can
		//   skip over trailing newline characters
		if (WS->flags & WF_HEADER
				&& (self)->GetChar(pos) == '\n') {
			pos ++;
			if ((self)->GetChar(pos) == '\n') pos++;
			if (pos>endpos)
				pos = endpos;
		}

		// end processing styled text

	    } // end start a nested environment: style or object
	    else {
		/* no styles here, so just take care of the characters */
		c= (self)->GetChar(pos++);
		if (isalnum(c)) {
				/* cheat, for better performance.
				Don't worry, nasty stuff inside **symbol**
				styles is taken care of above */
			putc(c, file);
			linelength++;
		}
		else if (c==' ') {
			if (linelength > 65 
					&& ! (nlflags & WNL_Literal))
				putnl(file);
			else {linelength++;  putc(c, file);}
		}
		else if (c=='\n') {
			// end the physical line
			if (linelength > 0)
				putnl(file);

			// count number of newlines (within style)
			int nnls = 0;
			while (pos+nnls < endpos
					&& self->GetChar(pos+nnls) == '\n')
				nnls++;

			// skip the following newlines
			pos += nnls;

			nnls++;		// include initial newline in count
			// put <br> for all newlines over 2
			while (--nnls > 1)
				fprintf(file, "%s\n", 
					(nlflags & WNL_Literal) ? "" : "<br>");
			// if there were more than two newlines, nnls is 1
			// otherwise, it is 0

			// output <br><br>, <br>, or <p>, depending on nlflags
			// the first two cases ignore ParaIndent
			const char *nltag;
			if (nlflags & WNL_Literal)
				// in literal mode, newlines act like <br>
				nltag = (nnls) ? "\n" : "";
			else if (nlflags & WNL_brATnl) {
				// in brATnl, newline is <br>
				// except it is omitted at end of style
				nltag = (nnls) ? "<br><br>" : "<br>";
				if (pos >= endpos) nltag += 4;
			}
			else if (nlflags & WNL_ParaIndent)
				// in ParaIndent, newlines mean <p>
				nltag = "<p>";
			else  // WNL_Normal
				// output <p> for multiple newlines, else <br>
				nltag = (nnls) ? "<p>" : "<br>";
			if (*nltag)
				fprintf(file, "%s\n", nltag);
		}   // end c=='\n'
		else if ((entry=LookupNameBySymbol(c, ""))!=NULL) {
			fprintf(file, "&%s;", entry->code);
		}
		else {
			linelength++;
			putc(c, file);
		}
	    } // end  (no style, just a plain char)
	} // end while(pos < endpos)
	return pos;
}


/* Write() writes out a datastream if it's an inset,
	or has been configured to do so.
	Otherwise, it just writes HTML+ markup.
*/
	long
htmltext::Write(FILE *file, long writeID, int level)  {
	long pos= 0, len= GetLength();

	if (level > 0 || GetWriteStyle() == text_DataStream)
		return (this)->text::Write(file, writeID, level);

	linelength = 0;
	dowrite(this, file, (htmlenv *)this->text::rootEnvironment, 0,
			len, WNL_Normal);
	putc('\n', file);

	build_first_tag(NULL, NULL);	/* clean up static var */
	return pos;
}


/* ==========================================

	DINGBAT OPERATIONS

	Dingbats are used in some lists, so they are combined here
	with the methods for RenumberList and AddDingbat
*/


	static htmlenv *
get_next_dingbat(htmlenv *list_env, htmlenv *last_dingbat,
		long pos)  {
	long list_start= (list_env)->Eval();
	long list_length= (list_env)->GetLength();
	htmlenv *next_env, *temp_env;

	if (last_dingbat)
		pos =  (last_dingbat)->GetLength()
				+ (last_dingbat)->Eval();
	else if (pos < list_start) pos= list_start;

	next_env= (htmlenv *)(list_env)->GetChild(pos);
	if (!next_env)
		next_env = (htmlenv *)(list_env)->
			GetNextChild(NULL, pos);

	while (next_env && pos < list_start+list_length) {
		const char *styname = (next_env->type==environment_Style)
			? (next_env->environment::data.style)->
					GetName() : "";
		if (strcmp(styname, "dingbat") == 0)
			return next_env;
		else if (strcmp(styname, "ordered") != 0 &&
				 strcmp(styname, "unordered") != 0 &&
				 strcmp(styname, "definition") != 0) {
			if ((temp_env=get_next_dingbat(next_env,
					NULL,pos)) != NULL)
				return temp_env;
		}
		next_env= (htmlenv *)(list_env)->
					GetNextChild(next_env, pos);
		if (next_env)
			pos = (next_env)->Eval();
	}
	return NULL;
}

	static htmlenv *
get_prev_dingbat(htmlenv *list_env, htmlenv *last_dingbat,
		long pos)  {
	long list_start= (list_env)->Eval();
	long list_length= (list_env)->GetLength();
	htmlenv *prev_env, *temp_env;

	if (last_dingbat)
		pos = (last_dingbat)->Eval()
				+(last_dingbat)->GetLength();
	else if (pos > list_start+list_length)
		pos= list_start+list_length+1;

	prev_env = (htmlenv *)(list_env)->GetChild(pos);
	if (!prev_env)
		prev_env = (htmlenv *)(list_env)->
				GetPreviousChild(NULL, pos);

	while (prev_env && pos >= list_start) {
		const char *styname = (prev_env->type==environment_Style)
				? (prev_env->environment::data.style)->
						GetName() : "";
		if (strcmp(styname, "dingbat") == 0)
			return prev_env;
		else if (strcmp(styname, "ordered") != 0 &&
				 strcmp(styname, "unordered") != 0 &&
				 strcmp(styname, "definition") != 0) {
			temp_env = get_prev_dingbat(prev_env,
						NULL, pos);
			if (temp_env != NULL)
				return temp_env;
		}
		prev_env = (htmlenv *)(list_env)->
				GetPreviousChild(prev_env, pos);
		if (prev_env)
			pos = (prev_env)->Eval()
				+ (prev_env)->GetLength();
	}
	return NULL;
}

/* takes a dingbat environment
	returns the numeric value of the dingbat
*/
	static int
get_this_dingbat_value(htmltext *self, long pos,
		htmlenv *dingbat_env)  {
	char dingbat[32], c;
	long prev_start, prev_len;
	long i=0;
	int num;

	if (!dingbat_env) return 0;

	prev_start= (dingbat_env)->Eval();
	prev_len= (dingbat_env)->GetLength();

	while (i < prev_len) {
		c= (self)->GetChar(prev_start+i);
		if (isdigit(c)) dingbat[i]= c;
		else break;
		++i;
	}
	dingbat[i+1]= 0;
	num= atoi(dingbat);
	if (num == 0)
		num= -1;
	return num;
}


/* renumbers the list in env beginning at position pos
*/
	void
htmltext::RenumberList(long pos, enum ListType ltype,
		htmlenv *env)  {
	long env_start, next_env_start,
		next_env_len, new_len = 0, so_far;
	int dingbat_counter= 1;
	htmlenv *next_env;
	char dingbat[32];

	if (env == NULL || (ltype != listtype_ORDERED
			&& ltype != listtype_UNORDERED
			&& ltype != listtype_DEFINITION))
		return;

	env_start= (env)->Eval();
	so_far= pos - env_start;

	if (so_far <0) { so_far= 0; pos= env_start;}

	if (so_far) {
		/* see if there is a dingbat before this,
			if so set the dingbat_counter to that */
		dingbat_counter = get_this_dingbat_value(this, pos,
				get_prev_dingbat(env, NULL, pos));
		if (dingbat_counter < 0) {
			dingbat_counter= 1;
			so_far= 0;
			pos= env_start;
		}
		else ++dingbat_counter;
	}

	next_env= get_next_dingbat(env, NULL, pos);

	while (next_env) {
		next_env_start= (next_env)->Eval();
		next_env_len= (next_env)->GetLength();

		if (ltype == listtype_ORDERED){
			sprintf(dingbat, "%d.\t", dingbat_counter);
			new_len= strlen(dingbat);
		}
		else if (ltype == listtype_UNORDERED){
			sprintf(dingbat, "7\t");
			new_len= strlen(dingbat);
		}
		else if (ltype == listtype_DEFINITION)
			new_len= 0;
			(next_env)->SetStyle(TRUE, TRUE);

			/* wrn:  IMPORTANT!  must insert characters at
			the end of the environment and then delete the
			characters at the beginning of the environment.
			This is for the case when you have two
			environments where the first environment ends at
			the same position the second one starts and the
			first's end flag is set to true and the second's begin
			flag is set to true.  When you insert characters at
			that position (where the first ends and the second
			starts) the characters will go into the first
			environment. */

			if (new_len > 0)
			InsertCharacters(next_env_start+next_env_len,
				dingbat, new_len);
			DeleteCharacters(next_env_start, next_env_len);
			(next_env)->SetStyle(FALSE, FALSE);

			if (ltype == listtype_UNORDERED) {
				style *sty= (GetStyleSheet())->
						Find("symbola");
			if (!sty) {				/* _create_ one; */
				sty= new style;
				(sty)->SetName("symbola");
				(sty)->SetFontFamily("symbola");
				(this->text::styleSheet)->Add(sty);
			}
			AddStyle(next_env_start, 1, sty);
		}

		++dingbat_counter;

		if (new_len)
			next_env= get_next_dingbat(env, next_env, 0);
		else
			next_env= get_next_dingbat(env, NULL, pos);
	}
}


/* adds a dingbat to the text at position pos.
*/
	htmlenv *
htmltext::AddDingbat(long pos, enum ListType ltype,
		htmlenv *env)  {
	htmlenv *dingbat_env=NULL;
	long dingbat_len=0;
	stylesheet *ss= GetStyleSheet();
	style *sty= (ss)->Find("symbola");
	style *dingbat_sty= (ss)->Find("dingbat");

	if (ltype!=listtype_ORDERED
			&& ltype!=listtype_UNORDERED)
		return NULL;

	dingbat_len= 2;
	InsertCharacters(pos, "7\t", dingbat_len);

	if (!sty) {		/* _create_ one; */
		sty= new style;
		(sty)->SetName("symbola");
		(sty)->SetFontFamily("symbola");
		(ss)->Add(sty);
	}
	AddStyle(pos, 1, sty);

	if (!dingbat_sty) {
		/* _create_ one; we can't live without a dingbat style */
		dingbat_sty= new style;
		(dingbat_sty)->SetName("dingbat");
		(ss)->Add(dingbat_sty);
	}
	dingbat_env=(htmlenv *)AddStyle(pos, dingbat_len,
			dingbat_sty);
	(dingbat_env)->SetStyle(FALSE, FALSE);
	if (ltype == listtype_ORDERED && env) {
		RenumberList(pos-1, ltype, env);
		dingbat_len= (dingbat_env)->GetLength();
	}
	return dingbat_env;
}



/* ==========================================

OTHER

*/


	void
htmltext::Clear()  {
	(this)->text::Clear();
	/* Now- we  first free the bogus rootenvironment
	 that super_Clear allocated */
	delete rootEnvironment;
	this->text::rootEnvironment = (environment *)
			htmlenv::GetTextRootEnvironment(this);
}

	void
htmltext::ClearCompletely()  {
	(this)->text::ClearCompletely();
	/* Now- we  first free the bogus rootenvironment
	 that super_Clear allocated */
	delete rootEnvironment;
	this->text::rootEnvironment= (environment *)
			htmlenv::GetTextRootEnvironment(this);
}


/* returns the href of the anchor found at position pos
	return NULL if it does not exist
*/
	const char *
htmltext::GetAnchor(long pos)  {
	htmlenv *env= (htmlenv *)
		((htmlenv *)this->text::rootEnvironment)->
				GetInnerMost(pos);
	while (env) {
		if (env->type == environment_Style
				&& strcmp((env->environment::data.style)
					->GetName(), "anchor") == 0)
		return getatt(env->attribs, "href");
		env= (htmlenv *)(env)->GetParent();
	}
	return NULL;
}

const char *htmltext::GetURL(long pos, long x, long  y) {
    htmlenv *env= (htmlenv *)
      ((htmlenv *)this->text::rootEnvironment)->
      GetInnerMost(pos);
    const char *mapp=getatt(env->attribs, "ismap");
    while (env) {
	if (env->type == environment_Style
	    && strcmp((env->environment::data.style)
		      ->GetName(), "anchor") == 0) break;
	env= (htmlenv *)(env)->GetParent();
    }
    if(env==NULL) return NULL;
    const char *url=getatt(env->attribs, "href");
    if(url==NULL) return NULL;
    static flex buf;
    // 1+1+2*20+1 -> NUL,sepchar,2 numbers, and a comma.
    buf.Remove((size_t)0,buf.GetN());
    char *p=buf.Insert((size_t)0,(size_t)strlen(url)+1+1+2*20+1);
    strcpy(p, url);
    char *e=p;
    if(mapp) {
	char monchar = '?';
	while(*e != '\0') {
	    e++;
	    if (*e == monchar) monchar = '@';
	}
	sprintf(e, "%c%ld,%ld", monchar, x, y);
    }
    return p;
}

	static long
locate_id(htmlenv *par, char *val)  {
	htmlenv *env;
	long temppos, pos= (par)->Eval();
	const char *value;

	/* is par the environment we are looking for? */
	if ((value= (par)->GetAttribValue("id"))
				&& strcmp(value, val)==0)
		return pos; /* found it */

	/* no, so check par's children */
	env= (htmlenv *)(par)->GetChild(pos);
	if (!env) env= (htmlenv *)(par)->GetNextChild(NULL, pos);

	while (env) {
		if (env->type == environment_Style) {
			pos= (env)->Eval();
			temppos= pos;
			if ((temppos= locate_id(env, val)) != -1)
				return temppos;
		}
		env= (htmlenv *)(par)->GetNextChild(env, pos);
	}
	return -1;
}

	long
htmltext::FindDestId(char *dest)  {
	return locate_id((htmlenv *)this->text::rootEnvironment,
				dest);
}

	static boolean
changevisibility(long viewcomments, text *txt, long pos,
			environment *env)  {
	if (env->type == environment_View) {
		viewref *vref= env->data.viewref;
		const char *name
				= (vref->dataObject)->GetTypeName();
		if (ATK::IsTypeByName(name,"hidden"))
		((hidden *)vref->dataObject)->
			SetVisibility(viewcomments
				? hidden_EXPOSED : hidden_HIDDEN);
	}
	return FALSE;
}

	void
htmltext::ShowComments(boolean viewcomments)  {
	EnumerateEnvironments(0, GetLength(),
			changevisibility, viewcomments);
}


/* XXX  KLUDGE.  The following definition should be exported by text.H */
#define TEXT_VIEWREFCHAR  '\377'

	long
htmltext::EnumerateInsets(htmlefptr f, arbval rock) {
	long pos = 0, endpos = GetLength();

	while (pos < endpos) {
		// fetch bufferful at pos
		long tlen;	// how much we really got
		char *patchloc, patchdata;
		char *body = GetBuf(pos, endpos-pos, &tlen);
		if (tlen <= 0) return -9;
		patchloc = body+tlen;

		// look for TEXT_VIEWREFCHAR
		patchdata = *patchloc;
		*patchloc = TEXT_VIEWREFCHAR;	// XXX KLUDGE
		char *vrc = strchr(body, TEXT_VIEWREFCHAR);
		*patchloc = patchdata;
		if ( ! vrc || vrc >= patchloc)
			pos += patchloc-body;
		else {
			// process an inset
			pos += vrc-body;	// pos of inset
			htmlenv *env;
			dataobject *d;
			env = (htmlenv *)rootEnvironment->GetInnerMost(pos);
			if ( ! env || env->type != environment_View)
				return -9;
			d = env->data.viewref->dataObject;
			if ( ! d)
				return -9;
			if (f(this, pos, env, d, rock))
				return pos;
			pos++;			// where to continue search
		}
	}
	return -1;
}


	htmlenv *
htmltext::AddImage(long pos, const char *file,
                   class attlist  *atts)  {
            image *dat=GetImage(file, atts);
            htmlenv *imgEnv = NULL;
	if (dat) {
		imgEnv= (htmlenv *)AlwaysAddView(pos,
				"htmlimagev", dat);
		NotifyObservers(0);
	}

	return imgEnv;
}

// Convert view to gif, via gs and pbmplus
//  		'v' is the view to gifify
//  		'filename' is the path and filename of the gif file
//		width and height are in points
//		scale is divided by 100 to get the actual scale factor
//	returns NULL for success or error message
//
	static const char *
GififyViaPS(view *v, const char *filename, long *width, long *height, long scale) {

	/* Could run the command in the background, but might get 
			too many processes.
	  (We should generate all the ps'es to a single file
	  interspersed with commands to generate separate
	  (or a single concatenated) gif file(s).
	  Then run gs over that file, creating a ppm file for each gif.
	  Run the ppm files through ppmquantall and ppmtogif.
	  All create a script aping ppmquantall.)
	*/
	char command[300];
	char tfnm[L_tmpnam];
	sprintf(tfnm, "%s/XXXXXX", P_tmpdir);
	close(mkstemp(tfnm));
	sprintf(command, "%s %s | %s | %s | %s > %s",
			"gs -q -sDEVICE=ppm -sOutputFile=-",
			tfnm,
			"pnmcrop",
			"pnmmargin 3",
			"ppmtogif -interlace 2>/dev/null",
			filename);

        long pagew, pageh;
#if PRINTXXX
	print p;	// printing context
	p.SetPaperSize(*width, *height);
	p.SetScale(scale/100.0);
	p.SetFromProperties(v, &pagew, &pageh);
	v->PrintPSDoc(p.GetFILE(), pagew, pageh);
	if ( ! p.WritePostScript(tfnm, NULL)) 
		return "*** Conversion to a Postscript file failed";

	system(command);
	*width += 6;
	*height += 6;
#endif
	unlink(tfnm);
	return NULL;
}


// Convert view to gif, by drawing to offscreen window
//  		'v' is the view to gifify
//  		'filename' is the path and filename of the gif file
//		width and height are in points
//	returns NULL for success, else an error message
// method
//	view->FullUpdate to offscreen image
//	X offscreen image ->-> image inset ->-> gif
//
	static const char *
GififyViaView(view *v, const char *filename, long width, long height) {
	im *realim = im::GetLastUsed();
	im *Ximg = NULL;
	graphic *drawbl = NULL;
	colormap *privateCmap = NULL;
	image *drawngif = NULL;
	const char *retval = NULL;
	static struct rectangle playpen = {0,0,0,0};

	if ( ! realim || ! realim->SupportsOffscreen()) 
		return "*** Cannot gifify without X windows";
	(v)->InsertView((view *)realim, &playpen);
	v->imPtr = realim;
	Ximg = im::CreateOffscreen(realim, width, height);
	if (Ximg == NULL) {
		retval = "*** Could not create offscreen view area";
		goto done;
	}
	privateCmap = Ximg->CreateColormap();
	if (privateCmap == NULL) {
		retval = "*** Could not create colormap";
		goto done;
	}
	v->WantColormap(v, &privateCmap);
	Ximg->SetView(v);
	im::ForceUpdate();
	drawbl = v->drawable;
	drawngif = new imageio;
	if (drawngif == NULL) {
		retval = "*** Could not create new gif";
		goto done;
	}
	drawbl->ReadImage(0, 0, drawngif, 0, 0, width, height);
	if (drawngif->WriteNative(NULL, filename) < 0) {
		retval = "*** Writing image as gif failed";
		goto done;
	}

done:
	// cleanup
	if (Ximg) Ximg->Destroy();
	if (drawngif) drawngif->Destroy();
	if (privateCmap) privateCmap->Destroy();
	return retval;
}

/*  convert an inset to a gif 
	returns error message, else NULL
*/
	const char *
htmltext::GififyInset(long pos, htmlenv *env, const char *giffile) {
	dataobject *dobj = env->data.viewref->dataObject;
	view *v;		// the view to generate a gif for
	static char attrs[60];	// "WIDTH=ddd, HEIGHT=ddd"
	const char *errmsg;
	long width, height, scale = 100;

	v = (view *)ATK::NewObject(env->data.viewref->viewType);
	if ( ! v) return "*** Could not create view";
	(v)->SetDataObject(dobj);

	width = 1024;
        height = 800;
#if PRINTXXX
	if (v->Gifify(giffile, &width, &height, NULL))
		errmsg = NULL;
        else
#endif

            errmsg = "force PS if view size too big";

	if (errmsg) {
		// determine size and choose gifify method
		v->DesiredSize(15*72/2, 10*72, view::NoSet,
			&width, &height);
		if (width <= 7.5*72 && height <= 10*72) 
			errmsg = GififyViaView(v, giffile, width, height);
	}
	if (errmsg) {
		// inset refuses to be small enough to print (or View failed)
		// use scaling and GififyViaPS

		v->DesiredPrintSize(15*72/2, 10*72, view::NoSet,
			&width, &height);
		// determine scaling factor
		scale = 100;				// default scape
		if (width * scale > 100*15*72/2)	// max width 7.5"
			scale = 100*15*72/(2*width);  	// scale for width
		if (height * scale > 100*10*72)		// max height 10"
			scale = 100*10*72 / height;   	// scale for height
		if (scale != 100) {
			width = scale * width / 100;
			height = scale * height / 100;
		}

		errmsg = GififyViaPS(v, giffile, &width, &height, scale);
	}

	v->Destroy();

	if (errmsg)
		return errmsg;
	if (access(giffile, F_OK) != 0)
		return "*** Gif file did not get created";

	sprintf(attrs, "WIDTH=%ld, HEIGHT=%ld", width, height);
	return attrs;
}

static boolean checkanchor(long rock, text *self, long pos, environment *env) {
    if(env->type!=environment_Style) return FALSE;
    style *s=env->data.style;
    const char *sname=s->GetName();
    if(sname && *sname=='a'
		&& strcmp(sname, "anchortarget")==0) {
	char *target=(char *)rock;
	htmlenv *henv=(htmlenv *)env;
	const char *name=getatt(henv->attribs, "name");
	if(strcmp(name,target)==0)
		return TRUE;
    }
    return FALSE;
}

long htmltext::FindNamedAnchor(const char *target, long pos, long len) {
    if(len==-1)
		len=GetLength()-pos;
    environment *env=EnumerateEnvironments(pos, len,
			checkanchor, (long)target);
    if(env)
		return env->Eval();
    return 0;
}


image *htmltext::GetImage(const char *file, attlist *atts) {
    image *dat = NULL;
    char *filename, *p;
    buffer *buffer;
    static char *defaultImage= NULL;

    if (*file != '/' && *file != '~') {
        buffer= buffer::FindBufferByData(this);
        if (buffer) {
            p= (buffer)->GetFilename();
            filename= (char *)malloc(strlen(p)+strlen(file)+1);
            strcpy(filename, p);
            if (*file != '/') {
                if ((p= strrchr(filename, '/'))) *(p+1)= '\0';
                strcat(filename, file);
                if (access(filename, R_OK) != 0) {
                    free(filename);
                    filename= NULL;
                }
            }
        }
        else   filename = strdup((char *)file);
    }
    else   filename = strdup((char *)file);

    if ((dat=(image*)ATK::NewObject("imageio")) != NULL) {
        int ok;
        ok= (filename) ? (dat)->Load(filename, NULL)
          : -1;
        if (ok < 0) {
            (dat)->Destroy();
            dat= NULL;
            if (!defaultImage) {
                /* defaultImage is static,
                 1st time through it is NULL */
                const char *s = environ::GetProfile(
                                              "defaultImage");
                if (s) {
                    defaultImage = (char *)malloc
                      (strlen(s)+1);
                    strcpy(defaultImage, s);
                }
            } // end initing defaultimage
            if (defaultImage &&
                (dat= (image*)ATK::NewObject("imageio")) != NULL) {
                FILE *f = fopen(defaultImage, "r");
                if (f != NULL) {
                    fclose(f);
                    (dat)->Load(defaultImage, NULL);
                }
                else {
                    (dat)->Destroy();
                    dat= NULL;
                }
            }
        }  // end ok<0
    } // end dat != NULL
	if (filename) free(filename);
    return dat;
}


#if 0

#define EMPH			0
#define MISC			1
#define ANCHOR		2
#define PARAGRAPH	3
#define LINE_BREAK	4
#define PARAS			5
#define TAB			6
#define LISTS			7
#define LIST_ITEM		8
#define SP_LIST		9
#define BLOCK		10
#define HEADING		11
#define INSET			12
#define IMG			13

#define NODATA		14
#define NONE			15

/* XXX - this structure is not currently being used in this code.
It is meant to be used in conjunction with the "nested" table below
to ensure that styles could not be nested improperly */

static struct stylemap styletable[] = {
    { "abbrev", EMPH },
    { "acronym", EMPH },
    { "added", EMPH },
    { "arg", EMPH },
    { "b", EMPH },
    { "cite", EMPH },
    { "code", EMPH },
    { "cmd", EMPH },
    { "dfn", EMPH },
    { "em", EMPH },
    { "i", EMPH },
    { "kbd", EMPH },
    { "person", EMPH },
    { "q", EMPH },
    { "removed", EMPH },
    { "s", EMPH },
    { "samp", EMPH },
    { "strong", EMPH },
    { "sub", EMPH },
    { "sup", EMPH },
    { "tt", EMPH },
    { "u", EMPH },
    { "var", EMPH },

    { "footnote", MISC },
    { "margin", MISC },

    { "a", ANCHOR },

    { "p", PARAGRAPH },

    { "l", LINE_BREAK },	/* not done */

    { "pre", PARAS },
    { "lit", PARAS },

    { "tab", TAB },		/* not done */

    { "ol", LISTS },
    { "ul", LISTS },

    { "li", LIST_ITEM },

    { "dir", SP_LIST },		/* not done */
    { "menu", SP_LIST },	/* not done */

    { "abstract", BLOCK },	/* not done */
    { "byline", BLOCK },	/* not done */
    { "math", BLOCK },		/* not done */
    { "note", BLOCK },		/* not done */
    { "blockquote", BLOCK },

    { "h1", HEADING },
    { "h2", HEADING },
    { "h3", HEADING },
    { "h4", HEADING },
    { "h5", HEADING },
    { "h6", HEADING },
    { "img", IMG },
    { "table", INSET },		/* not done */
    { "image", INSET },		/* not done */

    { "form", INSET },
    { "select", INSET },
    { "textarea", INSET },
    { "input", INSET },

#if 0
    { "box", INSET },		/* not done */
    { "fig", INSET },		/* not done */
#endif

    { "br", NODATA },
    { "changed", NODATA },	/* not done */
    { NULL, NONE }
};

/* XXX - this was used to make sure the nesting of environments
is valid.  It is not being used in the code now. */
#define AAEM (1 << EMPH | 1 << MISC | \
		1 << ANCHOR | 1<< ANCHORTARGET)

static long nested[] = {
/*EMPH */	1 << NODATA | AAEM,
/*MISC*/		1 << NODATA | AAEM,
/*ANCHOR*/	0,
/*PARAGRAPH*/	1<<LINE_BREAK | 1 << NODATA | AAEM,
/*LINE_BREAK*/	1 << NODATA | AAEM,
/*PARAS*/	1 << TAB | 1 << NODATA | AAEM,
/*TAB*/		0,
/*LISTS*/		1 << LISTS | 1 << LIST_ITEM | 1 << NODATA
			 | AAEM | 1 << PARAGRAPH,
/*LIST_ITEM*/	1 << PARAGRAPH | 1 << NODATA | AAEM,
/*SP_LIST*/	1 << LIST_ITEM,
/*BLOCK*/	0,
/*HEADING*/	1 << EMPH | 1<< ANCHORTARGET,
/*INSET*/  	0,
/*IMG*/  		0
};
#endif

