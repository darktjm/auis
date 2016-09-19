/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

#define AUXMODULE 1
#include <print.H>

#include <view.H>
#include <fontdesc.H>
#include <proctable.H>
#include <message.H>
#include <buffer.H>
#include <environ.H>

/* #define DEBUG 1 */

#ifdef DEBUG
#define PRINTD(p)  printf p
#else
#define PRINTD(p)
#endif

#define PSHASHSIZE 33

static afm_font_hashtable *AFMCache;
static struct font_afm *symbolfont = NULL;
static class atom *A_landscape, *A_scale, *A_papersize;

struct afm_keyword {
    const char *name;
    int val;
};

struct afm_font_symbolchar {
    int localcode, symbolcode;
};

struct cleanup {
    print_cleanup_fptr cleanproc;
    void *rock;
    struct cleanup *next;
};

struct genheader {
    print_header_fptr headproc;
    const void *rock;
};

void printp_init();


static int lookup_afmkey(char *keywd, const struct afm_keyword *wordlist,int defval);
static void suck_keywd(char **ppt, char *keywd);
static void suck_string_keywd(char **ppt, char *keywd);
static int suck_int_keywd(char **ppt);
static double suck_num_keywd(char **ppt);
static boolean suck_bool_keywd(char **ppt);
static void suck_char_metrics(char **ppt, struct font_afm *res, char *keywd);
static void suck_composites(char **ppt, struct font_afm *res, char *keywd);
static boolean ReadAFMFile(FILE *infl,const char *nameexpected,struct font_afm *res);
static int printp_GetPSHashKey(const char *name);

static afm_font_hashtable *printp_CreatePSHashTable();
static void printp_DestroyPSHashTable(afm_font_hashtable *tab, 
		boolean freedata);
static void *printp_LookUpPSHashTable(afm_font_hashtable *tab, const char *name);
static boolean printp_InsertInPSHashTable(afm_font_hashtable *tab, const char *name, 
		void *dat, int *entryindex);
static boolean printp_IsEmptyPSHashTable(afm_font_hashtable *tab);
static void dumpregfont_splot(char *nam, void *dat, void *rock);
static void dumpregdef_splot(char *nam, void *dat, void *rock);
static void dumpreghead_splot(char *nam, void *dat, void *rock);

extern void print_EnsureClassInitialized();

/* Maps the 256 chars of the ISO8859 fonts to AGP encoding.
 Things not resolved:
	notchedbar (currently prints as |)
	Dslash (currently prints as D), 
	Yacute (currently prints as Y), yacute (currently prints as y), 
	2super, 3super, 1super, 1/4, 1/2, 3/4, thorn, Thorn, 
		slashedpartdiff (all currently print as zero-width blanks)
 */
static short ISO8859_map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, -1,
    /* end of nice simple characters */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 161, 162, 163, 168, 165, 124, 167, 200, 312, 227, 171, 313, 177, 315, 197,
    202, 317,  -1,  -1, 194, 320, 182, 180, 203,  -1, 235, 187,  -1,  -1,  -1, 191,
    259, 256, 257, 261, 258, 260, 225, 262, 266, 263, 264, 265, 270, 267, 268, 269,
    68,  271, 275, 272, 273, 276, 274, 318, 233, 281, 278, 279, 280,  89,  -1, 251,
    287, 284, 285, 289, 286, 288, 241, 290, 294, 291, 292, 293, 298, 295, 296, 297,
    -1, 299, 303, 300, 301, 304, 302, 319, 249, 309, 306, 307, 308, 121, -1, 310
};

/* Maps the 256 chars of the Adobe Symbol fonts to AGP encoding. Since AGP is identical to Adobe encoding for values less than 256, this is mostly the identity mapping, but some values are not valid.
 Things not resolved:
     240 is an Apple logo. The AFM file admits this, but the image is missing in the printer font (for trademark reasons, one assumes.) We handle this by sending the character anyway; it will be a apple-width blank space on the page, and if they ever change their minds about the printer font, it will start working. */
static short AdobeSymbol_map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, -1
};

/* Maps the 256 chars of the SymbolA font to AGP encoding. Only some values are valid, and all of them are equivalent to AdobeSymbol minus 128.
 Things not resolved:
     32 displays (uglily) as a capital upsilon which has a zero width. I think probably people shouldn't use it. We print that as a zero-width blank, thus using the printing system as a moral weapon.
     112 is an Apple logo (maps to Symbol 240). See note above. */
static short SymbolAndy_map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

#define afmkey_unknown		0
#define afmkey_StartFontMetrics	1
#define afmkey_EndFontMetrics	2
#define afmkey_FontName		3
#define afmkey_IsFixedPitch	4
#define afmkey_UnderlinePosition 5
#define afmkey_UnderlineThickness 6
#define afmkey_CapHeight	7
#define afmkey_XHeight		8
#define afmkey_Descender	9
#define afmkey_Ascender		10
#define afmkey_StartCharMetrics 11
#define afmkey_EndCharMetrics	12
#define afmkey_StartKernData	13
#define afmkey_StartKernPairs	14
#define afmkey_EndKernData	15
#define afmkey_EndKernPairs	16
#define afmkey_StartComposites	17
#define afmkey_EndComposites	18
#define afmkey_CharWidth	19
#define afmkey_StartDirection	20
#define afmkey_ItalicAngle	21
#define afmkey_Characters	22
#define afmkey_FontBBox		23

static const struct afm_keyword afm_keywords[] = {
    {"Ascender", afmkey_Ascender},
    {"CapHeight", afmkey_CapHeight},
    {"Characters", afmkey_Characters},
    {"CharWidth", afmkey_CharWidth},
    {"Descender", afmkey_Descender},
    {"EndCharMetrics", afmkey_EndCharMetrics},
    {"EndComposites", afmkey_EndComposites},
    {"EndFontMetrics", afmkey_EndFontMetrics},
    {"EndKernData", afmkey_EndKernData},
    {"EndKernPairs", afmkey_EndKernPairs},
    {"FontBBox", afmkey_FontBBox},
    {"FontName", afmkey_FontName},
    {"IsFixedPitch", afmkey_IsFixedPitch},
    {"ItalicAngle", afmkey_ItalicAngle},
    {"StartCharMetrics", afmkey_StartCharMetrics},
    {"StartComposites", afmkey_StartComposites},
    {"StartDirection", afmkey_StartDirection},
    {"StartFontMetrics", afmkey_StartFontMetrics},
    {"StartKernData", afmkey_StartKernData},
    {"StartKernPairs", afmkey_StartKernPairs},
    {"UnderlinePosition", afmkey_UnderlinePosition},
    {"UnderlineThickness", afmkey_UnderlineThickness},
    {"XHeight", afmkey_XHeight},
    {NULL, 0}
};

/*
    {"", afmkey_},
*/

