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
ATK_IMPL("style.H")
#include <style.H>

#include <namespace.H>
#include <fontdesc.H>

#define iswhite(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')

/*
 * Utility routines
 */

/* Convert any unit to unit style_RawDots */


ATKdefineRegistry(style, ATK, NULL);
static long CVDots(long  amt, enum style_Unit  unit);
static int style_freeattributes(long  procdata, class Namespace  * curnamespace, int  indexvalue );
static int style_copyattributes(long  procdata, class Namespace  * curnamespace, int  indexvalue );
static int style_writeAdditionalAttribute(FILE  *fileptr, class Namespace  * curnamespace, int  indexvalue );
static long ReadDevice(class style  *self , FILE  *fp );


static long CVDots(long  amt, enum style_Unit  unit)
{
    switch (unit) {
        case style_RawDots:
            return amt;
        case style_Inches:
            return (amt * 72) >> 16;
        case style_CM:
            return (amt * 28) >> 16;
        case style_Points:
            return amt;
        case style_Ems:     /* amt * 10? */
            return 0;
        case style_Lines:   /* amt * 10? */
            return 0;
    }

    return 0;
}

static int style_freeattributes(long  procdata, class Namespace  * curnamespace, int  indexvalue )
{
    char * tmpValue;
    if ((curnamespace)->BoundpAt(indexvalue)) {
	tmpValue = (char *) (curnamespace)->ValueAt(indexvalue);
	if (tmpValue) free(tmpValue);
    }
    return TRUE;
}


/*
 * Datastream alias table for attributes
 */

static const char * const AttributeAlias[] = {
    "LeftMargin",
    "RightMargin",
    "Indent",
    "Script",
    "Justification",
    "Spacing",
    "Spread",
    "Tabs",
    "Flags",
    "Function",
    "",             /* unused */
    "FontFamily",
    "FontFace",
    "ExternalSize",
    "FontSize",	    /* hack to avoid splitting FontMod yet */
    "TopMargin",
    "BottomMargin",
    "Above",
    "Below",
    "CounterName",
    "CounterParent",
    "CounterScope",
    "CounterPosition",
    "CounterInitialValue",
    NULL
};

static const char * const OptypeAlias[] = {
    "Inch",
    "Cm",
    "Point",
    "Em",
    "Line",
    "String",
    "Int",
    NULL
};

static const char * const OpcodeAlias[] = {
    "Noop",
    "Copy",
    "Add",
    "Set",
    "Clear",
    NULL
};

static const char * const TabAlignmentAlias[] = {
    "",
    "LeftAligned",
    "RightAligned",
    "CenteredOnTab",
    "CenteredBetweenTab",
    "TabDivide",
    "CharAligned",
    "TabClear",
    "AllClear",
    NULL
};

static const char * const JustificationAlias[] = {
    "PreviousJustification",
    "LeftJustified",
    "RightJustified",
    "Centered",
    "LeftAndRightJustified",
    "LeftThenRightJustified",
    NULL
};

static const char * const ScopeAlias[] = {
    "GlobalScope",
    "LocalScope",
    NULL
};

static const char * const PositionAlias[] = {
    "EnvironmentCtr",
    "ParagraphCtr",
    "LineCtr",
    NULL
};

static const char * const MarginValueAlias[] = {
    "ConstantMargin",
    "LeftMargin",
    "LeftEdge",
    "RightMargin",
    "RightEdge",
    "TopMargin",
    "TopEdge",
    "BottomMargin",
    "BottomEdge",
    "PreviousIndentation",
    NULL
};

static const char * const SpacingValueAlias[] = {
    "ConstantSpacing",
    "InterlineSpacing",
    "InterparagraphSpacing",
    "AboveSpacing",
    "BelowSpacing",
    NULL
};

static const char * const FontSizeAlias[] = {
    "PreviousFontSize",
    "ConstantFontSize",
    NULL
};

static const char * const ScriptMovementAlias[] = {
    "PreviousScriptMovement",
    "ConstantScriptMovement",
    NULL
};

static const char * const FlagAlias[] = {
    "Underline",
    "Hidden",
    "ReadOnly",
    "PassThru",
    "Icon",		/* unused */
    "ContinueIndent",
    "Hinge",
    "NewPage",
    "ChangeBar",
    "OverBar",
    "NoWrap",
    "NoFill",
    "KeepPriorNL",
    "KeepNextNL",
    "TabsCharacters",
    "DottedBox",
    "StrikeThrough",
    "IncludeBeginning",
    "IncludeEnd",
    NULL
};

static const char * const FaceStyleAlias[] = {
    "Bold",
    "Italic",
    "Shadow",
    "FixedFace",
    "Outline",
    "Thin",
    "Black",
    "Medium",
    "Heavy",
    "Condense",
    NULL
};

/* The following must be kept in sync with the flag defs in style.H */

enum style_FlagIndex {
    style_UnderlineIndex=0,
    style_HiddenIndex=1,
    style_ReadOnlyIndex=2,
    style_PassThruIndex=3,
    style_IconIndex=4,
    style_ContinueIndentIndex=5,
    style_HingeIndex=6,
    style_NewPageIndex=7,
    style_ChangeBarIndex=8,
    style_OverBarIndex=9,
    style_NoWrapIndex=10,
    style_NoFillIndex=11,
    style_KeepPriorNLIndex=12,
    style_KeepNextNLIndex=13,
    style_TabsCharactersIndex=14,
    style_DottedBoxIndex=15,
    style_StrikeThroughIndex=16,
    style_IncludeBeginningIndex=17,
    style_IncludeEndIndex=18
};

enum style_FontFaceIndex {
    style_BoldIndex=0,
    style_ItalicIndex=1,
    style_ShadowIndex=2,
    style_FixedFaceIndex=3,
    style_OutlineIndex=4,
    style_ThinIndex=5,
    style_BlackIndex=6,
    style_MediumIndex=7,
    style_HeavyIndex=8,
    style_CondenseIndex=9
};

/*
 * Class procedures
 */

static const char unknownstyle[]="unknown";

style::style()
{
    this->name = (char *)malloc(sizeof(unknownstyle));
    if(this->name==NULL) THROWONFAILURE(FALSE);
    strcpy(this->name, unknownstyle);
    this->menuName = NULL;
    this->template_c = FALSE;

    this->FontFamily = NULL;
    this->TabChangeList = NULL;
    this->NumTabChanges = 0;
    this->CounterName = NULL;
    this->CounterParent = NULL;
    this->CounterStyles = NULL;

    this->AdditionalAttributes = NULL;
    (this)->Reset();

    THROWONFAILURE( TRUE);
}

style::~style()
{
    if (this->name != NULL)
        free(this->name);
    if (this->menuName != NULL)
        free(this->menuName);

    if (this->FontFamily != NULL)
        free(this->FontFamily);
    if (this->CounterName != NULL)
        free(this->CounterName);
    if (this->CounterParent != NULL)
        free(this->CounterParent);

    (this)->ClearCounterStyles();
    (this)->ClearTabChanges();

    if (this->AdditionalAttributes) {
	/* First get rid of any strings left as values */
	(void) (this->AdditionalAttributes)->Enumerate( style_freeattributes, 0);
	delete this->AdditionalAttributes;
	this->AdditionalAttributes=NULL;
    }
}

/*
 * Methods
 */

