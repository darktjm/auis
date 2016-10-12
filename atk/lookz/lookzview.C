/* 
 *	Copyright IBM Corporation 1988,1991 - All Rights Reserved
 *	Copyright Carnegie Mellon University 1992 - All Rights Reserved
 *	For full copyright information see:'andrew/doc/COPYRITE'
 */

/* lookzv.c	

	The view module for the lookz dataobject

	When initiated as an inset in a document, the FullUpdate routine 
	attempts to find a stylesheet associated with the parent text.  
	Otherwise a stylesheet can be specified by a client calling 
	SetStyleSheet.  Either way, lookzview displays some of the 
	components of the style via stringtbls.  Clicks on the 
	tables cause the associated attributes to be changed in the stylesht.

Deferred:
	Want to avoid direct modification of the style in the surrounding text.
		Instead, make a copy of the style, modify the copy, and
		call style_Copy to replace the original style.
	Menu options: Add Font, Rename Style, Cancel
	Baseline separation and minimum and maximum shim size.
*/
#define label gezornenplatz
#include <andrewos.h> /* strings.h */
ATK_IMPL("lookzview.H")
#include <stdio.h>
/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
   Avoid this by temporarily redefining "label" to be something else in the 
preprocessor. */
#undef label
#include <util.h>
#include <ctype.h>

#include <message.H>
#include <rect.h>
#include <environ.H>
#include <environment.H>
#include <graphic.H>
#include <dataobject.H>
#include <view.H>
#include <text.H>
#include <lpair.H>
#include <stylesheet.H>
#include <style.H>
#include <tabs.H>

#include <bpair.H>
#include <stringtbl.H>
#include <strtblview.H>
#include <lprruler.H>
#include <lprrulerview.H>
#include <tabrulerview.H>
#include <label.H>
#include <labelview.H>

#include <fontdesc.H>

#include <lookz.H>
#include <lookzview.H>

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <proctable.H>
#include <bind.H>

#include <buffer.H>
#include <frame.H>
#include <im.H>

#if 0
#define DEBUG(s) (printf s, fflush(stdout))
#define ENTER(r) DEBUG(("Enter %s(0x%lx)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%lx)\n", Stringize(r), this))
#else
#define DEBUG(s)
#define ENTER(r)
#define LEAVE(r)
#endif

	/* case insensitive compare from libutil */

