/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * BE1 to BE2 conversion package
 */


#include <andrewos.h>
ATK_IMPL("be1be2.H")
#include <ctype.h>

#include <style.H>
#include <stylesheet.H>
#include <environment.H>
#include <fontdesc.H>
#include <text.H>

#include <be1be2.H>

/* Primitive environment attributes */

#define SVLeftMargin 0		/* text left margin */
#define SVRightMargin 1		/* text right margin */
#define SVIndent 2		/* para. indentation */
#define SVScript 3		/* superscript distance */
#define SVMode 4		/* e.g. center, filled */
#define SVSpacing 5		/* interline spacing */
#define SVSpread 6		/* interpara. spacing */
#define SVTabs 7		/* list of tab stops */
#define SVFlags 8		/* random flags */
#define SVFunc 9		/* name of procedures */
#define SVUnused 10             /* Slot 10 unused */
#define SVFont 11		/* current font family */
#define SVFontMod 12		/* font size, facecode */
#define SVFontFace 12
#define SVFontSize 12
#define SVXSize 13		/* size of ext. struct. */
#define SVMyFont 13		/* internal font struct. */
#define SVInset 14		/* inset pointer */
#define SVIcon 15		/* active icon pointer */
#define SVSize 16		/* total size of all */

/*SVFlag encodings */

#define FUnderline 1
#define FHidden 2		/* invisible text */
#define FReadOnly 4
#define FPassthru 8		/* to justifier */
#define FIcon 16                /* SVFunc=icon name; else inset name */
#define FContinueIndent	32

/* SVMode encodings (justification) */

#define MCenter 0
#define MLeft 1
#define MRight 2
#define MFilled 3
#define MJust 4
#define MLeftRight 5

/* Style sheet operand types (units) */

#define STInch 0
#define STCM 1
#define STPoint 2
#define STEm 3
#define STLine 4
#define STString 5
#define STInt 6

/* Style sheet modification opcodes */

#define SOpNop 0
#define SOpCopy 1
#define SOpAdd 2
#define SOpSet 3
#define SOpClear 4

/*
 * Package classprocedures
 */


ATKdefineRegistry(be1be2, ATK, NULL);
static long CVDots(long  amt, enum style_Unit  unit);
static boolean ConvertStyle(class style  *self, long  attr , long  opcode , long  optype , long  opparm);


boolean be1be2::CheckBE1(class text  *text)
{
    long gotlen, len = (text)->GetLength();
    int pos = 0;
    unsigned char *p;
    int envcount = 0, defcount = 0, bincount = 0;

    while (pos < len)
        for (p = (unsigned char *) (text)->GetBuf( pos, 1024, &gotlen); gotlen--; p++, pos++)
            if (*p & 0x80) {
                bincount++;
                switch (*p) {
                    case 0x80:
                        envcount--;
                        break;
                    case 0x81:
                        break;
                    case 0x82:
                        defcount--;
                        break;
                    case 0x83:
                        defcount++;
                        break;
                    default:
                        envcount++;
                }
            }

    /*
     * Heuristics:
     *
     *   If the document has at least a few non-ascii characters,
     *   the number of unclosed environments is small, and the
     *   number of unclosed style definitions is small, the document
     *   is almost surely BE1.  By not requiring exactness, there's
     *   room for slightly buggy BE1 files.
     */

    return (bincount >= 4) &&
           (envcount >= -2 && envcount <= 2) &&
           (defcount >= -2 && defcount <= 2);
}

/*
 * Main Convert Routines:
 *    Convert, derived from old BE2's text.c (ConvertFromBE1)
 *    ConvertStyle, derived from old BE2's style.c (style_ReadBE1).
 *    CVDots, copied from style.c
 */

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

