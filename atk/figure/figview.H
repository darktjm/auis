ATK_INTER
#ifndef _figview_H_
#define _figview_H_
/* figv.ch - drawing object view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figview.H,v 3.8 1995/02/28 23:37:46 Zarf Stab74 $
*/

#include <rect.h>
 
#include <view.H>
#include <figobj.H>

#define figview_Grain (64)
#define figview_NormScale (8)
#define figview_FigUPerPix (figview_Grain/figview_NormScale)
/* fig-units per pixel at normal scale */

#define figview_SpotRad (2)
#define figview_AnchRad (4)
/* distance (in pixels) one must move mouse to register a drag */
#define figview_MouseHysteresis (4)

#define figview_SelectDelta (2)

#define figview_OpCopy (1)
#define figview_OpCopyInset (2)
#define figview_OpCut (3)
#define figview_OpPaste (4)
#define figview_OpPasteRotate (5)

struct figv_oref {
    class figobj *o;
    struct rectangle vbox, vselbox;
    boolean ignorevbox;
    boolean selected;
    short drawnselected;
    boolean knownselected;
    boolean selectdamaged;
    class view *insetv;
    class dataobject *inseto;
    struct rectangle insetb;
    boolean insetbmoved;
    boolean wantupdate;
    long timestamp;
};

struct figv_highlight {
    struct rectangle r, old;
    boolean changed, oldon;
    boolean focgone;
};

struct figv_redraw_item {
    long oref;
    long clip;
};


typedef void (*figview_esfptr)(class figobj *this_c, long ix, class figview *self, long rock);

