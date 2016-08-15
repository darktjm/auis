ATK_INTER
#ifndef _style_H_
#define _style_H_
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


#include <namespace.H>

#define style_VERSION   1

/**************************************************************
 * Style Definitions
 **************************************************************/

#define style_OPNOOP        0
#define style_OPCOPY        1
#define style_OPADD         2
#define style_OPSET         3
#define style_OPCLEAR       4

enum style_Attribute {
    style_LeftMarginAttr=0,
    style_RightMarginAttr=1,
    style_IndentAttr=2,
    style_ScriptAttr=3,
    style_JustificationAttr=4,
    style_SpacingAttr=5,
    style_SpreadAttr=6,
    style_TabsAttr=7,
    style_FlagsAttr=8,
    style_FunctionAttr=9,
    style_FontFamilyAttr=11,
    style_FontFaceAttr=12,
    style_FontSizeAttr=14,
    style_TopMarginAttr=15,
    style_BottomMarginAttr=16,
    style_AboveAttr=17,
    style_BelowAttr=18,
    style_CounterNameAttr=19,
    style_CounterParentAttr=20,
    style_CounterScopeAttr=21,
    style_CounterPositionAttr=22,
    style_CounterInitialValueAttr=23
};

enum style_Unit {
    style_Inches=0,
    style_CM=1,
    style_Points=2,
    style_Ems=3,
    style_Lines=4,
    style_RawDots=6
};

enum style_TabAlignment {
    style_LeftAligned=1,
    style_RightAligned=2,
    style_CenteredOnTab=3,
    style_CenteredBetweenTab=4,
    style_TabDivide=5,
    style_CharAligned=6,
    style_TabClear=7,
    style_AllClear=8
};

struct tabentry {
    enum style_TabAlignment TabOpcode;
    long Location;
    enum style_Unit LocationUnit;
    long DotLocation;
};

enum style_Justification {
    style_PreviousJustification,
    style_LeftJustified,
    style_RightJustified,
    style_Centered,
    style_LeftAndRightJustified,
    style_LeftThenRightJustified
};

enum style_Scope {
    style_GlobalScope,
    style_LocalScope
};

enum style_Position {
    style_EnvironmentCtr,
    style_ParagraphCtr,
    style_LineCtr
};

enum style_MarginValue {
    style_ConstantMargin,
    style_LeftMargin,
    style_LeftEdge,
    style_RightMargin,
    style_RightEdge,
    style_TopMargin,
    style_TopEdge,
    style_BottomMargin,
    style_BottomEdge,
    style_PreviousIndentation
};

struct marginstyle {
    enum style_MarginValue MarginBasis;
    long Operand;
    enum style_Unit MarginUnit;
    long DotCvtOperand;
};

enum style_SpacingValue {
    style_ConstantSpacing,
    style_InterlineSpacing,
    style_InterparagraphSpacing,
    style_AboveSpacing,
    style_BelowSpacing
};

struct spacingstyle {
    enum style_SpacingValue SpacingBasis;
    long Operand;
    enum style_Unit SpacingUnit;
    long DotCvtOperand;
};

enum style_FontSize {
    style_PreviousFontSize,
    style_ConstantFontSize
};

struct fontsizestyle {
    enum style_FontSize SizeBasis;
    long Operand;
};

enum style_ScriptMovement {
    style_PreviousScriptMovement,
    style_ConstantScriptMovement
};

struct fontscriptstyle {
    enum style_ScriptMovement ScriptBasis;
    long Operand;
    enum style_Unit SizeUnit;
    long DotCvtOperand;
};

/* Flags for styles controlling the appearance of text */

#define style_NoFlags		0
#define style_AllFlags          ~0

/* The following must be kept in sync with the enum defs in style.C */

#define style_Underline         (1<<0)
#define style_Hidden		(1<<1)
#define style_ReadOnly          (1<<2)
#define style_PassThru          (1<<3)
#define style_Icon              (1<<4)	/* unused */
#define style_ContinueIndent 	(1<<5)
#define style_Hinge		(1<<6)
#define style_NewPage		(1<<7)
#define style_ChangeBar         (1<<8)
#define style_OverBar		(1<<9)
#define	style_NoWrap		(1<<10)
	/* the next three are for translation from Scribe to ATK */
