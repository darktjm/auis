/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("fontdesc.H")
#include <graphic.H>
#include <im.H>
#include <fontdesc.H>

#include <ctype.h>



ATKdefineRegistryNoInit(fontdesc, ATK);
static class fontdesc *fontdesc_CreateUsingDescriptor(struct fontnamedesc  *FontName, long  FontStyle, long  FontSize);


char *fontdesc::GetFontFamily()
{
    return this->FontName->name;
}

struct fontnamedesc *fontdesc::GetFontFamilyDesc()
{
    return this->FontName;
}

long fontdesc::GetFontSize()
{
    return this->FontSize;
}

long fontdesc::GetFontStyle()
{
    return this->FontStyles;
}

class graphic *fontdesc::CvtCharToGraphic(class graphic  *graphic, char  SpecialChar)
{
    /* Override Me */
    return NULL;
}

struct font *fontdesc::GetRealFontDesc(class graphic  *graphic)
{
    /* Override Me */
    return NULL;
}

long fontdesc::TextSize(class graphic  *graphic, const char  *text, long  TextLength, long  *XWidth, long  *YWidth)
{
    /* Override Me */
    return 0;
}

struct FontSummary *fontdesc::FontSummary(class graphic  *graphic)
{
    struct FontSummary *tsp;

    tsp = &this->summary;
    if (this->DescValid)
        return tsp;
    /* Ensure self->MachineDep... is valid */ 
    (this)->GetRealFontDesc( graphic);
    return tsp;
}

short *fontdesc::WidthTable(class graphic  *graphic)
{
    /* Override Me */
    return NULL;
}

short *fontdesc::HeightTable(class graphic  *graphic)
{
    /* Override Me */
    return NULL;

}

long fontdesc::StringSize(class graphic  *graphic,  const char *string,long  *XWidth,long  *YWidth)
{
    /* Override Me */
    return 0;
}

void fontdesc::CharSummary(class graphic  *gr,char  LookUpChar,struct fontdesc_charInfo  *retVal)
{
    /* Override Me */
}

/* Warning: The following routines are Textview critical code */

static class fontdesc *fontdesc_CreateUsingDescriptor(struct fontnamedesc  *FontName, long  FontStyle, long  FontSize)
{
    class fontdesc *retVal;

    for (retVal = FontName->fontList; retVal != NULL; retVal = retVal->next)
        if (retVal->FontStyles == FontStyle && retVal->FontSize == FontSize)
            return retVal;
    
    retVal = im::GetFontdesc();
    
    retVal->FontName = FontName;
    retVal->FontSize = FontSize;
    retVal->FontStyles = FontStyle;
    retVal->DescValid = FALSE;
    retVal->next = FontName->fontList;
    FontName->fontList = retVal;

    return retVal;
}

class fontdesc *fontdesc::Create(const char  *fontName, long  fontStyle, long  fontSize)
{
    char tempFontName[256], *s = tempFontName;
    do {    /* Fold lowercase */
        *s++ = isupper(*fontName) ? tolower(*fontName) : *fontName;
    } while (*fontName++);
    return fontdesc_CreateUsingDescriptor
      (fontdesc::GetFontNameDesc(tempFontName), fontStyle, fontSize);
}

struct fontnamedesc *fontdesc::GetFontNameDesc(char  *fontName)
{
    static struct fontnamedesc *allFontNames = NULL;
    struct fontnamedesc *tp;
    
    for (tp = allFontNames; tp != NULL; tp = tp->next)
        if (*tp->name == *fontName && ! strcmp(tp->name, fontName))
	    return tp;

    tp = (struct fontnamedesc *) malloc(sizeof (struct fontnamedesc));
    tp->next = allFontNames;
    allFontNames = tp;
    tp->fontList = NULL;
    tp->name = strdup(fontName);
    return tp;
}

class fontdesc *fontdesc::Allocate()
    {
    return (class fontdesc *) malloc(sizeof(class fontdesc));
}

void fontdesc::Deallocate(class fontdesc  *self)
        {
/* Fontdesc structures are never deallocated since they are reused. */
}

