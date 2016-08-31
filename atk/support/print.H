ATK_INTER
#ifndef _print_H_
#define _print_H_
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


 

/* The following defines are to be used as the second argument to print_ProcessView 
	or the first argument to GetPrintCmd */

#define print_PRINTPOSTSCRIPT 129 /*  send postscript directly to printer */
#define print_PREVIEWPOSTSCRIPT  128 /* preview postscript */
#define print_PRINTTROFF 1	/* send through troff and print  (default) */
#define print_PREVIEWTROFF 0	/* send through troff and preview */

#include <ATK.H>
#include <util.h>
class view;
class fontdesc;

/* all these structures are used in the PS printing stuff. */

#define print_UsualPageNumbering (-32767)
#define print_PSFontNameID ("tF")
#define print_SymbolChar   32768

typedef void (*print_cleanup_fptr)(const void *rock);
typedef void (*print_header_fptr)(FILE *outfile, const void *rock);

struct afm_font_hashnode {
    char *name;
    void *data;
    int entrynum;
    struct afm_font_hashnode *chain, *next;
};

typedef struct _afm_font_hashtable {
    struct afm_font_hashnode *chain, **chainnext;
    struct afm_font_hashnode **tab;
    int numentries;
} afm_font_hashtable;

struct font_afm_dir {
    double underlineposition, underlinethickness;
    double italicangle;
    boolean isfixedpitch;
    double charwidthx, charwidthy;
};

struct font_afm_comppart {
    int num; /* Adobe Standard index -- ie, index back into afm->ch[] */
    double xoff, yoff;
};

struct font_afm_char {
    double x, y;
    /* may in future want the ligature and bounding-box fields */
    int numparts; /* if this == 0, it's a normal char; if print_SymbolChar, it's the char with index u.symbolchar in the Symbol font; otherwise, it's the size of u.parts.
     For print_SymbolChar, the x and y values are *undefined*. */
    union {
	int symbolchar; /* if the glyph is from the Symbol font */
	struct font_afm_comppart *parts; /* if the glyph is composite (array of parts) */
    } u;
};

#define font_afm_AGPsize 321 /* Total number of characters in AGP encoding */

/* in using these values, remember that in PostScript, y=0 is at the bottom */
struct font_afm {
    char *fontname;
    double fontbbox_l, fontbbox_t, fontbbox_r, fontbbox_b;
    int characters; /* unfortunately, optional */
    double capheight, xheight, ascender, descender;

    struct font_afm_dir h;
    /* if we ever care about vertical metrics, add another struct font_afm_dir called v. */

    struct font_afm_char ch[font_afm_AGPsize];
    /* this is indexed by AGP-encoded values. These are identical to the Adobe font encodings for the values 0..255. Higher values should only be taken from the pprint_EncodeISO8859() map or something similar. */
};

class print : public ATK  {
    public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	print();
	virtual ~print();

	static int ProcessView(view *v, int print, int dofork, 
		const char *DocumentName, const char *prarg)  ;

	static const char * GetPrintCmd(int print)  ;
	static boolean LookUpColor(const char *name, 
		double *rval, double *gval, double *bval)  ;
	static struct font_afm *GetPSFontAFM(const char  *fontname);
	static short *EncodeISO8859();
	static short *EncodeAdobeSymbol();
	static short *EncodeSymbolAndy();
	static boolean LookUpPSFont(char *result, short **encoding, 
		fontdesc *fd, const char *family, long size, long style);
	static double ComputePSCharWidth(int chi, struct font_afm *afm, 
		int size);
	static void OutputPSString(FILE *outfile, const char *str, int maxlen);
	static void GeneratePSWord(FILE *outfile, char *buf, int len, 
		double xpos, short *encoding, struct font_afm *afm, 
		int fontsize);
	static int PSRegisterFont(const char *fontname);
	static void PSRegisterDef(const char *procname, const char *defstring);
	static void PSRegisterHeader(const char *headname, 
		print_header_fptr headproc, const void *rock);
	static void PSRegisterCleanup(print_cleanup_fptr cleanproc,void *rock);
	static boolean PSNewPage(int pagenum);

	void SetFromProperties(view *v, long *pwidth, long *pheight);
	boolean WritePostScript(char *filename, char *title);

	static inline print *GetCurrentPrint() { return current_print; }

	inline FILE* GetFILE() { return bodystream; }
	inline void SetLandscape(boolean v) { landscape = v; }
	inline boolean GetLandscape() { return landscape; }
	inline void SetScale(double v) { scale = v; }
	inline double GetScale() { return scale; }
	inline void SetPaperSize(int width, int height) 
		{ papersize_w = width; papersize_h = height; }
	inline void GetPaperSize(int *width, int *height) 
		{ *width = papersize_w; *height = papersize_h; }
	inline void SetFileName(char *fnm) { filename = NewString(fnm); }
	inline char *GetFileName() { return filename; }
	inline void SetTitle(const char *tnm) { title = NewString(tnm); }
	inline const char *GetTitle() { return title; }

	char *filename;
	char *title;

	afm_font_hashtable *fontsregistered, 
			*defsregistered, *headsregistered;
	struct cleanup *cleanupsregistered, **cleanupsptr;
	short defsused;
	int pageordinal;
	boolean landscape;
	double scale;
	int papersize_w, papersize_h;
	FILE *bodystream;

	print *next_earlier;
	static print *current_print;	// head of chain linked by next_earlier
};

#endif /* _print_H_ */
