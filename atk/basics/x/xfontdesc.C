/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <sys/param.h> /* For MAXPATHLEN. */


#include <ctype.h>

#ifdef SGI_4D_ENV
#include <sys/stat.h>
#endif
ATK_IMPL("xfontdesc.H")
#include <xgraphic.H>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cmenu.h>

#include <fontdesc.H>
#include <im.H>
#include <xim.H>
#include <xfontdesc.H>
#include <environ.H>

/* Filled in in InitializeClass from a user preference.
 * If TRUE, font substitutions result in a warning being printed on stderr.
 */
static boolean announceSubstitutions;

#define GETXFONT(self, graphic)  \
	((XFontStruct *)(self)->GetRealFontDesc( graphic))
#define MDFD ((struct fcache *)(((class fontdesc *)self)->MachineDependentFontDescriptor))

/* the MachineDependentFontDescriptor field of the fontdesc points to a list of fcache values
	GetRealFontDescriptor searches this list.  
	Unless there are connections to multiple servers, the list will have only one element.

	The host field is the file descriptor (fd in the Display) because
	    X guarantees that fonts are uniquely defined for the server (not just the Display).
	{{So why is the argument to XFreeFont a Display rater than a server?}}
*/
struct fcache {
	Display *dpy;		/* first display the font was opened on (for XFreeFont) */
	unsigned long host;		/* the host file descriptor */
	XFontStruct *font;		/* the value for wm_SelectFont() */
	struct fcache *next;
};




/* ************** auxiliary static procedures ******** */

/* This procedure creates the name of the andrew font file
given a particular set of characteristics */


ATKdefineRegistry(xfontdesc, fontdesc, xfontdesc::InitializeClass);
static struct FontSummary *GetFontSummary(class xfontdesc  *self);
static void AddStyleModifiers(char  *string, int  styles);
static const char *GetNthDash(const char  *p, int  cnt);
static boolean XExplodeFontName(const char  *fontName, char  *familyName, long  bufSize, long  *fontStyle, long  *fontSize);
static struct bestfont *ClosestFonts(const char  **possibleNames, int  numNames, const char *desiredFamily, int  desiredSize, int  desiredStyle, int  *numBest, boolean  andyName);
static XFontStruct *GetClosestFont(class xgraphic  *graphic, const char *matchStr, const char *desiredName, int  desiredSize, int  desiredStyle, boolean  andyName, char *substitute);
static XFontStruct * xfontdesc_LoadXFont(class xfontdesc  * self, class xgraphic  *graphic);
static class xgraphic * GetAnyOldGraphic();
static class xgraphic * EnsureGraphic( class xgraphic  *graphic );



static struct dlist {
    int dpi;
    int fudge;
    struct dlist *next;
} *dfirst=NULL;

static struct dlist def[]={
    {75, 2, NULL}
};

static void FillDlist()
{
    const char *p=environ::GetProfile("AndyFontsFudgeFactor");
    if(p==NULL) {
	dfirst=def;
	return;
    }
    while(*p) {
	struct dlist *d;
	int dpi;
	int fudge=0;
	while(*p && isspace(*p)) p++;
	if(*p=='\0') return;
	dpi=atoi(p);
	p=index(p, ':');
	if(p==NULL) return;
	p++;
	while(*p && isspace(*p)) p++;
	if(*p=='\0') return;
	fudge=atoi(p);
	d=(struct dlist *)malloc(sizeof(struct dlist));
	if(d==NULL) return;
	d->dpi=dpi;
	d->fudge=fudge;
	d->next=dfirst;
	dfirst=d;
	p=index(p, ',');
	if(p==NULL) return;
	p++;
    }
}

static int BestFudgeFactor(long vdpi)
{
    int bestfudge=2;
    int bestdpidiff=99999;
    struct dlist *d=dfirst;
    if(d==NULL) {
	FillDlist();
	d=dfirst;
    }
    while(d) {
	if(ABS(d->dpi-vdpi)<bestdpidiff) {
	    bestfudge=d->fudge;
	    bestdpidiff=ABS(d->dpi-vdpi);
	}
	d=d->next;
    }
    return bestfudge;
}

