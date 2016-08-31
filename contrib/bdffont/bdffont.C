/* bdffont.c	font editor object */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
	Copyright Carnegie Mellon University 1991, 1992 - All rights reserved
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


#include <andrewos.h>
#include <math.h>

#include <bdffont.H>
#include <text.H>

#include <ansitext.h>
#include <mathaux.h>
#include <util.h>

struct bdffont_fontchar bdffont_ReadCharDefn;


/* declarations for parser */

#define bdfprop_INT_LEN (10)
/* include parser tables and actions */

ATKdefineRegistry(bdffont, dataobject, NULL);
static unsigned char  bdflex_ComposeByte(char  c1 , char  c2);
void bdffont_EnsureDefns(class bdffont  *self, long  num);
static void parseerror(class parse  *p, int  s, char  *m);
static void WriteCharacter(FILE  *file, struct bdffont_fontchar  *defn);
static long bdffont_AppendProperty(char  **props, long  length, const char  *p, long  psize, long  v);
static long FilterPropertiesOut(class bdffont  *self, char  **props);
static void RecomputeFontExtent(class bdffont  *self);
static void RecomputeFontOrigin(class bdffont  *self);


static const char bdffont_FONT_VERSION[] = "2.1";
static const char bdffont_FONT_NAME[] = "NewFont";
static const char bdffont_COMMENT[] = "COMMENT Created with bdffont (Andrew User Interface System)\n";
static const char bdffont_FOUNDRY[] = "AUIS";

static const char bdfprop_FOUNDRY[] = "FOUNDRY";		/* string */
static const char bdfprop_DEFAULTCHAR[] = "DEFAULT_CHAR";	/* non-neg int */
static const char bdfprop_DEFAULTWIDTH[] = "DEFAULT_WIDTH";	/* positive int */
static const char bdfprop_DEFAULTHEIGHT[] = "DEFAULT_HEIGHT";	/* positive int */
static const char bdfprop_DEFAULTX[] = "DEFAULT_X";		/* non-neg int */
static const char bdfprop_DEFAULTY[] = "DEFAULT_Y";		/* non-neg int */
static const char bdfprop_DEFAULTDX[] = "DEFAULT_DX";		/* non-neg int */
static const char bdfprop_DEFAULTDY[] = "DEFAULT_DY";		/* non-neg int */
static const char bdfprop_ASCENT[] = "FONT_ASCENT";		/* positive int */
static const char bdfprop_DESCENT[] = "FONT_DESCENT";		/* positive int */
static const char bdfprop_FAMILY[] = "FAMILY";		/* string */
static const char bdfprop_RESX[] = "RESOLUTION_X";		/* positive int */
static const char bdfprop_RESY[] = "RESOLUTION_Y";		/* positive int */
static const char bdfprop_WEIGHTNAME[] = "WEIGHT_NAME";	/* string */
static const char bdfprop_WEIGHT[] = "WEIGHT";		/* non-neg int */


	static unsigned char 
bdflex_ComposeByte(char  c1 , char  c2)
    {
    unsigned char result;

    if (('0' <= c1) && (c1 <= '9')) {
	result = (c1 - '0') << 4;
    }
    else if (('a' <= c1) && (c1 <= 'f')) {
	result = (c1 - 'a' + 10) << 4;
    }
    else { /* (('A' <= c1) && (c1 <= 'F')) */
	result = (c1 - 'A' + 10) << 4;
    }

    if (('0' <= c2) && (c2 <= '9')) {
	return (result | (c2 - '0'));
    }
    else if (('a' <= c2) && (c2 <= 'f')) {
	return (result | (c2 - 'a' + 10));
    }
    else { /* (('A' <= c2) && (c2 <= 'F')) */
	return (result | (c2 - 'A' + 10));
    }
} /* bdflex_ComposeByte */


void bdffont_EnsureDefns(class bdffont  *self, long  num)
{
    int ix;
    long old;

    if (!self->defns) {
	self->defns_size = num+1;
	self->defns = (struct bdffont_fontchar *)malloc(sizeof(struct bdffont_fontchar) * self->defns_size);

	for (ix=0; ix<self->defns_size; ix++) {
	    self->defns[ix].encoding = ix;
	    self->defns[ix].bitmap = NULL;
	}
    }
    else {
	if (self->defns_size >= num+1)
	    return;
	old = self->defns_size;
	while (self->defns_size < num+1)
	    self->defns_size *= 2;

	self->defns = (struct bdffont_fontchar *)realloc(self->defns, sizeof(struct bdffont_fontchar) * self->defns_size);

	for (ix=old; ix<self->defns_size; ix++) {
	    self->defns[ix].encoding = ix;
	    self->defns[ix].bitmap = NULL;
	}
    }
}

