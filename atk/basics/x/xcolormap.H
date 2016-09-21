#ifndef _xcolormap_H_
#define _xcolormap_H_
ATK_INTER
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <color.H>
#include <image.H>
#include <xcolor.H>
#include <ahash.H>
#include <event.H>

#define xcolormap_NEWMANAGER 100
#define xcolormap_NEWIMAGECOLORS 101
#define xcolormap_LOSTMANAGER 102

class XCMapImageRGB {
  public:
    unsigned short r, g, b;
    inline XCMapImageRGB() {
	r=g=b=0;
    }
    inline XCMapImageRGB(const XCMapImageRGB &src) {
	r=src.r;
	g=src.g;
	b=src.b;
    }
    inline XCMapImageRGB &operator=(const XCMapImageRGB &src) {
	r=src.r;
	g=src.g;
	b=src.b;
	return *this;
    }
    inline XCMapImageRGB(unsigned short red, unsigned short green, unsigned short blue) {
	r=red;
	g=green;
	b=blue;
    }
    inline ~XCMapImageRGB() {
    }
    inline size_t Hash() const {
	return (r<<16)^(g<<8)^b;
    }
    inline boolean operator==(const XCMapImageRGB &rhs) {
	return rhs.r==r && rhs.g==g && rhs.b==b;
    }
};

#define keys dummykeys
DEFINE_AMHASH_CLASS(XCMapImageHist,XCMapImageRGB,unsigned int);
DEFINE_AHASH_CLASS_ITER(XCMapImageHist,XCMapImageRGB,unsigned int);
DEFINE_MFLEX_CLASS(XCMapColorReq,CARD16,5);
#undef keys

enum xcolormap_ManagerStart {
    xcolormap_AutoStart,
    xcolormap_PromptStart,
    xcolormap_ManualStart
};

enum xcolormap_ManagerState {
    xcolormap_Dead,
    xcolormap_Starting,
    xcolormap_Running,
    xcolormap_DelayedStart
};

struct xcolormap_pixelmap {
    unsigned int redcolors, greencolors, bluecolors;
    int firstredbit, firstgreenbit, firstbluebit;
    unsigned long red_mask, green_mask, blue_mask;
    unsigned int bits_per_pixel;
};

struct xcolormap_cube {
    xcolor *cube[6][6][6];
    unsigned short samplesPerComponent;
};

class xcolormap  : public  colormap  {
  public:
    virtual ATKregistryEntry *ATKregistry();
    xcolormap(Display *dpy=NULL, Colormap def=None);
    virtual ~xcolormap();
    virtual ddcolor *Alloc(const atom *name);
    virtual ddcolor *Alloc(unsigned short r, unsigned short g, unsigned short b);
    virtual ddcolor *AllocExact(unsigned short r, unsigned short g, unsigned short b, boolean exact);
    ddcolor *Find(unsigned short r, unsigned short g, unsigned short b);
    ddcolor *Find(const atom *name); 
    xcolor *FindClosest(unsigned short r, unsigned short g, unsigned short b);
 // not supported yet.   virtual int ChangeColor(color *xc);
    virtual void ObservedChanged(observable *changed, long value);
    void InitializeColorCube();

    inline unsigned long CubePixel(unsigned short r, unsigned short g, unsigned short b) {
	unsigned long result=0;
	if(colorCube==NULL && !map_info  && rampsamples==0) InitializeColorCube();
#define SCALE(r) ((((r)+32767/(colorCube->samplesPerComponent-1))*(colorCube->samplesPerComponent-1))/(65535))
	if(map_info) {
	    result=((map_info)->base_pixel + 
		    (((r>>8) * (map_info)->red_max + (1<<(8-1))) >> 8) * 
		    (map_info)->red_mult + 
		    (((g>>8) * (map_info)->green_max + (1<<(8-1))) >> 8) * 
		    (map_info)->green_mult + 
		    (((b>>8) * (map_info)->blue_max + (1<<(8-1))) >> 8) * 
		    (map_info)->blue_mult);
	} else if(colorCube && colorCube->cube[SCALE(r)][SCALE(g)][SCALE(b)]) {
	    result=
	      colorCube->cube[SCALE(r)][SCALE(g)][SCALE(b)]->tcolor.pixel;
	} else if(rampsamples && redramp && greenramp && blueramp) {
	    unsigned long maxsample=rampsamples-1;
	    result=
	      redramp[r*maxsample/65535] |
	      greenramp[g*maxsample/65535] |
	      blueramp[b*maxsample/65535];
	}
	return result;
#undef SCALE
    }
    // Used to return a the pixel for a particular image color when using a color cube.
    inline Display *XDisplay() {
	return display;
    }
    inline Colormap XColormap() {
	return XColorMap;
    }
    static void ExplodePixelMasks(xcolormap_pixelmap *pm, unsigned long red_mask, unsigned long green_mask, unsigned long blue_mask);
    // Breaks out the information encoded in the red_mask, green_mask, and blue_mask
    // of an X Visual structure into the xcolormap_pixelmap structure.
    // The information in this structure is useful in mapping color values to pixel
    // values for TrueColor displays, or component colormap indicies to pixel values
    // when using a DirectColor display.  At the present time this function is not used.
    Colormap XColorMap;
    Display *display;
    