void style::Reset()
{
    (this)->SetNewLeftMargin( style_LeftMargin, 0, style_Points);
    (this)->SetNewRightMargin( style_RightMargin, 0, style_Points);
    (this)->SetNewTopMargin( style_TopMargin, 0, style_Points);
    (this)->SetNewBottomMargin( style_BottomMargin, 0, style_Points);

    (this)->SetNewIndentation( style_PreviousIndentation, 0, style_Points);
    (this)->SetJustification( style_PreviousJustification);

    (this)->SetNewInterparagraphSpacing( style_InterparagraphSpacing, 0, style_Points);
    (this)->SetNewAbove( style_AboveSpacing, 0, style_Points);
    (this)->SetNewBelow( style_BelowSpacing, 0, style_Points);
    (this)->SetNewInterlineSpacing( style_InterlineSpacing, 0, style_Points);

    if (this->FontFamily != NULL) {
	free(this->FontFamily);
        this->FontFamily = NULL;
    }

    (this)->SetFontSize( style_PreviousFontSize, 0);
    (this)->SetFontScript( style_PreviousScriptMovement, 0, style_Points);
    this->AddFontFaces= (long) fontdesc_Plain;
    this->OutFontFaces= ~ (long) fontdesc_Plain;

    (this)->ClearTabChanges();

    if (this->CounterName != NULL) {
	free(this->CounterName);
	this->CounterName = NULL;
    }

    this->CounterScope = style_GlobalScope;
    this->CounterPosition=style_EnvironmentCtr;

    if (this->CounterParent != NULL) {
	free(this->CounterParent);
	this->CounterParent = NULL;
    }

    this->CounterInitialValue = 0;
    (this)->ClearCounterStyles();

    this->AddMiscFlags = style_NoFlags;
    this->OutMiscFlags = ~style_NoFlags;

    if (this->AdditionalAttributes) {
	/* First get rid of any strings left as values */
	(void) (this->AdditionalAttributes)->Enumerate( style_freeattributes, 0);
	delete this->AdditionalAttributes;
	this->AdditionalAttributes=NULL;
    }
}

static int style_copyattributes(long  procdata, class Namespace  * curnamespace, int  indexvalue )
{
    char * tmpValue;
    char * tmpAttributeName;
    class atom * tmpAttributeAtom;
    if ((curnamespace)->BoundpAt(indexvalue)) {
	tmpValue = (char *) (curnamespace)->ValueAt(indexvalue);
	tmpAttributeAtom = (curnamespace)->NameAt( indexvalue);
	tmpAttributeName = (tmpAttributeAtom)->Name();
	((class style*) procdata)->AddAttribute( tmpAttributeName, tmpValue);
    }
    return TRUE;
}

/* AdjustOperandValue(s, p, isConst)
	KLUDGE
	s and p point to struct marginstyle or struct fontscriptstyle, BOTH
		OF WHICH HAVE THE SAME FORMAT (right now)
	The value of s is adjusted per that of p.
	If isConst is TRUE, the value of s is set to that of p,
	otherwise {the value of s is incremented by that of p,
		the Units are reset to RawDots,
		and the DotCvtOperand is used as the Operand value.}

	It would be preferable to do this is a cleaner fashion, 
	but there is no good way to add Operands if Units differ.
*/
	static void 
AdjustOperandValue(struct marginstyle  *s , struct marginstyle  *p, boolean  isConst) {
	if (isConst) {
		/* just change to the size set by p */
		*s = *p;
	}
	else if (p->DotCvtOperand != 0) {
		/* p makes a relative change to s */
		s->DotCvtOperand += p->DotCvtOperand;
		s->MarginUnit = style_RawDots;
		s->Operand = s->DotCvtOperand;
	}
}


/* MergeInto(style *dest)
	merges all styles from this style into dest.
	XXX:	We process only some of the attributes.  
		The style attributes  
			LeftMargin,   RightMargin,  Indent,  Script, and FontSize
		are treated incrementally.  The attributes 
			Justification,  Flags,  FontFamily,  FontFace,
			Line Spacing, Paragraph Spacing, tabs, and color
		are treated as new values.
*/

void style::MergeInto(class style  *dest) {

    enum style_FontSize SizeBasis;
    long Operand, newOperand;
    enum style_Justification Justification;
    long mask;

    enum style_Unit Unit;
    enum style_SpacingValue Basis;
    char *color;
    struct tabentry **tabchanges;
    long n;

 /* LeftMargin */
    AdjustOperandValue(&dest->NewLeftMargin,
                       &this->NewLeftMargin,
                       this->NewLeftMargin.MarginBasis ==
                       style_ConstantMargin);
 /* RightMargin */
    AdjustOperandValue(&dest->NewRightMargin,
                       &this->NewRightMargin,
                       this->NewRightMargin.MarginBasis ==
                       style_ConstantMargin);
 /* Indent */
    AdjustOperandValue(&dest->NewIndentation,
                       &this->NewIndentation,
                       this->NewIndentation.MarginBasis ==
                       style_ConstantMargin);
 /* Script */
    AdjustOperandValue((struct marginstyle *)&dest->FontScript,
                       (struct marginstyle *)&this->FontScript,
                       this->FontScript.ScriptBasis ==
                       style_ConstantScriptMovement);
 /* FontSize  */
    (this)->GetFontSize( &SizeBasis, &Operand);
    if (SizeBasis == style_ConstantFontSize)
        (dest)->SetFontSize( SizeBasis, Operand);
    else if (SizeBasis == style_PreviousFontSize) {
        (dest)->GetFontSize( &SizeBasis, &newOperand);
        (dest)->SetFontSize( SizeBasis,
                             newOperand+Operand);
    }
 /* FontFamily */
    if (this->FontFamily != NULL)
        (dest)->SetFontFamily( this->FontFamily);
 /* Justification */
    Justification = (this)->GetJustification();
    if (Justification != style_PreviousJustification)
        (dest)->SetJustification( Justification);
 /* Flags */
    mask = this->AddMiscFlags  |  ~ this->OutMiscFlags;
    dest->AddMiscFlags = 
      (mask & this->AddMiscFlags)
      | ((~mask) & dest->AddMiscFlags);
    dest->OutMiscFlags = 
      (mask & this->OutMiscFlags)
      | ((~mask) & dest->OutMiscFlags);
 /* FontFace */
    mask = this->AddFontFaces  |  ~ this->OutFontFaces;
    dest->AddFontFaces = 
      (mask & this->AddFontFaces)
      | ((~mask) & dest->AddFontFaces);
    dest->OutFontFaces = 
      (mask & this->OutFontFaces)
      | ((~mask) & dest->OutFontFaces);
 /* Spacing (of lines) */
    (dest)->GetNewInterlineSpacing( &Basis, &Operand, &Unit);
    if (Basis == style_ConstantSpacing && Unit == style_Points)
        (dest)->SetNewInterlineSpacing( Basis, Operand, Unit);
 /* Spread (between paragraphs) */
    (dest)->GetNewInterparagraphSpacing( &Basis, &Operand, &Unit);
    if (Basis == style_ConstantSpacing && Unit == style_Points)
        (dest)->SetNewInterparagraphSpacing( Basis, Operand, Unit);
 /* Tabs */
    (this)->GetTabChangeList( &n, &tabchanges);
    if (n != 0) {
        (dest)->ClearTabChanges();
        while (n-- > 0) 
            (dest)->AddTabChange(
                                 tabchanges[n]->TabOpcode,
                                 tabchanges[n]->Location,
                                 tabchanges[n]->LocationUnit);
    }
    if (tabchanges)
        free(tabchanges);
 /* Color */
    color = (this)->GetAttribute( "color");
    if (color != NULL) 
        (dest)->AddAttribute( "color", color);
}