class figview  : public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();

      static boolean InitializeClass();

      figview();

      virtual ~figview();


      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
      void Update();
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *desiredWidth, long *desiredHeight)  ; 
      class view * Hit(enum view_MouseAction action, long x, long y, long n)	 ;
      void WantUpdate(class view *requestor);
      void ReceiveInputFocus();
      void LoseInputFocus();
      void ObservedChanged(class observable *dobj, long status);
      void Print(FILE *file, char *processor, char *finalFormat, boolean toplevel);
      class view * GetApplicationLayer()  ;
      const void * GetInterface(const char *interface_name)  ;
      void PostMenus(class menulist *ml);
      void PostKeyState(class keystate *ks);
      void SetDataObject(class dataobject *dobj);
      void LinkTree(class view *parent);
      void ExposeChild(class view *v);
      boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
      boolean RecSrchResume(struct SearchPattern *pat);
      boolean RecSrchReplace(class dataobject *text, long pos, long len);
      void RecSrchExpose(const struct rectangle &logica, struct rectangle &hit);
      void PrintPSDoc(FILE *outfile, long pagew, long pageh);
      void *GetPSPrintInterface(char *printtype);
      void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);
      void DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight);


      virtual void BuildToolList(struct figtoolview_tool_t *&list, int &listnum, int &listsize);
      virtual void DestroyToolset();
      virtual void FlushDataChanges();
      virtual void ChangeZoom(long dir);
      virtual void ClearSelection();
      virtual void Select(class figobj *o);
      virtual void SelectByRef(long ref);
      virtual void ToggleSelect(class figobj *o);
      virtual void ToggleSelectByRef(long ref);
      virtual void Unselect(class figobj *o);
      virtual void UnselectByRef(long ref);
      virtual boolean IsSelected(long ref)  ;
      virtual void EnumerateSelection(figview_esfptr func, long rock);
      virtual long GetOneSelected()  ;
      virtual void SetFocusByRef(long ref);
      virtual void SetExpertMode(boolean val);
      virtual void SetBuildKeystate(class keystate *ks);
      virtual void SetNumHighlights(int num);
      virtual void BlockUpdates(boolean val);
      virtual void CutNPaste(short operation, long rock);


      inline long   ToPixX(long val)  { return ((((val))*(this)->scale*(this)->GetHorizontalResolution())/(figview_Grain*72)-(this)->ppanx); }
      inline long   ToPixY(long val)  { return ((((val))*(this)->scale*(this)->GetVerticalResolution())/(figview_Grain*72)-(this)->ppany); }
      inline long ToPixW(long val)  { return (((val)*(this)->scale*(this)->GetHorizontalResolution())/(figview_Grain*72)); }
      inline long ToPixH(long val)  { return (((val)*(this)->scale*(this)->GetVerticalResolution())/(figview_Grain*72)); }
      inline long ToFigX(long val)  { return (((((val)+(this)->ppanx)*figview_Grain*72)/((this)->scale*(this)->GetHorizontalResolution()))); }
      inline long ToFigY(long val)  { return (((((val)+(this)->ppany)*figview_Grain*72)/((this)->scale*(this)->GetVerticalResolution()))); }
      inline long ToFigW(long val)   { return (((val)*figview_Grain*72)/((this)->scale*(this)->GetHorizontalResolution())); }
      inline long ToFigH(long val)   { return (((val)*figview_Grain*72)/((this)->scale*(this)->GetVerticalResolution())); }
      inline long ToPrintPixX(long val)  { return (((val)-(this)->originx)/figview_FigUPerPix); }
      inline long ToPrintPixY(long val)  { return (((val)-(this)->originy)/figview_FigUPerPix); }
      inline long ToPrintPixW(long val)  { return ((val)/figview_FigUPerPix); }
      inline long ToPrintPixH(long val)  { return ((val)/figview_FigUPerPix); }
      inline long FromPrintPixX(long val)  { return ((val)*figview_FigUPerPix) + (this)->originx; }
      inline long FromPrintPixY(long val)  { return ((val)*figview_FigUPerPix) + (this)->originy; }
      inline long FromPrintPixW(long val)  { return ((val)*figview_FigUPerPix); }
      inline long FromPrintPixH(long val)  { return ((val)*figview_FigUPerPix); }
      inline long   ToDefFigX(long val)  { return (((((val))*figview_FigUPerPix*72)/((this)->GetHorizontalResolution()))+(this)->originx); }
      inline long   ToDefFigY(long val)  { return (((((val))*figview_FigUPerPix*72)/((this)->GetVerticalResolution()))+(this)->originy); }
      inline long ToDefFigW(long val)   { return (((val)*figview_FigUPerPix*72)/((this)->GetHorizontalResolution())); }
      inline long ToDefFigH(long val)   { return (((val)*figview_FigUPerPix*72)/((this)->GetVerticalResolution())); }


      inline long   GetNumSelected()  { return ((this)->numselected); }
      inline long   GetFocusRef()  { return ((this)->focusgroup); }
      inline void SetFocusChangeFlag(boolean val)  { ((this)->focuschange = (val)); }
      inline int   GetNumHighlights()  { return ((this)->numhighlights); }
      inline void SetHighlightSize(int val, long x, long y, long w, long h)  { ((this)->highlights[val].changed = TRUE, rectangle_SetRectSize(&((this)->highlights[val].r), (x), (y), (w), (h))); }
      inline void SetHighlight(int val, struct rectangle *rr)  { ((this)->highlights[val].changed = TRUE, (this)->highlights[val].r = (*(rr))); }
      inline struct rectangle *GetHighlight(int val)  { return (&((this)->highlights[val].r)); }
      inline void SetPrintRect(struct rectangle *rect)  { ((this)->PrintRect = (rect)); }
      inline class region  * GetCurrentClipRegion()  { return ((this)->currentclipreg); }
      inline struct rectangle   *GetClippedUpdateRect() { return (&(this)->ClippedUpdateRect); }

    
      class figtoolview *toolset;

      boolean expertmode;
      boolean focuschange;

      struct figv_oref *objs;
      long objs_size;

      long numselected;

      int numhighlights;
      struct figv_highlight *highlights;
      long focusgroup;
      long focussib;

      struct rectangle UpdateRect, MustEraseRect;

      long originx, originy;
      long panx, pany;
      long scale;

      long *tmplist;
      long tmp_size;
      long tmpnum;

      class region **clipreglist;
      class region *tmpregion, *currentclipreg;
      long clipreg_size;

      struct figv_redraw_item *redrawlist;
      long redraw_size;
      long redrawnum;

      long lastx, lasty, rockx, rocky;
      long lastpaste[2], lastpasteoffset;
      long lastupdater;

      class menulist *Menus;
      class keystate *Keystate;
      class keystate *BuildKeystate;
      const char *ForegroundColor, *BackgroundColor, *FigBackColor;

      boolean OnScreen;		/* if not view_Removed */
      boolean embedded;		/* TRUE if no call to GetApplicationLayer */
      boolean HasInputFocus;	/* T if received input focus */
      boolean InputFocusClick;
      boolean ignoreUp;		/* T iff have just asked for InputFocus */
      boolean NeedFullUpdate;
      boolean DoingFullUpdate;
      boolean UpdateCached;
      boolean UpdatesBlocked;
      boolean ShowFocusAttachments;
      boolean ShowPrintArea;

      char *PSFileName;
      struct rectangle *PrintRect;
      long lastwidth, lastheight;
      class region *figureclip;   /* used only if embedded is TRUE */ 
      long ppanx, ppany;
      struct rectangle ClippedUpdateRect;

      long recsearchpos;		/* position of last RecSearch */
      boolean recsearchvalid;		/* was last RecSearch successful? */
      class view *recsearchchild;		/* child in which last RecSearch found something */
};


#endif /* _figview_H_ */