static struct FontSummary *GetFontSummary(class xfontdesc  *self)
    {

     struct FontSummary *tsp;
     XCharStruct *maxChar;
     XFontStruct *fontInfo;
     unsigned int i;

     tsp = &self->summary;	/* for quick reference */
     /* otherwise DescValid is set to true in LoadAndrewFont   
		-- (I think this comment is meaningless -wjh) */
     memset(tsp, 0, sizeof (struct FontSummary));

     if (MDFD == NULL || MDFD->font == NULL) return tsp;
     fontInfo = MDFD->font;

     maxChar = &fontInfo->max_bounds;

     /* Read the font information from the max character in the font
	information structure */
     /* Not sure about the definition of first two statements */
     tsp->maxWidth = maxChar->rbearing - maxChar->lbearing; /*LR*/
     tsp->maxSpacing = maxChar->width; /* note different terminology */

     tsp->maxHeight = maxChar->ascent + maxChar->descent;
     tsp->newlineHeight = fontInfo->ascent + fontInfo->descent;
     if (tsp->newlineHeight == 0)
         tsp->newlineHeight = tsp->maxHeight;
     tsp->maxBelow = maxChar->descent;
     tsp->maxLeft = -(maxChar->lbearing);
     for(i=0;i<fontdesc_NumIcons;i++) {
	if (fontInfo->min_byte1 || fontInfo->max_byte1) 
	    (self)->SetCharValid( i);
	else if (i >= fontInfo->min_char_or_byte2 &&
			i <= fontInfo->max_char_or_byte2)  
	    (self)->SetCharValid( i);
     }
     return tsp;

}

static void AddStyleModifiers(char  *string, int  styles)
        {

    char *oldString = string;

    if (styles & fontdesc_Bold)
        *string++ = 'b';
    if (styles & fontdesc_Italic)
        *string++ = 'i';
    if (styles & fontdesc_Fixed)
        *string++ = 'f';
    if (styles & fontdesc_Shadow)
        *string++ = 's';

    if (oldString != string) /* Added something. */
        *string++ = '\000'; /* Add terminating NULL. */
}

struct bestfont {
    int index;
    int size;
    int style;
    int cost;
};

/* Number of good close fonts to try */
/* Maximum number of tries at loading fonts. This is done to either handle
 * a race condition in which the font path is reset between the XListFonts
 * call and the XLoadQueryFont call or if the server can not properly load
 * the font (a file server is down, or it has found a bad font)
 */

#define MAXBEST 3

static const char *GetNthDash(const char  *p, int  cnt)
{
    while (*p != '\0') {
	if (*p == '-') {
	    cnt--;
	    if (cnt <= 0) {
		return p;
	    }
	}
	p++;
    }
    return p;
}

static boolean XExplodeFontName(const char  *fontName, char  *familyName, long  bufSize, long  *fontStyle, long  *fontSize)
{
    const char *p;
    const char *end;
    int style = 0;
    int size = 0;
    int length;

    if((fontName == NULL) || (*fontName == '\0'))
	return FALSE;

    p = GetNthDash(fontName, 2);
    p++;
    end = GetNthDash(p, 1);
    if ((length = (end - p)) > (bufSize - 1)) {
	length = bufSize - 1;
    }
    strncpy(familyName, p, length);
    familyName[length] = '\0';
    
    p = end+1;
    if (strncmp(p, "bold-", 5) == 0) {
	style |= fontdesc_Bold;
    }
    else if (strncmp(p, "demibold-", 9) == 0) {
	style |= fontdesc_Medium;
    }

    p = GetNthDash(p, 1);
    p++;
    if (strncmp(p, "i-", 2) == 0 || strncmp(p, "o-", 2) == 0) {
	style |= fontdesc_Italic;
    }

    p = GetNthDash(p, 1);
    p++;
    if (strncmp(p, "narrow-", 7) == 0 || strncmp(p, "semicondensed-", 14) == 0 || strncmp(p, "condensed-", 10) == 0) {
	style |= fontdesc_Condense;
    }

    p = GetNthDash(p, 3);
    p++;
    if (*p >= '1' && *p <= '9') {
	size = 0;
	while (*p >= '0' && *p <= '9') {
	    size = size * 10 + (*p - '0');
	    p++;
	}
	size = (size + 5) / 10;
    }
    else {
	size = 12;
    }

    p = GetNthDash(p, 3);
    p++;
    if (strncmp(p, "m-", 2) == 0 || strncmp(p, "c-", 2) == 0) {
	style |= fontdesc_Fixed;
    }

    /* Fill in return values as appropriate. */

    if (fontStyle != NULL)
        *fontStyle = style;

    if (fontSize != NULL)
        *fontSize = size;
    
    return TRUE;
}