#define NOMENUSTRING "<No Menu>"

  
static long CVDots(long  amt, enum style_Unit  unit);
static long  CVFractionalPoints(long  amt, enum style_Unit  unit);
static long MapStringToVal(const struct strTbl  tbl[], const char  *s);
static const char * MapValToString(const struct strTbl  tbl[], long  val);
static char * MapDeltaToString(long  dots, enum style_FontSize  basis);
static void MapStringToDelta(char  *str, long  *dots, enum style_FontSize  *basis);
static void  ChangeStyle(class lookzview  *self);
static void RulerValueChanged(class lprrulerview  *rv, class lookzview  *self, enum lprrulerview_iconcode  icon, long  newvalue);
static void TabsValueChanged(class lprrulerview  *rv, class lookzview  *self, long  pos, enum style_TabAlignment  op, enum style_Unit  unit);
static void SetBitsForCode(unsigned long  code, const struct strTbl  names[], class stringtbl  *tbl, long  parity);
static void  UnpackStyle(class lookzview  *self);
static const char * InitialWord(const char  *s);
static const char * clean(const char  *s);
static void SetStyleDisplay(class lookzview  *self, class style  *st);
static void ClearStyleDisplay(class lookzview  *self);
static void CloseStyleSheet(class lookzview  *self);
static boolean AddCardName(class style  *s, class stringtbl  *st);
static void OpenStyleSheet(class lookzview  *self, class stylesheet  *ss);
static boolean FindAndUnpack(class style  *s, class lookzview  *lv);
static boolean FindAndUnpackNoMenus(class style  *s, class lookzview  *lv);
static void HitStyleName(class stringtbl  *st1, class lookzview  *lv, long  accnum);
static void HitStyleNameNoMenus(class stringtbl  *st1, class lookzview  *lv, long  accnum);
static boolean AddStyleName(class style  *s, class lookzview  *lv);
static boolean AddStyleNameNoMenus(class style  *s, class lookzview  *lv);
static void HitMenuCard(class stringtbl  *st0, class lookzview  *lv, long  accnum);
static char * GetStringValue(class stringtbl  *st, class lookzview  *self, short  accnum, const char  *prompt);
static char * GetDelimitedStringValue(class stringtbl  *st, class lookzview  *self, short  accnum, const char  *prompt);
static void HitFont(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitSize(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitMode(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitSubscr(class stringtbl  *st, class lookzview  *self, long  accnum);
static void FinagleStyleBits(class lookzview  *self, class stringtbl  *st, long  accnum, long  parity, unsigned long  *faces , unsigned long  *flags);
static void HitEnable(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitDisable(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitSpacing(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitSpread(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitColor(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitTabFill(class stringtbl  *st, class lookzview  *self, long  accnum);
static void HitShrink(class labelview  *shrink, enum view::MouseAction   action, class lookzview  *self);
static void ChooseShrinkIcon(class lookzview  *self);

enum dir {H, V};
static class lpair * LL(enum dir  dir, long  pct, class lpair  *top , class lpair  *bot);
static class lpair * LBL (const char  *label, class view  *view);
static  void EditStylesInWindow(class view  *textv);
static  void NewTextObject(class lookzview  *self, class text  *text);
static void BuildImage(class lookzview   *self);
static void ChopTree(class lpair  *branch, struct ATKregistryEntry   *lpairInfo);
static void  UpdateDocument(class lookzview  *self);
static boolean FindCardName(class style  *s, char  *nm);
static char * NewMenuName(class lookzview  *lv, char  *name);
static void  AddStyle(class lookzview  *self);
static boolean CheckDelStyle(class style  *sty, class text  *txt, int  level, class environment  *env);
static void DeleteStyle(class lookzview  *self);
static void SetBGcolor(class lookzview  *self);
static void SetROBGcolor(class lookzview  *self);


static struct bind_Description MenuOptions[] = {
        /* Update Document must be the first entry.  See InitializeClass. */
	{"lookzview-update-document", "\033U", 0, "Update Document", 0, 0,
		(proctable_fptr)UpdateDocument, "Cause document to redisplay itself", NULL},
	{"lookzview-delete-style", "\033D", 0, "Delete Style", 0, 0,
		(proctable_fptr)DeleteStyle, "Delete a style from the stylesheet", NULL},
	{"lookzview-add-style", "\033A", 0, "Add Style", 0, 0,
		(proctable_fptr)AddStyle, "Add a new style to stylesheet", NULL},
	{"lookzview-set-bgcolor", 0, 0, "Set Background~10", 0, 0,
		(proctable_fptr)SetBGcolor, "Set stylesheet background color", NULL},
	{"lookzview-set-robgcolor", 0, 0, "Set R/O Background~11", 0, 0,
		(proctable_fptr)SetROBGcolor, "Set stylesheet readonly background color", NULL},
	NULL
};

static class menulist  *MenuList;
static class keymap  *Keymap;

static class fontdesc *StrtblFont;
static boolean AutoUpdate;

struct styleeditorlist {
    class text *text;
    class lookz *lookz;
};

static struct styleeditorlist *styleEditors = NULL;
static long numStyleEditors = 0;
static long maxStyleEditors = 0;

/* indices into arrays st[] and stv[] */
enum TableNum {
	TNmenucard, TNstylename, TNfont, TNsize, 
	TNmode, TNsubscr, TNenable, TNdisable, TNspacing, TNspread,
	TNcolor, TNtabfill
};

/* set up initial strings for stringtbls.  
		Where a string in this table is supposed to be the 
		same as a string in one of the XxxxNames tables, 
		both must be spelled identically */
	static const char * const Imenucard[] = {"", NULL};
	static const char * const Istylename[] = {"", NULL};
	static const char * const Ifont[] = {"Andy", "AndySans", "AndyType", "<other>", 
				NULL};
	static const char * const Isize[] = {"-2", "+2", "7", "8", "9", "10", "12", "14",
				"16", "18", "24", "30", "36", "<other>", NULL};
	static const char * const Imode[] = {"Center", "Left flush", "Right flush", 
			"Justified", "Left-right", NULL};
	static const char * const Isubscr[] = {"super (-6)", 
			"sub (+2)", "<other>", NULL};
	static const char * const Ienable[] = {"Bold", "Italic", "Underline",
					"Dotted Box", "Tab by spaces", 
					"<other>", NULL};
	static const char * const Idisable[] ={"Bold", "Italic", "Underline",
					"Dotted Box", "Tab by spaces", 
					"<other>", NULL};
	static const char * const Ispacing[] = {"0", "2", "4", "6", "8", "10", "12", 
					"<other>", NULL};
	static const char * const Ispread[] = {"0", "2", "4", "6", "8", "10", "12", "14", 
					"16", "18", "20", "<other>", NULL};
	static const char * const Icolor[] = {"Black", "Red", "Green", "Blue", "Magenta", 
					"Cyan", "Yellow", "<other>", NULL};
	static const char * const Itabfill[] = {"[. ]", "<other>", NULL};

static const char * const * const InitialStrings[]=
		{Imenucard, Istylename, Ifont, Isize, Imode, Isubscr, Ienable,
			Idisable, Ispacing, Ispread, Icolor, Itabfill, NULL};

struct strTbl {const char *str; long val;};

/* map justification mode names to and from justification codes*/
static const
struct strTbl ModeNames[] = {
		{"Center", (long)style_Centered},
		{"Left flush", (long)style_LeftJustified},
		{"Right flush", (long)style_RightJustified},
		{"Justified", (long)style_LeftAndRightJustified},
		{"Left-right", (long)style_LeftThenRightJustified},
		{NULL, 0L},
};
/* map flag names to and from flag codes*/
static const
struct strTbl FlagNames[] = {
		{"Underline", style_Underline},
		{"Hidden", style_Hidden},
		{"Read Only", style_ReadOnly},
		{"Format Note", style_PassThru},
		{"Icon", style_Icon},
		{"Continue Indent", style_ContinueIndent},
		{"Hinge", style_Hinge},
		{"New Page", style_NewPage},
		{"Change Bar", style_ChangeBar},
		{"Over Bar", style_OverBar},
		{"No Wrap", style_NoWrap},
		{"No Fill", style_NoFill},
		{"Keep Prior NL", style_KeepPriorNL},
		{"Keep Next NL", style_KeepNextNL},
		{"Tab by spaces", style_TabsCharacters},
		{"Dotted Box", style_DottedBox},
		{"Strike Through", style_StrikeThrough},
		{"Include Beginning", style_IncludeBeginning},
		{"Include End", style_IncludeEnd},
		{NULL, 0L},
};
/* map face names to and from face codes*/
static const
struct strTbl FaceNames[] = {
		{"Bold", fontdesc_Bold},
		{"Italic", fontdesc_Italic},
		{"Shadow", fontdesc_Shadow},
		{"Fixed width", fontdesc_Fixed},
		{"Outline", fontdesc_Outline},
		{"Thin", fontdesc_Thin},
		{"Black", fontdesc_Black},
		{"Medium", fontdesc_Medium},
		{"Heavy", fontdesc_Heavy},
		{"Condense", fontdesc_Condense},
		{NULL, 0L},
};

/* map baseline codes to distance */
static const
struct strTbl SubScrNames[] = {
		{"super (-6)", -6},
		{"super (-2)", -2},
		{"up 15", -15},
		{"up 6", -6},
		{"up 4", -4},
		{"up 2", -2},
		{"sub (+2)", 2},
		{"down 2", 2},
		{"down 4", 4},
		{"down 6", 6},
		{NULL, 0L},
};

/* long CVDots(amt, unit)
	Convert the distance 'amt' measured in the given 'units' 
	to a distance measured in pixels (points)
	(should be a class procedure in style.c) 
*/


ATKdefineRegistry(lookzview, view, lookzview::InitializeClass);


static
long CVDots(long  amt, enum style_Unit  unit)
		{
	switch (unit) {
            case style_RawDots:
                return amt;
            case style_Inches:
                return (amt * 72) >> 16;
            case style_CM:
                return (amt * 225 / 127) >> 12;
            case style_Points:
                return amt;
            default: 		/* style_Ems,  style_Lines */
                return 0;
        }
}

/* CVFractionalPoints(amt, unit)
	convert a value to a fixed-point floating value in points
              (long with binary point in the middle) 
*/

static long 
CVFractionalPoints(long  amt, enum style_Unit  unit)
		{
	switch (unit) {
            case style_RawDots:
                return amt << 16;
            case style_Inches:
                return amt * 72;
            case style_CM:
                return (amt<<4)*225/127;
            case style_Points:
                return amt << 16;
            default:
                return 0;		/* style_Ems,  style_Lines */
        }
}

static long
MapStringToVal(const struct strTbl  tbl[], const char  *s)
		{
    long i;
    if(s==NULL) return -999L;
	for (i = 0; tbl[i].str; i++)
		if (FOLDEDEQ(tbl[i].str, s)) 
			return (tbl[i].val);
	return -999L;
}

static const char *
MapValToString(const struct strTbl  tbl[], long  val)
		{
	long i;
	for (i = 0; tbl[i].str; i++)
		if (tbl[i].val == val)
			return (tbl[i].str);
	return NULL;
}

/* MapDeltaToString(dots, basis)
	convert dots value dd to "dd" if ConstantSize or "+dd" or "-dd" for 
PreviousSize 
		result is in a static buffer 
		(NOTE: used for both FontSize and ScriptMovement: 
		ASSUMES parallel definition of style_FontSize 
			and style_ScriptMovement) */
	static char *
MapDeltaToString(long  dots, enum style_FontSize  basis)
		{
	static char buf[15];
	if (dots == 0) 
		*buf = '\0';
	else 
		sprintf(buf, "%s%ld", 
			(basis == style_ConstantFontSize) ? "" 
				:  ((dots >= 0) ? "+" : ""), 
			dots);
	return buf;
}

/* MapStringToDelta(str, dots, basis)
	convert string generated by MapDeltaToString back to SizeBasis and dots value 
*/
	static void
MapStringToDelta(char  *str, long  *dots, enum style_FontSize  *basis)
			{
	*basis = (*str == '+' || *str == '-') ? style_PreviousFontSize 
				: style_ConstantFontSize;
	sscanf(str, "%ld", dots);
}

	static void 
ChangeStyle(class lookzview  *self)
	{
	self->curstyle->template_c = 0;
	(self->curss)->NotifyObservers( (long) self);	/*BOGUS XXX
			The protocol for stylesheet observed_changed status is
			to send the address of the view that caused the change;
			Thus its own ObservedChanged routine can reject it.  */
	if (AutoUpdate)
	    UpdateDocument(self);
}

/* RulerValueChanged(rv, self, icon, newvalue)
	update the style parameter given by setProc 
		relative to basis to the given value 
	set values in centimeters  {2.54/72 ==  127/(225*16)}
	use relative zero to mean no value */

	static void
RulerValueChanged(class lprrulerview  *rv, class lookzview  *self, enum lprrulerview_iconcode  icon, long  newvalue)
				{
	if (self->curstyle == NULL) return;
	switch (icon) {
		case leftIcon:
			if (newvalue <= lprrulerview_NoValue) newvalue = 0;
			(self->curstyle)->SetNewLeftMargin( style_LeftMargin,
					(newvalue>>4)*127/225, style_CM);
			break;
		case rightIcon:
			if (newvalue <= lprrulerview_NoValue) newvalue = 0;
			(self->curstyle)->SetNewRightMargin( style_RightMargin,
					(newvalue>>4)*127/225, style_CM);
			break;
		case paraIcon:
			if (newvalue <= lprrulerview_NoValue) newvalue = 0;
			(self->curstyle)->SetNewIndentation( style_LeftMargin,
					(newvalue>>4)*127/225, style_CM);
			break;
		case noIcon:
			break;
	}
	ChangeStyle(self);
}

	static void
TabsValueChanged(class lprrulerview  *rv, class lookzview  *self, long  pos, enum style_TabAlignment  op, enum style_Unit  unit)
					{
	if (self->curstyle == NULL) return;

	if (pos == -1) /* Cannot have negative tabstops - this is cancel code*/
		(self->curstyle)->ClearTabChanges();
	else
		(self->curstyle)->AddTabChange( op, pos, unit);
	ChangeStyle(self);
}

/* SetBitsForCode(code, names, tbl, parity)
	set bits in stringtbl -tbl- for the names given in -names- 
		for the bits in -code- 
	parity adjusts for the nonsense of the OutXxxx stuff 
*/
	static void
SetBitsForCode(unsigned long  code, const struct strTbl  names[], class stringtbl  *tbl, long  parity)
				{
	long i;
	short accnum;
	for (i = 1;   code;   i <<= 1, code >>= 1) 
		if ((1 & code) ^ parity) {
			accnum = (tbl)->AddString( 
				MapValToString(names, i)); 
			(tbl)->SetBitOfEntry( accnum, 1);
		}
}

static void 
UnpackStyle(class lookzview  *self)
	{
	class style *style = self->curstyle;
	class stringtbl **st = self->st;

	if (style == NULL) {
		return;
	}
	{	/* justification mode */
		(st[(long)TNmode])->ClearBits();
		(st[(long)TNmode])->SetBit( 
			MapValToString(ModeNames,
					(style)->GetJustification()),
			1);
	}
	{	/* font family */
		char font[50];
		(st[(long)TNfont])->ClearBits();
		(style)->GetFontFamily( font, sizeof (font));
		(st[(long)TNfont])->SetBitOfEntry(
				(st[(long)TNfont])->AddString( font), 1);
	}
	{	/* font size */
		enum style_FontSize basis;
		long points;
		(st[(long)TNsize])->ClearBits();
		(style)->GetFontSize( &basis, &points);
		(st[(long)TNsize])->SetBitOfEntry(
			(st[(long)TNsize])->AddString( 
				(char *)MapDeltaToString(points, basis)), 
			1);
	}
	{	/* font script movement  (baseline adjustment) */
		enum style_ScriptMovement basis;
		long amt;
		enum style_Unit unit;
		const char *str;
		(st[(long)TNsubscr])->ClearBits();
		(style)->GetFontScript( &basis, &amt, &unit);
		str = MapValToString(SubScrNames, CVDots(amt, unit));
		(st[(long)TNsubscr])->SetBitOfEntry(
			(st[(long)TNsubscr])->AddString( 
				(char *)((str) ? str : 
				MapDeltaToString(CVDots(amt, unit), (enum style_FontSize)basis))), 
			1);
	}

	(st[(long)TNenable])->ClearBits();
	(st[(long)TNdisable])->ClearBits();
	(st[(long)TNenable])->RemoveString( "<other>");
	(st[(long)TNdisable])->RemoveString( "<other>");
	{	/* font face codes */
		SetBitsForCode((style)->GetAddedFontFaces(), 
				FaceNames, st[(long)TNenable], 0);
		SetBitsForCode((style)->GetRemovedFontFaces(), 
				FaceNames, st[(long)TNdisable], 1);
	}
	{	/* flags */
			/* KLUDGE:  missing macro in style.ch 
				access fields of style directly */
		SetBitsForCode(style->AddMiscFlags, 
				FlagNames, st[(long)TNenable], 0);
		SetBitsForCode(style->OutMiscFlags, 
				FlagNames, st[(long)TNdisable], 1);
	}
	(st[(long)TNenable])->AddString( "<other>");
	(st[(long)TNdisable])->AddString( "<other>");

	{	/* leftmargin, rightmargin, paraindent */
		long leftmargin, rightmargin, paraindent;
		enum style_MarginValue Basis;
		long Operand;
		enum style_Unit Unit;
		leftmargin = rightmargin = paraindent 
				= lprrulerview_NoValue;	/* initially no icons */
		(style)->GetNewLeftMargin( &Basis, &Operand, &Unit);
		if (Basis == style_LeftMargin && Operand != 0)
			leftmargin = CVFractionalPoints(Operand, Unit);
		(style)->GetNewRightMargin( &Basis, &Operand, &Unit);
		if (Basis == style_RightMargin && Operand != 0)
			rightmargin = CVFractionalPoints(Operand, Unit);
		(style)->GetNewIndentation( &Basis, &Operand, &Unit);
		if (Basis == style_LeftMargin && Operand != 0)
			paraindent = CVFractionalPoints(Operand, Unit);
		(self->rulerview)->SetValues( leftmargin, rightmargin, 
				paraindent);
	}
	{ /* Interline/Interparagraph Spacing */
		enum style_SpacingValue Basis;
		long Operand;
		enum style_Unit Unit;
		char value[100];

		(st[(long)TNspacing])->ClearBits();
		(style)->GetNewInterlineSpacing( &Basis, &Operand, &Unit);
		if (Basis == style_ConstantSpacing && Unit == style_Points) {
			sprintf(value, "%ld", Operand);
			(st[(long)TNspacing])->SetBitOfEntry(
					(st[(long)TNspacing])->AddString( 
					value), 1); 
		}
		(st[(long)TNspread])->ClearBits();
		(style)->GetNewInterparagraphSpacing( &Basis, &Operand, &Unit);
		if (Basis == style_ConstantSpacing && Unit == style_Points) {
			sprintf(value, "%ld", Operand);
			(st[(long)TNspread])->SetBitOfEntry(
					(st[(long)TNspread])->AddString( 
					value), 1); 
		}
	}
	{ /* Color */
		const char *color;
		(st[(long) TNcolor])->ClearBits();
		color = (self->curstyle)->GetAttribute("color");
		if(color != NULL) 
			(st[(long)TNcolor])->SetBitOfEntry( 
				(st[(long)TNcolor])->AddString( color), 1);
	}
	{ /* TabFill */
		char *tabfill;
		char delimtabfill[300];
		(st[(long) TNtabfill])->ClearBits();
		tabfill = (self->curstyle)->GetAttribute("tabfill");
		if(tabfill != NULL) {
		    sprintf(delimtabfill, "[%s]", tabfill);
		    (st[(long)TNtabfill])->SetBitOfEntry ((st[(long)TNtabfill])->AddString( delimtabfill), 1);
		}
	}
	{ /* Tabs */
		long numTabChanges;
		struct tabentry **TabChangeArray;
		long i;
		class tabs *tabs;
		tabs = tabs::Create();
		(style)->GetTabChangeList( &numTabChanges, &TabChangeArray);
		for (i = 0; i < numTabChanges; i++)
			tabs = (tabs)->ApplyChange( TabChangeArray[i]);
		if (TabChangeArray)
			free(TabChangeArray);
		(self->tabrulerv)->SetValues( tabs);
	}
	self->NeedsUnpack = FALSE;
}

static const char *
InitialWord(const char  *s)
	{
	static char buf[50];
	char *bx;
	const char*sx;
	long i;
	if(s==NULL) {
	    buf[0]='\0';
	    return buf;
	}
	for (bx=buf, sx=s, i=48;   i && *sx && *sx != '~' && *sx != ',';   i--)
		*bx++ = *sx++;
	*bx++ = '\0';
	return buf;
}

/* clean(s) 
	removes spaces and non-alphabetics.  It changes uppercase to lower 
	result must be less than 50 bytes 
	returns a pointer to an internal buffer
*/
	static const char *
clean(const char  *s)
	{
	static char buf[50];
	char *bx;
	const char*sx;
	long i;
	if(s==NULL) {
	    buf[0]='\0';
	    return buf;
	}
	for (bx=buf, sx=s, i=48;   i && *sx;   i--) {
		char c = *sx++;
		if ( ! isalnum(c)) {}
		else if (isupper(c)) 
			*bx++ = tolower(c);
		else
			*bx++ = c;
	}
	*bx++ = '\0';
	return buf;
}

/* SetStyleDisplay(self, st)
	initiates editing the style 'st'
*/
	static void
SetStyleDisplay(class lookzview  *self, class style  *st)
		{
    if(st) {
	self->curstyle = self->curss->MakeWritable(st->GetName());
	UnpackStyle(self);
    } else self->curstyle=NULL;
}

/* ClearStyleDisplay(self)
	removes highlights from all attributes in the tableux
*/

	static void
ClearStyleDisplay(class lookzview  *self)
	{
	long i;
	for (i=(long)TNfont; i<=(long)TNcolor; i++)
		(self->st[i])->ClearBits();

	(self->rulerview)->SetValues( lprrulerview_NoValue,
			 lprrulerview_NoValue, lprrulerview_NoValue);
	(self->tabrulerv)->SetValues( NULL);
}

/* CloseStyleSheet(self)
	terminates editing the current style
*/
	static void
CloseStyleSheet(class lookzview  *self)
	{
	/* The document is updated for all changes, so there is no need
	to do anything about the style currently displayed */
	
	if ( ! self->foundstylesheet) return;
	ClearStyleDisplay(self);
	(self->st[(long)TNmenucard])->Clear();
	(self->st[(long)TNstylename])->Clear();
	self->foundstylesheet = FALSE;
	(self->curss)->RemoveObserver( self);
	self->curss = NULL;
	self->curstyle = NULL;
}

/* called by EnumerateStyles in OpenStyleSheet */
	static boolean
AddCardName(class style  *s, class stringtbl  *st)
		{
	const char *mnnm = (s)->GetMenuName();
	if (mnnm != NULL)    (st)->AddString( InitialWord(mnnm));
	return FALSE;
}

	static void
OpenStyleSheet(class lookzview  *self, class stylesheet  *ss)
		{
	self->curss = ss;
	if (ss != NULL) {
		(ss)->AddObserver( self);
		/* set up the cardmenu and stylename data objects */
		(self->curss)->EnumerateStyles((stylesheet_efptr)AddCardName,
					   (long) self->st[(long)TNmenucard]);
		(self->st[(long)TNmenucard])->AddString( NOMENUSTRING);
	}
	self->foundstylesheet = TRUE;
}


static char *TempStyleName;  /* holds the stylename during 
		EnumerateStyles (...FindAndUnpack...) */
/* FindAndUnPack is called by Enumerate styles in HitStyleName
		it finds the style named by lv->curcard and TempStyleName
		then it unpacks that style */
	static boolean
FindAndUnpack(class style  *s, class lookzview  *lv)
		{
	const char *mnnm = (s)->GetMenuName();
	if (mnnm != NULL && ULstrcmp(lv->curcard, InitialWord(mnnm)) == 0) {
		const char *ThisStylename = strchr(mnnm, ',');
		if (ThisStylename
				&& ULstrcmp(InitialWord(ThisStylename+1), 
							TempStyleName) == 0) {
			SetStyleDisplay(lv, s);
			return TRUE;  /* finished */
		}
	}
	return FALSE;
}

	static boolean
FindAndUnpackNoMenus(class style  *s, class lookzview  *lv)
		{
	const char *mnnm = (s)->GetMenuName();
	if (mnnm == NULL) {
		const char *ThisStylename = (s)->GetName();
		if (ThisStylename
				&& ULstrcmp(InitialWord(ThisStylename), 
							TempStyleName) == 0) {
			SetStyleDisplay(lv, s);
			return TRUE;  /* finished */
		}
	}
	return FALSE;
}

/* HitStyleName is called as an ItemHitProc for the stylename stringtbl */
	static void
HitStyleName(class stringtbl  *st1, class lookzview  *lv, long  accnum)
			{
	strtblview::OneOnly(st1, lv, accnum);
	ClearStyleDisplay(lv);
	TempStyleName = (char *) 
			(lv->st[(long)TNstylename])->GetStringOfEntry(
			accnum);	/* side arg to FindAndUnpack KLUDGE */
	(lv->curss)->EnumerateStyles((stylesheet_efptr)FindAndUnpack, (long) lv);
}

	static void
HitStyleNameNoMenus(class stringtbl  *st1, class lookzview  *lv, long  accnum)
			{
	strtblview::OneOnly(st1, lv, accnum);
	ClearStyleDisplay(lv);
	TempStyleName = (char *) 
			(lv->st[(long)TNstylename])->GetStringOfEntry(
			accnum);	/* side arg to FindAndUnpack KLUDGE */
	(lv->curss)->EnumerateStyles((stylesheet_efptr)FindAndUnpackNoMenus, (long) lv);
}

/* called by EnumerateStyles in HitMenuCard */
	static boolean
AddStyleName(class style  *s, class lookzview  *lv)
		{
	const char *mnnm = (s)->GetMenuName(), *ThisStylename;
	if (mnnm != NULL && ULstrcmp(lv->curcard, InitialWord(mnnm)) == 0) {
		ThisStylename = strchr(mnnm, ',');
		if (ThisStylename) 
			(lv->st[(long)TNstylename])->AddString( 
				(char *)InitialWord(ThisStylename+1));
	}
	return FALSE;
}

	static boolean
AddStyleNameNoMenus(class style  *s, class lookzview  *lv)
		{
	const char *mnnm = (s)->GetMenuName(), *ThisStylename;
	if (mnnm == NULL) {
		ThisStylename = (s)->GetName();
		if (ThisStylename) {
			(lv->st[(long)TNstylename])->AddString( 
				(char *)InitialWord(ThisStylename));
		}
	}
	return FALSE;
}

/* called as an ItemHitProc for stringtbl[TNmenucard] 
		set up the stylename stringtbl */
	static void
HitMenuCard(class stringtbl  *st0, class lookzview  *lv, long  accnum)
			{
	strtblview::OneOnly(st0, lv, accnum);
	(lv->st[(long)TNstylename])->Clear();
	ClearStyleDisplay(lv);
	lv->curcard = (char *)(st0)->GetStringOfEntry( accnum);
	lv->curstyle = NULL;
	if (ULstrcmp(lv->curcard, NOMENUSTRING) != 0) {
		(lv->curss)->EnumerateStyles((stylesheet_efptr)AddStyleName, (long) lv);
		(lv->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) 
					HitStyleName, (long)lv);
	}
	else {
		(lv->curss)->EnumerateStyles((stylesheet_efptr)AddStyleNameNoMenus, 
					(long) lv);
		(lv->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) 
					HitStyleNameNoMenus, (long)lv);
	}
}

static char newString[300], newString2[302];

	static char *
GetStringValue(class stringtbl  *st, class lookzview  *self, short  accnum, const char  *prompt)
				{
	char *str;
	str = (char *) (st)->GetStringOfEntry( accnum);
	if (str && ULstrcmp(str, "<other>") == 0) {
		long ans = message::AskForString(self, 0, prompt, NULL, 
				newString, sizeof(newString));
		(st)->SetBitOfEntry( accnum, 0);

		if (ans == -1) {
			UnpackStyle(self);
			return NULL;
		}
		str = newString;
		accnum = (st)->GetEntryOfString( str, 0);
		if (accnum < 0) {
			(st)->RemoveString( "<other>");
			accnum = (st)->AddString( str);
			(st)->AddString( "<other>");
		}
		(st)->SetBitOfEntry( accnum, 1);
	}
	return str;
}

/* this is like GetStringValue, but it sticks brackets around the string in the display list. */
	static char *
GetDelimitedStringValue(class stringtbl  *st, class lookzview  *self, short  accnum, const char  *prompt)
				{
	char *str;
	int len;

	str = (char *) (st)->GetStringOfEntry( accnum);
	if (str && ULstrcmp(str, "<other>") == 0) {
		long ans = message::AskForString(self, 0, prompt, NULL, 
				newString, sizeof(newString));
		(st)->SetBitOfEntry( accnum, 0);

		if (ans == -1) {
			UnpackStyle(self);
			return NULL;
		}
		str = newString;
		sprintf(newString2, "[%s]", str);
		accnum = (st)->GetEntryOfString( newString2, 0);
		if (accnum < 0) {
			(st)->RemoveString( "<other>");
			accnum = (st)->AddString( newString2);
			(st)->AddString( "<other>");
		}
		(st)->SetBitOfEntry( accnum, 1);
	}
	else if (str) {
	    if (*str == '[')
		str++;
	    strcpy(newString, str);
	    len = strlen(newString);
	    if (newString[len-1] == ']')
		newString[len-1] = '\0';
	    str = newString;
	}
	return str;
}

/* called as an ItemHitProc for stringtbl[TNfont] 
		modify font  in the stylesheet */
	static void
HitFont(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum)) {
		char *str =  GetStringValue(st, self, accnum, "New font: ");
		if (str != NULL) 
			(self->curstyle)->SetFontFamily( str);
	}
	else {
		/* delete font reference from style.   
			KLUDGE KLUDGE: style_SetFontFamily 
			is BOGUS: we do it directly here */
		if (self->curstyle->FontFamily != NULL) {
			free(self->curstyle->FontFamily);
			self->curstyle->FontFamily = NULL;
		}
	}
	ChangeStyle(self);
}

/* WARNING:  KLUDGE KLUDGE
	style.c, style.ch, and default.tpl treat the value of FontSize as Points,
		but do not shift the value left 16 bits.
	the same modules treat the FontScriptMovement value as points, 
		but DO shift the values left 16 bits.
	The code below reflects this "convention".
*/

/* called as an ItemHitProc for stringtbl[TNsize] 
		modify size  in the stylesheet */
	static void
HitSize(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	enum style_FontSize basis;
	long points;
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum)) {
		char *str =  GetStringValue(st, self, accnum, 
			"New font size: ");
		if (str != NULL) {
			MapStringToDelta(str, &points, &basis);
			(self->curstyle)->SetFontSize( basis, points);
		}
	}
	else {
		points = 0, basis = style_PreviousFontSize;
		(self->curstyle)->SetFontSize( basis, points);
	}
	ChangeStyle(self);
}

/* called as an ItemHitProc for stringtbl[TNmode] 
		modify JustificationMode  in the stylesheet */
	static void
HitMode(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum)) {
		/* (The braces around this clause are NECESSARY because
			style_SetJustification is surrounded by braces) */
		(self->curstyle)->SetJustification( 
			(enum style_Justification)MapStringToVal(
					ModeNames, 
					((char *)(st)->GetStringOfEntry(accnum))));
	}
	else 
		(self->curstyle)->SetJustification( style_PreviousJustification);
	ChangeStyle(self);
}