fontdesc::fontdesc()
{
    this->FontName = NULL;
    this->FontStyles = fontdesc_Plain;
    this->FontSize = 12;
    this->DescValid = FALSE;
    this->widthTable = NULL;
    this->heightTable = NULL;
    this->MachineDependentFontDescriptor = NULL;
    memset(this->charValid, 0, sizeof(this->charValid));
    THROWONFAILURE( TRUE);
}

fontdesc::~fontdesc()
{
    if (this->widthTable)
        free(this->widthTable);
    if (this->heightTable)
        free(this->heightTable);
    if (this->FontName)
        free(this->FontName);
}

/* This code parses backward looking for fonts of the following format:
 *
 * <family name><optional "-" or "-s"><point size><style modifiers>
 *
 * Where the style modifiers consist of the following single modifier codes:
 *
 *     b    bold
 *     i    italic
 *     f    fixed width    (This is bogus and should disappear)
 *     s    shadow
 *
 * If there is no font size in the name (i.e. no numbers in the name), the
 * entire name is returned as the familyName and the size and style are set
 * to 0.
 *
 * This works fairly well except for fonts like "6x10" and
 * "helvetica-bold-s12". In the first case, it gets the name wrong and in the
 * second case, it loses the bold style modifier.
 *
 * Without a consistent usable X font naming convention, there is no point in
 * worrying about any fonts that don't follow Andrew conventions.
 * Furthermore, this routine should not exists. The user interface should
 * never have to know about font naming. If the window system module needs to
 * know, it ought to have a local routine to parse the fontnames.
 */

boolean fontdesc::ExplodeFontName(const char  *fontName, char  *familyName, long  bufSize, long  *fontStyle, long  *fontSize)
{
    const char *endName;
    int style = 0;
    int size = 0;
    int length;

    if((fontName == NULL) || (*fontName == '\0'))
	return FALSE;
    else 
	endName = fontName + strlen(fontName) - 1;

    /* Parse style modifiers. Assumes there will be a font size between the
     * family name and the end of the whole name. */

    while (!isdigit(*endName)) {
        switch (*endName) {
            case 'b': 
                style |= fontdesc_Bold;
                break;
            case 'i': 
                style |= fontdesc_Italic;
                break;
            case 'f': 
                style |= fontdesc_Fixed;
                break;
            case 's': /* Shadow font */
                style |= fontdesc_Shadow;
                break;
            default: 
                break;
        }
	if(endName > fontName) 
	    endName--;
	else 
	    break;
    }

    if (endName == fontName) { /* No font size. Whole name is family name. */
	size = 12;
        style = 0;
        length = bufSize;
    }
    else {
        int tensPlace = 1;

        /* Snag the fontsize. */
        while (endName >= fontName && isdigit(*endName)) {
            size += (*endName-- - '0') * tensPlace;
            tensPlace *= 10;
        }

        /* Parse optional "-" or "-s". */
        if (endName >= fontName && *endName == '-')
            --endName;
        else if (endName > fontName && endName[-1] == '-' && endName[0] == 's')
            endName -= 2;

        /* If there is no family name, this call fails. */
        if (endName < fontName)
            return FALSE;

        length = endName - fontName + 1;
        if (length >= bufSize)
            length = bufSize - 1;
    }

    /* Fill in return values as appropriate. */

    if (fontStyle != NULL)
        *fontStyle = style;

    if (fontSize != NULL)
        *fontSize = size;
    
    if (familyName != NULL) {
        strncpy(familyName, fontName, length);
        familyName[length] = '\0';
    }

    return TRUE;
}

long fontdesc::StringBoundingBox(class graphic  *graphic, const char  *string, int  *width, int  *height)
{
  long w, a, d, ascent, descent, junk;
  short *fwt, *fht;
  const char *p;
  static struct fontdesc_charInfo ci;

  fwt = (this)->WidthTable ( graphic);
  fht = (this)->HeightTable ( graphic);
  if (fwt == NULL || fht == NULL)
    return 0;
  (this)->StringSize ( graphic, string, &w, &junk);
  for (p = string, a = 0, d = 0;  *p != (char) 0;  p += 1)
    {
      (this)->CharSummary ( graphic, *p, &ci);
      ascent = ci.yOriginOffset;
      descent = ci.height - ascent;
      a = MAX(a,ascent);
      d = MAX(d,descent);
    }
  *width = w;
  *height = a + d;
  return (w);
}  