/* This routine trys to find the closest match to the desired size and style in
 * a list of names. (It assumes that the family name of all names in the list
 * is contains the desired family name as an initial substr.) The match is
 * done as follows:
 *
 *     First the font size is matched. This takes priority over the style mask.
 *
 *     Second the closest style is chosen. Closest here is defined as the least
 *     Hamming distance between the two style masks.
 *
 *     If the above come out equal, the larger font is favored so that any
 *     bounding boxes calculated from the font size will be bigger as opposed to smaller.
 *
 * This routine depends on fontdesc_ExplodeFont name to the names in the list
 * into their size and style.
 */
static struct bestfont *ClosestFonts(const char  **possibleNames, int  numNames, const char  *desiredFamily, int  desiredSize, int  desiredStyle, int  *numBest, boolean  andyName)
                            {

    int index;
    long style;
    long size;
    static struct bestfont bestFonts[MAXBEST + 1];
    int familyLength = strlen(desiredFamily);
    char familyName[MAXPATHLEN];
    int numDiscarded = 0;

/* These are two sentinels. The first is used to provide a bad font that will always get replaced. The second is used to provide a bad font so the loop searching for a bad font will always find one. After the first MAXBEST fonts are filled in, the first sentinel is gone. */
    bestFonts[0].cost = 1000000;
    bestFonts[MAXBEST].cost = 1000000;

    for (index = 0; index < numNames; ++index) {

        struct bestfont *bestFont;
        int cost;
        static char numBitsSet[16] = {0, 1, 1, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2};

	if (andyName) {
	    fontdesc::ExplodeFontName((char *)possibleNames[index], familyName, sizeof(familyName), &style, &size);
	}
	else {
	    XExplodeFontName(possibleNames[index], familyName, sizeof(familyName), &style, &size);
	}

/* Verify that the family name is the same. We know that the initial
 * substrings are the same since the names from possible names all match the
 * regular expression <family>*
 */
	if (familyName[familyLength] != '\0') {
	    numDiscarded++;
	    continue;
	}

        cost = (abs(size - desiredSize) * 4) + numBitsSet[((desiredStyle ^ style) & 0x0f)];

/* Insert it in the array at the appropriate place if its cost within the
 * MAXBEST smallest costs we have seen.
 */

        for (bestFont = bestFonts; cost > bestFont->cost || (cost == bestFont->cost && size <= bestFont->size); ++bestFont)
            ;

        if (bestFont != bestFonts + MAXBEST) {

            struct bestfont *tempFont;

            for (tempFont = bestFonts + MAXBEST - 2; tempFont >= bestFont; --tempFont) {
                tempFont[1] = tempFont[0];
            }

            bestFont->index = index;
            bestFont->size = size;
            bestFont->style = style;
            bestFont->cost = cost;
        }
    }

    numNames -= numDiscarded;

    *numBest = (numNames > MAXBEST) ? MAXBEST : numNames;
    return bestFonts;
}


/* Maximum number of font names to ask the server for to do a closest match
 * against.
 */

#define MAXNAMES 50

static XFontStruct *GetClosestFont(class xgraphic  *graphic, const char *matchStr, const char *desiredName, int  desiredSize, int  desiredStyle, boolean  andyName, char *substitute)
{
    char **fontNames = NULL;
    int numNames;
    int numBest;
    XFontStruct *font = NULL;
    struct bestfont *bestFonts;

    fontNames = XListFonts((graphic)->XDisplay(), matchStr, MAXNAMES, &numNames);

    if (numNames > 0) {
	bestFonts = ClosestFonts((const char **)fontNames, numNames, desiredName, desiredSize, desiredStyle, &numBest, andyName);

	while (numBest > 0 && font == NULL) {
	    font = XLoadQueryFont((graphic)->XDisplay(), fontNames[bestFonts->index]);
	    if (bestFonts->size != desiredSize || bestFonts->style != desiredStyle)
		strcpy(substitute, fontNames[bestFonts->index]);
	    --numBest;
	    ++bestFonts;
	}
    }
    if (fontNames != NULL)
	XFreeFontNames(fontNames);

    return font;
}