/* This table maps character names to AGP encoded values.
 For values below 256, the character is direct; the AGP value can be dumped directly to a PS file.
 For values 256 and up, the flag print_SymbolChar indicates that the glyph must be pulled from the Symbol font; otherwise, the character is a composite. */
static const struct afm_keyword encoded_names[] = {
    {"space",		32},
    {"exclam",		33},
    {"quotedbl",	34},
    {"numbersign",	35},
    {"dollar",		36},
    {"percent",		37},
    {"ampersand",	38},
    {"quoteright",	39},
    {"parenleft",	40},
    {"parenright",	41},
    {"asterisk",	42},
    {"plus",		43},
    {"comma",		44},
    {"hyphen",		45},
    {"period",		46},
    {"slash",		47},
    {"zero",		48},
    {"one",		49},
    {"two",		50},
    {"three",		51},
    {"four",		52},
    {"five",		53},
    {"six",		54},
    {"seven",		55},
    {"eight",		56},
    {"nine",		57},
    {"colon",		58},
    {"semicolon",	59},
    {"less",		60},
    {"equal",		61},
    {"greater",		62},
    {"question",	63},
    {"at",		64},
    {"A",		65},
    {"B",		66},
    {"C",		67},
    {"D",		68},
    {"E",		69},
    {"F",		70},
    {"G",		71},
    {"H",		72},
    {"I",		73},
    {"J",		74},
    {"K",		75},
    {"L",		76},
    {"M",		77},
    {"N",		78},
    {"O",		79},
    {"P",		80},
    {"Q",		81},
    {"R",		82},
    {"S",		83},
    {"T",		84},
    {"U",		85},
    {"V",		86},
    {"W",		87},
    {"X",		88},
    {"Y",		89},
    {"Z",		90},
    {"bracketleft",	91},
    {"backslash",	92},
    {"bracketright",	93},
    {"asciicircum",	94},
    {"underscore",	95},
    {"quoteleft",	96},
    {"a",		97},
    {"b",		98},
    {"c",		99},
    {"d",		100},
    {"e",		101},
    {"f",		102},
    {"g",		103},
    {"h",		104},
    {"i",		105},
    {"j",		106},
    {"k",		107},
    {"l",		108},
    {"m",		109},
    {"n",		110},
    {"o",		111},
    {"p",		112},
    {"q",		113},
    {"r",		114},
    {"s",		115},
    {"t",		116},
    {"u",		117},
    {"v",		118},
    {"w",		119},
    {"x",		120},
    {"y",		121},
    {"z",		122},
    {"braceleft",	123},
    {"bar",		124},
    {"braceright",	125},
    {"asciitilde",	126},
    {"exclamdown",	161},
    {"cent",		162},
    {"sterling",	163},
    {"fraction",	164},
    {"yen",		165},
    {"florin",		166},
    {"section",		167},
    {"currency",	168},
    {"quotesingle",	169},
    {"quotedblleft",	170},
    {"guillemotleft",	171},
    {"guilsinglleft",	172},
    {"guilsinglright",	173},
    {"fi",		174},
    {"fl",		175},
    {"endash",		177},
    {"dagger",		178},
    {"daggerdbl",	179},
    {"periodcentered",	180},
    {"paragraph",	182},
    {"bullet",		183},
    {"quotesinglbase",	184},
    {"quotedblbase",	185},
    {"quotedblright",	186},
    {"guillemotright",	187},
    {"ellipsis",	188},
    {"perthousand",	189},
    {"questiondown",	191},
    {"grave",		193},
    {"acute",		194},
    {"circumflex",	195},
    {"tilde",		196},
    {"macron",		197},
    {"breve",		198},
    {"dotaccent",	199},
    {"dieresis",	200},
    {"ring",		202},
    {"cedilla",		203},
    {"hungarumlaut",	205},
    {"ogonek",		206},
    {"caron",		207},
    {"emdash",		208},
    {"AE",		225},
    {"ordfeminine",	227},
    {"Lslash",		232},
    {"Oslash",		233},
    {"OE",		234},
    {"ordmasculine",	235},
    {"ae",		241},
    {"dotlessi",	245},
    {"lslash",		248},
    {"oslash",		249},
    {"oe",		250},
    {"germandbls",	251},
    {"Aacute",		256},
    {"Acircumflex", 	257},
    {"Adieresis", 	258},
    {"Agrave",		259},
    {"Aring",		260},
    {"Atilde",		261},
    {"Ccedilla", 	262},
    {"Eacute",		263},
    {"Ecircumflex", 	264},
    {"Edieresis", 	265},
    {"Egrave",		266},
    {"Iacute",		267},
    {"Icircumflex", 	268},
    {"Idieresis", 	269},
    {"Igrave",		270},
    {"Ntilde",		271},
    {"Oacute",		272},
    {"Ocircumflex", 	273},
    {"Odieresis", 	274},
    {"Ograve",		275},
    {"Otilde",		276},
    {"Scaron",		277},
    {"Uacute",		278},
    {"Ucircumflex", 	279},
    {"Udieresis", 	280},
    {"Ugrave",		281},
    {"Ydieresis", 	282},
    {"Zcaron",		283},
    {"aacute",		284},
    {"acircumflex", 	285},
    {"adieresis", 	286},
    {"agrave",		287},
    {"aring",		288},
    {"atilde",		289},
    {"ccedilla", 	290},
    {"eacute",		291},
    {"ecircumflex", 	292},
    {"edieresis", 	293},
    {"egrave",		294},
    {"iacute",		295},
    {"icircumflex", 	296},
    {"idieresis", 	297},
    {"igrave",		298},
    {"ntilde",		299},
    {"oacute",		300},
    {"ocircumflex", 	301},
    {"odieresis", 	302},
    {"ograve",		303},
    {"otilde",		304},
    {"scaron",		305},
    {"uacute",		306},
    {"ucircumflex", 	307},
    {"udieresis", 	308},
    {"ugrave",		309},
    {"ydieresis", 	310},
    {"zcaron",		311},
    {"copyright", 	312 | print_SymbolChar},
    {"logicalnot", 	313 | print_SymbolChar},
    {"minus",		314 | print_SymbolChar},
    {"registered", 	315 | print_SymbolChar},
    {"trademark", 	316 | print_SymbolChar},
    {"plusminus", 	317 | print_SymbolChar},
    {"multiply", 	318 | print_SymbolChar},
    {"divide",		319 | print_SymbolChar},
    {"mu",		320 | print_SymbolChar},
    {NULL,		0}
};

static const struct afm_font_symbolchar symbolcharlist[] = {
    {312, 211},
    {313, 216},
    {314, 45},
    {315, 210},
    {316, 212},
    {317, 177},
    {318, 180},
    {319, 184},
    {320, 109},
    {0, 0}
};