bdffont::bdffont()
{
    int i;

    this->version = (char *)bdffont_FONT_VERSION; /* never freed */
    this->comments = (char *)bdffont_COMMENT; /* never freed */
    this->fontname = NULL;
    this->pointsize = 0;
    this->resx = 0;
    this->resy = 0;
    this->bbw = 0;
    this->bbh = 0;
    this->bbx = 0;
    this->bby = 0;
    this->fontfamily = NULL;
    this->fontweight = -1;	/* indicates default weight for given face */
    this->fontface = 0;		/* indicates roman, normal type face */
    this->ascent = 0;
    this->descent = 0;
    this->properties = NULL;
    this->proplength = 0;
    this->activedefns = 0;
    this->defaultw = 0;
    this->defaulth = 0;
    this->defaultx = 0;
    this->defaulty = 0;
    this->defaultdx = 0;
    this->defaultdy = 0;
    this->defaultchar = -1;

    this->defns_size = 0;
    this->defns = NULL;

    bdffont_EnsureDefns(this, 255);

    THROWONFAILURE( (TRUE));
} /* bdffont__InitializeObject */

bdffont::~bdffont()
{
    if (this->version && (this->version != bdffont_FONT_VERSION)) {
	free(this->version);
    }
    if (this->comments && (this->comments != bdffont_COMMENT)) {
	free(this->comments);
    }
    if (this->fontname) {
	free(this->fontname);
    }
    if (this->fontfamily) {
	free(this->fontfamily);
    }
    if (this->properties) {
	free(this->properties);
    }
} /* bdffont__FinalizeObject */

#define bdffont_DefaultWidth	(64)
#define bdffont_DefaultHeight	(64)
#define bdffont_DefaultOriginX	(0)
#define bdffont_DefaultOriginY	(0)

#define RoundUp(x) ((long) ((x) + 0.5))

/* pts in points, resx/y in dots per inch */
class bdffont *bdffont::CreateNewFont(long  pts, long  resx, long  resy)
{
    class bdffont *self;
    double fontsize;

    self = new bdffont;

    self->comments = (char *)bdffont_COMMENT; /* never freed */
    self->pointsize = pts;
    self->resx = resx;
    self->resy = resy;

    fontsize = (self)->ComputeFontSize();

    self->ascent = /*mathaux_*/RoundUp(ansitext_ComputeAscent(fontsize));
    self->descent = /*mathaux_*/RoundUp(ansitext_ComputeDescent(fontsize));
    (self)->SetBoundingBox(
			    bdffont_DefaultWidth, bdffont_DefaultHeight,
			    bdffont_DefaultOriginX, bdffont_DefaultOriginY);

    self->defaultw = bdffont_DefaultWidth;
    self->defaulth = bdffont_DefaultHeight;
    self->defaultx = bdffont_DefaultOriginX;
    self->defaulty = bdffont_DefaultOriginY;
    self->defaultdx = bdffont_DefaultWidth;
    self->defaultdy = 0;
    self->defaultchar = 32;

    return (self);
} /* bdffont__CreateNewFont */



static char *ReadLine(FILE *file) {
    static char *line=NULL;
    static int linesize=0;
    int c;
    int pos=0;
    while((c=getc(file))!=EOF && c!='\n') {
	if(pos>=linesize-2) {
	    linesize+=100;
	    if(line) line=(char *)realloc(line, linesize);
	    else line=(char *)malloc(linesize);
	    if(line==NULL) return NULL;
	}
	line[pos++]=c;
    }
    line[pos]='\0';
    return line;
}


enum keyvals {
    FOUNDRY,
    FAMILY,
    WEIGHT,
    DEFAULT_CHAR,
    DEFAULT_WIDTH,
    DEFAULT_HEIGHT,
    DEFAULT_X,
    DEFAULT_Y,
    DEFAULT_DX,
    DEFAULT_DY,
    FONT_ASCENT,
    FONT_DESCENT,
    RESOLUTION_X,
    RESOLUTION_Y,
    WEIGHT_NAME,
    
    STARTFONT, 
    SIZE, 
    FONTBOUNDINGBOX, 
    STARTPROPERTIES, 
    ENDPROPERTIES, 
    CHARS, 
    ENCODING, 
    SWIDTH, 
    DWIDTH, 
    BBX, 
    ATTRIBUTES, 
    ENDCHAR, 
    ENDFONT,
    COMMENT,
    COPYRIGHT,
    FONT,
    BITMAP,
    STARTCHAR,
    UNKNOWNKEY
};


static const char * const keys[]= {
    "FOUNDRY",
    "FAMILY",
    "WEIGHT",
    "DEFAULT_CHAR",
    "DEFAULT_WIDTH",
    "DEFAULT_HEIGHT",
    "DEFAULT_X",
    "DEFAULT_Y",
    "DEFAULT_DX",
    "DEFAULT_DY",
    "FONT_ASCENT",
    "FONT_DESCENT",
    "RESOLUTION_X",
    "RESOLUTION_Y",
    "WEIGHT_NAME",

    "STARTFONT", 
    "SIZE", 
    "FONTBOUNDINGBOX", 
    "STARTPROPERTIES", 
    "ENDPROPERTIES", 
    "CHARS", 
    "ENCODING", 
    "SWIDTH", 
    "DWIDTH", 
    "BBX", 
    "ATTRIBUTES", 
    "ENDCHAR", 
    "ENDFONT",
    "COMMENT",
    "COPYRIGHT",
    "FONT",
    "BITMAP",
    "STARTCHAR",
    NULL
};