static boolean ConvertStyle(class style  *self, long  attr , long  opcode , long  optype , long  opparm)
{
    boolean retVal = TRUE;

    static enum style_Unit UnitConversion[7] = {
        style_Inches,
        style_CM,
        style_Points,
        style_Ems,
        style_Lines,
        style_RawDots,
        style_RawDots
    };

    /* Take apart each array element and patch it together */

    switch (attr) {
        case SVLeftMargin:
            switch (opcode) {
                case SOpNop:
                    (self)->SetNewLeftMargin( style_LeftMargin, 0, style_RawDots);
                    break;
                case SOpCopy:
                    (self)->SetNewLeftMargin( style_ConstantMargin, opparm, UnitConversion[optype]);
                    break;
                case SOpAdd:
                    (self)->SetNewLeftMargin( style_LeftMargin, opparm, UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetNewLeftMargin( style_LeftMargin, 0, style_RawDots);
                    break;
            }
            break;

        case SVRightMargin:
            switch (opcode) {
                case SOpNop:
                    (self)->SetNewRightMargin( style_RightMargin, 0, style_RawDots);
                    break;
                case SOpCopy:
                    (self)->SetNewRightMargin( style_ConstantMargin, opparm, UnitConversion[optype]);
                    break;
                case SOpAdd:
                    (self)->SetNewRightMargin( style_RightMargin, opparm, UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetNewRightMargin( style_RightMargin, 0, style_RawDots);
                    break;
            }
            break;

        case SVIndent:
            switch (opcode) {
                case SOpNop:
                    (self)->SetNewIndentation( style_PreviousIndentation, 0, style_RawDots);
                    break;
                case SOpCopy:
                    (self)->SetNewIndentation( style_LeftEdge, opparm, UnitConversion[optype]);
                    break;
                case SOpAdd:
                    (self)->SetNewIndentation( style_LeftMargin, opparm, UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetNewIndentation( style_PreviousIndentation, 0, style_RawDots);
                    break;
            }
            break;

        /*
         * BE1 font scripting expressed the scripting in
         * points but used fixed-point <16 bit>.<16 bit>
         * notation inconsistent with all other values.  BE2
         * used to do this but not any more.  Therefore
         * when the unit is points an adjustment is made.
         * Some accuracy may be lost.
         */

        case SVScript:
            switch (opcode) {
                case SOpNop:
                    (self)->SetFontScript( style_PreviousScriptMovement, 0, style_RawDots);
                    break;
                case SOpCopy:
                    if (UnitConversion[optype] == style_Points)
                        opparm >>= 16;
                    (self)->SetFontScript( style_ConstantScriptMovement, opparm, UnitConversion[optype]);
                    break;
                case SOpAdd:
                    if (UnitConversion[optype] == style_Points)
                        opparm >>= 16;
                    (self)->SetFontScript( style_PreviousScriptMovement, opparm, UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetFontScript( style_PreviousScriptMovement, 0, style_RawDots);
                    break;
            }
            break;

        case SVMode:
            switch (opcode) {
                case SOpNop:
                    (self)->SetJustification( style_PreviousJustification);
                    break;
                case SOpCopy: 
                    switch (opparm) {
                        case MCenter:
                            (self)->SetJustification( style_Centered);
                            break;
                        case 6:
                        case MLeft:
                            (self)->SetJustification( style_LeftJustified);
                            break;
                        case MRight:
                            (self)->SetJustification( style_RightJustified);
                            break;
                        case MFilled:
                            (self)->SetJustification( style_LeftJustified);
                            break;
                        case MJust:
                            (self)->SetJustification( style_LeftAndRightJustified);
                            break;
                        case MLeftRight:
                            (self)->SetJustification( style_LeftThenRightJustified);
                            break;
                        default:
                            retVal = FALSE;
                            (self)->SetJustification( style_PreviousJustification);
                            break;
                    }
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetJustification( style_PreviousJustification);
                    break;
            }
            break;

        case SVSpacing:
            switch (opcode) {
                case SOpNop:
                    (self)->SetNewInterlineSpacing( style_InterlineSpacing, 0, style_RawDots);
                    break;
                case SOpCopy:
                    (self)->SetNewInterlineSpacing( style_ConstantSpacing, opparm, UnitConversion[optype]);
                    break;
                case SOpAdd:
                    (self)->SetNewInterlineSpacing( style_InterlineSpacing, opparm, UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetNewInterlineSpacing( style_InterlineSpacing, 0, style_RawDots);
                    break;
            }
            break;

        case SVSpread:
            switch (opcode) {
                case SOpNop:
                    (self)->SetNewInterparagraphSpacing( style_InterparagraphSpacing, 0, style_RawDots);
                    break;
                case SOpCopy:
                    (self)->SetNewInterparagraphSpacing( style_ConstantSpacing, CVDots(opparm, UnitConversion[optype]), UnitConversion[optype]);
                    break;
                case SOpAdd:
                    (self)->SetNewInterparagraphSpacing( style_InterparagraphSpacing, CVDots(opparm, UnitConversion[optype]), UnitConversion[optype]);
                    break;
                default:
                    retVal = FALSE;
                    (self)->SetNewInterparagraphSpacing( style_InterparagraphSpacing, 0, style_RawDots);
                    break;
            }

        case SVTabs:
            if (opcode == SOpCopy && opparm != 0) {
                long TabCount, TabLoc, i;
                char *StrPtr;

                (self)->AddTabChange( style_AllClear, 0, style_RawDots);

                StrPtr = (char *) opparm;
                (long) sscanf(StrPtr, "%ld", &TabCount);
                for (i = 0; i < TabCount; i++) {
                    /* Skip leading white space */
                    while (*StrPtr == ' ') StrPtr++;
                    if (!StrPtr) break;
                    /* Skip over digits that were read */
                    while (*StrPtr> '0' && *StrPtr < '9') StrPtr++;
                    if (!StrPtr) break;
                    /* Get the next tab stop */
                    sscanf(StrPtr,"%ld",&TabLoc);
                    (self)->AddTabChange( style_LeftAligned, TabLoc, style_Points);
                }
            }
            break;

        case SVFlags:
            if ((opparm & FUnderline) && opcode==SOpSet) (self)->AddUnderline();
            if ((opparm & FUnderline) && opcode==SOpClear) (self)->RemoveUnderline();

            if ((opparm & FHidden) && opcode==SOpSet) (self)->AddHidden();
            if ((opparm & FHidden) && opcode==SOpClear) (self)->RemoveHidden();

            if ((opparm & FReadOnly) && opcode==SOpSet) (self)->AddReadOnly();
            if ((opparm & FReadOnly) && opcode==SOpClear) (self)->RemoveReadOnly();

            if ((opparm & FPassthru) && opcode==SOpSet) (self)->AddPassThru();
            if ((opparm & FPassthru) && opcode==SOpClear) (self)->RemovePassThru();

            if (opparm & FIcon) {
                /* This is a hack and should be removed */
                self->AddMiscFlags |= style_Icon;
                self->OutMiscFlags |= style_Icon;
            }

            if (opparm & FContinueIndent) {
                /* This is a hack and should be removed */
                self->AddMiscFlags |= style_ContinueIndent;
                self->OutMiscFlags |= style_ContinueIndent;
            }

            if (opparm > FContinueIndent || opparm < 0)
                retVal = FALSE;
            break;

        case SVFunc:    /* We don't know what this is (element 9). */
            break;
                        /* We also don't know what element 10 is. */
        case SVFont:
            (self)->SetFontFamily( (char *) opparm);
            break;

        case SVFontMod:
            if ((opparm & BoldFace) && opcode == SOpSet)
                (self)->AddNewFontFace( fontdesc_Bold);
            if ((opparm & BoldFace) && opcode == SOpClear)
                (self)->RemoveOldFontFace( fontdesc_Bold);

            if ((opparm & ItalicFace) && opcode == SOpSet)
                (self)->AddNewFontFace( fontdesc_Italic);
            if ((opparm & ItalicFace) && opcode == SOpClear)
                (self)->RemoveOldFontFace( fontdesc_Italic);

            if ((opparm & ShadowFace) && opcode == SOpSet)
                (self)->AddNewFontFace( fontdesc_Shadow);
            if ((opparm & ShadowFace) && opcode == SOpClear)
                (self)->RemoveOldFontFace( fontdesc_Shadow);

            if ((opparm & FixedWidthFace) && opcode == SOpSet)
                (self)->AddNewFontFace( fontdesc_Fixed);
            if ((opparm & FixedWidthFace) && opcode == SOpClear)
                (self)->RemoveOldFontFace( fontdesc_Fixed);


            if ((opparm & -16) && opcode==SOpSet) 
                (self)->SetFontSize(style_ConstantFontSize,opparm >> 4);
            else if ((opparm & -16) && opcode==SOpAdd) 
                (self)->SetFontSize(style_PreviousFontSize,opparm >> 4);
            break;

        /* Elements 12, 14, 15 and 16 are all internal information that can be ignored */

        default:
            break;
    }

    return retVal;
}

#define MAXBE1STYLES 256
#define MAXENVSTACK 100

struct envElem {
    class environment *env;
    long pos;
};

boolean be1be2::Convert(class text  *text)
{
    static struct envElem envStack[MAXENVSTACK], *envSP;
    static char *BE1map[MAXBE1STYLES];  /* Maps index to style name */
    static int BE1mapIndex;
    class style *stylep;
    class environment *rootenv;
    long i, pos = 0, savePos, len, textlen;
    boolean retVal = TRUE;

    for (i = 0; i < MAXBE1STYLES; i++)
        BE1map[i] = NULL;

    rootenv = (class environment *)(text->rootEnvironment)->GetEnclosing( pos);

    envSP = envStack;
    envSP->env = rootenv;
    envSP->pos = (rootenv)->Eval();

    textlen = (text)->GetLength();
    pos = 0;

    while (pos < textlen) {
        unsigned char p;
        savePos = pos;
        p = (text)->GetChar( pos++);

        if (p == 0x83) {	/* Style definition */
            long attr, opcode, optype, opparm;
            char stylename[100], menuname[100], parmbuffer[400];
            char *tp;

            /* Read index */

            BE1mapIndex = 0;
            while (1) {
                p = (text)->GetChar( pos++);
                if (! isdigit(p))
                    break;
                BE1mapIndex = BE1mapIndex * 10 + p - '0';
            }

            if (p != ',') {    /* Expected comma */
                retVal = FALSE; /* Flag error, then leave junk */
                continue;       /* in document (rather than crap out) */
            }

            /* Read menu name */

            tp = menuname;
            while (pos < textlen) {
                p = (text)->GetChar( pos++);
                if (p == 0x82 || p == '\n')
                    break;
                *tp++ = p;
            }
            if (p == 0x82) {
                retVal = FALSE;
                continue;
            }
            *tp = 0;

            /* Style name is lowercased tail of menu name */

            i = 0;
            tp = menuname;
            while (*tp != 0) {
                stylename[i++] = isupper(*tp) ? tolower(*tp) : *tp;
                if (*tp == ',')
                    i = 0;
                tp++;
            }
            stylename[i] = '\0';

            /* Create a style in stylesheet, replacing it if it exists */

	    if ((stylep = (text->styleSheet)->Find( stylename))) {
		class style st;
		stylep=(text->styleSheet)->MakeWritable(stylename);
		st.Copy(stylep);
            } else
                stylep = new style;

            (stylep)->SetName( stylename);
            (text->styleSheet)->Add( stylep);
            (stylep)->SetMenuName( menuname);

            /* Put it in the style index-to-name map */

            BE1map[BE1mapIndex] = strdup(stylename);

            /* Now fill in the state vector change list */

            while (pos < textlen) {
                p = (text)->GetChar( pos++);

                if (p == 0x82)
                    break;              /* End style sheet */
                attr = p - 0x41;      /* Get attribute number */      
                p = (text)->GetChar( pos++);
                opcode = p - 0x40;	/* 0x40 is not a typo */
                p = (text)->GetChar( pos++);
                optype = p - 0x41;

                if (optype == STString) {
                    i = 0;

                    while (1) {
                        p = (text)->GetChar( pos++);
                        if (p == '\012')
                            break;
                        parmbuffer[i] = p;
                        i++;
                    }
                    parmbuffer[i] = '\0';
                    tp = strdup(parmbuffer);
                    opparm = (long) tp;
                } else {
                    int tval;
                    boolean negate;

                    p = (text)->GetChar( pos++);
                    tval = 0;
                    if ((negate = (p == '-')) || p == '+')
                        p = (text)->GetChar( pos++);
                    while (isdigit(p) )  {
                        tval = tval * 10 + p - '0';
                        p = (text)->GetChar( pos++);
                    }
                    if (negate)
                        tval = -tval;

                    opparm = tval;
                }

                if (ConvertStyle(stylep, attr, opcode, optype, opparm) == FALSE)
                    retVal = FALSE;

                if ((optype == STString) && (tp != NULL))
                    free (tp);
            }
            /* Remove the style definition from the document */
            (text)->AlwaysDeleteCharacters( savePos, pos - savePos);
            textlen -= pos - savePos;
            pos = savePos;
            continue;
        }

        if ((0xc0 & p) == 0xc0 && p != 0xff) {    /* Begin env */
            class environment *newenv;
            stylep = NULL;

            if (BE1map[p - 0xc0] != NULL)
                stylep = (text->styleSheet)->Find( BE1map[p - 0xc0]);
            if (stylep == NULL) {    /* Bogus environment begin code */
                retVal = FALSE;
                continue;
            }
            newenv = (envSP->env)->InsertStyle( pos - 1 - envSP->pos, stylep, TRUE);
            envSP++;
            envSP->env = newenv;
            envSP->pos = pos - 1;
            /* Remove the environment-begin code from document */
            (text)->AlwaysDeleteCharacters( savePos, 1);
            textlen--;
            pos = savePos;
            continue;
        }

        if (p == 0x80) {    /* End environment */
            if (envSP != envStack)  {
                if ((len = (pos - 1 - envSP->pos)) > 0) {
                    (envSP->env)->SetLength( len);
                    envSP--;
                } else {

                    (envSP->env)->Delete();
                    envSP--;
                }
                /* Remove the environment-end code from document */
                (text)->AlwaysDeleteCharacters( savePos, 1);
                textlen--;
                pos = savePos;
            }
            continue;
        }

        /* Leave the character alone */
    }

    for (i = 0; i < MAXBE1STYLES; i++)
        if (BE1map[i] != NULL)
            free(BE1map[i]);

    return retVal;
}