/* called as an ItemHitProc for stringtbl[TNsubscr] 
		modify subscription  in the stylesheet */
	static void
HitSubscr(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	enum style_ScriptMovement  basis = style_PreviousScriptMovement;
	long points = 0;
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum)) {
		char *str =  GetStringValue(st, self, accnum, "New script position: ");

		if (str != NULL) {
			points = MapStringToVal(SubScrNames, str);
			if (points == -999L) 
				MapStringToDelta(str, &points, 
						(enum style_FontSize *)&basis);
			(self->curstyle)->SetFontScript( basis, points, 
						style_Points);
		}
	}
	else 
		(self->curstyle)->SetFontScript( basis, points, style_Points);
	ChangeStyle(self);
}

/* FinagleStyleBits is called by HitEnable and HitDisable to modify the
	XxxFontFaces or XxxMiscFlags fields of the style.
	KLUDGE KLUDGE  This routine does not use the
	access procedures defined in style.ch;  that module 
	DOES NOT DEFINE six of the eight functions that are required 
*/
	static void
FinagleStyleBits(class lookzview  *self, class stringtbl  *st, long  accnum, long  parity, unsigned long  *faces , unsigned long  *flags)
					{
	char *str;
	unsigned long bit;
	unsigned long *which;
	long val;

	str =  GetStringValue(st, self, accnum, "New attribute: ");
	if(str==NULL) return;
	bit = (st)->GetBit( str);
	val = MapStringToVal(FaceNames, str);
	if (val != -999L) 
		which = faces;
	else {
		val = MapStringToVal(FlagNames, str);
		if (val != -999L) 
			which = flags;
		else {
			/* unknown str value */
			(st)->RemoveString( str);
			return;	
		}
	}
	if (bit ^ parity)
		*which |= val;		/* OR in a bit */
	else
		*which &= ~val;	/* remove a bit */
}