enum keyvals FindKey(const char *key)
{
    int i=0;
    const char * const *p=keys;
    while(*p) {
	if(strcmp(key, *p)==0) return (enum keyvals)i;
	i++;
	p++;
    }
    return (enum keyvals)i;
}

static char *bdfparse_Next=NULL;
static char *bdfparse_Limit=NULL;
#define bdfparse_Increment 100

static void bdfparse_EnsureStorage(char  **stg, long  length)
{
    long next;
    long newsize;

    if (bdfparse_Next + length > bdfparse_Limit) {
	next = bdfparse_Next - *stg;
	newsize = bdfparse_Limit - *stg;

	do {
	    newsize += bdfparse_Increment;
	} while (next + length > newsize);

	*stg = (char *) realloc(*stg, newsize);
	bdfparse_Next = *stg + next;
	bdfparse_Limit = *stg + newsize;
    }
} /* bdfparse_EnsureStorage */

static void AddComment(class bdffont *self, const char *key, const char *comment) {
    long length = strlen(comment)+strlen(key);
    bdfparse_EnsureStorage(&self->comments, length+3);
    strcpy(bdfparse_Next, key);
    strcat(bdfparse_Next, " ");
    strcat(bdfparse_Next, comment);
    strcat(bdfparse_Next, "\n");
    bdfparse_Next += length+2; 
}

static void AddProperty(class bdffont *self, long *propsize, const char *key, const char *prop) { 
    long length = strlen(prop)+strlen(key)+3;
    long proplen=self->properties?strlen(self->properties):0;
    if(proplen+length>=*propsize-1) {
	*propsize+=length+100;
	if(self->properties) self->properties=(char *)realloc(self->properties, *propsize);
	else {
	    self->properties=(char *)malloc(*propsize);
	    if(self->properties==NULL) return;
	    self->properties[0]='\0';
	}
	if(self->properties==NULL) return;
    }
    strcpy(self->properties+proplen, key);
    strcat(self->properties+proplen, " ");
    strcat(self->properties+proplen, prop);
    strcat(self->properties+proplen, "\n"); 
}

static char *Tokenize(char **p) {
    char *tok;
    while(**p && isspace(**p)) (*p)++;
    tok=(*p);
    while(**p && !isspace(**p)) (*p)++;
    if(**p) {
	**p='\0';
	(*p)++;
    }
    while(**p && isspace(**p)) (*p)++;
    return tok;
}

#define ERROR(p,where) if(*p=='\0') { fprintf(stderr, "bdffont: Bad format in %s directive.\n", keys[where]); return -1;}