/* This routine works by constructing the Andrew fontname for the requested
 * font. If this fails, it then constructs an X fontname. If that fails, it
 * then asks the server for all the font names that
 * begin with the family name of the requested font. It then runs a closest
 * match against the returned names. If the resulting names cannot be loaded,
 * the process is repeated. If this fails three times, the "fixed" or
 * "variable" font is requested. If this fails, the server is asked for any font
 * name and that is loaded. If that fails, the routine gives up.
 */
	static XFontStruct *
xfontdesc_LoadXFont(class xfontdesc  * self, class xgraphic  *graphic)
		{

    XFontStruct *font = NULL;
    int desiredSize = self->FontSize;
    int fudge;
    int desiredStyle = self->FontStyles;
    /* Need bounds checking. */
    char fontName[MAXPATHLEN];
    char *familyEnd;
    char **fontNames = NULL;
    char substitute[MAXPATHLEN];
    struct fcache *oldMDFD = MDFD;
    char xstyleName[MAXPATHLEN];
    const char *xfamily;
    static const char *charset = 0;

    substitute[0] = '\0';

    if (!charset) {
	charset = environ::Get("MM_CHARSET");
	if (!charset) {
#ifdef ISO80_FONTS_ENV
	    charset = "iso-8859-1";
#else
	    charset = "us-ascii";
#endif
	}
	char *s = strdup(charset);
	charset = s;
	for (; *s; s++) {
	    if (isupper(*s)) *s = tolower(*s);
	}
    }

    /* First try andy style name as is. */
    strcpy(fontName, self->FontName->name);
    familyEnd = fontName + strlen(fontName);
    if (desiredSize != 0)
	sprintf(familyEnd, "%d", desiredSize);
    AddStyleModifiers(familyEnd + strlen(familyEnd), desiredStyle);

    /* Don't try non-symbol andy fonts if we want more than us-ascii */
    if (!strcmp(charset, "us-ascii") ||
	(strcmp(fontName, "andy") &&
	 strcmp(fontName, "andysans") &&
	 strcmp(fontName, "andytype")))
	font = XLoadQueryFont((graphic)->XDisplay(), fontName);

    /* Then try X naming convention. */
    if (font == NULL) {
        const char *andyfamily;
        const char *weight;
        const char *slant;
        const char *spacing;
	static int UseXLFDNames = -1;

	if (UseXLFDNames == -1)
	    UseXLFDNames = environ::GetProfileSwitch("UseXLFDNames", TRUE);

        andyfamily = self->FontName->name;
        if (UseXLFDNames && strncmp(andyfamily, "andy", 4) == 0) {
            fudge = BestFudgeFactor((graphic)->GetVerticalResolution());
            if (andyfamily[4] == '\0') {
                xfamily = "times";
            }
            else if (strcmp(andyfamily+4, "sans") == 0) {
                xfamily = "helvetica";
            }
            else if (strcmp(andyfamily+4, "type") == 0) {
                xfamily = "courier";
            }
            else if (strcmp(andyfamily+4, "symbol") == 0) {
                xfamily = "symbol";
            }
            else {
                xfamily = andyfamily;
                fudge = 0;
            }
        }
        else {
            xfamily = andyfamily;
            fudge = 0;
        }

        weight = (desiredStyle & fontdesc_Bold) ? "bold" : 
          ((desiredStyle & fontdesc_Medium) ? "demibold" : "medium");
        slant = (desiredStyle & fontdesc_Italic) ? "i" : "r";
	spacing = (desiredStyle & fontdesc_Fixed) ? "m" : "p";

        sprintf(xstyleName, "-*-%s-%s-%s-*-*-*-%d-*-*-%s-*-*-*", xfamily, weight, slant, 10 * (desiredSize + fudge), spacing);
        font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);

        /* try character cell */
        if (font == NULL && (desiredStyle & fontdesc_Fixed)) {
            sprintf(xstyleName, "-*-%s-%s-%s-*-*-*-%d-*-*-c-*-*-*", xfamily, weight, slant, 10 * (desiredSize + fudge));
            font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);
        }

        /* try w/o spec'ing the spacing */
        if (font == NULL) {
            sprintf(xstyleName, "-*-%s-%s-%s-*-*-*-%d-*-*-*-*-*-*", xfamily, weight, slant, 10 * (desiredSize + fudge));
            font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);
        }


        /* try olblique */
        if (font == NULL && (desiredStyle & fontdesc_Italic)) {
            sprintf(xstyleName, "-*-%s-%s-o-*-*-*-%d-*-*-*-*-*-*", xfamily, weight, 10 * (desiredSize + fudge));
            font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);
        }

        /* try demibold */
        if (font == NULL && (desiredStyle & fontdesc_Bold)) {
            sprintf(xstyleName, "-*-%s-demibold-%s-*-*-*-%d-*-*-*-*-*-*", xfamily, slant, 10 * (desiredSize + fudge));
            font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);
        }

        /* try very loosely */
        if (font == NULL) {
            sprintf(xstyleName, "-*-%s-*-*-*-*-*-%d-*-*-*-*-*-*", xfamily, 10 * (desiredSize + fudge));
            font = XLoadQueryFont((graphic)->XDisplay(), xstyleName);
        }

    }

    /* Look for close fonts in the andy name space. */
    if (font == NULL) {
	char matchStr[MAXPATHLEN];
	strcpy(matchStr, self->FontName->name);
	strcat(matchStr, "*");

	font = GetClosestFont(graphic, matchStr, self->FontName->name, desiredSize, desiredStyle, TRUE, substitute);

    }

    /* Look for close fonts in the x name space. */
    if (font == NULL) {
	sprintf(xstyleName, "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", xfamily);
	font = GetClosestFont(graphic, xstyleName, xfamily, (desiredSize + fudge), desiredStyle, FALSE, substitute);
    }

    /* If above failed, try the two "guaranteed" X fonts. */
    if (font == NULL) {
	if (desiredStyle & fontdesc_Fixed) {
	    strcpy(substitute, "fixed");
	    font = XLoadQueryFont((graphic)->XDisplay(), substitute);
	}
	else {
	    strcpy(substitute, "variable");
	    font = XLoadQueryFont((graphic)->XDisplay(), substitute);
	}
    }

    /* Out of desperation, ask the server for anything. */
    if (font == NULL) {

        int numNames;

        fontNames = XListFonts((graphic)->XDisplay(), "*", 1, &numNames);
	if (numNames != 0) {
	    strcpy(substitute, fontNames[0]);
	    font = XLoadQueryFont((graphic)->XDisplay(), substitute);
	}
	if (fontNames != NULL)
	    XFreeFontNames(fontNames);
    }