void style::Copy(class style  *dest)
{
    int i;
    char **counterstyle;
    struct tabentry *tabchange;

    (dest)->SetName( this->name);
    (dest)->SetMenuName( this->menuName);
    dest->template_c = this->template_c;

    (dest)->SetNewLeftMargin( this->NewLeftMargin.MarginBasis, this->NewLeftMargin.Operand, this->NewLeftMargin.MarginUnit);
    (dest)->SetNewRightMargin( this->NewRightMargin.MarginBasis, this->NewRightMargin.Operand, this->NewRightMargin.MarginUnit);
    (dest)->SetNewTopMargin( this->NewTopMargin.MarginBasis, this->NewTopMargin.Operand, this->NewTopMargin.MarginUnit);
    (dest)->SetNewBottomMargin( this->NewBottomMargin.MarginBasis, this->NewBottomMargin.Operand, this->NewBottomMargin.MarginUnit);

    (dest)->SetNewIndentation( this->NewIndentation.MarginBasis, this->NewIndentation.Operand, this->NewIndentation.MarginUnit);
    (dest)->SetJustification(this->NewJustification);

    (dest)->SetNewInterparagraphSpacing( this->NewInterparagraphSpacing.SpacingBasis, this->NewInterparagraphSpacing.Operand, this->NewInterparagraphSpacing.SpacingUnit);
    (dest)->SetNewAbove( this->NewAbove.SpacingBasis, this->NewAbove.Operand, this->NewAbove.SpacingUnit);
    (dest)->SetNewBelow( this->NewBelow.SpacingBasis, this->NewBelow.Operand, this->NewBelow.SpacingUnit);
    (dest)->SetNewInterlineSpacing( this->NewInterlineSpacing.SpacingBasis, this->NewInterlineSpacing.Operand, this->NewInterlineSpacing.SpacingUnit);

    (dest)->SetFontFamily( this->FontFamily);
    (dest)->SetFontSize( this->FontSize.SizeBasis, this->FontSize.Operand);
    (dest)->SetFontScript( this->FontScript.ScriptBasis, this->FontScript.Operand, this->FontScript.SizeUnit);
    dest->AddFontFaces= (long) this->AddFontFaces;
    dest->OutFontFaces= (long) this->OutFontFaces;

    (dest)->ClearTabChanges();
    dest->NumTabChanges= 0;
    dest->TabChangeList=NULL;
    for (i = 0, tabchange = this->TabChangeList; i < this->NumTabChanges; i++, tabchange++)
	(dest)->AddTabChange( tabchange->TabOpcode, tabchange->Location, tabchange->LocationUnit);

    (dest)->ClearCounterStyles();
    (dest)->SetCounterName( this->CounterName);
    dest->CounterScope = this->CounterScope;
    dest->CounterPosition = this->CounterPosition;
    (dest)->SetCounterParent( this->CounterParent);
    dest->CounterInitialValue = this->CounterInitialValue;
    dest->NumCounterStyles = this->NumCounterStyles;
    for (i = 0, counterstyle = this->CounterStyles; i < this->NumCounterStyles; i++, counterstyle++)
	(dest)->AddCounterStyle( *counterstyle);

    dest->AddMiscFlags = this->AddMiscFlags;
    dest->OutMiscFlags = this->OutMiscFlags;

    if (this->AdditionalAttributes)
	(void) (this->AdditionalAttributes)->Enumerate( style_copyattributes, (long)dest);
}

/* Attribute Setting and Clearing Functions */

void style::SetName(const char  *name)
{
    if (this->name != NULL)
        free(this->name);
    if (name == NULL) name=unknownstyle;
    this->name = (char *)malloc(strlen(name) + 1);
    if(this->name==NULL) return;
    strcpy(this->name, name);
}

const char *style::GetName()
{
    return this->name;
}

void style::SetMenuName(const char  *menuName)
{
    if (this->menuName != NULL)
        free(this->menuName);
    if (menuName == NULL)
        this->menuName = NULL;
    else {
	this->menuName = (char *)malloc(strlen(menuName)+ 1);
	if(this->menuName==NULL) return;
        strcpy(this->menuName, menuName);
    }
}

const char *style::GetMenuName()
{
    return this->menuName;
}

void style::SetNewLeftMargin(enum style_MarginValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewLeftMargin.MarginBasis = Basis;
    this->NewLeftMargin.Operand = Operand;
    this->NewLeftMargin.MarginUnit = Unit;
    this->NewLeftMargin.DotCvtOperand = CVDots(Operand,Unit);
}

void style::GetNewLeftMargin(enum style_MarginValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis =  this->NewLeftMargin.MarginBasis;
    *RefOperand = this->NewLeftMargin.Operand;
    *RefUnit = this->NewLeftMargin.MarginUnit;
}

void style::SetNewRightMargin(enum style_MarginValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewRightMargin.MarginBasis = Basis;
    this->NewRightMargin.Operand = Operand;
    this->NewRightMargin.MarginUnit = Unit;
    this->NewRightMargin.DotCvtOperand = CVDots(Operand,Unit);
}

void style::GetNewRightMargin(enum style_MarginValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis =  this->NewRightMargin.MarginBasis;
    *RefOperand = this->NewRightMargin.Operand;
    *RefUnit = this->NewRightMargin.MarginUnit;
}