static const char * const FaceNames[] = {
    "Roman",
    "Bold",
    "Italic",
    "BoldItalic",
    "Fixed",
    "FixedBold",
    "FixedItalic",
    "FixedBoldItalic",
    "Shadowed",
    "ShadowedBold",
    "ShadowedItalic",
    "ShadowedBoldItalic",
    "ShadowedFixed",
    "ShadowedFixedBold",
    "ShadowedFixedItalic",
    "ShadowedFixedBoldItalic" 
};
static long ReadIt(class bdffont *self, FILE *file) {
    int done=0;
    int inprops=0;
    long propsize=0;
    while(!feof(file) && !done) {
	enum keyvals kv;
	char *line=ReadLine(file);
	char *p=line;
	char *key;
	char *value;
	key=Tokenize(&p);
	value=p;
	kv=FindKey(key);
	switch(kv) {
	    case STARTFONT:
		{
		self->version=NewString(value);
		self->comments=(char *)malloc(bdfparse_Increment);
		self->comments[0]='\0';
		bdfparse_Next=self->comments;
		bdfparse_Limit=self->comments+bdfparse_Increment;
		}		
		break;
	    case COMMENT:
	    case COPYRIGHT:
		if(inprops) AddProperty(self, &propsize, key, value);
		else AddComment(self, key, value);
		break;
	    case FONT:
		self->fontname=NewString(value);
		break;
	    case SIZE:
 		p=Tokenize(&value);
		ERROR(p,SIZE);
		self->pointsize=atol(p);
		p=Tokenize(&value);
		ERROR(p,SIZE);
		self->resx=atol(p);
		p=Tokenize(&value);
		ERROR(p,SIZE);
		self->resy=atol(p);
		break;
	    case FONTBOUNDINGBOX:
		p=Tokenize(&value);
		ERROR(p,FONTBOUNDINGBOX);
		self->bbw=atol(p);
		p=Tokenize(&value);
		ERROR(p,FONTBOUNDINGBOX);
		self->bbh=atol(p);
		p=Tokenize(&value);
		ERROR(p,FONTBOUNDINGBOX);
		self->bbx=atol(p);
		p=Tokenize(&value);
		ERROR(p,FONTBOUNDINGBOX);
		self->bby=atol(p);
		break;
	    case STARTPROPERTIES:
		inprops=1;
		break;
	    case ENDPROPERTIES:
		inprops=0;
		break;
	    case CHARS:
		break;
	    case STARTCHAR:
		bdffont_ReadCharDefn.attributes=0;
		self->activedefns++;
		strncpy(bdffont_ReadCharDefn.name, value, sizeof(bdffont_ReadCharDefn.name));
		bdffont_ReadCharDefn.name[sizeof(bdffont_ReadCharDefn.name) - 1] = '\0';
		break;
	    case ENCODING:
		p=Tokenize(&value);
		ERROR(p, ENCODING);
		if(*value) p=Tokenize(&value);
		bdffont_ReadCharDefn.encoding=atol(p);
		break;
	    case SWIDTH:
		p=Tokenize(&value);
		ERROR(p, SWIDTH);
		bdffont_ReadCharDefn.swx=atol(p);
		p=Tokenize(&value);
		ERROR(p, SWIDTH);
		bdffont_ReadCharDefn.swy=atol(p);
		break;
	    case DWIDTH:
		p=Tokenize(&value);
		ERROR(p, DWIDTH);
		bdffont_ReadCharDefn.dwx=atol(p);
		p=Tokenize(&value);
		ERROR(p, DWIDTH);
		bdffont_ReadCharDefn.dwy=atol(p);
		break;
	    case BBX:
		p=Tokenize(&value);
		ERROR(p, BBX);
		bdffont_ReadCharDefn.bbw=atol(p);
		p=Tokenize(&value);
		ERROR(p, BBX);
		bdffont_ReadCharDefn.bbh=atol(p);
		p=Tokenize(&value);
		ERROR(p, BBX);
		bdffont_ReadCharDefn.bbx=atol(p);
		p=Tokenize(&value);
		ERROR(p, BBX);
		bdffont_ReadCharDefn.bby=atol(p);
		break;
	    case ATTRIBUTES:
		sscanf(value, "%lX", &bdffont_ReadCharDefn.attributes);
		break;
	    case BITMAP:
		{
		long row, column;
		long width;
		long alignedwidth;
		long pad;
		int c1, c2;
		unsigned char *bytes;

		bdffont_ReadCharDefn.bitmap =
		  (unsigned char *)malloc(bdffont_BitmapSize(&bdffont_ReadCharDefn));
		bytes = bdffont_ReadCharDefn.bitmap;

		width = bdffont_WidthInBytes(bdffont_ReadCharDefn.bbw);
		alignedwidth = bdffont_AlignedWidthInBytes(&bdffont_ReadCharDefn);
		pad = alignedwidth - width;

		c1 = getc(file);
		for (row = bdffont_ReadCharDefn.bbh; row > 0; row--) {
		    column = width;
		    while (column > 0) 
			if (isspace(c1)) 
			    c1 = getc(file);
			else {
			    c2 = getc(file);
			    *bytes++ = bdflex_ComposeByte(c1, c2);
			    c1 = getc(file);
			    column--;
			}

		     /* Ignore any trailing padding present in the file */
		    while (c1 != EOF && c1 != '\n')
			c1 = getc(file);
		     /* Do the padding ourselves */
		    memset(bytes, '\0', pad);
		    bytes += pad;
		}
		}
		break;
	    case ENDCHAR:
		{
		if (bdffont_ReadCharDefn.encoding >= 0) {
		    bdffont_EnsureDefns(self, bdffont_ReadCharDefn.encoding);
		    self->defns[bdffont_ReadCharDefn.encoding] =
		      bdffont_ReadCharDefn;
		    self->lastcharloaded = bdffont_ReadCharDefn.encoding;
		}
		else {
		    int cnum;
		    self->lastcharloaded++;
		    cnum = self->lastcharloaded;
		    bdffont_EnsureDefns(self, cnum);
		    self->defns[cnum] = bdffont_ReadCharDefn;
		}
		}
		break;
	    case ENDFONT:
		return 0;
		break;
	    case DEFAULT_CHAR:
		AddProperty(self, &propsize,  key, value);
		self->defaultchar=atol(value);
		break;
	    case DEFAULT_WIDTH:
		AddProperty(self, &propsize,  key, value);
		self->defaultw=atol(value);
		break;
	    case DEFAULT_HEIGHT:
		AddProperty(self, &propsize,  key, value);
		self->defaulth=atol(value);
		break;
	    case DEFAULT_X:
		AddProperty(self, &propsize,  key, value);
		self->defaultx=atol(value);
		break;
	    case DEFAULT_Y:
		AddProperty(self, &propsize,  key, value);
		self->defaulty=atol(value);
		break;
	    case DEFAULT_DX:
		AddProperty(self, &propsize,  key, value);
		self->defaultdx=atol(value);
	    case DEFAULT_DY:
		AddProperty(self, &propsize,  key, value);
		self->defaultdy=atol(value);
		break;
	    case FONT_ASCENT:
		AddProperty(self, &propsize,  key, value);
		self->ascent=atol(value);
		break;
	    case FONT_DESCENT:
		AddProperty(self, &propsize,  key, value);
		self->descent=atol(value);
		break;
	    case RESOLUTION_X:
		AddProperty(self, &propsize,  key, value);
		self->resx=atol(value);
		break;
	    case RESOLUTION_Y:
		AddProperty(self, &propsize,  key, value);
		self->resy=atol(value);
		break;
	    case WEIGHT:
		AddProperty(self, &propsize,  key, value);
		self->fontweight=atol(value);
		break;
	    case FAMILY:
		self->fontfamily=NewString(value);
		break;
	    case WEIGHT_NAME:
		{
		int i;
		char *p=Tokenize(&value);
		char *q;
		if(*p=='"') p++;
		q=strchr(p, '"');
		if(q) *q='\0';
		for(i=0;i<sizeof(FaceNames)/sizeof(char *);i++) {
		    if(ULstrcmp(FaceNames[i], p)==0) {
			self->fontface=i;
			break;
		    }
		}
		}
		break;
	    case FOUNDRY:
	    default:
		if(inprops) AddProperty(self, &propsize,  key, value);
		continue;
	}
    }
}
long bdffont::Read(FILE  *file, long  id)
{
    int severity;
    long read_status;
    this->activedefns = 0;
    this->defaultchar = -1;
    this->lastcharloaded = (-1);

    read_status=ReadIt(this, file);
    if (read_status != dataobject_NOREADERROR)
	fprintf(stderr, "bdffont: an error occurred while reading the font file.\n");

    return (read_status);
} /* bdffont__Read */