/* add to cache of defined fonts, one for each server
 * we can assume that this server is not in the list 
 * because otherwise LoadFont is not called 
 */
    {
	struct fcache **fcp, *fc;
	/* find pointer to NULL */
	for (		fcp = (struct fcache **)
				(&self->MachineDependentFontDescriptor); 
			*fcp != NULL;
			fcp = &((*fcp)->next))
		{}
	/* now fcp points to a pointer to NULL 
		build new list elt in fc */
	fc = (struct fcache *)malloc(sizeof(struct fcache));
	fc->next = NULL;
	fc->dpy = (graphic)->XDisplay();
	fc->host = ConnectionNumber(fc->dpy);
	fc->font = font;
	*fcp = fc;	/* link the new one into the list */	
    }
    self->DescValid = (MDFD != NULL);

    if (substitute[0] != '\0' && announceSubstitutions && font != NULL)
        fprintf(stderr, "%s: Substituting font %s for %s .\n", im::GetProgramName(), substitute, fontName);


    /* font summary info is treated here as being the same on all servers
     *	if this is invalid, the fontsummary info has to be added to fcache
     */
    if (oldMDFD == NULL)
	GetFontSummary(self);		/* compute font summary info */
    return font;
}

/* KLUDGE  called by  ENSUREGRAPHIC() */
/* GetAnyOldGraphic()
	Finds a graphic (from the im) in case a call has been made 
	from a view which is not linked.
*/
	static class xgraphic *