#define style_NoFill		(1<<11)
#define style_KeepPriorNL	(1<<12)
#define style_KeepNextNL	(1<<13)
#define	style_TabsCharacters	(1<<14)
#define	style_DottedBox		(1<<15)
#define	style_StrikeThrough	(1<<16)
#define style_IncludeBeginning	(1<<17)
#define style_IncludeEnd	(1<<18)


/**************************************************************
 * Style Class
 **************************************************************/

#include <ATK.H>
#include <fontdesc.H>

class style : public ATK {
  public:
    virtual ATKregistryEntry *ATKregistry();

    style();
    virtual ~style();

    virtual void Reset();
    virtual void Copy(class style *dest);

    virtual void SetName(char *name);
    virtual char *GetName();

    virtual void SetMenuName(char *name);
    virtual char *GetMenuName();

    virtual void SetNewLeftMargin(enum style_MarginValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewLeftMargin(enum style_MarginValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewRightMargin(enum style_MarginValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewRightMargin(enum style_MarginValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewTopMargin(enum style_MarginValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewTopMargin(enum style_MarginValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewBottomMargin(enum style_MarginValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewBottomMargin(enum style_MarginValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewIndentation(enum style_MarginValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewIndentation(enum style_MarginValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);    

    virtual void GetNewInterparagraphSpacing(enum style_SpacingValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);
    virtual void SetNewInterparagraphSpacing(enum style_SpacingValue Basis, long Operand, enum style_Unit Unit);

    virtual void SetNewAbove(enum style_SpacingValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewAbove(enum style_SpacingValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewBelow(enum style_SpacingValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewBelow(enum style_SpacingValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetNewInterlineSpacing(enum style_SpacingValue Basis, long Operand, enum style_Unit Unit);
    virtual void GetNewInterlineSpacing(enum style_SpacingValue *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetFontFamily(const char *NewFont);
    virtual void GetFontFamily(char *FontName, int bufsize);

    virtual void SetFontSize(enum style_FontSize Basis, long Operand);
    virtual void GetFontSize(enum style_FontSize *RefBasis, long *RefOperand);

    virtual void SetFontScript(enum style_ScriptMovement Basis, long Operand, enum style_Unit Unit);
    virtual void GetFontScript(enum style_ScriptMovement *RefBasis, long *RefOperand, enum style_Unit *RefUnit);

    virtual void SetCounterName(char *NewCName);
    virtual void GetCounterName(char *RetrievedCName);

    virtual void SetCounterParent(char *NewParent);
    virtual void GetCounterParent(char *RetrievedParent);

    virtual void ClearCounterStyles();
    virtual void AddCounterStyle(char *NewStyle);
    virtual void GetCounterStyles(long *RefNumStyles, char ***RefStyleStrings);

    virtual void ClearTabChanges();
    virtual void AddTabChange(enum style_TabAlignment TabOp, long Where, enum style_Unit Unit);
    virtual void GetTabChangeList(long *RefNumTabChanges, struct tabentry ***RefTabChanges);

    /* Provision for arbitrary attributes */

    virtual void AddAttribute(char *name, char *value);
    virtual void RemoveAttribute(char *name);
    virtual char *GetAttribute(char *name);
    virtual boolean IsAttribute(char *name);

    /* File I/O */

    virtual void WriteMenu(FILE *fp);
    virtual long ReadMenu(FILE *fp);

    virtual void WriteAttr(FILE *fp);
    virtual long ReadAttr(FILE *fp);

    virtual void Write(FILE *fp);
    virtual long Read(FILE *fp);

    // XXX MergeInto processes only some of the attributes.   
    //  The style attributes  LeftMargin,   RightMargin,
    // Indent,  Script, and FontSize are treated incrementally.
    // The attributes  Justification,  Flags,  FontFamily,
    // FontFace, Line Spacing, Paragraph Spacing, tabs,
    // and color are treated as new values.
    virtual void MergeInto(style *dest);    
    /* Macros modifying font face flags; a style may add and/or */
    /* remove face flags from the surrounding envrment. */

    inline void ClearNewFontFaces() { ((this)->AddFontFaces = (long) fontdesc_Plain); }
    inline void AddNewFontFace(long newface) { ((this)->AddFontFaces |= (long) (newface)); }
    inline void ClearOldFontFaces() { ((this)->OutFontFaces = ~ (long) fontdesc_Plain); }
    inline void RemoveOldFontFace(long oldface) { ((this)->OutFontFaces &= ~ (long) (oldface)); }
    inline long GetAddedFontFaces() { return ((this)->AddFontFaces); }
    inline long GetRemovedFontFaces() { return ((this)->OutFontFaces); }
    inline boolean IsAllAddedFontFace(long testface) { return (((this)->AddFontFaces & (long) (testface)) == (long) (testface)); }
    inline boolean IsAllRemovedFontFace(long testface) { return ((~ (this)->OutFontFaces & (long) (testface)) == (long) (testface)); }
    inline boolean IsAnyAddedFontFace(long testface) { return ((this)->AddFontFaces & (long) (testface)); }
    inline boolean IsAnyRemovedFontFace(long testface) { return (~ (this)->OutFontFaces & (long) (testface)); }

    /* Macros for changing a style's associated counter data */

    inline void SetCounterScope(enum style_Scope newscope) { ((this)->CounterScope = (newscope)); }
    inline enum style_Scope GetCounterScope() { return ((this)->CounterScope); }
    inline void SetCounterPosition(enum style_Position newpos) { ((this)->CounterPosition = (newpos)); }
    inline style_Position GetCounterPosition() { return ((this)->CounterPosition); }
    inline void SetCounterInitialValue(long newval) { ((this)->CounterInitialValue = (newval)); }
    inline long  GetCounterInitialValue() { return ((this)->CounterInitialValue); }

    /* Macros for changing a style's justification mode; a style */
    /* may change the justification of the surrounding envrment. */

    inline void SetJustification(enum style_Justification just) { ((this)->NewJustification = (just)); }
    inline style_Justification GetJustification() { return ((this)->NewJustification); }

    /* PRIVATE -- used below.  In the future, these may be */
    /* used publically and the repetitious stuff below removed. */
    /* In the AddMiscFlags, all bits are '0' except flags to be added. */
    /* In the OutMiscFlags, all bits are '1' except flags to be removed. */

    inline void AddFlag(long flag) { ((this)->AddMiscFlags |= (flag), (this)->OutMiscFlags |= (flag)); }
    inline void RemoveFlag(long flag) { ((this)->AddMiscFlags &= ~(flag), (this)->OutMiscFlags &= ~(flag)); }
    inline void UseOldFlag(long flag) { ((this)->AddMiscFlags &= ~(flag), (this)->OutMiscFlags |= ~(flag)); }
    inline boolean TestAddFlag(long flag) { return ((this)->AddMiscFlags & (flag)); }
    inline boolean TestRemoveFlag(long flag) { return (~ (this)->OutMiscFlags & (flag)); }
    inline boolean TestUseOldFlag(long flag) { return (! (this)->TestAddFlag( flag) && ! (this)->TestRemoveFlag( flag)); }

    inline void AddHinge() { ((this))->AddFlag( style_Hinge); }
    inline void RemoveHinge() { ((this))->RemoveFlag( style_Hinge); }
    inline void UseOldHinge() { ((this))->UseOldFlag( style_Hinge); }
    inline boolean IsHingeAdded() { return ((this))->TestAddFlag( style_Hinge); }
    inline boolean IsHingeRemoved() { return ((this))->TestRemoveFlag( style_Hinge); }
    inline boolean IsHingeUnchange() { return ((this))->TestUseOldFlag( style_Hinge); }

    inline void AddNewPage() { ((this))->AddFlag( style_NewPage); }
    inline void RemoveNewPage() { ((this))->RemoveFlag( style_NewPage); }
    inline void UseOldNewPage() { ((this))->UseOldFlag( style_NewPage); }
    inline boolean IsNewPageAdded() { return ((this))->TestAddFlag( style_NewPage); }
    inline boolean IsNewPageRemoved() { return ((this))->TestRemoveFlag( style_NewPage); }
    inline boolean IsNewPageUnchange() { return ((this))->TestUseOldFlag( style_NewPage); }

    inline void AddHidden() { ((this))->AddFlag( style_Hidden); }
    inline void RemoveHidden() { ((this))->RemoveFlag( style_Hidden); }
    inline void UseOldHidden() { ((this))->UseOldFlag( style_Hidden); }
    inline boolean IsHiddenAdded() { return ((this))->TestAddFlag( style_Hidden); }
    inline boolean IsHiddenRemoved() { return ((this))->TestRemoveFlag( style_Hidden); }
    inline boolean IsHiddenUnchange() { return ((this))->TestUseOldFlag( style_Hidden); }

    inline void AddUnderline() { ((this))->AddFlag( style_Underline); }
    inline void RemoveUnderline() { ((this))->RemoveFlag( style_Underline); }
    inline void UseOldUnderline() { ((this))->UseOldFlag( style_Underline); }
    inline boolean IsUnderlineAdded() { return ((this))->TestAddFlag( style_Underline); }
    inline boolean IsUnderlineRemoved() { return ((this))->TestRemoveFlag( style_Underline); }
    inline boolean IsUnderlineUnchange() { return ((this))->TestUseOldFlag( style_Underline); }

    inline void AddPassThru() { ((this))->AddFlag( style_PassThru); }
    inline void RemovePassThru() { ((this))->RemoveFlag( style_PassThru); }
    inline void UseOldPassThru() { ((this))->UseOldFlag( style_PassThru); }
    inline boolean IsPassThruAdded() { return ((this))->TestAddFlag( style_PassThru); }
    inline boolean IsPassThruRemoved() { return ((this))->TestRemoveFlag( style_PassThru); }
    inline boolean IsPassThruUnchange() { return ((this))->TestUseOldFlag( style_PassThru); }

    inline void AddContinueIndent() { ((this))->AddFlag( style_ContinueIndent); }
    inline void RemoveContinueIndent() { ((this))->RemoveFlag( style_ContinueIndent); }
    inline void UseOldContinueIndent() { ((this))->UseOldFlag( style_ContinueIndent); }
    inline boolean IsContinueIndentAdded() { return ((this))->TestAddFlag( style_ContinueIndent); }
    inline boolean IsContinueIndentRemoved() { return ((this))->TestRemoveFlag( style_ContinueIndent); }
    inline boolean IsContinueIndentUnchange() { return ((this))->TestUseOldFlag( style_ContinueIndent); }

    inline void AddChangeBar() { ((this))->AddFlag( style_ChangeBar); }
    inline void RemoveChangeBar() { ((this))->RemoveFlag( style_ChangeBar); }
    inline void UseOldChangeBar() { ((this))->UseOldFlag( style_ChangeBar); }
    inline boolean IsChangeBarAdded() { return ((this))->TestAddFlag( style_ChangeBar); }
    inline boolean IsChangeBarRemoved() { return ((this))->TestRemoveFlag( style_ChangeBar); }
    inline boolean IsChangeBarUnchange() { return ((this))->TestUseOldFlag( style_ChangeBar); }

    inline void AddOverBar() { ((this))->AddFlag( style_OverBar); }
    inline void RemoveOverBar() { ((this))->RemoveFlag( style_OverBar); }
    inline void UseOldOverBar() { ((this))->UseOldFlag( style_OverBar); }
    inline boolean IsOverBarAdded() { return ((this))->TestAddFlag( style_OverBar); }
    inline boolean IsOverBarRemoved() { return ((this))->TestRemoveFlag( style_OverBar); }
    inline boolean IsOverBarUnchange() { return ((this))->TestUseOldFlag( style_OverBar); }

    inline void AddDottedBox() { ((this))->AddFlag( style_DottedBox); }
    inline void RemoveDottedBox() {  ((this))->RemoveFlag( style_DottedBox); }
    inline void UseOldDottedBox() {  ((this))->UseOldFlag( style_DottedBox); }
    inline boolean IsDottedBoxAdded() { return ((this))->TestAddFlag( style_DottedBox); }
    inline boolean IsDottedBoxRemoved() { return ((this))->TestRemoveFlag( style_DottedBox); }
    inline boolean IsDottedBoxUnchange() { return ((this))->TestUseOldFlag( style_DottedBox); }

    inline void AddStrikeThrough() {  ((this))->AddFlag( style_StrikeThrough); }
    inline void RemoveStrikeThrough() {  ((this))->RemoveFlag( style_StrikeThrough); }
    inline void UseOldStrikeThrough() {  ((this))->UseOldFlag( style_StrikeThrough); }
    inline boolean IsStrikeThroughAdded() { return ((this))->TestAddFlag( style_StrikeThrough); }
    inline boolean IsStrikeThroughRemoved() { return ((this))->TestRemoveFlag( style_StrikeThrough); }
    inline boolean IsStrikeThroughUnchange() { return ((this))->TestUseOldFlag( style_StrikeThrough); }

    inline void AddReadOnly() { ((this))->AddFlag( style_ReadOnly); }
    inline void RemoveReadOnly() { ((this))->RemoveFlag( style_ReadOnly); }
    inline void UseOldReadOnly() { ((this))->UseOldFlag( style_ReadOnly); }
    inline boolean IsReadOnlyAdded() { return ((this))->TestAddFlag( style_ReadOnly); }
    inline boolean IsReadOnlyRemoved() { return ((this))->TestRemoveFlag( style_ReadOnly); }
    inline boolean IsReadOnlyUnchange() { return ((this))->TestUseOldFlag( style_ReadOnly); }

    inline void AddNoWrap() { ((this))->AddFlag( style_NoWrap); }
    inline void RemoveNoWrap() { ((this))->RemoveFlag( style_NoWrap); }
    inline void UseOldNoWrap() { ((this))->UseOldFlag( style_NoWrap); }
    inline boolean IsNoWrapAdded() { return ((this))->TestAddFlag( style_NoWrap); }
    inline boolean IsNoWrapRemoved() { return ((this))->TestRemoveFlag( style_NoWrap); }
    inline boolean IsNoWrapUnchange() { return ((this))->TestUseOldFlag( style_NoWrap); }

    inline void AddNoFill() { ((this))->AddFlag( style_NoFill); }
    inline void RemoveNoFill() { ((this))->RemoveFlag( style_NoFill); }
    inline void UseOldNoFill() { ((this))->UseOldFlag( style_NoFill); }
    inline boolean IsNoFillAdded() { return ((this))->TestAddFlag( style_NoFill); }
    inline boolean IsNoFillRemoved() { return ((this))->TestRemoveFlag( style_NoFill); }
    inline boolean IsNoFillUnchange() { return ((this))->TestUseOldFlag( style_NoFill); }

    inline void AddKeepPriorNL() { ((this))->AddFlag( style_KeepPriorNL); }
    inline void RemoveKeepPriorNL() { ((this))->RemoveFlag( style_KeepPriorNL); }
    inline void UseOldKeepPriorNL() { ((this))->UseOldFlag( style_KeepPriorNL); }
    inline boolean IsKeepPriorNLAdded() { return ((this))->TestAddFlag( style_KeepPriorNL); }
    inline boolean IsKeepPriorNLRemoved() { return ((this))->TestRemoveFlag( style_KeepPriorNL); }
    inline boolean IsKeepPriorNLUnchange() { return ((this))->TestUseOldFlag( style_KeepPriorNL); }

    inline void AddKeepNextNL() { ((this))->AddFlag( style_KeepNextNL); }
    inline void RemoveKeepNextNL() { ((this))->RemoveFlag( style_KeepNextNL); }
    inline void UseOldKeepNextNL() { ((this))->UseOldFlag( style_KeepNextNL); }
    inline boolean IsKeepNextNLAdded() { return ((this))->TestAddFlag( style_KeepNextNL); }
    inline boolean IsKeepNextNLRemoved() { return ((this))->TestRemoveFlag( style_KeepNextNL); }
    inline boolean IsKeepNextNLUnchange() { return ((this))->TestUseOldFlag( style_KeepNextNL); }

    inline void AddTabsCharacters() { ((this))->AddFlag( style_TabsCharacters); }
    inline void RemoveTabsCharacters() { ((this))->RemoveFlag( style_TabsCharacters); }
    inline void UseOldTabsCharacters() { ((this))->UseOldFlag( style_TabsCharacters); }
    inline boolean IsTabsCharactersAdded() { return ((this))->TestAddFlag( style_TabsCharacters); }
    inline boolean IsTabsCharactersRemoved() { return ((this))->TestRemoveFlag( style_TabsCharacters); }
    inline boolean IsTabsCharactersUnchange() { return ((this))->TestUseOldFlag( style_TabsCharacters); }

    inline void AddIncludeBeginning() { ((this))->AddFlag( style_IncludeBeginning); }
    inline void RemoveIncludeBeginning() { ((this))->RemoveFlag( style_IncludeBeginning); }
    inline void UseOldIncludeBeginning() { ((this))->UseOldFlag( style_IncludeBeginning); }
    inline boolean IsIncludeBeginningAdded() { return ((this))->TestAddFlag( style_IncludeBeginning); }
    inline boolean IsIncludeBeginningRemoved() { return ((this))->TestRemoveFlag( style_IncludeBeginning); }
    inline boolean IsIncludeBeginningUnchange() { return ((this))->TestUseOldFlag( style_IncludeBeginning); }

    inline void AddIncludeEnd() { ((this))->AddFlag( style_IncludeEnd); }
    inline void RemoveIncludeEnd() { ((this))->RemoveFlag( style_IncludeEnd); }
    inline void UseOldIncludeEnd() { ((this))->UseOldFlag( style_IncludeEnd); }
    inline boolean IsIncludeEndAdded() { return ((this))->TestAddFlag( style_IncludeEnd); }
    inline boolean IsIncludeEndRemoved() { return ((this))->TestRemoveFlag( style_IncludeEnd); }
    inline boolean IsIncludeEndUnchange() { return ((this))->TestUseOldFlag( style_IncludeEnd); }

    char *name;
    char *menuName;                     /* e.g. "Font~1,Italic~11" */
    boolean template_c;                   /* Style from current template */

    struct marginstyle NewLeftMargin;
    struct marginstyle NewRightMargin;
    struct marginstyle NewTopMargin;
    struct marginstyle NewBottomMargin;

    struct marginstyle NewIndentation;
    enum style_Justification NewJustification;

    struct spacingstyle NewInterparagraphSpacing;
    struct spacingstyle NewAbove;
    struct spacingstyle NewBelow;
    struct spacingstyle NewInterlineSpacing;

    char *FontFamily;
    struct fontsizestyle FontSize;
    struct fontscriptstyle FontScript;
    long AddFontFaces;      /* OR'ed FontFaces */
    long OutFontFaces;      /* NOT'ed OR'ed FontFaces */

    long NumTabChanges;
    struct tabentry *TabChangeList;	/* ptr to array */

    char *CounterName;
    enum style_Scope CounterScope;
    enum style_Position CounterPosition;
    char *CounterParent;
    long CounterInitialValue;
    long NumCounterStyles;
    char **CounterStyles;               /* Array of strings */

    long AddMiscFlags;      /* Hinge, Pagebreak, Hidden, ... */
    long OutMiscFlags;

    class Namespace *AdditionalAttributes;
};

#endif /* _style_H_ */