void style::SetNewTopMargin(enum style_MarginValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewTopMargin.MarginBasis = Basis;
    this->NewTopMargin.Operand = Operand;
    this->NewTopMargin.MarginUnit = Unit;
    this->NewTopMargin.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewTopMargin(enum style_MarginValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis =  this->NewTopMargin.MarginBasis;
    *RefOperand = this->NewTopMargin.Operand;
    *RefUnit = this->NewTopMargin.MarginUnit;
}

void style::SetNewBottomMargin(enum style_MarginValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewBottomMargin.MarginBasis = Basis;
    this->NewBottomMargin.Operand = Operand;
    this->NewBottomMargin.MarginUnit = Unit;
    this->NewBottomMargin.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewBottomMargin(enum style_MarginValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis =  this->NewBottomMargin.MarginBasis;
    *RefOperand = this->NewBottomMargin.Operand;
    *RefUnit = this->NewBottomMargin.MarginUnit;
}

void style::SetNewIndentation(enum style_MarginValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewIndentation.MarginBasis = Basis;
    this->NewIndentation.Operand = Operand;
    this->NewIndentation.MarginUnit = Unit;
    this->NewIndentation.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewIndentation(enum style_MarginValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis =  this->NewIndentation.MarginBasis;
    *RefOperand = this->NewIndentation.Operand;
    *RefUnit = this->NewIndentation.MarginUnit;
}

void style::SetNewInterparagraphSpacing(enum style_SpacingValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewInterparagraphSpacing.SpacingBasis = Basis;
    this->NewInterparagraphSpacing.Operand = Operand;
    this->NewInterparagraphSpacing.SpacingUnit = Unit;
    this->NewInterparagraphSpacing.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewInterparagraphSpacing(enum style_SpacingValue  *RefBasis,long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis = this->NewInterparagraphSpacing.SpacingBasis;
    *RefOperand = this->NewInterparagraphSpacing.Operand;
    *RefUnit = this->NewInterparagraphSpacing.SpacingUnit;
}

void style::SetNewAbove(enum style_SpacingValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewAbove.SpacingBasis = Basis;
    this->NewAbove.Operand = Operand;
    this->NewAbove.SpacingUnit = Unit;
    this->NewAbove.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewAbove(enum style_SpacingValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis = this->NewAbove.SpacingBasis;
    *RefOperand = this->NewAbove.Operand;
    *RefUnit = this->NewAbove.SpacingUnit;
}

void style::SetNewBelow(enum style_SpacingValue  Basis, long  Operand, enum style_Unit  Unit)
{

    this->NewBelow.SpacingBasis = Basis;
    this->NewBelow.Operand = Operand;
    this->NewBelow.SpacingUnit = Unit;
    this->NewBelow.DotCvtOperand = CVDots(Operand,Unit);
}

void style::GetNewBelow(enum style_SpacingValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis = this->NewBelow.SpacingBasis;
    *RefOperand = this->NewBelow.Operand;
    *RefUnit = this->NewBelow.SpacingUnit;
}

void style::SetNewInterlineSpacing(enum style_SpacingValue  Basis, long  Operand, enum style_Unit  Unit)
{
    this->NewInterlineSpacing.SpacingBasis = Basis;
    this->NewInterlineSpacing.Operand = Operand;
    this->NewInterlineSpacing.SpacingUnit = Unit;
    this->NewInterlineSpacing.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetNewInterlineSpacing(enum style_SpacingValue  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis = this->NewInterlineSpacing.SpacingBasis;
    *RefOperand = this->NewInterlineSpacing.Operand;
    *RefUnit = this->NewInterlineSpacing.SpacingUnit;
}

void style::SetFontFamily(const char  *NewFont)
{
    if (this->FontFamily != NULL)
        free(this->FontFamily);

    if (NewFont == NULL || NewFont[0] == '\0')
        this->FontFamily = NULL;
    else 
        this->FontFamily = (char *)malloc(strlen(NewFont) + 1),
        strcpy(this->FontFamily, NewFont);
}

void style::GetFontFamily(char  *FontName, int  bufsize)
{
    char *s;

    if (this->FontFamily == NULL) {
	FontName[0] = '\0';
        return;
    }

    s = this->FontFamily;
    while (--bufsize && *s != '\0')
        *FontName++ = *s++;
    *FontName = '\0';
}

void style::SetFontSize(enum style_FontSize  Basis, long  Operand)
{

    this->FontSize.SizeBasis = Basis;
    this->FontSize.Operand = Operand;
}

void style::GetFontSize(enum style_FontSize  *RefBasis, long  *RefOperand)
{
    *RefBasis = this->FontSize.SizeBasis;
    *RefOperand = this->FontSize.Operand;
}

void style::SetFontScript(enum style_ScriptMovement  Basis, long  Operand, enum style_Unit  Unit)
{
    this->FontScript.ScriptBasis = Basis;
    this->FontScript.Operand = Operand;
    this->FontScript.SizeUnit = Unit;
    this->FontScript.DotCvtOperand = CVDots(Operand, Unit);
}

void style::GetFontScript(enum style_ScriptMovement  *RefBasis, long  *RefOperand, enum style_Unit  *RefUnit)
{
    *RefBasis = this->FontScript.ScriptBasis;
    *RefOperand = this->FontScript.Operand;
    *RefUnit = this->FontScript.SizeUnit;
}

void style::SetCounterName(char  *NewName)
{
    if (this->CounterName != NULL)
	free(this->CounterName);

    if (NewName == NULL || NewName[0] == '\0')
	this->CounterName = NULL;
    else
        this->CounterName = (char *)malloc(strlen(NewName) + 1),
	strcpy(this->CounterName, NewName);
}

void style::GetCounterName(char  *RetrievedName)
{
    if (this->CounterName == NULL)
	RetrievedName[0] = '\0';
    else
	strcpy(RetrievedName, this->CounterName);
}

void style::SetCounterParent(char  *NewParent)
{
    if (this->CounterParent != NULL)
	free(this->CounterParent);

    if (NewParent == NULL || NewParent[0] == '\0')
	this->CounterParent = NULL;
    else
	this->CounterParent = (char *)malloc(strlen(NewParent) + 1),
	strcpy(this->CounterParent, NewParent);
}

void style::GetCounterParent(char  *RetrievedParent)
{
    if (this->CounterParent == NULL)
	RetrievedParent[0] ='\0';
    else
	strcpy(RetrievedParent, this->CounterParent);
}

void style::ClearCounterStyles()
{
    int i;

    if (this->CounterStyles != NULL) {
	for (i = 0; i < this->NumCounterStyles; i++) free(this->CounterStyles[i]);
	free(this->CounterStyles);
    }
    this->NumCounterStyles = 0;
    this->CounterStyles = NULL;
}

void style::AddCounterStyle(char  *NewStyle)
{
    if (this->NumCounterStyles == 0)
	this->CounterStyles = (char **) malloc(sizeof(char *));
    else
	this->CounterStyles = (char **)
          realloc((char *) this->CounterStyles,
            (this->NumCounterStyles + 1) * sizeof(char *));

    this->CounterStyles[this->NumCounterStyles] = (char *) malloc(strlen(NewStyle) + 1);
    strcpy(this->CounterStyles[this->NumCounterStyles], NewStyle);

    this->NumCounterStyles++;
}

void style::GetCounterStyles(long  *RefNumStyles, char  ***RefStyleStrings)
{
    int i;

    *RefNumStyles = this->NumCounterStyles;
    *RefStyleStrings = (char **) malloc(this->NumCounterStyles * sizeof(char *));

    for (i = 0;i < this->NumCounterStyles; i++) {
        (*RefStyleStrings)[i] = (char *)    malloc(strlen(this->CounterStyles[i]) + 1);
        strcpy((*RefStyleStrings)[i], this->CounterStyles[i]);
    }
}

void style::ClearTabChanges()
{  
    if (this->TabChangeList != NULL)
	free(this->TabChangeList);
    this->NumTabChanges = 0;
    this->TabChangeList = NULL;
}

void style::AddTabChange(enum style_TabAlignment  TabOp, long  Where, enum style_Unit  Unit)
{
    if (TabOp == style_AllClear) {
	this->NumTabChanges = 0;
	if (this->TabChangeList != NULL) {
	    free(this->TabChangeList);
	}
    }
    else {
	long loc = CVDots(Where, Unit);
	long i;

	for (i = 0; i < this->NumTabChanges; i++) {
	    if (this->TabChangeList[i].DotLocation == loc &&
		this->TabChangeList[i].TabOpcode != style_AllClear) {
		this->TabChangeList[i].TabOpcode = TabOp;
		this->TabChangeList[i].Location = Where;
		this->TabChangeList[i].LocationUnit = Unit;
		return;
	    }
	}
    }

    if (this->NumTabChanges == 0)
	this->TabChangeList = (struct tabentry *) malloc(sizeof(struct tabentry));
    else
	this->TabChangeList = (struct tabentry *)
          realloc((char *) this->TabChangeList,
            (this->NumTabChanges + 1) * sizeof(struct tabentry));

    this->TabChangeList[this->NumTabChanges].TabOpcode = TabOp;
    this->TabChangeList[this->NumTabChanges].Location = Where;
    this->TabChangeList[this->NumTabChanges].LocationUnit = Unit;
    this->TabChangeList[this->NumTabChanges].DotLocation = CVDots(Where, Unit);

    this->NumTabChanges++;
}

void style::GetTabChangeList(long  *RefNumTabChanges, struct tabentry  ***RefTabChanges)
{
    int i;
    struct tabentry **Newptr;

    if ((*RefNumTabChanges = this->NumTabChanges) == 0)
	*RefTabChanges = NULL;
    else {
	Newptr = (struct tabentry **) malloc(this->NumTabChanges * sizeof(struct tabentry *));
	*RefTabChanges = Newptr;

	for (i = 0; i < this->NumTabChanges; i++)
	    *Newptr++ = &(this->TabChangeList[i]);
    }
}

void style::AddAttribute(const char  * NewAttributeName, const char  * NewAttributeValue )
{
    class atom *tmpAtom;
    char * tmpValue;

    tmpAtom = atom::Intern(NewAttributeName);
    if(NewAttributeValue) {
        tmpValue = (char *) malloc(strlen(NewAttributeValue)+1);
	(void) strcpy(tmpValue,NewAttributeValue);
    }
    else tmpValue = NULL;
    if (!this->AdditionalAttributes) this->AdditionalAttributes = new Namespace;
    if(this->AdditionalAttributes)    (this->AdditionalAttributes)->SetValue(tmpAtom, (long) tmpValue);
}

void style::RemoveAttribute(const char  * OldAttributeName)
{
    class atom * tmpAtom;
    long tmpValue;

    if (!this->AdditionalAttributes) return;
    tmpAtom = atom::Intern(OldAttributeName);
    tmpValue = (this->AdditionalAttributes)->GetValue( tmpAtom);
    (this->AdditionalAttributes)->Unbind( tmpAtom);
    if (tmpValue > 0) free( (char *)tmpValue);

}

char * style::GetAttribute(const char  * OldAttributeName )
{
    class atom * tmpAtom;
    long tmpValue;

    if (!this->AdditionalAttributes) return NULL;
    tmpAtom = atom::Intern(OldAttributeName);
    tmpValue =  (this->AdditionalAttributes)->GetValue( tmpAtom);
     if (tmpValue > 0) return (char *)tmpValue;
     else return NULL;
    
}

boolean style::IsAttribute(const char  * TestAttributeName)
{
    class atom * tmpAtom;

    if (!this->AdditionalAttributes) return FALSE;
    tmpAtom = atom::Intern(TestAttributeName);
    return (this->AdditionalAttributes)->Boundp( tmpAtom, NULL);
}

/* Datastream I/O: Menu fields */

void style::WriteMenu(FILE  *fp)
{
    if (this->menuName != NULL && this->menuName[0] != '\0')
        fprintf(fp, "menu:[%s]", this->menuName);
}

long style::ReadMenu(FILE  *fp)
{
    int c, pos;
    char name[128];

    pos = 0;
    while (1) {
        if ((c = getc(fp)) == EOF)
            return -1;
        if (c == ']')
            break;
        if (pos == sizeof (name) - 1)   /* Truncates */
            continue;
        name[pos++] = c;
    };

    name[pos] = '\0';

    (this)->SetMenuName( name);

    return 0;
}

/* Datastream I/O: attribute fields */

static int style_writeAdditionalAttribute(FILE  *fileptr, class Namespace  * curnamespace, int  indexvalue )
{
    char * tmpValue;
    char * tmpAttributeName;
    class atom * tmpAttributeAtom;

    if ((curnamespace)->BoundpAt(indexvalue)) {
	tmpAttributeAtom = (curnamespace)->NameAt( indexvalue);
	tmpAttributeName = (tmpAttributeAtom)->Name();
	tmpValue = (char *) (curnamespace)->ValueAt(indexvalue);
	fprintf(fileptr, "\nattr:['%s' ", tmpAttributeName);
	if (tmpValue) {
	    /*fprintf(fileptr, "'%s']",tmpValue);*/ /* changed to escape single-quotes */
	    putc('\'', fileptr);
	    while (*tmpValue) {
		switch (*tmpValue) {
		    case '\\':
		    case '\'':
			putc('\\', fileptr);
			/* fall through */
		    default:
			putc(*tmpValue, fileptr);
			tmpValue++;
			break;
		}
	    }
	    putc('\'', fileptr);
	    putc(']', fileptr);
	}
	else fprintf(fileptr, "'']");
    }
    return TRUE;
}

void style::WriteAttr(FILE  *fp)
{
    int i;
    long numtabchanges, operand;

    enum style_Unit unit;
    enum style_Justification newjust;
    enum style_Scope newscope;
    enum style_Position newposition;
    enum style_MarginValue newmargin;
    enum style_SpacingValue newspacing;
    enum style_FontSize newfsize;
    enum style_ScriptMovement newscript;

    char newfontfamily[200], newName[200], newcparent[200];
    struct tabentry **tabchanges;

    static const char AttrD[] = "\nattr:[%s %s %s %ld]";
    static const char AttrS[] = "\nattr:[%s %s %s %s]";

    (this)->GetNewLeftMargin( &newmargin, &operand, &unit);
    if (newmargin != style_LeftMargin || operand != 0 || unit != style_Points)
        fprintf(fp, AttrD, AttributeAlias[(int) style_LeftMarginAttr], MarginValueAlias[(int) newmargin], OptypeAlias[(int) unit], operand);

    (this)->GetNewRightMargin( &newmargin, &operand, &unit);
    if (newmargin != style_RightMargin || operand != 0 || unit != style_Points)
        fprintf(fp, AttrD, AttributeAlias[(int) style_RightMarginAttr], MarginValueAlias[(int) newmargin], OptypeAlias[(int) unit], operand);

    (this)->GetNewTopMargin( &newmargin, &operand, &unit);
    if (newmargin != style_TopMargin || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_TopMarginAttr], MarginValueAlias[(int) newmargin], OptypeAlias[(int) unit], operand);

    (this)->GetNewBottomMargin( &newmargin, &operand, &unit);
    if (newmargin != style_BottomMargin || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_BottomMarginAttr], MarginValueAlias[(int) newmargin], OptypeAlias[(int) unit], operand);

    (this)->GetNewIndentation( &newmargin, &operand, &unit);
    if (newmargin != style_PreviousIndentation || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_IndentAttr], MarginValueAlias[(int) newmargin], OptypeAlias[(int) unit], operand);

    (this)->GetFontScript( &newscript, &operand, &unit);
    if (newscript != style_PreviousScriptMovement || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_ScriptAttr], ScriptMovementAlias[(int) newscript], OptypeAlias[(int) unit], operand);

    newjust = (enum style_Justification) (this)->GetJustification();
    if (newjust != style_PreviousJustification)
	fprintf(fp, AttrD, AttributeAlias[(int) style_JustificationAttr], JustificationAlias[(int) newjust], OptypeAlias[(int) style_Points], 0L);

    (this)->GetNewInterlineSpacing( &newspacing, &operand, &unit);
    if (newspacing != style_InterlineSpacing || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_SpacingAttr], SpacingValueAlias[(int) newspacing], OptypeAlias[(int) unit], operand);

    (this)->GetNewAbove( &newspacing, &operand, &unit);
    if (newspacing != style_AboveSpacing || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_AboveAttr], SpacingValueAlias[(int) newspacing], OptypeAlias[(int) unit], operand);

    (this)->GetNewBelow( &newspacing, &operand, &unit);
    if (newspacing != style_BelowSpacing || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_BelowAttr], SpacingValueAlias[(int) newspacing], OptypeAlias[(int) unit], operand);

    (this)->GetNewInterparagraphSpacing( &newspacing, &operand, &unit);
    if (newspacing != style_InterparagraphSpacing || operand != 0 || unit != style_Points)
	fprintf(fp, AttrD, AttributeAlias[(int) style_SpreadAttr], SpacingValueAlias[(int) newspacing], OptypeAlias[(int) unit], operand);

    (this)->GetTabChangeList( &numtabchanges, &tabchanges);
    if (numtabchanges != 0) {
	struct tabentry **tc;

	for (i = 0, tc = tabchanges; i < numtabchanges; i++, tc++)
	    fprintf(fp, AttrD, AttributeAlias[(int) style_TabsAttr], TabAlignmentAlias[(int) (*tc)->TabOpcode], OptypeAlias[(int) (*tc)->LocationUnit], (*tc)->Location);
	free (tabchanges);   /* Correct a core leak with GetTabChangeList */
    }

    if (this->AddMiscFlags != style_NoFlags) {
	if ((this)->IsUnderlineAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_UnderlineIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsHiddenAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_HiddenIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsReadOnlyAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ReadOnlyIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsPassThruAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_PassThruIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsContinueIndentAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ContinueIndentIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsHingeAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_HingeIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsNewPageAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NewPageIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsChangeBarAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ChangeBarIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsOverBarAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_OverBarIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsDottedBoxAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_DottedBoxIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsStrikeThroughAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_StrikeThroughIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsNoWrapAdded())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NoWrapIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_NoFill))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NoFillIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_KeepPriorNL))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_KeepPriorNLIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_KeepNextNL))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_KeepNextNLIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_TabsCharacters))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_TabsCharactersIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_IncludeBeginning))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_IncludeBeginningIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->TestAddFlag( style_IncludeEnd))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_IncludeEndIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

    }

    if (this->OutMiscFlags != ~ style_NoFlags) {
	if ((this)->IsUnderlineRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_UnderlineIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsHiddenRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_HiddenIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsReadOnlyRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ReadOnlyIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsPassThruRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_PassThruIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsContinueIndentRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ContinueIndentIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsHingeRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_HingeIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsNewPageRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NewPageIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsUnderlineRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_UnderlineIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsChangeBarRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_ChangeBarIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsOverBarRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_OverBarIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsDottedBoxRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_DottedBoxIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsStrikeThroughRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_StrikeThroughIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsNoWrapRemoved())
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NoWrapIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_NoFill))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_NoFillIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_KeepPriorNL))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_KeepPriorNLIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_KeepNextNL))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_KeepNextNLIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_TabsCharacters))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_TabsCharactersIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_IncludeBeginning))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_IncludeBeginningIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->TestRemoveFlag( style_IncludeEnd))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FlagsAttr], FlagAlias[(int) style_IncludeEndIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);
    }
    if ((this)->GetAddedFontFaces()) {
	if ((this)->IsAnyAddedFontFace( fontdesc_Bold))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_BoldIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Italic))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ItalicIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Shadow))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ShadowIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Fixed))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_FixedFaceIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Outline))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_OutlineIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Thin))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ThinIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Black))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_BlackIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Medium))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_MediumIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Heavy))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_HeavyIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);

	if ((this)->IsAnyAddedFontFace( fontdesc_Condense))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_CondenseIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPSET]);
    }
    if ((this)->GetRemovedFontFaces()){
	if ((this)->IsAnyRemovedFontFace( fontdesc_Bold))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_BoldIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Italic))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ItalicIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Shadow))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ShadowIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Fixed))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_FixedFaceIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Outline))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_OutlineIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Thin))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_ThinIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Black))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_BlackIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Medium))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_MediumIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Heavy))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_HeavyIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);

	if ((this)->IsAnyRemovedFontFace( fontdesc_Condense))
	    fprintf(fp, AttrS, AttributeAlias[(int) style_FontFaceAttr], FaceStyleAlias[(int) style_CondenseIndex], OptypeAlias[(int) style_RawDots], OpcodeAlias[style_OPCLEAR]);
    }

    (this)->GetFontFamily(
      newfontfamily, sizeof (newfontfamily));
    if (newfontfamily[0] != '\0')
	fprintf(fp, AttrD, AttributeAlias[(int) style_FontFamilyAttr], newfontfamily, OptypeAlias[(int) style_RawDots], 0L);

    (this)->GetFontSize( &newfsize, &operand);
    if (newfsize != style_PreviousFontSize || operand != 0)
	fprintf(fp, AttrD, AttributeAlias[(int) style_FontSizeAttr], FontSizeAlias[(int) newfsize], OptypeAlias[(int) style_Points], operand);

    (this)->GetCounterName( newName);
    if (newName[0] != '\0') {
	fprintf(fp, AttrD, AttributeAlias[(int) style_CounterNameAttr], newName, OptypeAlias[(int) style_RawDots], 0L);

	(this)->GetCounterParent( newcparent);
	if (newcparent[0] != '\0')
	    fprintf(fp, AttrD, AttributeAlias[(int) style_CounterParentAttr], newcparent, OptypeAlias[(int) style_RawDots], 0L);

	newscope = (enum style_Scope) (this)->GetCounterScope();
	if (newscope != style_GlobalScope)
	    fprintf(fp, AttrD, AttributeAlias[(int) style_CounterScopeAttr], ScopeAlias[(int) newscope], OptypeAlias[(int) style_Points], 0L);

	newposition = (enum style_Position) (this)->GetCounterPosition();
	if (newposition != style_EnvironmentCtr)
	    fprintf(fp, AttrD, AttributeAlias[(int) style_CounterPositionAttr], PositionAlias[(int) newposition], OptypeAlias[(int) style_Points], 0L);

	operand = (this)->GetCounterInitialValue();
	if (operand != 0)
	    fprintf(fp, AttrD, AttributeAlias[(int) style_CounterInitialValueAttr], "none", OptypeAlias[(int) style_Points], operand);
    }

    if (this->AdditionalAttributes) {
	(void) (this->AdditionalAttributes)->Enumerate( (namespace_efptr)style_writeAdditionalAttribute, (long) fp);
    }
}