struct pagesize {
    const char *name; /* in lower case */
    long width, height; /* in PS units */
};
static const struct pagesize pagesizelist[] = {
    {"letter", 612, 792},
    {"a4", 595, 842},
    {"legal", 612, 1008},
    {"a3", 842, 1190},
    {"a5", 420, 595},
    {"b4", 729, 1032},
    {"b5", 516, 729},
    {"folio", 612, 936},
    {"statement", 396, 612},
    {"executive", 540, 720},
    {"tabloid", 792, 1224},
    {"quarto", 610, 780},
    {"ledger", 1224, 792},
    {NULL, 612, 792} /* default if type is not known */
};



static int lookup_afmkey(char *keywd, const struct afm_keyword *wordlist, 
		int defval) {
    const struct afm_keyword *pt;

    for (pt = wordlist; pt->name; pt++) {
	if (!strcmp(keywd, pt->name))
	    return pt->val;
    }

    return defval;
}

/* eat a keyword into keywd; ppt is moved to the next bit. If there is no keyword, ppt becomes NULL and keywd is undefined */
static void suck_keywd(char  **ppt, char  *keywd) {
    char *pt;

    if (!ppt) return;

    pt = (*ppt);
    while (*pt==' ' || *pt=='\t' || *pt=='\n')
	pt++;

    if (*pt == '\0') {
	*ppt = NULL;
	return;
    }

    while (!(*pt==' ' || *pt=='\t' || *pt=='\n' || *pt=='\0')) {
	*keywd = (*pt);
	keywd++;
	pt++;
    }

    *keywd = '\0';
    *ppt = pt;
}

/* suck from first non-whitespace to just before the newline */
static void suck_string_keywd(char  **ppt, char  *keywd) {
    char *pt;

    if (!ppt) return;

    pt = (*ppt);
    while (*pt==' ' || *pt=='\t' || *pt=='\n')
	pt++;

    if (*pt == '\0') {
	*ppt = NULL;
	return;
    }

    while (!(*pt=='\n' || *pt=='\0')) {
	*keywd = (*pt);
	keywd++;
	pt++;
    }

    *keywd = '\0';
    *ppt = pt;
}


static int suck_int_keywd(char  **ppt) {
    char keywd[260];
    suck_keywd(ppt, keywd);

    if (*ppt)
	return (atol(keywd));
    else
	return 0;
}

static double suck_num_keywd(char  **ppt) {
    char keywd[260];
    suck_keywd(ppt, keywd);

    if (*ppt)
	return (atof(keywd));
    else
	return 0.0;
}

static boolean suck_bool_keywd(char  **ppt) {
    char keywd[260];
    suck_keywd(ppt, keywd);

    if (*ppt) {
	if (!strcmp(keywd, "true"))
	    return TRUE;
	else
	    return FALSE;
    }
    else
	return FALSE;
}