/* called as an ItemHitProc for stringtbl[TNenable] 
		modify enable flags  in the stylesheet */
	static void 
HitEnable(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrMany(st, self, accnum);
	FinagleStyleBits(self, st, accnum, 0,
			((unsigned long *)&self->curstyle->AddFontFaces), 
			((unsigned long *)&self->curstyle->AddMiscFlags));
	ChangeStyle(self);
}
/* called as an ItemHitProc for stringtbl[TNdisable] 
		modify disable flags  in the stylesheet */
	static void
HitDisable(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrMany(st, self, accnum);
	FinagleStyleBits(self, st, accnum, 1,
			((unsigned long *)&self->curstyle->OutFontFaces), 
			((unsigned long *)&self->curstyle->OutMiscFlags));
	ChangeStyle(self);
}

	static void
HitSpacing(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	enum style_SpacingValue basis;
	long points;
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum))  {
		char *str =  GetStringValue(st, self, accnum, 
					"New interline spacing: ");
		if (str != NULL) {
			if (*str == '+' || *str == '-') 
				basis = style_InterlineSpacing;
			else 
				basis = style_ConstantSpacing;
			points = atoi(str);
			(self->curstyle)->SetNewInterlineSpacing( basis, 
					points, style_Points);
		}
	}
	else {
		points = 0;
		basis = style_ConstantSpacing;
		(self->curstyle)->SetNewInterlineSpacing( basis, 
				points, style_Points);
	}
	ChangeStyle(self);
}

	static void
HitSpread(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	enum style_SpacingValue basis;
	long points;
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum))  {
		char *str =  GetStringValue(st, self, accnum, 
					"New interparagraph spacing: ");

		if (str != NULL) {
			if (*str == '+' || *str == '-') 
				basis = style_InterparagraphSpacing;
			else 
				basis = style_ConstantSpacing;
			points = atoi(str);
			(self->curstyle)->SetNewInterparagraphSpacing( 
					basis, points, style_Points);
		}
	}
	else {
		points = 0;
		basis = style_ConstantSpacing;
		(self->curstyle)->SetNewInterparagraphSpacing( basis, 
				points, style_Points);
	}
	ChangeStyle(self);
}

	static void 
HitColor(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum))  {
		char *str =  GetStringValue(st, self, accnum, "New color: ");
		(self->curstyle)->AddAttribute( "color", str);
	}
	else 
		(self->curstyle)->RemoveAttribute("color");
	ChangeStyle(self);
}

	static void 
