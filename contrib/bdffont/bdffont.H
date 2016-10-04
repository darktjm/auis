#ifndef _bdffont_H_
#define _bdffont_H_
/* bdffont.H - font editor for bdf format fonts */
/** \addtogroup libbdffont Object bdffont
 *  A dynamically loadable ATK class file.  Contains bdffont, bdffontv
 * @{ */
#include <units.h>

/* value for NotifyObservers when bdffont__Write is called */
#define bdffont_Writing  4

#define bdffont_WidthInBytes(w) (((w) + 7) / 8)

/* Face codes */
#define bdffont_Bold		(0x1)
#define bdffont_Italic		(0x2)
#define bdffont_FixedWidth	(0x4)
#define bdffont_Shadowed	(0x8)

struct bdffont_fontchar
    {
	char name[16];
	int encoding;
	long swx, swy;
	long dwx, dwy;
	long bbw, bbh, bbx, bby;
	long attributes;
	unsigned char *bitmap;	/* NULL means not active */
    };

#define bdffont_Alignment (16)

#define bdffont_IsActive(fc) ((fc)->bitmap != NULL)

#define bdffont_GetCharEncoding(fc) ((fc)->encoding)
#define bdffont_GetCharName(fc) ((fc)->name)
#define bdffont_SetCharName(fc, n) strncpy((fc)->name, n, sizeof((fc)->name))
#define bdffont_GetSWidth(fc, x, y) ((*(x) = (fc)->swx), (*(y) = (fc)->swy))
#define bdffont_SetSWidth(fc, x, y) (((fc)->swx = (long) (x)), ((fc)->swy = (long) (y)))
#define bdffont_GetDWidth(fc, x, y) ((*(x) = (fc)->dwx), (*(y) = (fc)->dwy))
#define bdffont_SetDWidth(fc, x, y) (((fc)->dwx = (long) (x)), ((fc)->dwy = (long) (y)))
#define bdffont_GetOrigin(fc, x, y) ((*(x) = (fc)->bbx), (*(y) = (fc)->bby))
#define bdffont_SetOrigin(fc, x, y) (((fc)->bbx = (long) (x)), ((fc)->bby = (long) (y)))
#define bdffont_GetExtent(fc, w, h) ((*(w) = (fc)->bbw), (*(h) = (fc)->bbh))
#define bdffont_SetExtent(fc, w, h) (((fc)->bbw = (long) (w)), ((fc)->bbh = (long) (h)))
#define bdffont_GetAttrs(fc) ((fc)->attributes)
#define bdffont_SetAttrs(fc, p) ((fc)->attributes = (long) (p))
#define bdffont_GetBitmap(fc) ((fc)->bitmap)
#define bdffont_SetBitmap(fc, bm) ((fc)->bitmap = (unsigned char *) (bm))

#define bdffont_AlignedWidthInBytes(fc) \
	(bdffont_WidthInBytes(bdffont_Alignment) * \
		(((fc)->bbw + bdffont_Alignment - 1) / bdffont_Alignment))
#define bdffont_BitmapSize(fc) ((fc)->bbh * bdffont_AlignedWidthInBytes(fc))

#include <dataobject.H>
class bdffont : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	bdffont();
	
	virtual ~bdffont();
	static class bdffont * CreateNewFont(long pts, long resx, long resy)  ;
    
	 long Read(FILE *file, long id)  ;
	 long Write(FILE *file, long writeid, int level)  ;
    
	virtual void SetFontName(const char *fn);
	virtual void SetFontFamily(const char *fn);
	virtual void GetBoundingBox(long *w, long *h, long *x, long *y);
	virtual void SetBoundingBox(long w, long h, long x, long y);
	virtual void GetResolution(long *rx, long *ry);
	virtual void SetResolution(long rx, long ry);
	virtual void SetCharDWidth(int which, long x, long y);
	virtual void SetCharExtent(int which, long w, long h);
	virtual void SetCharOrigin(int which, long x, long y);
	virtual int GetDefaultChar()  ;
	virtual void SetDefaultExtent(long w, long h);
	virtual void GetDefaultExtent(long *w, long *h);
	virtual void SetDefaultOrigin(long x, long y);
	virtual void GetDefaultOrigin(long *x, long *y);
	virtual void SetDefaultDelta(long dx, long dy);
	virtual void GetDefaultDelta(long *dx, long *dy);
    
	inline char  * GetFontName() { return ((this)->fontname); }
	inline char  * GetFontFamily() { return ((this)->fontfamily); }
	inline long   GetPointSize() { return ((this)->pointsize); }
	inline void SetPointSize(long s) { ((this)->pointsize = (s)); }
	inline long   GetFontWeight() { return ((this)->fontweight); }
	inline void SetFontWeight(long w) { ((this)->fontweight = (w)); }
	inline long   GetFontFace() { return ((this)->fontface); }
	inline void SetFontFace(long f) { ((this)->fontface = (f)); }
	inline long   GetFontAscent() { return ((this)->ascent); }
	inline void SetFontAscent(long a) { ((this)->ascent = (a)); }
	inline long   GetFontDescent() { return ((this)->descent); }
	inline void SetFontDescent(long d) { ((this)->descent = (d)); }
	inline void SetDefaultChar(int c) { ((this)->defaultchar = (long) (c)); }
	inline long   GetActiveDefns() { return ((this)->activedefns); }
	inline void ModifyActiveDefnsBy(long count) { ((this)->activedefns += (count)); }
	inline struct bdffont_fontchar *GetDefinition(int encoding) { return (&(this)->defns[encoding]); }
	inline double ComputeFontSize() { return (((double) ((this)->pointsize * (this)->resy)) / units_POINTSperINCH); }
    
	class tlex *lex;
	char *version;
	char *comments;
	char *fontname;
	long pointsize;
	long resx;		/* RESOLUTION_X */
	long resy;		/* RESOLUTION_Y */
	long bbw, bbh, bbx, bby;
	long proplength;
	char *properties;	/* known properties ... */
	char *fontfamily;	/* FAMILY */
	long fontweight;	/* WEIGHT */
	long fontface;		/* WEIGHT_NAME */
	long ascent;		/* FONT_ASCENT */
	long descent;		/* FONT_DESCENT */
	long defaultw;		/* DEFAULT_WIDTH */
	long defaulth;		/* DEFAULT_HEIGHT */
	long defaultx;		/* DEFAULT_X */
	long defaulty;		/* DEFAULT_Y */
	long defaultdx;		/* DEFAULT_DX */
	long defaultdy;		/* DEFAULT_DY */
	long defaultchar;	/* DEFAULT_CHAR */
	long activedefns;
	struct bdffont_fontchar *defns;
	long defns_size;
	long lastcharloaded;
};
/** @} */
#endif /* _bdffont_H_ */