GetAnyOldGraphic()
{
	class im *im;
#ifdef DEVELOPER_ENV
	if(environ::GetProfileSwitch("ShowBadGraphicErrors", TRUE)) {
	    fprintf(stderr, "WARNING: GetAnyOldGraphic called.\n");
	}
#endif /* DEVELOPER_ENV */
	im = im::GetLastUsed();
	return  ((im == NULL) ? NULL : (class xgraphic *)im->GetDrawable());
}

static class xgraphic *
EnsureGraphic( class xgraphic  *graphic )
{
    if (graphic == NULL || ! (graphic)->Valid()) {
	graphic = GetAnyOldGraphic();
    	if (graphic == NULL || ! (graphic)->Valid()) 
	    return(NULL);
    }
    return(graphic);
}

/* ************* methods ****************** */

class graphic *xfontdesc::CvtCharToGraphic(class graphic  *graphic2, char  SpecialChar)
                {
    class xgraphic *graphic=(class xgraphic *)graphic2;
    class xgraphic * RetValue;
    long width, height;
    long x, y;
    XFontStruct *info;
    XCharStruct *ci;
    Pixmap newPixmap;
    GC gc;
    char str[1];
    XGCValues gcattr;
    int depth;
    unsigned char sc = SpecialChar;

    if((graphic = EnsureGraphic(graphic)) == NULL)
	return(NULL);
    depth = 1; /* Force depth of one for Stippling. was: DefaultDepth(xgraphic_XDisplay(graphic), xgraphic_XScreen(graphic)); */
    RetValue = new xgraphic;
    if (!RetValue) return NULL;
/* This is text code extracted from my test suite, not the real stuff */

    info = GETXFONT(this, graphic);
    if (sc >= info->min_char_or_byte2 && sc <= info->max_char_or_byte2) {
	ci = (info->per_char != NULL) ? &(info->per_char[sc - info->min_char_or_byte2]) : &(info->max_bounds);
	width = ci->width;
	height = info->max_bounds.ascent + info->max_bounds.descent;
/* Older code - no longer seems to work properly
	width = ci->rbearing - ci->lbearing;
	height = ci->descent + ci->ascent;
*/
	x = 0;
/* Older code - no longer seems to work properly
	x = -ci->lbearing;
*/
	y = ci->ascent;
	/* Note: we could have an empty character, in which case, we simulate it with a 1 by 1 character. Too bad X doesn't allow 0 sized pixmaps */
	if (!width) {
	    fprintf(stderr,"xfontdesc_CvtCharToGraphic: 0 width character %d in %p\n", SpecialChar, this);
	    width++;
	}
	if (!height) {
	    fprintf(stderr,"xfontdesc_CvtCharToGraphic: 0 height character %d in %p\n", SpecialChar, this);
	    height++;
	}
	newPixmap = XCreatePixmap((graphic)->XDisplay(), (graphic)->XWindow(), width, height, depth);
        /* The following is bogus: it should be a in-core graphic
	   and created the same way -- insert it into a "universal pixmap"
	   such as the root of the window and use ordinary graphic operations
	   to fill in the values instead of going through the X versions.
	   This should allow 1) transportability to other systems and
           2) consistent semantics with other xgraphic objects */

	gcattr.fill_style = FillSolid;
	gcattr.foreground = 0;
#ifndef PLANEMASK_ENV
	gc = XCreateGC((graphic)->XDisplay(), newPixmap, GCFillStyle | GCForeground, &gcattr);
#else /* PLANEMASK_ENV */
	gcattr.plane_mask = graphic->foregroundpixel ^ graphic->backgroundpixel;
	gc = XCreateGC((graphic)->XDisplay(), newPixmap, GCFillStyle | GCForeground | GCPlaneMask, &gcattr);
#endif /* PLANEMASK_ENV */
	XFillRectangle((graphic)->XDisplay(), newPixmap, gc, 0, 0, width, height);
	XSetFont((graphic)->XDisplay(), gc, info->fid);
	XSetForeground((graphic)->XDisplay(), gc, 1 );
	XSetFunction((graphic)->XDisplay(), gc, GXcopy);
	str[0] = SpecialChar;
	XDrawString((graphic)->XDisplay(), newPixmap, gc, x, y, str, 1);
	XFreeGC((graphic)->XDisplay(), gc);
    }
    else  {
	newPixmap = XCreatePixmap((graphic)->XDisplay(), (graphic)->XWindow(), 8, 8, depth);
    }
    RetValue->localWindow = newPixmap;	
    RetValue->displayUsed = (graphic)->XDisplay();
    RetValue->screenUsed = (graphic)->XScreen();
    /* oops */
    RetValue->localGraphicContext = NULL;
    RetValue->localFillGraphicContext = NULL;
    return (class graphic *) RetValue;
}

	struct font *