    struct xcolormap_cube *colorCube;
    // When using a colorcube on a non-TrueColor, non-DirectColor display this
    // structure represents the colorCube in use, if any.
    XStandardColormap *map_info;
    // If a standard colormap was found and is in use, this will be non-NULL.

    unsigned long *redramp, *greenramp, *blueramp;
    // The red, green, and blue pixel components for color cube intensities. (True&DirectColor only)
    unsigned int rampsamples;
    //  The number of color cube intensities.  (True&DirectColor only)
    xcolor **pixels;
    // The color objects used for the color cube on TrueColor & DirectColor displays.
    
    Window colorprotocol;
    // The window used in selection requests to the color manager.  (for PseudoColor displays)
    Window manager;
    // The window the manager uses to signal when it has exited.
    XCMapImageHist *allocs, *deallocs;
    // The histogram of colors used by images in this colormap.  (for PseudoColor displays)
    XCMapColorReq *allocreq, *deallocreq;
    // The arrays used to temporarily hold allocation and deallocation requests during
    // transfer to the color manager.
    boolean evaluate;
    // True iff the color manager should be asked to evaluate all changes as soon as
    // the allocation and deallocation requests complete.
    boolean pending;
    // True iff there are requests pending on the color manager's response.
    CARD32 *imagepixels;
    unsigned short *imagecolors;
    size_t imagecolorcount, imagepixelcount;
    //  The last set of colors returned by the color manager.  (for PseudoColor displays)
    xcolormap_ManagerStart startmanager;
    xcolormap_ManagerState state;
    event *delayedrestart;
    pid_t managerpid;
    // Indicates when/how the color manager should be started. see xcolormap_ManagerStart
    // enum definition above.
    Atom *AtomCache;
    // The X atoms used to communicate with the color manager.
    Time timestamp;
    // The timestamp used to make requests of the color manager.
    // This timestamp comes from the ClientMessage event
    // announcing the arrival of a new color manager, or from the
    // property change event caused by a zero length append on the
    // colorprotocol window
    boolean AllocateImageColors(XCMapImageHist &hist);
    // Sends a request to the color manager for the colors used in the image.  (for PseudoColor displays)
    boolean FreeImageColors(XCMapImageHist &hist);
    // Tells the color manager it may release the colors used by the image.
    boolean Evaluate();
    // Tells the color manager to evaluate any outstanding changes.
    CARD32 *MapImageColors(RGBMap *src);
    // Returns an array of the pixels for the  colors from the allocated_image_colors
    // which most closely match those in the src RGBMap.  (for PseudoColor displays)
    RGBMap *MapImageColorsToMap(RGBMap *src);
    // Returns a new RGBMap with the closest available colors from the allocated_image_colors.
    
    inline boolean ColorProtocolEvent(XEvent *e) {
	return (e->xany.window==manager || e->xany.window==colorprotocol || e->xany.window==RootWindow(e->xany.display, DefaultScreen(e->xany.display)));
    }
    // Returns TRUE, iff the event is potentially relevant to the color protocol, otherwise FALSE.

    inline boolean UsingColorManager() {
	if(state!=xcolormap_Dead) return TRUE;
	else return FALSE;
    }
    
    void HandleXEvent(XEvent *e);
    // Called by xim when an event arrives related to communication
    // with the color manager.

    boolean evalpending;
};
/** @} */
#endif /* _xcolormap_H_ */