static void suck_char_metrics(char **ppt, struct font_afm *res, char *keywd) {
    char name[260];
    int aseval = (-1);
    double xval = 0.0, yval = 0.0;
    int ix, ch;

    name[0] = '\0';

    while (*ppt) {
	if (!strcmp(keywd, "C")) {
	    aseval = suck_int_keywd(ppt);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else if (!strcmp(keywd, "CH")) {
	    suck_keywd(ppt, keywd);
	    aseval = 0;
	    for (ix=0; keywd[ix]; ix++) {
		ch = keywd[ix];
		if (ch>='0' && ch<='9')
		    aseval = 16*aseval + (ch-'0');
		else if (ch>='A' && ch<='F')
		    aseval = 16*aseval + (ch-'A'+10);
		else if (ch>='a' && ch<='f')
		    aseval = 16*aseval + (ch-'a'+10);
	    }
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else if (!strcmp(keywd, "N")) {
	    suck_keywd(ppt, name);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else if (!strcmp(keywd, "WX") || !strcmp(keywd, "W0X")) {
	    xval = suck_num_keywd(ppt);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else if (!strcmp(keywd, "WY") || !strcmp(keywd, "W0Y")) {
	    yval = suck_num_keywd(ppt);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else if (!strcmp(keywd, "W") || !strcmp(keywd, "W0")) {
	    xval = suck_num_keywd(ppt);
	    yval = suck_num_keywd(ppt);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	}
	else {
	    while (*ppt && strcmp(keywd, ";"))
		suck_keywd(ppt, keywd);
	}

	if (strcmp(keywd, ";")) {
	    PRINTD(("afm: bad line in char metrics\n"));
	    PRINTD(("|%s|\n", keywd));
	    return;
	}
	if (*ppt) suck_keywd(ppt, keywd); /* pull up the next token */
    }

    if (aseval == (-1)) {
	aseval = lookup_afmkey(name, encoded_names, -1);
	if (aseval==(-1)) {
	    PRINTD(("afm: found unrecognizable char name |%s|\n", name));
	    return;
	}
	if (aseval & print_SymbolChar) {
	    /* do nothing. This will be filled in later. */
	}
	else {
	    res->ch[aseval].x = xval;
	    res->ch[aseval].y = yval;
	    res->ch[aseval].numparts = 0; /* so far */
	}
    }
    else {
	res->ch[aseval].x = xval;
	res->ch[aseval].y = yval;
	res->ch[aseval].numparts = 0;
    }
}

static void suck_composites(char **ppt, struct font_afm *res, char *keywd) {
    char name[260];
    int nparts, partnum;
    int aseval, paseval;
    double xval, yval;
    struct font_afm_comppart *clist;

    name[0] = '\0';

    if (strcmp(keywd, "CC")) {
	PRINTD(("afm: composites line started with |%s|\n", keywd));
	return;
    }
    if (*ppt) suck_keywd(ppt, name);
    if (*ppt) nparts = suck_int_keywd(ppt);
    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */

    if (!(*ppt)) {
	PRINTD(("afm: composites line too short\n"));
	return;
    }

    aseval = lookup_afmkey(name, encoded_names, -1);
    if (aseval==(-1)) {
	PRINTD(("afm: found unrecognizable char name |%s|\n", name));
	return;
    }

    if (nparts<=1) {
	PRINTD(("afm: only %d parts listed\n", nparts));
	return;
    }
    clist = (struct font_afm_comppart *)malloc(sizeof(struct font_afm_comppart) * nparts);
    partnum = 0;

    while (*ppt) {
	if (!strcmp(keywd, "PCC")) {
	    suck_keywd(ppt, name);
	    if (*ppt) xval = suck_num_keywd(ppt);
	    if (*ppt) yval = suck_num_keywd(ppt);
	    if (*ppt) suck_keywd(ppt, keywd); /* eat the semicolon */
	    if (!(*ppt)) {
		PRINTD(("afm: composites line middle too short\n"));
		return;
	    }
	    paseval = lookup_afmkey(name, encoded_names, -1);
	    if (paseval==(-1)) {
		PRINTD(("afm: found unrecognizable char name |%s|\n", name));
		return;
	    }
	    if (partnum >= nparts) {
		PRINTD(("afm: too many parts composites line\n"));
		return;
	    }
	    clist[partnum].num = paseval;
	    clist[partnum].xoff = xval;
	    clist[partnum].yoff = yval;
	    partnum++;
	}
	else {
	    while (*ppt && strcmp(keywd, ";"))
		suck_keywd(ppt, keywd);
	}

	if (strcmp(keywd, ";")) {
	    PRINTD(("afm: bad line in char metrics\n"));
	    PRINTD(("|%s|\n", keywd));
	    return;
	}
	if (*ppt) suck_keywd(ppt, keywd); /* pull up the next token */
    }

    if (partnum < nparts) {
	PRINTD(("afm: too few parts composites line\n"));
	return;
    }
    res->ch[aseval].numparts = nparts;
    res->ch[aseval].u.parts = clist;
}


/* The following function reads an AFM file. 
	It is very generous (which is to say, trusting) 
	about the file's syntax. It only pays attention to 
	0-direction metric information (horizontal) 
	`res' is an allocated but uninitialized font_afm 
*/
static boolean ReadAFMFile(FILE *infl, const char *nameexpected,
		struct font_afm *res) {
    char buf[260]; /* AFM file lines are < 256 chars */
    char keywd[260];
    char *pt;
    char *ok;
    int startdirection;
    int ix, max;

    startdirection = 0;

    res->fontname = NULL;
    res->characters = 0;
    res->fontbbox_l = 0.0;
    res->fontbbox_r = 0.0;
    res->fontbbox_t = 0.0;
    res->fontbbox_b = 0.0;
    res->capheight = 0.0;
    res->xheight = 0.0;
    res->ascender = 0.0;
    res->descender = 0.0;
    res->h.underlineposition = 0.0;
    res->h.underlinethickness = 0.0;
    res->h.italicangle = 0.0;
    res->h.charwidthx = 0.0;
    res->h.charwidthy = 0.0;
    res->h.isfixedpitch = FALSE;
    for (ix=0; ix<font_afm_AGPsize; ix++) {
	res->ch[ix].x = 0.0;
	res->ch[ix].y = 0.0;
	res->ch[ix].numparts = 0;
    }

    ok = fgets(buf, sizeof(buf), infl);
    if (!ok) {
	PRINTD(("afm: unable to read first line\n"));
	return FALSE;
    }
    pt = buf;
    suck_keywd(&pt, keywd);
    if (!pt
	|| lookup_afmkey(keywd, afm_keywords, afmkey_unknown)!=afmkey_StartFontMetrics) {
	PRINTD(("afm: first keywd was not StartFontMetrics\n"));
	return FALSE;
    }
    /* check AFM version? */

    while (1) {
	ok = fgets(buf, sizeof(buf), infl);
	if (!ok) {
	    PRINTD(("afm: unexpected end-of-file\n"));
	    return FALSE;
	}
	pt = buf;
	suck_keywd(&pt, keywd);
	if (!pt)
	    continue; /* empty line */

	switch (lookup_afmkey(keywd, afm_keywords, afmkey_unknown)) {
	    case afmkey_EndFontMetrics:
		return TRUE;
	    case afmkey_StartFontMetrics:
		PRINTD(("afm: found StartFontMetrics in odd place\n"));
		return FALSE;

	    case afmkey_StartDirection:
		startdirection = suck_int_keywd(&pt);
		break;
	    case afmkey_UnderlinePosition:
		if (startdirection==0)
		    res->h.underlineposition = suck_num_keywd(&pt);
		break;
	    case afmkey_UnderlineThickness:
		if (startdirection==0)
		    res->h.underlinethickness = suck_num_keywd(&pt);
		break;
	    case afmkey_ItalicAngle:
		if (startdirection==0)
		    res->h.italicangle = suck_num_keywd(&pt);
		break;
	    case afmkey_IsFixedPitch:
		if (startdirection==0)
		    res->h.isfixedpitch = suck_bool_keywd(&pt);
		break;
	    case afmkey_CharWidth:
		if (startdirection==0) {
		    res->h.charwidthx = suck_num_keywd(&pt);
		    res->h.charwidthy = suck_num_keywd(&pt);
		}
		break;

	    case afmkey_FontName:
		suck_string_keywd(&pt, keywd);
		if (nameexpected && strcmp(keywd, nameexpected)) {
		    PRINTD(("afm: FontName was not what was expected\n"));
		    return FALSE;
		}
		res->fontname = strdup(keywd);
		break;
	    case afmkey_FontBBox:
		res->fontbbox_l = suck_num_keywd(&pt);
		res->fontbbox_b = suck_num_keywd(&pt);
		res->fontbbox_r = suck_num_keywd(&pt);
		res->fontbbox_t = suck_num_keywd(&pt);
		break;
	    case afmkey_Characters:
		res->characters = suck_int_keywd(&pt);
		break;
	    case afmkey_CapHeight:
		res->capheight = suck_num_keywd(&pt);
		break;
	    case afmkey_XHeight:
		res->xheight = suck_num_keywd(&pt);
		break;
	    case afmkey_Ascender:
		res->ascender = suck_num_keywd(&pt);
		break;
	    case afmkey_Descender:
		res->descender = suck_num_keywd(&pt);
		break;

	    case afmkey_StartCharMetrics:
		max = suck_int_keywd(&pt);
		ix = 0;
		while (1) {
		    ok = fgets(buf, sizeof(buf), infl);
		    if (!ok) {
			PRINTD(("afm: unexpected end-of-file in StartCharMetrics\n"));
			return FALSE;
		    }
		    pt = buf;
		    suck_keywd(&pt, keywd);
		    if (!pt)
			continue; /* empty line */
		    if (!strcmp(keywd, "EndCharMetrics"))
			break;
		    suck_char_metrics(&pt, res, keywd);
		    ix++;
		}
		if (ix!=max) {
		    PRINTD(("afm: got %d lines instead of %d\n", ix, max));
		}
		/* now we add the Symbol-font substitutions. This can be done in any font, because only screen fonts that really want symbol substitutions will generate these AGP values. */
		for (ix=0; encoded_names[ix].name; ix++) {
		    const struct afm_font_symbolchar *tmp;
		    int aseval;
		    aseval = (encoded_names[ix].val);
		    if (aseval & print_SymbolChar) {
			aseval &= (~print_SymbolChar);
			for (tmp = symbolcharlist; tmp->localcode; tmp++) {
			    if (tmp->localcode==aseval)
				break;
			}
			if (!tmp->localcode) {
			    PRINTD(("afm: unable to find %d in symbolcharlist\n", aseval));
			    return FALSE;
			}
			res->ch[aseval].x = 0.0;
			res->ch[aseval].y = 0.0;
			res->ch[aseval].numparts = print_SymbolChar;
			res->ch[aseval].u.symbolchar = tmp->symbolcode;
		    }
		}
		break;

	    case afmkey_StartComposites:
		max = suck_int_keywd(&pt);
		ix = 0;
		while (1) {
		    ok = fgets(buf, sizeof(buf), infl);
		    if (!ok) {
			PRINTD(("afm: unexpected end-of-file in StartComposites\n"));
			return FALSE;
		    }
		    pt = buf;
		    suck_keywd(&pt, keywd);
		    if (!pt)
			continue; /* empty line */
		    if (!strcmp(keywd, "EndComposites"))
			break;
		    suck_composites(&pt, res, keywd);
		    ix++;
		}
		if (ix!=max) {
		    PRINTD(("afm: got %d lines instead of %d\n", ix, max));
		}
		break;
	    default:
	    case afmkey_unknown:
		break;
	}
    }
}

void printp_init() {
    /* called by print class initialization */

    A_scale = atom::Intern("scale");
    A_landscape = atom::Intern("landscape");
    A_papersize = atom::Intern("papersize");

    AFMCache = (afm_font_hashtable *)printp_CreatePSHashTable();
}

static int printp_GetPSHashKey(const char  *name) {
    unsigned int res = 0;
    int mul = 3;
    while (*name) {
	res = (res+mul*(*name));
	name++;
	mul += 4;
    }
    return res % PSHASHSIZE;
}

	static afm_font_hashtable *
printp_CreatePSHashTable() {
    afm_font_hashtable *res;
    int ix;

    res = (afm_font_hashtable *)malloc(sizeof(afm_font_hashtable));

    res->tab = (struct afm_font_hashnode **)malloc(sizeof(struct afm_font_hashnode *) * PSHASHSIZE);
    for (ix=0; ix<PSHASHSIZE; ix++)
	res->tab[ix] = NULL;
    res->chain = NULL;
    res->chainnext = (&(res->chain));
    res->numentries = 0;

    return res;
}

	static void 
printp_DestroyPSHashTable(afm_font_hashtable  *tab, boolean  freedata) {
    int ix;
    struct afm_font_hashnode *tmp, **tablist;

    tablist = tab->tab;

    for (ix=0; ix<PSHASHSIZE; ix++) {
	while (tablist[ix]) {
	    if (freedata && tablist[ix]->data)
		free(tablist[ix]->data);
	    free(tablist[ix]->name);
	    tmp = (tablist[ix]->next);
	    free(tablist[ix]);
	    tablist[ix] = tmp;
	}
    }

    free(tablist);
    free(tab);
}

	static void *
printp_LookUpPSHashTable(afm_font_hashtable  *tab, const char  *name) {
    int buk = printp_GetPSHashKey(name);
    struct afm_font_hashnode *tmp = tab->tab[buk];

    while (tmp) {
	if (!strcmp(name, tmp->name))
	    return tmp->data;
	tmp = tmp->next;
    }
    return NULL;
}

/* returns TRUE if inserted, FALSE if already there. */
	static boolean 
printp_InsertInPSHashTable(afm_font_hashtable *tab, const char *name,
		void *dat, int *entryindex) {
    int buk = printp_GetPSHashKey(name);
    struct afm_font_hashnode *res;
    struct afm_font_hashnode *tmp = tab->tab[buk];

    while (tmp) {
	if (!strcmp(name, tmp->name)) {
	    if (entryindex)
		*entryindex = tmp->entrynum;
	    return FALSE;
	}
	tmp = tmp->next;
    }

    /* not found */

    res = (struct afm_font_hashnode *)malloc(sizeof(struct afm_font_hashnode));

    res->name = strdup(name);
    res->data = dat;
    res->next = tab->tab[buk];
    res->chain = NULL;
    tab->tab[buk] = res;
    res->entrynum = tab->numentries;
    tab->numentries++;

    (*tab->chainnext) = res;
    tab->chainnext = (&(res->chain));

    if (entryindex)
	*entryindex = res->entrynum;
    return TRUE;
}

	static boolean 
printp_IsEmptyPSHashTable(afm_font_hashtable  *tab) {
    if (tab->numentries==0)
	return TRUE;
    else
	return FALSE;
}

static void printp_EnumeratePSHashTable(afm_font_hashtable *tab,
		void (*splot)(char *nam, void *dat, void *rock), 
		void *rock) {
    struct afm_font_hashnode *tmp;

    for (tmp=tab->chain; tmp; tmp=tmp->chain) {
	(*splot)(tmp->name, tmp->data, rock);
    }
}

static void dumpregfont_splot(char *nam, void *dat, void *rock) {
    FILE *frock=(FILE *)rock;
    fprintf(frock, "%s ", nam);
}

static void dumpregdef_splot(char *nam, void *dat, void *rock) {
    FILE *frock=(FILE *)rock;
    fprintf(frock, "/%s %s def\n", nam, (const char *)dat);
}

static void dumpreghead_splot(char  *nam, void  *dat, void *rock) {
    FILE *frock=(FILE *)rock;
    struct genheader *hd = (struct genheader *)dat;

    (*hd->headproc)(frock, hd->rock);
}

static void parse_papersize(print *self, char *size) {
    char buf[64];
    char *cx;
    const struct pagesize *sptr;
    int res;
    double fwid, fhgt, convert;

    strncpy(buf, size, sizeof(buf)-1);
    for (cx=buf; *cx; cx++)
	if (isupper(*cx))
	    *cx = tolower(*cx);

    if (!(isdigit(buf[0]))) {
	for (sptr = pagesizelist; sptr->name; sptr++)
	    if (!strcmp(sptr->name, buf))
		break;
	if (!sptr->name) 
	    fprintf(stderr, "paper size '%s' is not known. Using 8.5x11.\n", 
				size);
	self->SetPaperSize(sptr->width, sptr->height);
	return;
    }

    /* parse numbers out */
    res = sscanf(buf, "%lf%*[^0-9.]%lf", &fwid, &fhgt);
    if (res != 2 || fwid <= 0.01 || fhgt <= 0.01) {
	fprintf(stderr, "paper size '%s' is incomprehensible. Using 8.5x11.\n",
			size);
	sptr = pagesizelist + sizeof(pagesizelist) 
		/ sizeof(struct pagesize) - 1; // last (default) entry
	self->SetPaperSize(sptr->width, sptr->height);
	return;
    }
    if (strchr(buf, 'c'))
	convert = 28.34645669291339; /* PS units per centimeter */
    else
	convert = 72.0; /* PS units per inch */
    self->SetPaperSize((long)(fwid * convert), (long)(fhgt * convert));
    return;
}

print::print() {
	headsregistered = printp_CreatePSHashTable();
	fontsregistered = printp_CreatePSHashTable();
	defsregistered = printp_CreatePSHashTable();
	cleanupsregistered = NULL;
	cleanupsptr = (&cleanupsregistered);
	defsused = 0;
	pageordinal = 0;
	landscape = FALSE;
	scale = 1.0;
	papersize_w = 72*17/2;	// 8.5 inches
	papersize_h = 72 * 11;	// 11 inches

	bodystream = tmpfile();

	filename = NewString(tmpnam(NULL));
	title = NewString("Andrew Document");

	next_earlier = current_print;
	current_print = this;

	// Register some defn's (current_print == this)
	print::PSRegisterDef("pagesave", "0");
	print::PSRegisterDef("beginpage", "{/pagesave save store}");
	print::PSRegisterDef("endpage", "{showpage pagesave restore}");
}

print::~print() {
	if (bodystream)
		fclose(bodystream);

	struct cleanup *cleaner, *nextclean;
	for (cleaner = cleanupsregistered; cleaner; cleaner = nextclean) {
		nextclean = cleaner->next;
		cleaner->cleanproc(cleaner->rock);
		free(cleaner);
	}

	// remove `this' from print::current_print list
	if ( ! current_print) {}	// ERROR: `this' is not on list
	else if (current_print == this)
		current_print = next_earlier;
	else {
		// scan list to find `this' and delete it
		print *prev = current_print, 
			*curr = current_print->next_earlier;
		while (curr != this && curr) {
			prev = curr;  curr = curr->next_earlier;
		}
		if (curr)
			prev->next_earlier = curr->next_earlier;
	}

	// free allocated stuff
	printp_DestroyPSHashTable(headsregistered, TRUE);
	printp_DestroyPSHashTable(fontsregistered, FALSE);
	printp_DestroyPSHashTable(defsregistered, TRUE);

	if (filename) free(filename);
	if (title) free(title);
}

// set member values of p according to properties of the view, v
// set *pwidth and *pheight to revised width and height
//	to account for scaling and rotation in PSNewPage
//
	void
print::SetFromProperties(view *v, long *pwidth, long *pheight) {
	long pscale;
	char *ppsize;
	int pagew, pageh;

	SetLandscape(v->GetPrintOption(A_landscape));
	ppsize = (char *)v->GetPrintOption(A_papersize);
	parse_papersize(this, ppsize);
	pscale = v->GetPrintOption(A_scale);
	if (pscale == 100 || pscale <= 1)  // get 1.0 for either 100 or 1
		scale = 1.0;
	else
		scale = (double)pscale / 100.0;

	GetPaperSize(&pagew, &pageh);
	if (scale != 1.0) {
		pagew = (long)((double)pagew / scale);
		pageh = (long)((double)pageh / scale);
	}
	if (landscape) {
		long lx = pageh;
		pageh = pagew;
		pagew = lx;
	}
	if (pwidth) *pwidth = pagew;
	if (pheight) *pheight = pageh;

	class buffer *buf;
	buf = buffer::FindBufferByData(v->GetDataObject());
	if (buf) title = NewString(buf->GetName());
}

	boolean
print::WritePostScript(char *fnmarg, char *titlearg) {
	if ( ! fnmarg) fnmarg = filename;	// use value from `this'
	if ( ! titlearg) titlearg = title;	// use value from `this'
	if (pageordinal == 0) {
		message::DisplayString(NULL, 10, 
				"This type of document cannot be printed.");
		return FALSE;
	}

	FILE *outfile = fopen(fnmarg, "w");

	/* generate header */
	fprintf(outfile, "%%!PS-Adobe-1.0\n");
	fprintf(outfile, "%%%%Title: %s\n", titlearg);

	time_t tim;
	tim = time(NULL);
	fprintf(outfile, "%%%%CreationDate: %s", ctime(&tim)); //ctime adds \n
	fprintf(outfile, "%%%%BoundingBox: %d %d %d %d\n", 0, 0, 
			papersize_w, papersize_h); // physical page limits
	/* %%Creator */
	fprintf(outfile, "%%%%Pages: %d\n", pageordinal); 
	if (!printp_IsEmptyPSHashTable(fontsregistered)) {
		fprintf(outfile, "%%%%DocumentFonts: ");
		printp_EnumeratePSHashTable(fontsregistered, 
						dumpregfont_splot, outfile);
		fprintf(outfile, "\n");
	}
	fprintf(outfile, "%%%%EndComments\n\n");

	fprintf(outfile, "/atkDict %d dict def\natkDict begin\n\n", 
				defsregistered->numentries);
	fprintf(outfile, "gsave\n");
	printp_EnumeratePSHashTable(defsregistered, dumpregdef_splot, outfile);
	printp_EnumeratePSHashTable(headsregistered,dumpreghead_splot,outfile);
	fprintf(outfile, "grestore\n\n");
	fprintf(outfile, "%%%%EndProlog\n\n");

	/* copy body text */
	rewind(bodystream);
	int copychar;
	while ((copychar=getc(bodystream))!=EOF) {
		putc(copychar, outfile);
	}
	fclose(bodystream);
	bodystream = NULL;

	// terminate last page and file
	fprintf(outfile, "endpage\n\n");
	fprintf(outfile, "%%%%Trailer\n");

	fclose(outfile);
	return TRUE;
}

struct ps_fontfamily_lookup {
    const char *xname;
    const char *psname;
    const char * const *facesuffix; /* pointer to array[4] of char *  */
    int encoding;
};

static const char * const facesuffix_Roman[4] = {"-Roman", "-Italic", "-Bold", "-BoldItalic"};
static const char * const facesuffix_None[4] = {"", "", "", ""};
static const char * const facesuffix_BoldOblique[4] = {"", "-Oblique", "-Bold", "-BoldOblique"};
static const char * const facesuffix_BookOblique[4] = {"-Book", "-BookOblique", "-Demi", "-DemiOblique"};
static const char * const facesuffix_LightItalic[4] = {"-Light", "-LightItalic", "-Demi", "-DemiItalic"};

/*encoding means:
 1: ISO8859-Latin-1
 2: Adobe Symbol font encoding
 3: SymbolA font encoding
 */
static const struct ps_fontfamily_lookup fontfamilylist[] = {
    {"andy", "Times", facesuffix_Roman, 1},
    {"times", "Times", facesuffix_Roman, 1},
    {"andysans", "Helvetica", facesuffix_BoldOblique, 1},
    {"helvetica", "Helvetica", facesuffix_BoldOblique, 1},
    {"andytype", "Courier", facesuffix_BoldOblique, 1},
    {"courier", "Courier", facesuffix_BoldOblique, 1},
    {"avantgarde", "AvantGarde", facesuffix_BookOblique, 1},
    {"palatino", "Palatino", facesuffix_Roman, 1},
    {"bookman", "Bookman", facesuffix_LightItalic, 1},
    {"newcenturyschlbk", "NewCenturySchlbk", facesuffix_Roman, 1},
    {"new century schoolbook", "NewCenturySchlbk", facesuffix_Roman, 1},
    {"symbol", "Symbol", facesuffix_None, 2},
    {"andysymbol", "Symbol", facesuffix_None, 2},
    {"symbola", "Symbol", facesuffix_None, 3},
    {NULL, "Courier", facesuffix_BoldOblique, 1} /* the default */
};

boolean print::LookUpPSFont(char  *result, short  **encoding, class fontdesc  *fd, const char  *family, long  size, long  style)
{
    char buf[256];
    char cx;
    int ix, faceval;
    const struct ps_fontfamily_lookup *tmp;
    boolean resval;

    print_EnsureClassInitialized();

    if (fd) {
	family = (fd)->GetFontFamily();
	style = (fd)->GetFontStyle();
    }

    for (ix=0; family[ix]; ix++) {
	cx = family[ix];
	if (isupper(cx))
	    buf[ix] = tolower(cx);
	else
	    buf[ix] = cx;
    }
    buf[ix] = '\0';

    for (tmp = fontfamilylist; TRUE; tmp++)
	if (!tmp->xname || !strcmp(buf, tmp->xname))
	    break;

    resval = (tmp->xname ? TRUE : FALSE);
    strcpy(result, tmp->psname);

    switch (style & (fontdesc_Bold | fontdesc_Italic)) {
	case fontdesc_Plain:
	    faceval = 0;
	    break;
	case fontdesc_Italic:
	    faceval = 1;
	    break;
	case fontdesc_Bold:
	    faceval = 2;
	    break;
	case fontdesc_Bold | fontdesc_Italic:
	    faceval = 3;
	    break;
    }
    strcat(result, tmp->facesuffix[faceval]);
    switch (tmp->encoding) {
	case 1:
	    *encoding = print::EncodeISO8859();
	    break;
	case 2:
	    *encoding = print::EncodeAdobeSymbol();
	    break;
	case 3:
	    *encoding = print::EncodeSymbolAndy();
	    break;
	default:
	    fprintf(stderr, "font %s has illegal encoding code.\n", family);
	    *encoding = NULL;
	    break;
    }
    return resval;
}

/* if maxlen < 0, it is ignored */
void print::OutputPSString(FILE *outfile, const char *str, int maxlen)
{
    boolean ignoremax;
    int ch;

    print_EnsureClassInitialized();

    if (!str || !outfile)
	return;

    for (ignoremax = (maxlen < 0);
	 (ignoremax || maxlen >= 0) && *str;
	 str++, (ignoremax || maxlen--)) {

	ch = (*str);
	if (ch==' ') {
	    putc(ch, outfile);
	}
	else if (ch=='\\' || ch=='(' || ch==')' || !isgraph(ch)) {
	    fprintf(outfile, "\\%03o", ch);
	}
	else {
	    putc(ch, outfile);
	}
    }
}

/* chi is an AGP-encoded value; result is in PS units (or whatever units size is in) */
double print::ComputePSCharWidth(int  chi, struct font_afm  *afm, int  size)
{
    double res;
    int chs, nump;

    if (chi < 0)
	return 0;

    nump = afm->ch[chi].numparts;
    if (nump==print_SymbolChar) {
	res = 0;
	/* pull width from symbol AFM */
	if (!symbolfont) {
	    symbolfont = print::GetPSFontAFM("Symbol");
	}
	chs = afm->ch[chi].u.symbolchar;
	res = symbolfont->ch[chs].x;
    }
    else {
	/* for normal or composite chars, the size is available here. */
	res = afm->ch[chi].x;
    }

    return (res * (double)size) / 1000.0;
}

static void GeneratePSLetter(FILE  *outfile, int  ch)
{
    if (ch>=128 || ch<32)
	fprintf(outfile, "\\%3o", ch); 
    else switch (ch) {
	case '(':
	case ')':
	case '\\':
	    putc('\\', outfile);
	    putc(ch, outfile);
	    break;
	default:
	    putc(ch, outfile);
	    break;
    }
}

#define AFMToPS(val, size) (((val) * (size)) / 1000.0)

/* generate PS to print a string of encoding-encoded chars. The graphic environment (font, etc) is already set, except for position, which is (xpos, tY).  */
void print::GeneratePSWord(FILE  *outfile, char  *buf, int  len, double  xpos, short  *encoding, struct font_afm  *afm, int  fontsize)
{
    int ix, px;
    int ch, chi, chs, nump;
    boolean firstword = TRUE;
    int symbolfontindex;
    int defsused; /* 1==tCmp, 2==tW, 4==tY */
    defsused = (2|4); /* always */

    fprintf(outfile, "(");
    for (ix=0; ix<len; ix++) {
	ch = (unsigned char)(buf[ix]);
	chi = encoding[ch];
	if (chi<0) {
	    /* do nothing */
	}
	else {
	    nump = afm->ch[chi].numparts;
	    if (nump==0) {
		/* normal char chi */
		GeneratePSLetter(outfile, chi);
	    }
	    else if (nump==print_SymbolChar) {
		/* char afm->ch[chi].u.symbolchar, in the Symbol font */
		chs = afm->ch[chi].u.symbolchar;
		if (firstword)
		    fprintf(outfile, ") %g tW\n", xpos);
		else
		    fprintf(outfile, ") show\n");
		firstword = FALSE;

		if (!symbolfont) 
		    symbolfont = print::GetPSFontAFM("Symbol");
		symbolfontindex = print::PSRegisterFont("Symbol"); 
			/* (not the most efficient way to do this) */
		fprintf(outfile, "gsave\n%s%d %d scalefont setfont\n", 
			print_PSFontNameID, symbolfontindex, fontsize);
		fprintf(outfile, "(");
		GeneratePSLetter(outfile, chs);
		fprintf(outfile, ") show grestore\n");
		fprintf(outfile, "%g 0 rmoveto\n(", 
			AFMToPS(symbolfont->ch[chs].x, fontsize));
	    }
	    else {
		/* composite */
		struct font_afm_comppart *parts = afm->ch[chi].u.parts;
		if (firstword)
		    fprintf(outfile, ") %g tW\n", xpos);
		else
		    fprintf(outfile, ") show\n");
		firstword = FALSE;

		for (px=0; px<nump; px++) {
		    defsused |= 1;
		    fprintf(outfile, "(");
		    GeneratePSLetter(outfile, parts[px].num);
		    fprintf(outfile, ") %g %g tCmp\n", AFMToPS(parts[px].xoff, fontsize), AFMToPS(parts[px].yoff, fontsize));
		}

		fprintf(outfile, "%g 0 rmoveto\n(", AFMToPS(afm->ch[chi].x, fontsize));
	    }
	}
    }
    if (firstword) {
	fprintf(outfile, ") %g tW\n", xpos);
    }
    else {
	fprintf(outfile, ") show\n");
    }

    short ttfoo = 0;
    short *pdefsused = (current_print) ? &current_print->defsused 
				: &ttfoo;
    if (!(*pdefsused & 1) && (defsused & 1)) {
	print::PSRegisterDef("tCmp", "{gsave rmoveto show grestore} bind"); /* str xoff yoff tCmp */
	*pdefsused |= 1;
    }
    if (!(*pdefsused & 2) && (defsused & 2)) {
	print::PSRegisterDef("tW", "{tY moveto show} bind");
	*pdefsused |= 2;
    }
    if (!(*pdefsused & 4) && (defsused & 4)) {
	print::PSRegisterDef("tY", "0");
	*pdefsused |= 4;
    }
}

int print::PSRegisterFont(const char  *fontname)
{
    int eix;
    boolean res;
    char buf[32];
    char buf2[256];

    print_EnsureClassInitialized();
    if ( ! current_print)  return (-1);

    res = printp_InsertInPSHashTable(current_print->fontsregistered,
		fontname, NULL, &eix);
    if (res) {
	/* new one */
	sprintf(buf, "%s%d", print_PSFontNameID, eix);
	sprintf(buf2, "/%s findfont", fontname);
	print::PSRegisterDef(buf, buf2);
    }

    return eix;
}

void print::PSRegisterDef(const char  *procname, const char  *defstring)
{
    char *tmp;

    print_EnsureClassInitialized();
    if ( ! current_print)  return;

    tmp = strdup(defstring);

    printp_InsertInPSHashTable(current_print->defsregistered,
		procname, tmp, NULL);
}

void print::PSRegisterHeader(const char *headname, 
		print_header_fptr headproc, const void *rock) {
    struct genheader *tmp;

    print_EnsureClassInitialized();
    if ( ! current_print) return;

    tmp = (struct genheader *)malloc(sizeof(struct genheader));
    tmp->headproc = headproc;
    tmp->rock = rock;

    printp_InsertInPSHashTable(current_print->headsregistered,
		headname, tmp, NULL);
}

void print::PSRegisterCleanup(print_cleanup_fptr cleanproc, 
		void *rock) {
    struct cleanup *tmp;

    print_EnsureClassInitialized();
    if ( ! current_print)  return;

    tmp = (struct cleanup *)malloc(sizeof(struct cleanup));
    tmp->cleanproc = cleanproc;
    tmp->rock = rock;
    tmp->next = NULL;
    *current_print->cleanupsptr = tmp;
    current_print->cleanupsptr = (&(tmp->next));
}

boolean print::PSNewPage(int  pagenum) {
    print_EnsureClassInitialized();
    if ( ! current_print)  return FALSE;

    print *cp = current_print;
    FILE *bs = cp->bodystream;
    if (cp->pageordinal) {
	/* end the previous page */
	fprintf(bs, "endpage\n\n");
    }

    cp->pageordinal++;
    if (pagenum==print_UsualPageNumbering)
	pagenum = cp->pageordinal;
    /* start this page */
    fprintf(bs, "%%%%Page: %d %d\n", pagenum, cp->pageordinal);
    fprintf(bs, "beginpage\n");
    if (cp->landscape) {	// rotate widdershins ??
	fprintf(bs, "90 rotate 0 %d translate\n", -cp->papersize_w);
    }
    if (cp->scale != 1.0) {
	fprintf(bs, "%g dup scale\n", cp->scale);
    }

    return TRUE;
}

#define MAXLEN (14)
/* copied from afm/afmmangle.c */
static const char *SquishAFMFileName(const char *orig)
{
    static char result[MAXLEN+1];
    int total[MAXLEN];
    int ix, val;

    for (ix=0; ix<MAXLEN; ix++)
	total[ix] = 0;

    for (ix=0; *orig; orig++) {

	val = *orig;
	if (val >= 'a' && val <= 'z')
	    val = val-'a'+0;
	else if (val >= 'A' && val <= 'Z')
	    val = val-'A'+26;
	else if (val >= '0' && val <= '9')
	    val = val-'0'+52;
	else val = 62;

	total[ix] = (total[ix]+val) % 52 + 1;

	ix++;
	if (ix >= MAXLEN)
	    ix = 0;
    }

    for (ix=0; ix<MAXLEN; ix++) {
	if (total[ix]==0)
	    result[ix] = '\0';
	else if (total[ix] <= 26)
	    result[ix] = total[ix]-1+'a';
	else if (total[ix] <= 52)
	    result[ix] = total[ix]-27+'A';
	else
	    result[ix] = '-';
    }

    return result;
}


struct font_afm *print::GetPSFontAFM(const char  *fontname)
{
    const char *tpath;
    char afmname[1024];
    FILE *infl;
    struct font_afm *res;
    boolean ok;

    print_EnsureClassInitialized();

    /* check for cached structure */
    res = (struct font_afm *)printp_LookUpPSHashTable(AFMCache, fontname);
    if (res)
	return res;

    res = (struct font_afm *)malloc(sizeof(struct font_afm));

    /* ### one day, this should check a preference / env var, and search a whole bunch of directories. */
    tpath = environ::AndrewDir("/lib/afm");
    /* check mangled name first */
    sprintf(afmname, "%s/%s", tpath, SquishAFMFileName(fontname));
    infl = fopen(afmname, "r");
    if (!infl) {
	/* check unmangled name */
	sprintf(afmname, "%s/%s", tpath, fontname);
	infl = fopen(afmname, "r");
    }
    if (!infl) {
	fprintf(stderr, "unable to read font metric file (%s)\n", afmname);
	return NULL;
    }

    ok = ReadAFMFile(infl, fontname, res);

    fclose(infl);
    if (!ok) {
	fprintf(stderr, "error in font metric file (%s)\n", afmname);
	free(res);
	return NULL;
    }
/*
    PRINTD(("AFM: read ok\n"));
    printf("name = |%s|\n", res->fontname);
    printf("bbox = %g %g %g %g\n", res->fontbbox_l, res->fontbbox_b, res->fontbbox_r, res->fontbbox_t);
    printf("characters = %d\n", res->characters);
    printf("capheight = %g, xheight = %g, ascender = %g, descender = %g\n", res->capheight, res->xheight, res->ascender, res->descender);
    printf("underlinepos = %g, thick = %g\n", res->h.underlineposition, res->h.underlinethickness);
    printf("ital angle = %g, charwidth = %g %g, isfixedpitch = %d\n", res->h.italicangle, res->h.charwidthx, res->h.charwidthy, res->h.isfixedpitch);
*/
    printp_InsertInPSHashTable(AFMCache, fontname, res, NULL);

    return res;
}

short *print::EncodeISO8859()
{
    return ISO8859_map;
}
short *print::EncodeAdobeSymbol()
{
    return AdobeSymbol_map;
}
short *print::EncodeSymbolAndy()
{
    return SymbolAndy_map;
}