xfontdesc::GetRealFontDesc(class graphic  *graphic2)
	{
	    class xfontdesc *self=this;
	    struct fcache *fc;
	    class xgraphic *graphic=(class xgraphic *)graphic2;
	if((graphic = EnsureGraphic(graphic)) == NULL)
	    return(NULL);
	for (		fc = MDFD; 
			fc != NULL && (int)fc->host != ConnectionNumber((graphic)->XDisplay()); 
			fc = fc->next)
		{}
	if (fc == NULL)
		return (struct font *)xfontdesc_LoadXFont(this, graphic);
	else return (struct font *)fc->font;
}

long xfontdesc::TextSize(class graphic  *graphic2, const char  * text, long  TextLength, long  * XWidth, long  * YWidth)
                        {
    XFontStruct *font;
    long retWidth = 0;
    class xgraphic *graphic=(class xgraphic *)graphic2;

    if((graphic = EnsureGraphic(graphic)) == NULL)
	return(0);
    font = GETXFONT(this,graphic);

    if (font)
        retWidth = XTextWidth(font,text,TextLength);
    if (XWidth) *XWidth = retWidth;
    if (YWidth) *YWidth = 0;
    return retWidth;
}

/* This procedure returns the font size table for all characters in a font 
	XXX the value should depend on which display we are on */
short *xfontdesc::WidthTable(class graphic  *graphic2)
		{
	XFontStruct *font;
	short * fontWidthTable;
	unsigned int i;
	class xgraphic *graphic=(class xgraphic *)graphic2;

	if (this->widthTable)
		return this->widthTable;

	if((graphic = EnsureGraphic(graphic)) == NULL)
	    return(NULL);
	fontWidthTable = (short *) malloc(fontdesc_NumIcons*sizeof(short));
	this->widthTable = fontWidthTable;
	font = GETXFONT(this, graphic);

	/* Is it a "small" font that we can handle? */
	if (font->min_byte1 || font->max_byte1) 
		/* oh well, we have a 16 bit font and we are screwed.
		   try to keep going by returning the same size for
		   everything */
		for(i=0; i < fontdesc_NumIcons; i++) 
			fontWidthTable[i] = font->max_bounds.width;
	else 
		for(i=0; i < fontdesc_NumIcons; i++) {
			if (i < font->min_char_or_byte2 ||
					i > font->max_char_or_byte2)
				 fontWidthTable[i] = 0;
			else
				fontWidthTable[i] = (font->per_char)
					? font->per_char[i-font->min_char_or_byte2].width 
					: font->max_bounds.width;
	}
	return fontWidthTable;
}

/* This procedure returns the font size table for all characters in a font 
	XXX the value should depend on which display we are on */