static void WriteCharacter(FILE  *file, struct bdffont_fontchar  *defn)
{
    unsigned char *bm;
    long row, col, rowbytes, pad;

    if (defn->bitmap) {
	if (defn->name) {
	    fprintf(file, "STARTCHAR %s\n", defn->name);
	}
	else {
	    fprintf(file, "STARTCHAR c%o\n", (unsigned) defn->encoding);
	}

	fprintf(file, "ENCODING %d\n", (unsigned) defn->encoding);
	fprintf(file, "SWIDTH %ld %ld\n", defn->swx, defn->swy);
	fprintf(file, "DWIDTH %ld %ld\n", defn->dwx, defn->dwy);
	fprintf(file, "BBX %ld %ld %ld %ld\n",
		defn->bbw, defn->bbh, defn->bbx, defn->bby);

	if (defn->attributes) {
	    fprintf(file, "ATTRIBUTES %lX\n", defn->attributes);
	}

	bm = defn->bitmap;
	rowbytes = bdffont_WidthInBytes(defn->bbw);
	pad = bdffont_AlignedWidthInBytes(defn) - rowbytes;

	fprintf(file, "BITMAP\n");
	for (row = 0; row < defn->bbh; row++) {
	    for (col = 0; col < rowbytes; col++) {
		fprintf(file, "%02X", *bm++);
	    }
	    bm += pad;
	    fprintf(file, "\n");
	}
	fprintf(file, "ENDCHAR\n");
    }
} /* WriteCharacter */

int bdffont::GetDefaultChar()
{
    int encoding;

    if (this->defaultchar == -1) {
	encoding = (int) ' ';
	while ((encoding < this->defns_size) &&
	       ! bdffont_IsActive(&this->defns[encoding]))
	{
	    encoding++;
	}

	if (encoding == this->defns_size) {
	    encoding = 0;
	    while ((encoding < (int) ' ') &&
		   ! bdffont_IsActive(&this->defns[encoding]))
	    {
		encoding++;
	    }
	}

	return (encoding);
    }

    return ((int) this->defaultchar);
} /* bdffont_GetDefaultChar */

#define bdffont_IsPrefix(main, prefix) \
	((strncmp(main, prefix, sizeof(prefix) - 1) == 0) && \
	isspace(main[sizeof(prefix)-1]))

static long bdffont_AppendProperty(char  **props, long  length, const char  *p, long  psize, long  v)
{
    long newlength = length + psize + bdfprop_INT_LEN + 1;
    /* includes property label, space, value, and line-feed */

    *props = (char *) realloc(*props, newlength + 1); /* include NUL */
    sprintf(*props + length, "%s %*ld\n", p, bdfprop_INT_LEN, v);

    return (newlength);
} /* bdffont_AppendProperty */

