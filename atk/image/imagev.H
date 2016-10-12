#ifndef _imagev_H_
#define _imagev_H_
/* imagev.H - class description for view on images */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

/** \addtogroup libimagev Object imagev
 *  A dynamically loadable ATK class file.  Contains imagev.
 * @{ */
#include <rect.h>
 
#define imagev_Grain (64)
#define imagev_NormScale (8)
#define imagev_ImagesPerPix (imagev_Grain/imagev_NormScale)
/* image-units per pixel at normal scale */
#define imagev_PURE_POSTSCRIPT (-1) /* When this is passed to writePS as the toplevel arg, we're just writing pure postscript. */
#define imagev_REGISTERED_POSTSCRIPT (-2) /* When this is passed to writePS as the toplevel arg, we're writing pure postscript but with the new printing mechanism. */

#include <scroll.H>
#include <scrolling.H>
#include <ddimage.H>

class imagev;
class imagevInterface : public ScrollInterface {
  public:
    imagevInterface(class imagev *iva) : iv(iva) {
    }	
    ~imagevInterface() {
    }
    void Absolute(long totalx, long x, long totaly, long y);
    void ScreenDelta(long dx, long dy);
    void Shift(scroll_Direction dir);
    void Extreme(scroll_Direction dir);
    void UpdateRegions(class scroll &scrollview);

    class imagev *iv;
};

#include <view.H>
class imagev : public  view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();


    imagev();
    virtual ~imagev();


    class view * Hit( enum view::MouseAction action, long x, long y, long numberOfClicks )  ;
    view::DSattributes DesiredSize( long width, long height, enum view::DSpass pass, long *dWidth, long *dheight )  ;
    void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
    void PostMenus( class menulist *menulist )  ;
    void PostKeyState( class keystate *ks )  ;
    void ObservedChanged( class observable *changed, long value )  ;
    void Update()  ;
    void ReceiveInputFocus();
    void LoseInputFocus();
    void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );
    void ReceiveColormap(class colormap *cmap);
    void LoseColormap(class colormap *cmap);
    void UnlinkNotification(class view *unlinkedTree);
    void LinkTree(class view *parent);
    class view * GetApplicationLayer()  ;
    void DeleteApplicationLayer(class view *scrollbar);
    void SetDataObject( class dataobject *image);
    void PrintPSDoc(FILE *outfile, long pagew, long pageh);
    void *GetPSPrintInterface(const char *printtype);
    void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);
    void DesiredPrintSize(long width, long height, enum view::DSpass pass, long *desiredwidth, long *desiredheight);

    virtual boolean Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect);

    ScrollInterface *GetScrollInterface();
 
    inline long ToPrintPixX(long val)  { return (((val)-(this)->originx)/imagev_ImagesPerPix); }
    inline long ToPrintPixY(long val)  { return (((val)-(this)->originy)/imagev_ImagesPerPix); }
    inline long ToPrintPixW(long val)  { return ((val)/imagev_ImagesPerPix); }
    inline long ToPrintPixH(long val)  { return ((val)/imagev_ImagesPerPix); }

    inline class dataobject *GetRealDataObject() { return ((this)->orig); }

    class image *orig, *scaled;
    class cursor *cursor;    
    boolean cursorPosted;
    int cursorMode;
    class menulist *menulistp;
    class keystate *keystatep;
    boolean do_fullupdate, do_renderupdate;
    boolean image_modified;
    struct sbutton_prefs *buttonprefs;
    boolean haveFocus, isLinked, onScreen;
    struct rectangle *canvas;
    boolean embedded, havePrivateCmap;
    long scale;
    long panx, pany;
    long originx, originy;
    long rockx, rocky;
    long lastx, lasty;
    int bordersize;
    int panStyle;
    struct rectangle *lastPixRect;
    class colormap *privateCmap;
    boolean click_to_draw, click_skip_update;
    ddimage dimage;
};
/** @} */
#endif /* _imagev_H_ */