HitTabFill(class stringtbl  *st, class lookzview  *self, long  accnum)
			{
	if (self->curstyle == NULL) return;
	strtblview::ZeroOrOne(st, self, accnum);
	if ((st)->GetBitOfEntry( accnum))  {
		char *str =  GetDelimitedStringValue(st, self, accnum, "New tab fill string: ");
		(self->curstyle)->AddAttribute( "tabfill", str);
	}
	else 
		(self->curstyle)->RemoveAttribute("tabfill");
	ChangeStyle(self);
}

	static void
HitShrink(class labelview  *shrink, enum view::MouseAction   action, class lookzview  *self)
			{
	class lookz *lookz;
	if (action != view::LeftDown && action != view::RightDown)
		return;
	lookz = (class lookz *) (self)->GetDataObject();
	if (! (self)->GetVisibility() ||  (lookz)->GetCanClose()) {
		(self)->SetVisibility( ! (self)->GetVisibility());
		(self)->WantNewSize( self);
		(self)->WantInputFocus( self);
	}
}
	

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * 
 *	Now the routines for the view 
 *		(Above were the routines for the application
 *	
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	void
ChooseShrinkIcon(class lookzview  *self)
	{
	((class label *)(self->shrinkicon)->GetDataObject())->SetText( 
			(char *)((self->HasInputFocus) ? "-" : ","));
}

/* LL - generate a horizontal or vertical pair of views 
	first arg is H or V to choose direction, second is percent of split,
	third and fourth args are the views 
	all boundaries are movable */

	static class lpair *
LL(enum dir  dir, long  pct, class lpair  *top , class lpair  *bot)
			{
	/* XXX (sigh) would really like lines to be movable, 
			but the wm has a terrible time leak in that area */
	class lpair *ret = new lpair;
	if (dir == H) 
		(ret)->HSplit( top, bot, pct, FALSE);
	else
		(ret)->VSplit( top, bot, pct, FALSE);
	return ret;
}

/* LBL - generate an lpair with a fixed size top having a label */
	static class lpair *
LBL (const char  *labelp, class view  *view)
		{
	static class fontdesc *asb8 = NULL;
	static class im *icache = NULL;
	static int hcache = 13;

	class im *im;	
	int w;
	class label *l = new label;
	class labelview *lv = new labelview;

	(l)->SetFont("andysans", fontdesc_Bold, 8);
	(l)->SetText(labelp);
	(l)->SetFlags(label_VCENTERJUST);
	(lv)->SetDataObject((class dataobject *) l);
	im = (class im *)(view)->GetIM();
	if (im == NULL) 
		im = (class im *)im::GetLastUsed();
	if (im == NULL)
		hcache = 13;
	else if (icache != im) {
		icache = im;
		if (asb8 == NULL)
			asb8 = fontdesc::Create("andysans", fontdesc_Bold, 8);
		(asb8)->StringBoundingBox( 
				(class graphic *)(icache)->GetDrawable(),
				"Xy", &w, &hcache);
		DEBUG(("LBL top height - im: 0x%lx    asb8: 0x%lx    h: %d\n", 
				im, asb8, hcache+5)); 
	}
	return  (class lpair *)((new bpair)->VTFixed( lv, view, 
					hcache+5, FALSE));
}

#define Text(v)	(class text *) ((v)->GetDataObject())

	static void
EditStylesInWindow(class view  *textv)
	{
	class text *d;
	class lookz *newobject;
	int i;
	int openSlot = -1;
	class buffer *buffer = NULL;

	d = (class text *) (textv)->GetDataObject();

	for (i = 0; i < numStyleEditors; i++) {
		if (styleEditors[i].text == d) {
			buffer = buffer::FindBufferByData(styleEditors[i].lookz);
			break;
		}
		else if (styleEditors[i].text == NULL) 
			openSlot = i;
	}

	if (buffer == NULL) {
		if ((newobject = (class lookz *) ATK::NewObject("lookz")))  {
			char bufferName[50];

			(newobject)->SetTextObject( d);
			(newobject)->SetCanClose( FALSE);
			buffer::GetUniqueBufferName("Style-Editor", bufferName,
					sizeof(bufferName));
			buffer = buffer::Create(bufferName, NULL, "lookzview",
					newobject);
			(buffer)->SetDestroyData( TRUE);

			if (openSlot == -1) {
				if (numStyleEditors == maxStyleEditors) {
					if (maxStyleEditors == 0) {
						maxStyleEditors = 4;
						styleEditors = (struct styleeditorlist *)
							 malloc(sizeof(
									struct styleeditorlist) 
								* maxStyleEditors);
					}
					else {
						maxStyleEditors += 4;
						styleEditors = (struct styleeditorlist *)
							realloc(styleEditors, 
								sizeof(struct styleeditorlist) 
								* maxStyleEditors);
					}
				}
				openSlot = numStyleEditors++;
			}
			styleEditors[openSlot].text = d;
			styleEditors[openSlot].lookz = newobject;
		}
	}
	if (buffer != NULL) {
		/* get a view on the buffer */
		class frame *frame = frame::GetFrameInWindowForBuffer(buffer);
		class view *view;
		class im *im;
		if (frame == NULL || (im = (frame)->GetIM()) == NULL) {
			message::DisplayString(textv, 50, "Couldn't find window.");
			return;
		}
		view = (frame)->GetView();
		(view)->WantInputFocus(view);
		/* pop to top window */
		(im)->ExposeWindow();
	}
}

	static  void
NewTextObject(class lookzview  *self, class text  *text)
		{
	CloseStyleSheet(self);
	if (text != NULL) 
		(self)->SetStyleSheet( (text)->GetStyleSheet());
	(self)->WantUpdate( self);
}

/* we defer building the image until LinkTree so LBL can compute label size */
	static void
BuildImage(class lookzview   *self)
	{
	class lpair *R[7];
	class labelview *emptylabelview;
	class label *emptylabel;

	emptylabel = new label;
	emptylabelview = new labelview;
	(emptylabel)->SetFont( "icon", fontdesc_Plain, 12);
	(emptylabel)->SetText( "");
	(emptylabelview)->SetDataObject((class dataobject *) emptylabel);

	/* build image 
		there are ten apparent rectangles arranged like this:
	
	icon		MENUCARD		MENUNAME

	FONT  	ENABLE	DISABLE	JUSTIFI-	BASE-
				CATION		LINE
	FONTSIZE		 		COLOR

	LINE SPACING			PARAGRAPH SPACING
		
	MARGINS AND PARAGRAPH INDENT RULER

	TABS RULER

	All but the icon rectangle have a small upper rectangle for the label.
	There is a small and empty label icon between the top row and the rest.
	There are seven groups of rectangles:
		R[0]	(icon, MENUCARD, MENUNAME)
		R[1]	(FONT, ENABLE, DISABLE)
		R[2]	(JUSTIFICATION, BASELINE)
		R[3]	(FONTSIZE, COLOR)
		R[4]	(MARGIN/PARA RULER)
		R[5]	(VERTICAL SPACING)
		R[6]	(TABRULER)
	We first create each group and then arrange them as a whole.
*/

	R[0] = LL(V, 4, 
		self->shrinkparent = LL(H, 94,
			(class lpair *)self->shrinkicon,
			LL (H, 69,
				LBL("menu card", 
					(class view *)self->stv[(long)TNmenucard]),
				LBL("name on menu card", 
					(class view *)self->stv[(long)TNstylename])
			)
		),
		(class lpair *)emptylabelview  /* dummy for thin line below menu stuff */
	);
	R[1] = LL(H, 70,
		LBL("font", (class view *)self->stv[(long)TNfont]),
		LL(H, 50,
			LBL("enable", (class view *)self->stv[(long)TNenable]),
			LBL("disable", (class view *)self->stv[(long)TNdisable])
		)
	);
	R[2] = LL(H, 50,
		LBL("justify", (class view *)self->stv[(long)TNmode]),
		LBL("baseline", (class view *)self->stv[(long)TNsubscr])
	);
	R[3] = LL(H, 20,
		LL(H, 50,
			LBL("font size", (class view *)self->stv[(long)TNsize]),
			LBL("color", (class view *) self->stv[(long)TNcolor])
		), 
		LBL("tab filler", (class view *) self->stv[(long)TNtabfill])
	);
	R[4] = LBL("relative margins and paragraph indent", 
				(class view *)self->rulerview);
	R[5] = LL(H, 50,
		  LBL("line spacing", (class view *) self->stv[(long) TNspacing]),
		  LBL("paragraph spacing", (class view *) self->stv[(long) TNspread]));
	R[6] = LBL("tab stops - relative to left margin", 
				(class view *)self->tabrulerv);

/* copy the following formulas and paste them in a 6x6 table inset to adjust the percentages
and see the resulting vertical sizes.  For the percentages 30, 39, 60, 50, 50, 
the resulting split of 700 vertical pixels is: 119.6 179.3 95.6 95.6 105.0 105.0

{F} =.30	{F1} 700	{F1} =[r,c-1]*(1-[r,c-2])	{F1} =[r,c-2]*[r,c-3]	{F1} =[r+2,c-2]	menus
{F} =.39	{F1} =[r-1,c+1]	{F1} =[r,c-1]*(1-[r,c-2])	{F1} =[r,c-2]*[r,c-3]	{F1} =[r+1,c-1]	font...
{F} =.6	{F1} =[r-1,c+1]	{F1} =[r,c-1]*(1-[r,c-2])	{F1} =[r,c-2]*[r,c-3]	{F1} =[r+1,c-2]	fontsize...
{F} =.5	{F1} =[r-2,c+2]	{F1} =[r,c-1]*(1-[r,c-2])	{F1} =[r,c-2]*[r,c-3]	{F1} =[r,c-1]	spacing
{F} =.5	{F1} =+[r-4,c+2]	{F1} =[r,c-1]*(1-[r,c-2])	{F1} =[r,c-2]*[r,c-3]	{F1} =[r,c-2]	margins
 	{F1}  	{F1}  	{F1}  	{F1} =[r-1,c-1]	tabs
*/

	self->image = 
		LL(V, 30,
			LL(V, 39,
			    LL(V, 60,
				R[0],		/* menucard and menuname */
				LL(H, 33,
				    R[1],		/* font, enable, disable */
				    R[2]				/* justification and baseline */
				)
			    ),
			    LL(V, 50,
				R[3],		/* fontsize   color*/
				R[5]		/* spacing */
			    )
			),
			LL(V, 50,
			    R[4],			/* ruler */
			    R[6]			/* tabs */
			)
		);

	ChooseShrinkIcon(self);
}

	boolean
lookzview::InitializeClass()
	{
	struct ATKregistryEntry  *textviewClassinfo;

	textviewClassinfo = ATK::LoadClass("textview");
	proctable::DefineProc("lookzview-edit-styles", (proctable_fptr)EditStylesInWindow, 
			textviewClassinfo, NULL,
			"Bring up style editor in a seprate window");

	MenuList = new menulist;
	Keymap = new keymap;
	AutoUpdate = environ::GetProfileSwitch("lookzautoupdate", FALSE);
	if (AutoUpdate) {
	    /* Skip first menu entry (Update Document) */
	    bind::BindList(MenuOptions+1, Keymap, MenuList,
		   &lookzview_ATKregistry_ );
	} else {
	    bind::BindList(MenuOptions, Keymap, MenuList,
		   &lookzview_ATKregistry_ );
	}
	/* We *really* want the default font to be smaller - it just looks
		Sooo gross if you have the default 12pt thing... 
		so while we're at it, let the user have a choice */
	StrtblFont = fontdesc::Create("andysans", fontdesc_Plain,
			environ::GetProfileInt("lookzfontsize", 10));

	return TRUE;
}

	lookzview::lookzview()
		{
	ATKinit;

	long i, j;
	class label *shrinklabel;

ENTER(lookzview::lookzview);
	this->MyMenus = (MenuList)->DuplicateML( this);
	this->Keystate = keystate::Create(this, Keymap);
	this->embedded = TRUE;
	this->foundstylesheet = FALSE;
	this->curcard = NULL;
	this->curss = NULL;
	this->curstyle = NULL;
	this->HasInputFocus = FALSE;
	this->NeedsUnpack = FALSE;
	this->OnScreen = FALSE;
	this->Linked = FALSE;
	this->OnceOnlyInUpdate = FALSE;	/* We haven't been thru yet */

	shrinklabel = new label;
	(shrinklabel)->SetFont( "icon", fontdesc_Plain, 12);
	this->shrinkicon = new labelview;
	(this->shrinkicon)->SetHitProc( (labelview_hitfptr) HitShrink, 
			(char *) this);
	(this->shrinkicon)->SetDataObject((class dataobject *) shrinklabel);
	/* defer initial ChooseShrinkIcon because it will call SetText,
		which calls NotifyObservers, 
		which calls ObservedChanged in labelview, 
		which calls WantNewSize, 
		which requires having a parent 
		(The error is in having SetText call NotifyObservers
		rather than have that done by the caller of SetText.)
	*/

	for (i = 0; InitialStrings[i]; i++) {
		this->st[i] = new stringtbl;
		for (j = 0; InitialStrings[i][j]; j++)
			(this->st[i])->AddString( 
				(char *)InitialStrings[i][j]);
		this->stv[i] = new strtblview;
		(this->stv[i])->SetDataObject((class dataobject *) this->st[i]);
	}
	(this->stv[(long)TNmenucard])->SetItemHitProc((strtblview_hitfptr) HitMenuCard, (long)this);
	(this->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) HitStyleName, (long)this);
	(this->stv[(long)TNfont])->SetItemHitProc((strtblview_hitfptr) HitFont, (long)this);
	(this->stv[(long)TNsize])->SetItemHitProc((strtblview_hitfptr) HitSize, (long)this);
	(this->stv[(long)TNmode])->SetItemHitProc((strtblview_hitfptr) HitMode, (long)this);
	(this->stv[(long)TNsubscr])->SetItemHitProc((strtblview_hitfptr) HitSubscr, (long)this);
	(this->stv[(long)TNenable])->SetItemHitProc((strtblview_hitfptr) HitEnable, (long)this);
	(this->stv[(long)TNdisable])->SetItemHitProc((strtblview_hitfptr) HitDisable, (long)this);
	(this->stv[(long)TNspacing])->SetItemHitProc((strtblview_hitfptr) HitSpacing, (long)this);
	(this->stv[(long)TNspread])->SetItemHitProc((strtblview_hitfptr) HitSpread, (long)this);
	(this->stv[(long)TNcolor])->SetItemHitProc((strtblview_hitfptr) HitColor, (long)this);
	(this->stv[(long)TNtabfill])->SetItemHitProc((strtblview_hitfptr) HitTabFill, (long)this);

	this->ruler = new lprruler;
	this->tabruler = new lprruler;
	this->tabrulerv = new tabrulerview;
	this->rulerview = new lprrulerview;
	(this->tabrulerv)->SetDataObject((class dataobject *) this->tabruler);
	(this->rulerview)->SetDataObject((class dataobject *) this->ruler);
	(this->rulerview)->SetValues( lprrulerview_NoValue,
			 lprrulerview_NoValue, lprrulerview_NoValue);
	(this->rulerview)->SetValueChangeProc((lprrulerview_valuefptr)RulerValueChanged, (long)this);
	(this->tabrulerv)->SetValues( NULL);
	(this->tabrulerv)->SetValueChangeProc((tabrulerview_valuefptr) TabsValueChanged, (long)this);
	
	this->image = NULL;
	this->shrinkparent = NULL;
	/* image is assembled in BuildImage(), called from LinkTree  */
LEAVE(lookzview::lookzview);

	THROWONFAILURE( TRUE);
}

