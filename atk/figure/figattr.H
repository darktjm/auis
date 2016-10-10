#ifndef _figattr_H_
#define _figattr_H_
/** \addtogroup libfigure
 * @{ */
/* figattr.H - attributes for fig objects */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#define	figattr_MaskAll	    (0x0fff) /* binary 1111 1111 1111 */
#define figattr_NumAttributes (12)

#define	figattr_Shade	    (0)
#define	figattr_Color	    (1)
#define	figattr_LineWidth   (2)
#define	figattr_RRectCorner (3)
#define	figattr_FontSize    (4)
#define	figattr_FontStyle   (5)
#define	figattr_FontFamily  (6)
#define	figattr_TextPos	    (7)
#define figattr_ArrowSize   (8)
#define figattr_ArrowPos    (9)
#define figattr_Arrow       (10)
#define figattr_LineStyle   (11)

/* definitions:
Shade: long: value from 0 (white) to 8 (black). -1 means clear. 
LineWidth: long: 0 for hairline (always drawn at 1/72"); positive values for positive widths. The width in fig coords is val*figview_FigUPerPix.
RRectCorner: long: 0 for none; positive values for positive widths. The width in fig coords is val*figview_FigUPerPix.
Color: char *: lowercase string containing an X (or whatever) color name.
TextPos: long: figattr_PosLeft, figattr_PosCenter, figattr_PosRight 
ArrowPos: long: figattr_Arrow{None,Tail,Head,Both}
ArrowSize: long: positive values, equal to radius of arrowheads. The width in fig coords is val*figview_FigUPerPix.
Arrow: long: figattr_IsoTriangle, figattr_Circle, figattr_Square
LineStyle: long: figattr_LineSolid, figattr_LineSingle, figattr_LineDouble, figattr_LineQuad, figattr_LineDot, figattr_Line2Dot
*/

#define figattr_ShadeClear  (-1)
/* denominator for view::GrayPattern() calls */
#define figattr_ShadeDenom  (8)

/* values for the ArrowPos attribute */
#define figattr_ArrowNone   (0x00)
#define figattr_ArrowTail   (0x01)
#define figattr_ArrowHead   (0x02)
#define figattr_ArrowBoth   (0x03)

/* values for the Arrow attribute */
#define figattr_IsoTriangle	(0x0000)
#define figattr_Circle		(0x0010)
#define figattr_Square		(0x0020)
#define figattr_EquTriangle	(0x0030)
/* might want to add support for different kinds of arrows, in which case, here are some ideas...
#define figattr_Arrowhead	(0x0040)
#define figattr_InvTriangle	(0x0050)
#define figattr_Custom		(0x0060)
*/

/* values for the TextPos attribute */
#define	figattr_PosCenter   (0x00)
#define	figattr_PosLeft	    (0x01)
#define	figattr_PosRight    (0x02)
/*  I have no intention of supporting vertical positioning, but if anybody does, here are some values. OR them with the first three to create the TextPos value. Note that old TextPos values will be forwards-compatible.
#define	figattr_PosVCenter  (0x00)
#define	figattr_PosTop	    (0x10)
#define	figattr_PosBottom   (0x20)
*/

/* values for the LineStyle attribute */
#define figattr_LineSolid	(0x00)
#define figattr_LineSingle	(0x01)
#define figattr_LineDouble	(0x02)
#define figattr_LineQuad	(0x03)
#define figattr_LineDot		(0x04)
#define figattr_Line2Dot	(0x05)

#include <dataobject.H>
#include <point.h>

struct figattr_arrowhead {
    long ashape;
    struct point arrowtemp[6];
};

class figattr : public  dataobject  {
  public:
      virtual ATKregistryEntry *ATKregistry();

      figattr();
      virtual ~figattr();

      long Write(FILE *fp, long writeid, int level)  ;
      long Read(FILE *file, long id)  ;

      virtual class figattr * CopySelf()  ;
      virtual void CopyData(class figattr *src, unsigned long mask);

      static char *LineStylePattern(long val, long lwidth);
      static void DrawArrowHead(class figview *v, long tipx, long tipy, long *vecx, long *vecy, long shape, long size);
      static void SetupPrintArrowHead(struct figattr_arrowhead *arrowtemp, class figview *v, long tipx, long tipy, long *vecx, long *vecy, long ashape, long asize);
      static void PrintArrowHead(FILE *file, const char *prefix, struct figattr_arrowhead *arrowtemp);

      inline void SetActive(int att, unsigned long val)  { ((this)->active = ((val) ? ((this)->active | ((unsigned long)1<<(att))) : ((this)->active & ~((unsigned long)1<<(att))))); }
      inline unsigned long IsActive(int att)	{ return ((this)->active & ((unsigned long)1<<(att))); }

      inline void SetShadeVal(long val)  { ((this)->shade = (val)); }
      inline long   GetShadeVal()  { return ((this)->shade); }
      inline void SetShade(long val)  { ((this)->shade = (val), (this)->active |= ((unsigned long)1<<figattr_Shade)); }
      inline long GetShade(class figattr *def)  { return (((this))->IsActive( figattr_Shade) ? ((this)->shade) : ((def)->shade)); }

      inline void SetLineWidthVal(long val)  { ((this)->linewidth = (val)); }
      inline long   GetLineWidthVal()  { return ((this)->linewidth); }
      inline void SetLineWidth(long val)  { ((this)->linewidth = (val), (this)->active |= ((unsigned long)1<<figattr_LineWidth)); }
      inline long GetLineWidth(class figattr *def)  { return (((this))->IsActive( figattr_LineWidth) ? ((this)->linewidth) : ((def)->linewidth)); }