short *xfontdesc::HeightTable(class graphic  *graphic2)
		{

	XFontStruct *font;
	short * fontHeightTable;
	unsigned int i;
	class xgraphic *graphic=(class xgraphic *)graphic2;

	if (this->heightTable)
		return this->heightTable;

	if((graphic = EnsureGraphic(graphic)) == NULL)
	    return(NULL);
	fontHeightTable = (short *) malloc(fontdesc_NumIcons*sizeof(short));
	this->heightTable = fontHeightTable;
	font = GETXFONT(this, graphic);

	/* Is it a "small" font that we can handle? */
	if (font->min_byte1 || font->max_byte1)
		/* oh well, we have a 16 bit font and we are screwed.
		   try to keep going by returning the same size for
		   everything */
		for(i=0; i<fontdesc_NumIcons; i++)
			fontHeightTable[i] = font->max_bounds.ascent +
					font->max_bounds.descent;
	else
		for(i=0; i<fontdesc_NumIcons; i++) {
			if (i < font->min_char_or_byte2 ||
					i > font->max_char_or_byte2)
				fontHeightTable[i] = 0;
			else if (font->per_char != NULL)
				fontHeightTable[i] =
					font->per_char[i-font->min_char_or_byte2].ascent +
					font->per_char[i-font->min_char_or_byte2].descent;
			else
				fontHeightTable[i] = font->max_bounds.ascent +
					font->max_bounds.descent;
		}
	return fontHeightTable;
}

long xfontdesc::StringSize(class graphic  *graphic2, const char * string,long  * XWidth,long  * YWidth)
                    {

    XFontStruct  *font;
    long retWidth = 0;
    class xgraphic *graphic=(class xgraphic *)graphic2;

    if((graphic = EnsureGraphic(graphic)) == NULL)
	return(0);
    font = GETXFONT(this, graphic);
    if (font)
        retWidth = XTextWidth(font,string,strlen((char *) string));
    if (XWidth) *XWidth = retWidth;
    if (YWidth) *YWidth = 0;
    return retWidth;
}

void xfontdesc::CharSummary(class graphic  * graphic2, char  LookUpChar, struct fontdesc_charInfo  *RetValue)
                {

	XFontStruct *font;
	class xgraphic *graphic=(class xgraphic *)graphic2;
    if (!RetValue) return;
    if((graphic = EnsureGraphic(graphic)) == NULL)
	return;
    font = GETXFONT(this, graphic);

    /* Is it a "small" font that we can handle? */
    if (font->min_byte1 || font->max_byte1) {
	/* oh well, we have a 16 bit font and we are screwed.
	   try to keep going by returning the same size for
	   everything */
	XCharStruct * maxChar = &font->max_bounds;
	RetValue->width = maxChar->rbearing-maxChar->lbearing;
	RetValue->height = maxChar->ascent+maxChar->descent;
	RetValue->xOriginOffset = -(maxChar->lbearing);
	RetValue->yOriginOffset = maxChar->ascent;     
	RetValue->xSpacing = maxChar->width;
	RetValue->ySpacing = 0;
    }
    else {
	if ((unsigned char)LookUpChar<font->min_char_or_byte2 ||
	    (unsigned char)LookUpChar>font->max_char_or_byte2) {
	    RetValue->width = 0;
	    RetValue->height = 0;
	    RetValue->xOriginOffset = 0;
	    RetValue->yOriginOffset = 0;
	    RetValue->xSpacing = 0;
	    RetValue->ySpacing = 0;
		}
	else {
	    unsigned char lc = LookUpChar;
	    XCharStruct * c = (font->per_char != NULL && lc <= font->max_char_or_byte2) 
			? &font->per_char[lc-font->min_char_or_byte2] 
			: &(font->max_bounds);
	    RetValue->width = c->rbearing- c->lbearing;
	    RetValue->height = c->ascent + c->descent;
	    RetValue->xOriginOffset = -(c->lbearing);
	    RetValue->yOriginOffset = c->ascent;     
	    RetValue->xSpacing = c->width;
	    RetValue->ySpacing = 0;
	}
    }
}

/* ************* predefines ************** */

xfontdesc::xfontdesc()
		{
	ATKinit;

	this->MachineDependentFontDescriptor = NULL;
	THROWONFAILURE( TRUE);
}

xfontdesc::~xfontdesc()
		{
	ATKinit;
	class xfontdesc *self=this;
	struct fcache *fc, *tfc;
	for (fc = MDFD; fc != NULL; fc = tfc) {
		tfc = fc->next;
		XFreeFont(fc->dpy, fc->font);
	}
}

	boolean
xfontdesc::InitializeClass()
	{

	announceSubstitutions = 
		environ::GetProfileSwitch("AnnounceFontSubstitutions", FALSE);
	return TRUE;
}