static void
ChopTree(class lpair  *branch, struct ATKregistryEntry   *lpairInfo)
		{
	/* destroy both children if the branch is either an lpair or
		a sub class of lpair (such as bpair) 
		XXX use knowledge that class_GetType returns ptr to classinfo */
	if (branch == NULL) {}
	else if (branch->IsType(lpairInfo)) {
		class view* o0 = (branch)->GetNth( 0);
		class view* o1 = (branch)->GetNth( 1);
		(branch)->SetNth( 0, NULL);
		(branch)->SetNth( 1, NULL);
		(branch)->UnlinkTree();
		ChopTree((class lpair *)o0, lpairInfo);
		ChopTree((class lpair *)o1, lpairInfo);
		(branch)->Destroy();
	}
	else {
		class view *v = (class view *)branch;
		/* destroy a childview and its data object */
		class dataobject *dobj = ((class view *)branch)->GetDataObject();
		(v)->UnlinkTree();
		(v)->Destroy();
		(dobj)->Destroy();
	}
}

lookzview::~lookzview()
		{
	ATKinit;

ENTER(lookzview::~lookzview);
	if (this->foundstylesheet)
		(this->curss)->RemoveObserver( this);

	(this)->SetVisibility( TRUE);
	(this)->LinkTree( this->GetParent());

	ChopTree(this->image, (this->image)->ATKregistry());

	delete this->MyMenus;
	delete this->Keystate;
LEAVE(lookzview::~lookzview);
}

class view *
lookzview::GetApplicationLayer()
	{
	this->embedded = FALSE;
	/* XXX ought to delete the shrink-icon from the view tree */
	(this)->WantInputFocus( this);
	return this;
}

	void
lookzview::LinkTree(class view  *parent)
		{
ENTER(lookzview::LinkTree);
	(this)->view::LinkTree( parent);
	if (this->image == NULL)
		BuildImage(this);
	if ((this)->GetVisibility()) {
		(this->shrinkicon)->LinkTree( NULL);
		(this->shrinkparent)->SetNth( 0, this->shrinkicon);
		(this->image)->LinkTree( this);
	}
	else {
		(this->image)->LinkTree( NULL);
		(this->shrinkparent)->SetNth( 0, NULL);
		(this->shrinkicon)->LinkTree( this);
	}
	this->Linked = TRUE;
LEAVE(lookzview::LinkTree);
}

/* intercept all requests from children for inputfocus 
		and cause the lookz itself to request inputfocus */
	void 
lookzview::WantInputFocus(class view  *child)
		{
ENTER(lookzview::WantInputFocus);
	if ( ! this->HasInputFocus)
		(this)->view::WantInputFocus( this);
LEAVE(lookzview::WantInputFocus);
}

	void 
lookzview::ReceiveInputFocus()
	{
ENTER(lookzview::ReceiveInputFocus);
	(this)->PostMenus( this->MyMenus);

	this->Keystate->next = NULL;
	(this)->PostKeyState( this->Keystate);
	this->HasInputFocus = TRUE;
	if (this->Linked) {
		ChooseShrinkIcon(this);
		(this)->WantUpdate(  this );
	}
LEAVE(lookzview::ReceiveInputFocus);
}

	void 
lookzview::LoseInputFocus()
	{
ENTER(lookzview::LoseInputFocus);
	this->HasInputFocus = FALSE;
	ChooseShrinkIcon(this);
	(this)->WantUpdate( this);
LEAVE(lookzview::LoseInputFocus);
}

	void 