static long FilterPropertiesOut(class bdffont  *self, char  **props)
{
    long count = 0;
    boolean needASCENT = TRUE;
    boolean needDESCENT = TRUE;
    boolean needDEFAULTCHAR = TRUE;
    boolean needDEFAULTW = TRUE;
    boolean needDEFAULTH = TRUE;
    boolean needDEFAULTX = TRUE;
    boolean needDEFAULTY = TRUE;
    boolean needDEFAULTDX = TRUE;
    boolean needDEFAULTDY = TRUE;
    boolean needFOUNDRY = TRUE;
    boolean needFAMILY = TRUE;
    boolean needWEIGHTNAME = TRUE;
    boolean needWEIGHT = TRUE;
    boolean needRESX = TRUE;
    boolean needRESY = TRUE;
    char *inattr = self->properties;
    char *next;
    char *outattr;
    long length;

    self->proplength=strlen(inattr);
    outattr =
	*props = (char *) malloc(self->proplength ? self->proplength + 1 : 72);

    if (inattr) while (*inattr) {
	long pos=outattr-*props;
	char *p=strchr(inattr, '\n');
	long len;
	count++;

	if(p) len=p-inattr+1;
	else len=strlen(inattr)+1;

	*props=(char *)realloc(*props, pos+len+1);
	next = inattr;
	while ((*outattr++ = *next++) != '\n') {
	    /* continue; */
	}
	if (bdffont_IsPrefix(inattr, bdfprop_FOUNDRY)) {
	    needFOUNDRY = FALSE;
	    pos=outattr-*props;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_ASCENT)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_ASCENT, sizeof(bdfprop_ASCENT), self->ascent);
	    needASCENT = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DESCENT)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DESCENT, sizeof(bdfprop_DESCENT), self->descent);
	    needDESCENT = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTCHAR)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTCHAR, sizeof(bdfprop_DEFAULTCHAR), self->defaultchar);
	    needDEFAULTCHAR = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTWIDTH)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTWIDTH, sizeof(bdfprop_DEFAULTWIDTH), self->defaultw);
	    needDEFAULTW = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTHEIGHT)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTHEIGHT, sizeof(bdfprop_DEFAULTHEIGHT), self->defaulth);
	    needDEFAULTH = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTX)) {
	    pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTX, sizeof(bdfprop_DEFAULTX), self->defaultx);
	    needDEFAULTX = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTY)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTY, sizeof(bdfprop_DEFAULTY), self->defaulty);
	    needDEFAULTY = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTDX)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTDX, sizeof(bdfprop_DEFAULTDX), self->defaultdx);
	    needDEFAULTDX = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_DEFAULTDY)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_DEFAULTDY, sizeof(bdfprop_DEFAULTDY), self->defaultdy);
	    needDEFAULTDY = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_FAMILY)) {
	    /* should never happen, input removed this from properties! */
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_WEIGHTNAME)) {
	    /* should never happen, input removed this from properties! */
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_WEIGHT)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_WEIGHT, sizeof(bdfprop_WEIGHT), self->fontweight);
	    needWEIGHT = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_RESX)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_RESX, sizeof(bdfprop_RESX), self->resx);
	    needRESX = FALSE;
	}
	else if (bdffont_IsPrefix(inattr, bdfprop_RESY)) {
	   pos=bdffont_AppendProperty(props, pos, bdfprop_RESY, sizeof(bdfprop_RESY), self->resy);
	    needRESY = FALSE;
	} else pos=outattr-(*props);
	outattr=(*props)+pos;
	inattr = next;
    }
    *outattr = '\0';

    length =  outattr-*props;

    if (needFOUNDRY /* && (self->version == bdffont_FONT_VERSION) */) {
	count++;
	*props = (char *)
		   realloc(*props,
			   length
			      + sizeof(bdfprop_FOUNDRY) /* counts space */
			      + sizeof(bdffont_FOUNDRY) /* counts line-feed */
			      + 3);		        /* quotes and NUL */
	sprintf(*props + length,
		"%s \"%s\"\n",
		bdfprop_FOUNDRY,
		bdffont_FOUNDRY);
	length += sizeof(bdfprop_FOUNDRY) + sizeof(bdffont_FOUNDRY) + 2;
    }

    if (needASCENT) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_ASCENT,
					sizeof(bdfprop_ASCENT),
					self->ascent);
    }

    if (needDESCENT) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DESCENT,
					sizeof(bdfprop_DESCENT),
					self->descent);
    }

    if (needDEFAULTCHAR) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTCHAR,
					sizeof(bdfprop_DEFAULTCHAR),
					(long) (self)->GetDefaultChar());
    }

    if (needDEFAULTW) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTWIDTH,
					sizeof(bdfprop_DEFAULTWIDTH),
					self->defaultw);
    }

    if (needDEFAULTH) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTHEIGHT,
					sizeof(bdfprop_DEFAULTHEIGHT),
					self->defaulth);
    }

    if (needDEFAULTX) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTX,
					sizeof(bdfprop_DEFAULTX),
					self->defaultx);
    }

    if (needDEFAULTY) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTY,
					sizeof(bdfprop_DEFAULTY),
					self->defaulty);
    }

    if (needDEFAULTDX) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTDX,
					sizeof(bdfprop_DEFAULTDX),
					self->defaultdx);
    }

    if (needDEFAULTDY) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_DEFAULTDY,
					sizeof(bdfprop_DEFAULTDY),
					self->defaultdy);
    }

    if (needFAMILY && self->fontfamily) {
	count++;
	*props = (char *)
		    realloc(*props,
			    length
				+ sizeof(bdfprop_FAMILY)    /* counts space */
				+ 2 * strlen(self->fontfamily) /* expansion */
				+ 4);		/* space, 2 quotes, and NUL */
	sprintf(*props + length,
		"%s \"",
		bdfprop_FAMILY);
	length += sizeof(bdfprop_FAMILY) + 1;	/* point to NUL */

	inattr = self->fontfamily;	/* as a temporary */
	while (*inattr) {
	    if (*inattr == '"') {
		(*props)[length++] = '"';
	    }
	    (*props)[length++] = *inattr++;
	}
	(*props)[length++] = '"';
	(*props)[length++] = '\n';
	(*props)[length] = '\0';
    }

    if (needWEIGHTNAME) {
	const char *facename;

	count++;
	facename = (self->fontface < sizeof(FaceNames)
			? FaceNames[self->fontface]
			: FaceNames[0]);
	*props = (char *)
		    realloc(*props,
		 	    length
				+ sizeof(bdfprop_WEIGHTNAME) /* counts space */
				+ strlen(facename)	    /* just the name */
				+ 4);		/* 2 quotes, line-feed & NUL */
	sprintf(*props + length,
		"%s \"%s\"\n",
		bdfprop_WEIGHTNAME,
		facename);
	length += sizeof(bdfprop_WEIGHTNAME) + strlen(facename) + 3;
    }

    if (needWEIGHT && (0 <= self->fontweight)) {
	count++;
	*props = (char *) realloc(*props,
		 	 length
			    + sizeof(bdfprop_WEIGHT)	/* counts space */
			    + bdfprop_INT_LEN		/* just the number */
			    + 2);			/* line-feed and NUL */
	sprintf(*props + length,
		"%s %*ld\n",
		bdfprop_WEIGHT,
		bdfprop_INT_LEN,
		self->fontweight);
	length += sizeof(bdfprop_WEIGHT) + bdfprop_INT_LEN + 1;
    }

    if (needRESX) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_RESX,
					sizeof(bdfprop_RESX),
					self->resx);
    }

    if (needRESY) {
	count++;
	length = bdffont_AppendProperty(props,
					length,
					bdfprop_RESY,
					sizeof(bdfprop_RESY),
					self->resy);
    }

    return (count);
} /* FilterPropertiesOut */