      inline void SetRRectCornerVal(long val)  { ((this)->rrectcorner = (val)); }
      inline long   GetRRectCornerVal()  { return ((this)->rrectcorner); }
      inline void SetRRectCorner(long val)  { ((this)->rrectcorner = (val), (this)->active |= ((unsigned long)1<<figattr_RRectCorner)); }
      inline long GetRRectCorner(class figattr *def)  { return (((this))->IsActive( figattr_RRectCorner) ? ((this)->rrectcorner) : ((def)->rrectcorner)); }

      inline void SetColorVal(const char *val)  { if((this)->color) free((this)->color); (this)->color = strdup(val); }
      inline const char  * GetColorVal()  { return ((this)->color); }
      inline void SetColor(const char *val)  { ((this)->SetColorVal( val), (this)->active |= ((unsigned long)1<<figattr_Color)); }
      inline const char *GetColor(class figattr *def)  { return (((this))->IsActive( figattr_Color) ? ((this)->color) : ((def)->color)); }

      inline void SetFontFamilyVal(const char *val)  { if((this)->fontfamily) free((this)->fontfamily); (this)->fontfamily = strdup(val); }
      inline char  * GetFontFamilyVal()  { return ((this)->fontfamily); }
      inline void SetFontFamily(const char *val)  { ((this)->SetFontFamilyVal( val), (this)->active |= ((unsigned long)1<<figattr_FontFamily)); }
      inline char *GetFontFamily(class figattr *def)  { return (((this))->IsActive( figattr_FontFamily) ? ((this)->fontfamily) : ((def)->fontfamily)); }

      inline void SetFontSizeVal(long val)  { ((this)->fontsize = (val)); }
      inline long   GetFontSizeVal()  { return ((this)->fontsize); }
      inline void SetFontSize(long val)  { ((this)->fontsize = (val), (this)->active |= ((unsigned long)1<<figattr_FontSize)); }
      inline long GetFontSize(class figattr *def)  { return (((this))->IsActive( figattr_FontSize) ? ((this)->fontsize) : ((def)->fontsize)); }

      inline void SetFontStyleVal(long val)  { ((this)->fontstyle = (val)); }
      inline long   GetFontStyleVal()  { return ((this)->fontstyle); }
      inline void SetFontStyle(long val)  { ((this)->fontstyle = (val), (this)->active |= ((unsigned long)1<<figattr_FontStyle)); }
      inline long GetFontStyle(class figattr *def)  { return (((this))->IsActive( figattr_FontStyle) ? ((this)->fontstyle) : ((def)->fontstyle)); }

      inline void SetTextPosVal(long val)  { ((this)->textpos = (val)); }
      inline long   GetTextPosVal()  { return ((this)->textpos); }
      inline void SetTextPos(long val)  { ((this)->textpos = (val), (this)->active |= ((unsigned long)1<<figattr_TextPos)); }
      inline long GetTextPos(class figattr *def)  { return (((this))->IsActive( figattr_TextPos) ? ((this)->textpos) : ((def)->textpos)); }

      inline void SetArrowPosVal(long val)  { ((this)->arrowpos = (val)); }
      inline long   GetArrowPosVal()  { return ((this)->arrowpos); }
      inline void SetArrowPos(long val)  { ((this)->arrowpos = (val), (this)->active |= ((unsigned long)1<<figattr_ArrowPos)); }
      inline long GetArrowPos(class figattr *def)  { return (((this))->IsActive( figattr_ArrowPos) ? ((this)->arrowpos) : ((def)->arrowpos)); }

      inline void SetArrowSizeVal(long val)  { ((this)->arrowsize = (val)); }
      inline long   GetArrowSizeVal()  { return ((this)->arrowsize); }
      inline void SetArrowSize(long val)  { ((this)->arrowsize = (val), (this)->active |= ((unsigned long)1<<figattr_ArrowSize)); }
      inline long GetArrowSize(class figattr *def)  { return (((this))->IsActive( figattr_ArrowSize) ? ((this)->arrowsize) : ((def)->arrowsize)); }

      inline void SetArrowVal(long val)  { ((this)->arrow = (val)); }
      inline long   GetArrowVal()  { return ((this)->arrow); }
      inline void SetArrow(long val)  { ((this)->arrow = (val), (this)->active |= ((unsigned long)1<<figattr_Arrow)); }
      inline long GetArrow(class figattr *def)  { return (((this))->IsActive( figattr_Arrow) ? ((this)->arrow) : ((def)->arrow)); }

      inline void SetLineStyleVal(long val)  { ((this)->linestyle = (val)); }
      inline long   GetLineStyleVal()  { return ((this)->linestyle); }
      inline void SetLineStyle(long val)  { ((this)->linestyle = (val), (this)->active |= ((unsigned long)1<<figattr_LineStyle)); }
      inline long GetLineStyle(class figattr *def)  { return (((this))->IsActive( figattr_LineStyle) ? ((this)->linestyle) : ((def)->linestyle)); }

    
      unsigned long active;  /* bits as defined above */

      long shade; 
      long linewidth;
      char *color;
      long rrectcorner;
      long fontsize;
      long fontstyle;
      char *fontfamily;
      long textpos;
      long arrowpos;
      long arrowsize;
      long arrow;
      long linestyle;
};
/** @} */
#endif /* _figattr_H_ */