lookzview::FullUpdate(enum view::UpdateType   type, long  left , long  top , long  width , long  height)
			{
	struct rectangle r;
	int i;
	enum view::UpdateType lpairtype;

	this->OnScreen = (type != view::Remove);
	if (! this->Linked)
		(this)->LinkTree( this->GetParent());
	if (type == view::FullRedraw)
		(this)->GetLogicalBounds( &r);
	else 
		rectangle_SetRectSize(&r, left, top, width, height);

DEBUG(("FullUpdate type %d  redraw (%d,%d,%d,%d) within (%d,%d,%d,%d)\n", 
			type, left, top, width, height, r.left, r.top, r.width, r.height));
DEBUG(("	Drawable at 0x%lx   Visibile: %s \n", (this)->GetDrawable(),
			((this)->GetVisibility()) ? "yes" : "no"));

	if ((type == view::FullRedraw || type == view::LastPartialRedraw)
			 && ! this->foundstylesheet) {
		/* BOGOSITY ALERT:  we grub around in the parent to
		  find its stylesheet */
		class view *v;
		for (v = (class view *)this; v != NULL; v = v->GetParent()) 
			if ((v)->IsType("textview")) {
				class text *t = (class text *)v->GetDataObject();
				if (t && t->styleSheet) 
					OpenStyleSheet(this, t->styleSheet);
				break;
			}
	}
	if (type == view::FullRedraw && this->embedded && this->OnScreen) {
		(this)->SetTransferMode( graphic::COPY);
		(this)->MoveTo( 0, 0);
		(this)->DrawLineTo( r.width-1, 0);
		(this)->DrawLineTo( r.width-1, r.height-1);
		(this)->DrawLineTo( 0, r.height-1);
		(this)->DrawLineTo( 0, 0);

		r.top++, r.left++, r.height-=2, r.width-=2;
	}

	if ((this)->GetVisibility()) {
		if (type != view::PartialRedraw 
			/* && type != view::LastPartialRedraw */) {
			(this->image)->InsertView( this, &r);
		}
		/* Now that we are here, the strtbl's have been instantiated,
		   so we can set their font's to something reasonable! */
		/* This should only be done once... */
		if (!this->OnceOnlyInUpdate) {
			for (i = 0; InitialStrings[i]; i++)
				(this->stv[i])->SetFont( StrtblFont);
			this->OnceOnlyInUpdate = TRUE;
		}

		lpairtype = (type == view::LastPartialRedraw) ? view::FullRedraw : type;
		DEBUG(("	FullUpdate lpair with type %d\n", lpairtype));
		(this->image)->FullUpdate( lpairtype, left, top, width, height);
	} else {
		if (type != view::PartialRedraw 
			 && type != view::LastPartialRedraw) 
			(this->shrinkicon)->InsertView( this, &r);
		(this->shrinkicon)->FullUpdate(  type, left, top, width, height);
	}
	LEAVE(lookzview::FullUpdate);
}

	void 
lookzview::Update( )
	{
ENTER(lookzview::Update);
	if (! this->OnScreen) return;
	if ( ! this->Linked)
		(this)->FullUpdate( view::FullRedraw, 0, 0,
				(this)->GetLogicalWidth(),
				(this)->GetLogicalHeight());
	else if ((this)->GetVisibility()) {
		if (this->NeedsUnpack) 
			UnpackStyle(this);
		(this->image)->Update();
	}
	else 
		(this->shrinkicon)->Update();
LEAVE(lookzview::Update);
}

	class view *
lookzview::Hit(enum view::MouseAction   action, long   x , long   y , long   num_clicks)
			{
	class view *ret;
	boolean oldvis=(this)->GetVisibility();
ENTER(lookzview::Hit);
	if (action == view::NoMouseEvent)
		return (class view *)this;
	if ( ! this->OnScreen) return NULL;
DEBUG(("	OldVis %d\n", oldvis));
	if (oldvis)
		ret = (class view *)(this->image)->Hit( action, x, y, num_clicks);
	else
		ret = (class view *)(this->shrinkicon)->Hit( action, x, y, 
				num_clicks);
LEAVE(lookzview::Hit);
	if (oldvis == (this)->GetVisibility())
		return ret;
	else return (class view *)this;	/* visibility changed */
}

	view::DSattributes
lookzview::DesiredSize( long  width, long  height, enum view::DSpass  pass, 
				long  *desiredWidth, long  *desiredHeight ) 
						{
	if ((this)->GetVisibility()) 
		*desiredWidth = 550,  *desiredHeight = 400;
	else
		*desiredWidth = 26,  *desiredHeight = 20;
DEBUG(("Desired Size %d x %d\n", *desiredWidth, *desiredHeight));
	return view::Fixed;
}

	void
lookzview::Print( FILE    *file, const char  	  *processor, const char  	  *format, boolean  	 level )
					{
	/* never print anything */
}

	void
lookzview::ObservedChanged(class observable   *dobj, long   status)
			{
	if (dobj == (this)->GetDataObject()) {
	    class lookz *lookz;
	    class text *text;

	    lookz = (class lookz *) (this)->GetDataObject();
	    text = (lookz)->GetTextObject();

	    if (status == lookz_TEXTOBJECTCHANGED) {
		NewTextObject(this, text);
	    }
	    if (status == observable::OBJECTDESTROYED) {
		long i;

		for (i = 0; i < numStyleEditors; i++) {
		    if (((class dataobject *) (styleEditors[i].lookz)) == dobj) {
			styleEditors[i].text = NULL;
			styleEditors[i].lookz = NULL;
			break;
		    }
		}
	    }
	}
	else if (dobj == (class dataobject *) this->curss) {
	    style *rep=NULL;
	    if(curss->FindReplacement(curstyle,&rep)) {
		curstyle=rep;
	    }
	    if (status == observable::OBJECTDESTROYED) {
		CloseStyleSheet(this);
	    }
	    else if (status == (long)this || this->curstyle == NULL) {
		return;	/* changed by self */
	    }
	    else {
		this->NeedsUnpack = TRUE;
	    }
	}

	(this)->WantUpdate( this);
}

	void
lookzview::SetVisibility(boolean  visibility)
		{
	if (visibility != (this)->GetVisibility()) {
DEBUG(("Change Visibility to %d\n", visibility));
		/* I am surprised I need the cast in the next line */
		((class lookz *)(this)->GetDataObject())->SetVisibility( 
				visibility);
		this->Linked = FALSE;
	}
}

	boolean
lookzview::GetVisibility()
	{
	return ((class lookz *)(this)->GetDataObject())->GetVisibility();
}

	void
lookzview::SetStyleSheet(class stylesheet  *ss)
		{
	CloseStyleSheet(this);
	OpenStyleSheet(this, ss);
}

	class stylesheet *
lookzview::GetStyleSheet()
	{
	return this->curss;
}

	void 
lookzview::SetEmbedded(boolean  isEmbedded)
		{
	this->embedded = isEmbedded;
}

	static void 
UpdateDocument(class lookzview  *self)
	{
	class lookz *lookz;
	class text *text;

	lookz = (class lookz *) (self)->GetDataObject();
	text = (lookz)->GetTextObject();

	if (text != NULL) {
	        (text)->SetModified();
		(text)->RegionModified( 0, (text)->GetLength());
		(text)->NotifyObservers( observable::OBJECTCHANGED);
	}
	if (self->embedded) {
		/* BOGOSITY ALERT:  we call Full_Update of parent */
		class view *v;
		for (v = (class view *)self; v != NULL; v = v->GetParent()) 
			if (strcmp("textview", (v)->GetTypeName()) == 0) {
				/* this is exceedingly dangerous XXX XXX
					 We call a FullUpdate 
					 without establishing any environment */
				(v)->FullUpdate( view::FullRedraw, 0, 0, 0, 0);
				break;
			}
	}
}

/* * * * * * * * * * * * */
/*   A D D   S T Y L E   */
/* * * * * * * * * * * * */

static char CardFound[50];   /* result from FindCardName */

/* FindCardName(s, nm)
	compare nm to the menu card name in s
	when match, set CardFound and exit
*/
	static boolean
FindCardName(class style  *s, char  *nm)
		{
	const char *mnnm = (s)->GetMenuName();
	const char *iw;
	if (mnnm != NULL  &&  (iw=InitialWord(mnnm)) != NULL
			&& FOLDEDEQ(nm, iw)) {
		/* found a match */
		long len;
		const char *comma;
		comma = strchr(mnnm, ',');
		if (comma == NULL)
			len = strlen(mnnm);
		else
			len = comma - mnnm;
		if (len > 49)  len = 49;
		strncpy(CardFound, mnnm, len);
		CardFound[len] = '\0';
		return TRUE;
	}
	return FALSE;
}

/* NewMenuName(name)
	compute a MenuName string for name
	return pointer to static storage containing the name
*/
	static char *
NewMenuName(class lookzview  *lv, char  *name)
		{
	static char buf[100];
	char *tilde, *comma;
	char *eltloc;

	tilde = strchr(name, '~');
	comma = strchr(name, ',');

	*CardFound = '\0';
	if (comma != NULL && (tilde == NULL || tilde > comma)) {
		/* There is no tilde for the card name,
			see if there is an existing menu card */
		*comma = '\0';
		(lv->curss)->EnumerateStyles((stylesheet_efptr)FindCardName, (long) name);
		*comma = ',';
	}
	if (*CardFound != '\0') {
		/* use existing card name and the remainder of 'name' */
		long len = strlen(CardFound);
		strcpy(buf, CardFound);
		eltloc = buf + len;
		strncpy(eltloc++, comma, 99 - len);
	}
	else {
		/* new card and new style.  just do capitalization */
		strncpy(buf, name, 99);
		if (islower(*buf))  *buf = toupper(*buf);
		eltloc = (comma == NULL) ? NULL 
			: buf + (comma-name) + 1;
	}
	buf[99] = '\0';
	if (eltloc != NULL && islower(*eltloc)) 
		*eltloc = toupper(*eltloc);
	return buf;
}

	static void 