long bdffont::Write(FILE  *file, long  id, int  level)
{
    long i;
    char *props;

    (this)->NotifyObservers( bdffont_Writing);

    fprintf(file, "STARTFONT %s\n", this->version);

    if (this->comments) {
	fprintf(file, "%s", this->comments);
    }

    fprintf(file, "FONT %s\n",
	     this->fontname ? this->fontname : bdffont_FONT_NAME);

    fprintf(file, "SIZE %ld %ld %ld\n", this->pointsize, this->resx, this->resy);

    fprintf(file, "FONTBOUNDINGBOX %ld %ld %ld %ld\n",
	     this->bbw, this->bbh, this->bbx, this->bby);

    fprintf(file, "STARTPROPERTIES %ld\n", FilterPropertiesOut(this, &props));
    fprintf(file, "%s", props);
    free(props);
    fprintf(file, "ENDPROPERTIES\n");

    if (this->activedefns > 0) {
	fprintf(file, "CHARS %ld\n", this->activedefns);

	for (i = 0; i < this->defns_size; i++) {
	    WriteCharacter(file, &this->defns[i]);
	}
    }

    fprintf(file, "ENDFONT\n");

    return ((this)->GetID());
} /* bdffont__Write */

void bdffont::SetFontName(const char  *fn)
{
    if (this->fontname) {
	free(this->fontname);
	this->fontname = NULL;
    }

    if (fn) {
	this->fontname = (char *) malloc(strlen(fn) + 1);
	strcpy(this->fontname, fn);
    }
} /* bdffont__SetFontName */

void bdffont::SetFontFamily(const char  *fn)
{
    if (this->fontfamily) {
	free(this->fontfamily);
	this->fontfamily = NULL;
    }

    if (fn) {
	this->fontfamily = (char *) malloc(strlen(fn) + 1);
	strcpy(this->fontfamily, fn);
    }
} /* bdffont__SetFontFamily */

void bdffont::GetBoundingBox(long  *w , long  *h , long  *x , long  *y)
{
    *w = this->bbw;
    *h = this->bbh;
    *x = this->bbx;
    *y = this->bby;
} /* bdffont__GetBoundingBox */

