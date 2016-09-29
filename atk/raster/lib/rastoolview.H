#ifndef _rastoolview_H_
#define _rastoolview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libraster
 * @{ */
#define	ZRTOOLS_NUM	(17)
#define	ZRCMDS_NUM	(6)
#define ZRPASTEMODES_NUM (4)
#define	ZRPATTERNS_NUM	(16)
#define	ZRBRUSHES_NUM	(14)

#define	RASTOOL_PAN	(1)
#define	RASTOOL_SELECT	(2)
#define RASTOOL_TOUCHUP (3)

#define	PASTEMODE_COPY	(1)
#define	PASTEMODE_OR	(2)
#define	PASTEMODE_XOR	(3)

struct span {
    long x, y;
    struct span *next;
};

#include <lpair.H>

typedef void (*rastoolview_toolfptr)(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
typedef void (*rastoolview_toolmodfptr)(class rastoolview *self, char *rock);
typedef void (*rastoolview_procfptr)(class rastoolview *self, char *rock);

class rastoolview  : public  lpair {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
 
      
	rastoolview();
      
	virtual ~rastoolview();
    
       void ObservedChanged(class observable *observed, long status);
       void UnlinkTree();
       void PostMenus(class menulist *ml);
       void PostKeyState(class keystate *ks);
    
      virtual boolean SetPrimaryView(class rasterview *zrview)  ;
      virtual boolean WantSelectionHighlighted()  ;
    
      inline class rasterview  * GetPrimaryView() { return ((this)->primaryview); }
      inline int   GetToolnum() { return ((this)->toolnum); }
      inline long   GetPasteMode() { return ((this)->pastemode); }
      inline rastoolview_toolfptr GetToolProc() { return ((this)->toolproc); }
      inline void SetMoribund(boolean val) { ((this)->moribund = (val)); }
    
      short moribund;
      class menulist *Menus;
      class keystate *Keystate;
      class stringtbl *patterntbl;
      class strtblview *vpatterntbl;
      short patternacc[ZRPATTERNS_NUM];
      class stringtbl *brushtbl;
      class strtblview *vbrushtbl;
      short brushacc[ZRBRUSHES_NUM];
      class stringtbl *commandtbl;
      class strtblview *vcommandtbl;
      short commandacc[ZRCMDS_NUM];
      class stringtbl *pastemodetbl;
      class strtblview *vpastemodetbl;
      short pastemodeacc[ZRPASTEMODES_NUM];
      class stringtbl *tooltbl;
      class strtblview *vtooltbl;
      short toolacc[ZRTOOLS_NUM];
      class lpair *lpair1, *lpair2, *lpair3;
      int patternnum;
      const unsigned char *pattern;
      int brushnum;
      const unsigned char *brush;
      long pastemode;
      int pastemodenum;

      int toolnum;
      rastoolview_toolfptr toolproc;
      class rasterview *primaryview;
      long rockx, rocky, rockw, rockh, rock;
      double lastvx, lastvy;
      long lastx, lasty;
      int sprayradius;
      double springconst;
      int color;
      int fillbit;
      struct span *fillstack;
      class rasterimage *fillpix;
      class raster *pasteraster, *unpasteraster;
      boolean unpaste;
      class dataobject *primaryobj;
};
/** @} */
#endif /* _rastoolview_H_ */