AddStyle(class lookzview  *self)
	{
	class style *newsty;
	const char *stylename;
	long accnum;
	char name[75], temp[75];

	if ( ! self->foundstylesheet) return;	/* can't add if have no style sheeet */
	ClearStyleDisplay(self);
	(self->st[(long)TNstylename])->ClearBits();

	sprintf(temp, "%s,", self->curcard);
	if (message::AskForString(self, 0, "Name for new style: ", temp, name, 75) < 0 
			|| *temp == '\0') {
		message::DisplayString(self, 0, "Cancelled.");
		return;
	}

	(self->st[(long)TNmenucard])->ClearBits();
	accnum = (self->st[(long)TNmenucard])->AddString( 
			(char *)InitialWord(name));
	(self->st[(long)TNmenucard])->SetBitOfEntry( accnum, 1);
	self->curcard = (char *)(self->st[(long)TNmenucard])->GetStringOfEntry( 
				accnum);

	stylename = strchr(name, ',');
	if (stylename) stylename = InitialWord(stylename+1);

	if ( stylename == NULL || *stylename == '\0') {
		message::DisplayString(self, 0, 
			"Missing comma or stylename.  Enter name as:  <MenuCard>,<StyleName>");
		return;
	}

	stylename = clean(stylename);
	/* by this time 'stylename' contains an all lowercase version of the 
		name after the comma without any ~00 */
	newsty=(self->curss)->Find( stylename);
	if(newsty) newsty=(self->curss)->MakeWritable(stylename);
	if (newsty == NULL) {
		/* create the style */
		newsty = new style;
		(newsty)->SetName( stylename);
		(self->curss)->Add( newsty);
		if (strncmp(name, NOMENUSTRING, strlen(NOMENUSTRING)) != 0) {
			(newsty)->SetMenuName( NewMenuName(self, name));
		}
	}
	else if (strchr(name, '~') != NULL) {
		/* existing style and the input contains a ~, 
				change the menu name */
		if (strncmp(name, NOMENUSTRING, strlen(NOMENUSTRING)) != 0) {
			(newsty)->SetMenuName( name);
		}
	}

	/* Update the display so the new style menu appears. */
	(self->st[(long)TNstylename])->Clear();
	if (self->curcard && (ULstrcmp(self->curcard, NOMENUSTRING) != 0)) {
	    (self->curss)->EnumerateStyles((stylesheet_efptr)AddStyleName, (long) self);
	    (self->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) HitStyleName, (long)self);
	}
	else {
	    (self->curss)->EnumerateStyles( (stylesheet_efptr)AddStyleNameNoMenus, (long) self);
	    (self->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) HitStyleNameNoMenus, (long)self);
	}

	SetStyleDisplay(self, newsty);
}

/*
 * This is the callback when enumerating the text environments.
 * We return TRUE when the given environment uses the style we are
 * deleting.
 */
static boolean CheckDelStyle(class style  *sty, class text  *txt, int  level, class environment  *env)
{
    return (env != NULL && env->type == environment_Style && env->data.style == sty);
}

static void DeleteStyle(class lookzview  *self)
	{
	class style *delsty;
	class lookz *lz;
	class text *txt;
	class environment *env;
	long pos, length;
	boolean askedfordelete;
	long result;
	const char *stylename, *curstylename;
	char name[75], temp[75];
	static const char * const choices[] = { "Yes", "No", NULL };

	if ( ! self->foundstylesheet) return;	/* can't delete if have no style sheeet */

	/* Ask for the style.  Use the currently selected style as the default. */
	curstylename = NULL;
	if (self->curstyle)
	    curstylename = (self->curstyle)->GetName();

	sprintf(temp, "%s,%s", self->curcard, curstylename ? curstylename : "");
	if (message::AskForString(self, 0, "Style name to delete: ", temp, name, 75) < 0) {
		message::DisplayString(self, 0, "Cancelled.");
		return;
	}
	stylename = strchr(name, ',');
	if (stylename) stylename = InitialWord(stylename+1);

	if ( stylename == NULL || *stylename == '\0') {
		message::DisplayString(self, 0, 
			"Missing comma or stylename.  Enter name as:  <MenuCard>,<StyleName>");
		return;
	}
	stylename = clean(stylename);
	/* by this time 'stylename' contains an all lowercase version of the 
		name after the comma without any ~00 */
	delsty=(self->curss)->Find( stylename);
	if (delsty == NULL) {
		message::DisplayString(self, 0, "Unknown style name.");
		return;
	}

	/* Delete the style from the template's initial text. */
	askedfordelete = FALSE;
	lz = (class lookz *)(self)->GetDataObject();
	if (lz) {
	    txt = (lz)->GetTextObject();
	    if (txt) {
		/* This is a bit inefficient, but it doesn't happen much. */
		do {
		    env = (txt)->EnumerateEnvironments( 0, (txt)->GetLength(), (text_eefptr)CheckDelStyle, (long)delsty);
		    if (env) {
			if (!askedfordelete) {
			    if (message::MultipleChoiceQuestion(self, 51,
				"The templates uses this style in its initial text.\nIs it ok to delete the style from the text?",
				1, &result, choices, NULL) < 0) {
				message::DisplayString(self, 0, "Cancelled.");
				return;
			    }
			    if (result == 1) {
				message::DisplayString(self, 0, "Style not deleted.");
				return;
			    }
			    askedfordelete = TRUE;
			}
			/* Delete this environment. */
			pos = (env)->Eval();
			length = (env)->GetLength();
			(txt->rootEnvironment)->Remove( pos, length, environment_Style, FALSE);
		    }
		} while (env != NULL);
	    }
	}

	if (self->curstyle == delsty)
	    self->curstyle = NULL;

	/* Now delete the style itself. */
	(self->curss)->Delete( delsty);

	/* Update the display so the deleted menu disappears. */
	(self->st[(long)TNstylename])->Clear();
	ClearStyleDisplay(self);
	if (self->curcard && (ULstrcmp(self->curcard, NOMENUSTRING) != 0)) {
	    (self->curss)->EnumerateStyles((stylesheet_efptr)AddStyleName, (long) self);
	    (self->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) HitStyleName, (long)self);
	}
	else {
	    (self->curss)->EnumerateStyles( (stylesheet_efptr)AddStyleNameNoMenus, (long) self);
	    (self->stv[(long)TNstylename])->SetItemHitProc((strtblview_hitfptr) HitStyleNameNoMenus, (long)self);
	}
	(self->curss)->NotifyObservers( (long) self);
	UpdateDocument(self);
	sprintf(temp, "Style \"%s\" deleted%s.", name,
		askedfordelete ? " and removed from the initial text" : "");
	message::DisplayString(self, 0, temp);
  }

static void SetBGcolor(lookzview *self)
{
    char *oldbgcolor;
    struct style *globalsty;
    char colorbuf[75];

    if ( ! self->foundstylesheet) return;	/* can't set if have no style sheeet */

    globalsty=self->curss->GetGlobalStyle();
    if (globalsty)
	oldbgcolor = globalsty->GetAttribute("bgcolor");
    else {
	/* Missing global style.  Create one. */
	globalsty = new style;
	if (globalsty == NULL) {
	    message::DisplayString(self, 0, "Could not create global style.");
	    return;
	}
	globalsty->SetName("global");
	self->curss->Add(globalsty);
	oldbgcolor = NULL;
    }

    if (message::AskForString(self, 0, "New Background Color: ", oldbgcolor, colorbuf, sizeof(colorbuf)) < 0) {
	message::DisplayString(self, 0, "Cancelled.");
	return;
    }
    if (colorbuf[0] == '\0') {
	/* Assume just an enter means delete the bgcolor. */
	globalsty->RemoveAttribute("bgcolor");
	message::DisplayString(self, 0, "Background color set to default application background color.");
    } else {
	globalsty->AddAttribute("bgcolor", colorbuf);
	message::DisplayString(self, 0, "Background color set.");
    }
    UpdateDocument(self);
}

static void SetROBGcolor(lookzview *self)
{
    char *oldbgcolor;
    struct style *globalsty;
    char colorbuf[75];

    if ( ! self->foundstylesheet) return;	/* can't set if have no style sheeet */

    globalsty=self->curss->GetGlobalStyle();
    if (globalsty)
	oldbgcolor = globalsty->GetAttribute("robgcolor");
    else {
	/* Missing global style.  Create one. */
	globalsty = new style;
	if (globalsty == NULL) {
	    message::DisplayString(self, 0, "Could not create global style.");
	    return;
	}
	globalsty->SetName("global");
	self->curss->Add(globalsty);
	oldbgcolor = NULL;
    }

    if (message::AskForString(self, 0, "New R/O Background Color: ", oldbgcolor, colorbuf, sizeof(colorbuf)) < 0) {
	message::DisplayString(self, 0, "Cancelled.");
	return;
    }
    if (colorbuf[0] == '\0') {
	/* Assume just an enter means delete the bgcolor. */
	globalsty->RemoveAttribute("robgcolor");
	message::DisplayString(self, 0, "R/O Background color set to default application background color.");
    } else {
	globalsty->AddAttribute("robgcolor", colorbuf);
	message::DisplayString(self, 0, "R/O Background color set.");
    }
    UpdateDocument(self);
}

	void 
lookzview::SetDataObject(class dataobject  *dobj)
		{
	(this)->view::SetDataObject(dobj);
	if (ATK::IsTypeByName((dobj)->GetTypeName(), "lookz")) {
		class lookz *lookz = (class lookz *) dobj;
		NewTextObject(this, (lookz)->GetTextObject());
	}
}