void bdffont::SetBoundingBox(long  w , long  h , long  x , long  y)
{
    this->bbw = w;
    this->bbh = h;
    this->bbx = x;
    this->bby = y;
} /* bdffont__SetBoundingBox */

void bdffont::GetResolution(long  *rx , long  *ry)
{
    *rx = this->resx;
    *ry = this->resy;
} /* bdffont__GetResolution */

void bdffont::SetResolution(long  rx , long  ry)
{
    this->resx = rx;
    this->resy = ry;
} /* bdffont__SetResolution */

void bdffont::GetDefaultExtent(long  *w , long  *h)
{
    *w = this->defaultw;
    *h = this->defaulth;
} /* bdffont__GetDefaultExtent */

void bdffont::SetDefaultExtent(long  w , long  h)
{
    this->defaultw = w;
    this->defaulth = h;
} /* bdffont__SetDefaultExtent */

void bdffont::GetDefaultOrigin(long  *x , long  *y)
{
    *x = this->defaultx;
    *y = this->defaulty;
} /* bdffont__GetDefaultOrigin */

void bdffont::SetDefaultOrigin(long  x , long  y)
{
    this->defaultx = x;
    this->defaulty = y;
} /* bdffont__SetDefaultOrigin */

void bdffont::GetDefaultDelta(long  *dx , long  *dy)
{
    *dx = this->defaultdx;
    *dy = this->defaultdy;
} /* bdffont__GetDefaultDelta */

void bdffont::SetDefaultDelta(long  dx , long  dy)
{
    this->defaultdx = dx;
    this->defaultdy = dy;
} /* bdffont__SetDefaultDelta */

void bdffont::SetCharDWidth(int  which, long  x , long  y)
{
    struct bdffont_fontchar *defn;

    defn = (this)->GetDefinition( which);

    bdffont_SetDWidth(defn, x, y);
    bdffont_SetSWidth(defn,
		      (long) ((x * 72000.0) / (this->resx * this->pointsize)),
		      (long) ((y * 72000.0) / (this->resy * this->pointsize)));
} /* bdffont__SetCharDWidth */

static void RecomputeFontExtent(class bdffont  *self)
{
    struct bdffont_fontchar *defn, *limit;
    long w, h;

    defn = &self->defns[0];
    limit = &self->defns[self->defns_size];

    self->bbw = mathaux_MININT;
    self->bbh = mathaux_MININT;
    while (defn < limit) {
	if (bdffont_IsActive(defn)) {
	    bdffont_GetExtent(defn, &w, &h);

	    if (self->bbw < w) {
		self->bbw = w;
	    }

	    if (self->bbh < h) {
		self->bbh = h;
	    }
	}

	defn++;
    }
} /* RecomputeFontExtent */

static void RecomputeFontOrigin(class bdffont  *self)
{
    struct bdffont_fontchar *defn, *limit;
    long x, y;

    defn = &self->defns[0];
    limit = &self->defns[self->defns_size];

    self->bbx = mathaux_MAXINT;
    self->bby = mathaux_MAXINT;
    while (defn < limit) {
	if (bdffont_IsActive(defn)) {
	    bdffont_GetOrigin(defn, &x, &y);

	    if (x < self->bbx) {
		self->bbx = x;
	    }

	    if (y < self->bby) {
		self->bby = y;
	    }
	}

	defn++;
    }
} /* RecomputeFontOrigin */

void bdffont::SetCharExtent(int  which, long  w , long  h)
{
    struct bdffont_fontchar *defn;
    long oldw, oldh;
    boolean recompute = FALSE;

    defn = (this)->GetDefinition( which);

    bdffont_GetExtent(defn, &oldw, &oldh);

    if (this->bbw <= w) {
	this->bbw = w;
    }
    else if (oldw == this->bbw) {
	recompute = TRUE;
    }

    if (this->bbh <= h) {
	this->bbh = h;
    }
    else if (oldh == this->bbh) {
	recompute = TRUE;
    }

    if (recompute) {
	RecomputeFontExtent(this);
    }

    bdffont_SetExtent(defn, w, h);
} /* bdffont__SetCharExtent */

void bdffont::SetCharOrigin(int  which, long  x , long  y)
{
    struct bdffont_fontchar *defn;
    long oldx, oldy;
    boolean recompute = FALSE;

    defn = (this)->GetDefinition( which);

    bdffont_GetOrigin(defn, &oldx, &oldy);

    if (x <= this->bbx) {
	this->bbx = x;
    }
    else if (oldx == this->bbx) {
	recompute = TRUE;
    }

    if (y <= this->bby) {
	this->bby = y;
    }
    else if (oldy == this->bby) {
	recompute = TRUE;
    }

    if (recompute) {
	RecomputeFontOrigin(this);
    }

    bdffont_SetOrigin(defn, x, y);
} /* bdffont__SetCharOrigin */