long style::ReadAttr(FILE  *fp)
{
    int j;
    int c, operand, basis,  newface;
    unsigned int flag;

    /* enum style_Unit unit;*/
    int unit;
    enum style_Attribute attr;
    enum style_MarginValue newmargin;
    enum style_SpacingValue newspacing;
    enum style_TabAlignment newtabalign;
    enum style_Justification newjust;
    enum style_Scope newscope;
    enum style_Position newposition;
    enum style_FontSize newfsize;
    enum style_ScriptMovement newscript;

    char Attribute[50], Basis[256], Unit[25], Operand[400];

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (iswhite(c));

    if (c == '\'') {
        /* Temporary hack for generic attributes. */
        j = 0;
        while (1) {
            if ((c = getc(fp)) == EOF)
                return -1;
            if (c == '\'')
                break;
            Attribute[j++] = c;
            if (j == sizeof (Attribute))
                goto bomb;
        }
        Attribute[j] = '\0';
        do {
            if ((c = getc(fp)) == EOF)
                return -1;
        } while (iswhite(c));
        if (c == ']') {
            (this)->AddAttribute( Attribute, NULL);
            return 0;
        }
        if (c != '\'')
            goto bomb;
        j = 0;
        while (1) {
            if ((c = getc(fp)) == EOF)
                return -1;
            if (c == '\'')
                break;
	    if (c == '\\') {
		if ((c = getc(fp)) == EOF)
		    return -1;
	    }
	    Operand[j++] = c;
            if (j == sizeof (Operand))
                goto bomb;
        }
        Operand[j] = '\0';
        do {
            if ((c = getc(fp)) == EOF)
                return -1;
        } while (iswhite(c));
        if (c != ']')
            goto bomb;
        (this)->AddAttribute( Attribute, Operand);
        return 0;
    }

    j = 0;
    do {
        if (c == ']')
            return -1;
        Attribute[j++] = c;
        if (j == sizeof (Attribute))
            goto bomb;
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (! iswhite(c));
    Attribute[j] = '\0';

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (iswhite(c));

    j = 0;
    do {
        if (c == ']')
            return -1;
        Basis[j++] = c;
        if (j == sizeof (Basis))
            goto bomb;
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (! iswhite(c));
    Basis[j] = '\0';

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (iswhite(c));

    j = 0;
    do {
        if (c == ']')
            return -1;
        Unit[j++] = c;
        if (j == sizeof (Unit))
            goto bomb;
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (! iswhite(c));
    Unit[j] = '\0';

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while(iswhite(c));

    if (c == ']')
        return -1;

    j = 0;
    do {
        if (c == ']')
            break;
        Operand[j++] = c;
        if (j == sizeof (Operand))
            goto bomb;
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (! iswhite(c));
    Operand[j] = '\0';

    while (iswhite(c))
        if ((c = getc(fp)) == EOF)
            return -1;

    if (c != ']')
        goto bomb;

    for (j = 0; AttributeAlias[j] != NULL; j++)
        if (strcmp(Attribute, AttributeAlias[j]) == 0)
            break;
    if (AttributeAlias[j] == NULL)
        return -1;

    attr = (enum style_Attribute) j;

    switch (attr) {
	case style_LeftMarginAttr:
	case style_RightMarginAttr:
	case style_IndentAttr:
	case style_TopMarginAttr:
	case style_BottomMarginAttr:

            for (basis = 0; MarginValueAlias[basis] != NULL; basis++)
		if (strcmp(Basis, MarginValueAlias[basis]) == 0)
                    break;
            if (MarginValueAlias[basis] == NULL)
                return -1;

	    newmargin = (enum style_MarginValue) basis;

            for (unit = style_Inches; OptypeAlias[unit] != NULL; unit++)
		if (strcmp(Unit, OptypeAlias[unit]) == 0)
                    break;
            if (OptypeAlias[unit] == NULL)
                return -1;

            operand = atoi(Operand);

            switch (attr) {
                case style_LeftMarginAttr:
                    (this)->SetNewLeftMargin( newmargin, operand, (enum style_Unit)unit);
		    break;
		case style_RightMarginAttr:
		    (this)->SetNewRightMargin( newmargin, operand, (enum style_Unit)unit);
		    break;
		case style_IndentAttr:
		    (this)->SetNewIndentation( newmargin, operand, (enum style_Unit)unit);
		    break;
		case style_TopMarginAttr:
                    (this)->SetNewTopMargin( newmargin, operand, (enum style_Unit)unit);
		    break;
		case style_BottomMarginAttr:
                    (this)->SetNewBottomMargin( newmargin, operand, (enum style_Unit)unit);
                    break;
		default: /* lots of unhandled cases */
		    break;
	    }
	    break;

	case style_ScriptAttr:

            for (basis = 0; ScriptMovementAlias[basis] != NULL; basis++)
		if (strcmp(Basis, ScriptMovementAlias[basis]) == 0)
                    break;
            if (ScriptMovementAlias[basis] == NULL)
                return -1;

	    newscript = (enum style_ScriptMovement) basis;

            for (unit = style_Inches; OptypeAlias[unit] != NULL; unit++)
		if (strcmp(Unit, OptypeAlias[unit]) == 0)
                    break;
            if (OptypeAlias[unit] == NULL)
                return -1;

	    operand = atoi(Operand);

            /* BE1 had an inconsistent representation, giving point size */
            /* in fixed-point form for scripting.  The following warns */
            /* about and fixes outrageous scripting values (>1 page) */

            if (unit == (int) style_Points &&
                (operand > 11*72 || operand < -11*72)) {
                operand >>= 16;
                fprintf(stderr, "Found BE1-format super/subscripting error.\n");
                fprintf(stderr, "Please save the file to correct this problem.\n");
                fprintf(stderr, "If the error persists, send mail to Advisor.\n");
                fprintf(stderr, "Include the name of the bad file.\n");
            }

	    (this)->SetFontScript( newscript, operand, (enum style_Unit)unit);
	    break;

	case style_JustificationAttr:

            for (basis = 0; JustificationAlias[basis] != NULL; basis++)
		if (strcmp(Basis, JustificationAlias[basis]) == 0)
                    break;
            if (JustificationAlias[basis] == NULL)
                return -1;

	    newjust = (enum style_Justification) basis;

	    (this)->SetJustification( newjust);
	    break;

	case style_SpacingAttr:
	case style_SpreadAttr:
	case style_AboveAttr:
	case style_BelowAttr:

            for (basis = 0; SpacingValueAlias[basis] != NULL; basis++)
		if (strcmp(Basis, SpacingValueAlias[basis]) == 0)
                    break;
            if (SpacingValueAlias[basis] == NULL)
                return -1;

	    newspacing = (enum style_SpacingValue) basis;

            for (unit = style_Inches; OptypeAlias[unit] != NULL; unit++)
		if (strcmp(Unit, OptypeAlias[unit]) == 0)
                    break;
            if (OptypeAlias[unit] == NULL)
                return -1;

            operand = atoi(Operand);

	    switch (attr) {
		case style_SpacingAttr:
		    (this)->SetNewInterlineSpacing( newspacing, operand, (enum style_Unit)unit);
		    break;
		case style_SpreadAttr:
		    (this)->SetNewInterparagraphSpacing( newspacing, operand, (enum style_Unit)unit);
		    break;
		case style_AboveAttr:
		    (this)->SetNewAbove( newspacing, operand, (enum style_Unit)unit);
		    break;
		case style_BelowAttr:
		    (this)->SetNewBelow( newspacing, operand, (enum style_Unit)unit);
		    break;
		default:
		    break;
	    }
	    break;

	case style_TabsAttr:

            for (basis = 0; TabAlignmentAlias[basis] != NULL; basis++)
		if (strcmp(Basis, TabAlignmentAlias[basis]) == 0)
                    break;
            if (TabAlignmentAlias[basis] == NULL)
                return -1;

	    newtabalign = (enum style_TabAlignment) basis;

            for (unit = style_Inches; OptypeAlias[unit] != NULL; unit++)
		if (strcmp(Unit, OptypeAlias[unit]) == 0)
                    break;
            if (OptypeAlias[unit] == NULL)
                return -1;

            operand = atoi(Operand);

	    (this)->AddTabChange( newtabalign, operand, (enum style_Unit)unit);
	    break;

	case style_FlagsAttr:

            for (basis = 0; FlagAlias[basis] != NULL; basis++)
		if (strcmp(Basis, FlagAlias[basis]) == 0)
                    break;
            if (FlagAlias[basis] == NULL)
                return -1;

	    flag = 1 << basis;

            for (operand = 0; OpcodeAlias[operand] != NULL; operand++)
		if (strcmp(Operand, OpcodeAlias[operand]) == 0)
                    break;
            if (OpcodeAlias[operand] == NULL)
                return -1;

	    switch (flag) {
		case style_Underline:
		    if (operand == style_OPSET) (this)->AddUnderline();
		    if (operand == style_OPCLEAR) (this)->RemoveUnderline();
		    break;
		case style_Hidden:
		    if (operand == style_OPSET) (this)->AddHidden();
		    if (operand == style_OPCLEAR) (this)->RemoveHidden();
		    break;
		case style_ReadOnly:
		    if (operand == style_OPSET) (this)->AddReadOnly();
		    if (operand == style_OPCLEAR) (this)->RemoveReadOnly();
		    break;
		case style_PassThru:
		    if (operand == style_OPSET) (this)->AddPassThru();
		    if (operand == style_OPCLEAR) (this)->RemovePassThru();
		    break;
		case style_ContinueIndent:
		    if (operand == style_OPSET) this->AddMiscFlags |= style_ContinueIndent;
		    if (operand==style_OPCLEAR) this->OutMiscFlags |= style_ContinueIndent;
		    break;
		case style_Hinge:
		    if (operand == style_OPSET) (this)->AddHinge();
		    if (operand == style_OPCLEAR) (this)->RemoveHinge();
		    break;
		case style_NewPage:
		    if (operand == style_OPSET) (this)->AddNewPage();
		    if (operand == style_OPCLEAR) (this)->RemoveNewPage();
		    break;
		case style_ChangeBar:
		    if (operand == style_OPSET) (this)->AddChangeBar();
		    if (operand == style_OPCLEAR) (this)->RemoveChangeBar();
		    break;
		case style_OverBar:
		    if (operand == style_OPSET) (this)->AddOverBar();
		    if (operand == style_OPCLEAR) (this)->RemoveOverBar();
		    break;
		case style_DottedBox:
		    if (operand == style_OPSET) (this)->AddDottedBox();
		    if (operand == style_OPCLEAR) (this)->RemoveDottedBox();
		    break;
		case style_StrikeThrough:
		    if (operand == style_OPSET) (this)->AddStrikeThrough();
		    if (operand == style_OPCLEAR) (this)->RemoveStrikeThrough();
		    break;
		case style_NoWrap:
		    if (operand == style_OPSET) (this)->AddNoWrap();
		    if (operand == style_OPCLEAR) (this)->RemoveNoWrap();
		    break;
		case style_NoFill:
		    if (operand == style_OPSET) 
			(this)->AddFlag( style_NoFill);
		    if (operand == style_OPCLEAR) 
			(this)->RemoveFlag( style_NoFill);
		    break;
		case style_KeepPriorNL:
		    if (operand == style_OPSET) 
			(this)->AddFlag( style_KeepPriorNL);
		    if (operand == style_OPCLEAR) 
			(this)->RemoveFlag( style_KeepPriorNL);
		    break;
		case style_KeepNextNL:
		    if (operand == style_OPSET) 
			(this)->AddFlag( style_KeepNextNL);
		    if (operand == style_OPCLEAR) 
			(this)->RemoveFlag( style_KeepNextNL);
		    break;
		case style_TabsCharacters:
		    if (operand == style_OPSET) 
			(this)->AddFlag( style_TabsCharacters);
		    if (operand == style_OPCLEAR) 
			(this)->RemoveFlag( style_TabsCharacters);
		    break;
		case style_IncludeBeginning:
		    if (operand == style_OPSET) (this)->AddFlag( style_IncludeBeginning);
		    if (operand == style_OPCLEAR) (this)->RemoveFlag( style_IncludeBeginning);
		    break;
		case style_IncludeEnd:
		    if (operand == style_OPSET) (this)->AddFlag( style_IncludeEnd);
		    if (operand == style_OPCLEAR) (this)->RemoveFlag( style_IncludeEnd);
		    break;
		default:
		    return -1;
	    }
	    break;

	case style_FontFaceAttr:

            for (basis = 0; FaceStyleAlias[basis] != NULL; basis++)
		if (strcmp(Basis, FaceStyleAlias[basis]) == 0)
                    break;
            if (FaceStyleAlias[basis] == NULL)
                return -1;

	    flag = 1 << basis;
	    newface = (int) flag;

            for (operand = 0; OpcodeAlias[operand] != NULL; operand++)
		if (strcmp(Operand, OpcodeAlias[operand]) == 0)
                    break;
            if (OpcodeAlias[operand] == NULL)
                return -1;

	    if (operand == style_OPSET) (this)->AddNewFontFace( newface);
	    if (operand == style_OPCLEAR) (this)->RemoveOldFontFace( newface);
	    break;

	case style_FontFamilyAttr:

	    (this)->SetFontFamily( Basis);
	    break;

	case style_FontSizeAttr:

            for (basis = 0; FontSizeAlias[basis] != NULL; basis++)
		if (strcmp(Basis, FontSizeAlias[basis]) == 0)
                    break;
            if (FontSizeAlias[basis] == NULL)
                return -1;

	    newfsize = (enum style_FontSize) basis;

            operand = atoi(Operand);

	    (this)->SetFontSize( newfsize, operand);
	    break;

	case style_CounterNameAttr:

	    (this)->SetCounterName( Basis);
	    break;

	case style_CounterParentAttr:

	    (this)->SetCounterParent( Basis);
	    break;

	case style_CounterScopeAttr:

            for (basis = 0; ScopeAlias[basis] != NULL; basis++)
		if (strcmp(Basis, ScopeAlias[basis]) == 0)
                    break;
            if (ScopeAlias[basis] == NULL)
                return -1;

	    newscope = (enum style_Scope) basis;

	    (this)->SetCounterScope( newscope);
	    break;

	case style_CounterPositionAttr:

            for (basis = 0; PositionAlias[basis] != NULL; basis++)
		if (strcmp(Basis, PositionAlias[basis]) == 0)
                    break;
            if (PositionAlias[basis] == NULL)
                return -1;

	    newposition = (enum style_Position) basis;

	    (this)->SetCounterPosition( newposition);
	    break;

	case style_CounterInitialValueAttr:

            operand = atoi(Operand);
	    (this)->SetCounterInitialValue( operand);
	    break;

	default:
            return -1;
    }

    return 0;

bomb:
    while ((c = getc(fp)) != EOF && c != ']')
        ;
    return -1;
}

/* Device fields, for backward compat, eventually will be removed. */
/* Not meant to be robust; seeks out attr:[] fields nested in device:[]. */

static long ReadDevice(class style  *self , FILE  *fp )
{
    int bra = 0;
    while (1) {
        switch (getc(fp)) {
        case '[': bra++; break;
        case ']': if (--bra < 0) return 0; break;
        case 'a':
            if (getc(fp) == 't' && getc(fp) == 't' && getc(fp) == 'r'
                 && getc(fp) == ':' && getc(fp) == '[')
                (self)->ReadAttr( fp);
        }
    }
}

/*
 * style_Write assumes that "\define{stylename\n" has
 * already been written.  Does not write trailing "}" and newline.
 */

void style::Write(FILE  *fp)
{
    (this)->WriteMenu( fp);
    (this)->WriteAttr( fp);
}

/*
 * style_Read assumes that the string "\define{name" has already been
 * read.  The '\n' after the "\define{name" may have or have not been read.
 * Handles menu:[] and attr:[] entries until it reaches the "}" that matches
 * the \define.  The brace, and the (mandatory) following newline, are also read.
 *
 * On a parsing error, the remainder of the style is read up to and
 * including the closing brace and newline, to facilitate the possibility
 * of ignoring the bad style definition.
 */

long style::Read(FILE  *fp)
{
    int c, pos;
    char token[17];

    (this)->Reset();

    while (1) {
        do {
            if ((c = getc(fp)) == EOF)
                return -1;
        } while (iswhite(c));

        if (c == '}') {
            if ((c = getc(fp)) == EOF || c != '\n')
                return -1;
            return 0;
        }

        pos = 0;

        if (c == ':')
            goto bomb;

        do {
            if (c == ':')
                break;
            token[pos++] = c;
            if (pos == sizeof (token))
                goto bomb;
            if ((c = getc(fp)) == EOF)
                return -1;
        } while (! iswhite(c));

        token[pos] = '\0';

        while (iswhite(c))
            if ((c = getc(fp)) == EOF)
                return -1;

        if (c != ':')
            goto bomb;

        do {
            if ((c = getc(fp)) == EOF)
                return -1;
        } while (iswhite(c));

        if (c != '[')
            goto bomb;

        if (strcmp(token, "menu") == 0) {
            if ((this)->ReadMenu( fp) < 0)
                goto bomb;
        } else if (strcmp(token, "attr") == 0) {
            if ((this)->ReadAttr( fp) < 0)
                goto bomb;
        } else if (strcmp(token, "device") == 0) {
            if (ReadDevice(this, fp) < 0)
                goto bomb;
        } else
            goto bomb;
    }

bomb:
    while ((c = getc(fp)) != EOF)
        if (c == '}') {
            c = getc(fp);   /* Eat the following newline */
            break;
        }

    return -1;
}
