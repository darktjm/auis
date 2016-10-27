/* Copyright 1995 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("xddimage.H")

#include <xddimage.H>
#include <im.H>
#include <windowsystem.H>
#include <xcolormap.H>
#include <xgraphic.H>
#include <xim.H>
#include <environ.H>
#include <event.H>
#include <workproc.H>
#include <physical.h>

class xddimage_draw : public WorkProc {
  public:
    TRACED_CLASS(xddimage_draw);
    WorkProc_Status Process();
    long left, top, width, height;
    long destLeft, destTop;
    int x, dx;
    int y, dy;
    xddimage *src;
    GC gc;
    Display *dpy;
    xddimage_draw(xddimage *src=NULL, long left=0, long top=0, long width=0, long height=0, long destLeft=0, long destTop=0);
    void ObservedChanged(observable *changed, long change);
};

xddimage_draw::xddimage_draw(xddimage *s, long l, long t, long dl, long dt, long w, long h) {
    src=s;
    left=l;
    top=t;
    width=w;
    height=h;
    destLeft=dl;
    destTop=dt;
    x=left;
    dx=destLeft;
    y=top;
    dy=destTop;
    gc=NULL;
    if(src==NULL) Stop();
    else {
	src->AddObserver(this);
	AddObserver(src);
    }	    
}

xddimage_draw::~xddimage_draw() {
    if(src) src->RemoveObserver(this);
}

void xddimage_draw::ObservedChanged(observable *changed, long change) {
    if(change==observable::OBJECTDESTROYED) {
	RemoveObserver(changed);
	if(changed==src) src=NULL;
    }
}

ATKdefineRegistryNoInit(xddimage, iddimage);
ATKdefineRegistryNoInit(xddimage_draw, WorkProc);

static int pixelsper=-1;

WorkProc_Status xddimage_draw::Process() {
    if(pixelsper<0) {
	pixelsper=environ::GetProfileInt("PixelsPerTransfer", 50);
    }
    if(src && src->ximage && src->xgd) {
	unsigned int h=(pixelsper/width);
	unsigned int w=width;
	SetInterval(event_MSECtoTU(100));
	h=MIN(h,height);
	if(h<1) {
	    h=1;
	    w=MIN(width-x+left,pixelsper);
	}
	XPutImage(src->xgd->XDisplay(), src->xgd->XWindow(), src->xgd->XGC(), src->ximage, x, y, (int)physical::LogicalXToGlobalX(src->xgd, dx), (int)physical::LogicalYToGlobalY(src->xgd, dy), (unsigned int)w, (unsigned int)h);
	x+=w;
	dx+=w;
	if(x>=left+width) {
	    x=left;
	    dx=destLeft;
	    y+=h;
	    dy+=h;
	    height-=h;
	}
	return height>0?WorkProc_Working:WorkProc_Done;
    } else return WorkProc_Done;
}

static void drelease(void *pd, long) {
    xddimage *xdd=(xddimage *)pd;
    xdd->Release(FALSE);
    xdd->delayedrelease=NULL;
}

void xddimage::Release(boolean delayok) {
    if(delayedrelease) {
	delayedrelease->Cancel();
	delayedrelease=NULL;
    }
    if(released) {
	return;
    }
    if(delayok) {
	delayedrelease=im::EnqueueEvent(drelease, (char *)this, 0);
	if(delayedrelease) return;
    }
    released=TRUE;
    ReleaseResources(delayok);
    ReleaseData(delayok);
}

static void deval(void *pd, long) {
    xddimage *xdd=(xddimage *)pd;
    xdd->delayedeval=NULL;
    if(xdd->cmap==NULL || *(xdd->cmap)==NULL) return;
    xcolormap *xcmap=*(xcolormap **)xdd->cmap;
    xcmap->Evaluate();
}

void xddimage::ReleaseResources(boolean delayok) {
    waiting=FALSE;
    if(draw) {
	draw->Destroy();
	draw=NULL;
    }
    if(xgd) xgd->RemoveObserver(this);
    if(cmap && *cmap && allocated.GetN()>0) {
	for(size_t i=0;i<allocated.GetN();i++) {
	    (*cmap)->Free(allocated[i]);
	}
	allocated.Remove((size_t)0, allocated.GetN());
    }
    if(histogram && cmap && *cmap!=NULL) {
	xcolormap *xcmap=*(xcolormap **)cmap;
	if(xcmap) {
	    if(xcmap->FreeImageColors(*histogram)) {
		if(delayok) {
		    if(delayedeval) delayedeval->Cancel();
		    delayedeval=im::EnqueueEvent(deval, (char *)this, 0);
		} else xcmap->Evaluate();
	    }
	}
    }
    delete histogram;
    histogram=NULL;
    outdated=TRUE;
}

void xddimage::ReleaseData(boolean /* delayok */) {
    if(ximage) {
	XDestroyImage(ximage);
	ximage=NULL;
    }
    if(tsrc) {
	tsrc->Destroy();
	tsrc=NULL;
    }
    if(pixmap!=None) {
	if(xgd) XFreePixmap(xgd->XDisplay(), pixmap);
	else {
	    fprintf(stderr, "xddimage: WARNING: couldn't free pixmap... No xgraphic available...\n");
	}
	pixmap=None;
	pixok=FALSE;
    }
    outdated=TRUE;
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
 */

static unsigned int 
bitsPerPixelAtDepth(Display       *disp, int           /*  scrn */, unsigned int depth)
               {
#ifdef PRE_X11R4_ENV
  unsigned int a;

  for (a= 0; a < disp->nformats; a++)
    if (disp->pixmap_format[a].depth == depth)
      return(disp->pixmap_format[a].bits_per_pixel);

#else /* the way things should be */
  XPixmapFormatValues *xf;
  int nxf, a;

  xf = XListPixmapFormats(disp, &nxf);
  for (a = 0; a < nxf; a++)
      if (xf[a].depth == (int)depth) {
	  unsigned int bpp=xf[a].bits_per_pixel;
	  XFree((char *)xf);
	  return bpp;
      }
  XFree((char *)xf);
#endif

  /* this should never happen; if it does, we're in trouble
   */

  fprintf(stderr, "xddimage: bitsPerPixelAtDepth: Can't find pixmap depth info!\n");
  return 1;
}

static int UseColorManager=(-1);
static int UseColorCube=(-1);

void xddimage::ProcessTrueOrDirectColor(graphic * /* dest */) {
}

void xddimage::ProcessWithStaticColors(graphic * /* dest */) {
}

void xddimage::ProcessWithColorCube(graphic * /* dest */) {
}

void xddimage::ProcessWithExactColors(graphic * /* dest */) {
}

void xddimage::ReProcess() {
    if(xgd==NULL || cmap==NULL || *cmap==NULL || outdated) return;
    image *src=tsrc?tsrc:source;
    xcolormap *xcmap=*(xcolormap **)cmap;
    if(xcmap==NULL) return;
    CARD32 *indx=NULL;
    if(xcmap==NULL) return;
    Display *disp;
    int scrn;
    Visual *visual; /* visual to use */

    disp = xgd->XDisplay();
    scrn = DefaultScreen(disp);

    visual = DefaultVisual(disp, scrn);
    static boolean DitherToCustomColors=environ::GetProfileSwitch( "DitherToCustomColors", FALSE);
    if(DitherToCustomColors && src->type!=IBITMAP) {
	RGBMap *rgb=xcmap->MapImageColorsToMap(src->rgb);
	if(rgb) {
	    src=src->ColorDither(rgb);
	    if(tsrc) {
		tsrc->Destroy();
		tsrc=NULL;
	    }
	    tsrc=src;
	}
    }
    indx=xcmap->MapImageColors(src->rgb);
    if(indx==NULL) {
	if(tsrc) {
	    tsrc->Destroy();
	    tsrc=NULL;
	}
	fprintf(stderr, "xddimage: ReProcess failed without color mapping from manager.\n");
	return;
    }

    unsigned int x, y, a;
    unsigned int linelen, dpixlen, dbits, ddepth;
    
    ddepth = DefaultDepth(disp, scrn);


    
    switch(src->type) {
	case IGREYSCALE:
	case IRGB:
	    /* modify image data to match visual and colormap
	     */

	    dbits = bitsPerPixelAtDepth(disp, scrn, ddepth);
	    dpixlen= (dbits + 7) / 8;

	    /* if our XImage doesn't have modulus 8 bits per pixel, it's unclear
	     * how to pack bits so we instead use an XYPixmap image.  this is
	     * slower.
	     */

	    if (dbits % 8) {
		byte *data, *destdata, *destptr, *srcptr, mask;
		Pixel pixmask, pixval;

		ximage = XCreateImage(disp, visual, ddepth, XYPixmap, 0, NULL, src->width, src->height, 8, 0);
		data = (byte *)malloc(src->width * src->height * dpixlen);
		ximage->data = (char *)data;
		memset(data, 0, src->width * src->height * dpixlen);
		ximage->bitmap_bit_order = MSBFirst;
		ximage->byte_order = MSBFirst;
		linelen = (src->width + 7) / 8;
		for (a = 0; a < dbits; a++) {
		    pixmask = 1 << a;
		    destdata = data + ((dbits - a - 1) * src->height * linelen);
		    srcptr = src->data;
		    for (y = 0; y < src->height; y++) {
			destptr = destdata + (y * linelen);
			*destptr = 0;
			mask = 0x80;
			for (x = 0; x < src->width; x++) {
			    pixval= memToVal(srcptr, src->pixlen);
			    srcptr += src->pixlen;
			    if (indx[pixval] & pixmask)
				*destptr |= mask;
			    mask >>= 1;
			    if (mask == 0) {
				mask = 0x80;
				destptr++;
			    }
			}
		    }
		}
	    }
	    else {
		byte *data, *srcptr, *destptr;

		ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0, NULL, src->width, src->height, 8, 0);
		dpixlen = (ximage->bits_per_pixel + 7) / 8;
		data = (byte*) malloc(src->width * src->height * dpixlen);
		ximage->data = (char *)data;
		ximage->byte_order = MSBFirst;
		srcptr = src->data;
		destptr = data;
		unsigned char *srcend=srcptr+src->height*src->width*src->pixlen;
		if(src->pixlen==1 && dpixlen==1) {
		    while(srcptr<srcend) {
			*destptr=(byte)indx[*srcptr];
			srcptr++;
			destptr++;
		    }
		} else {
		    while(srcptr<srcend) {
			valToMem(indx[memToVal(srcptr, src->pixlen)], destptr, dpixlen);
			srcptr += src->pixlen;
			destptr += dpixlen;
		    }
		}
	    }
	    break;
	case IBITMAP:
	case ITRUE:
	    fprintf(stderr, "xddimage: BOGUS: TRUECOLOR image encountered by ReProcess!\n");
	    break;
    }
    if(tsrc) {
	tsrc->Destroy();
	tsrc=NULL;
    }
    free(indx);
    waiting=FALSE;
    outdated=FALSE;
    released=FALSE;
}

static inline void ReferenceColor(XCMapImageHist &hist, XCMapImageRGB &key, unsigned int nc) {
    unsigned int *count=hist.Fetch(key);
    if(count) (*count)+=nc;
    else hist.Insert(key, nc);
}

void xddimage::ProcessWithColorManager(graphic *dest) {
    xcolormap *xcmap=*(xcolormap **)cmap;
    if(xcmap==NULL) return;

    image *src=source;
    byte *srcptr = src->data;
    Pixel pixval;

    switch (src->type) {
	case IBITMAP:
	    fprintf(stderr, "xddimage: BOGUS: BITMAP image encountered in ProcessWithColorManager.\n");
	    return;
	case ITRUE:
	   /* for (y = 0; y < realsrc->height; y++)
		for (x = 0; x < realsrc->width; x++) {
		    pixval = memToVal(srcptr, realsrc->pixlen);
		    XCMapImageRGB key(TRUE_RED(pixval), TRUE_GREEN(pixval), TRUE_BLUE(pixval));
		    ReferenceColor(hist, key);
		    srcptr += realsrc->pixlen;
	    } */
	    tsrc=src=source->Reduce(1024);
	case IGREYSCALE:
	case IRGB: {
	    delete histogram;
	    histogram=new XCMapImageHist();
	    unsigned int *pixhist=new unsigned int[1<<(8*src->pixlen)];
	    memset(pixhist, 0, sizeof(unsigned int)*(1<<(8*src->pixlen)));
	    byte *end=srcptr+src->height*src->width*src->pixlen;
	    if(src->pixlen==1) {
		while(srcptr<end) {
		    pixhist[*srcptr]++;
		    srcptr++;
		}
	    } else {
		while(srcptr<end) {
		    pixval=memToVal(srcptr, src->pixlen);
		    pixhist[pixval]++;
		    srcptr += src->pixlen;
		}
	    }
	    unsigned int i;
	    for(i=0;i<src->rgb->used;i++) {
		XCMapImageRGB key(src->rgb->red[i], src->rgb->green[i], src->rgb->blue[i]);
		ReferenceColor(*histogram, key, pixhist[i]);
	    }
	    delete [] pixhist;
	    }
	    break;
	default: /* something's broken */
	    return;
    }
    waiting=TRUE;
    if(xcmap->AllocateImageColors(*histogram)) {
	if(delayedeval) delayedeval->Cancel();
	delayedeval=im::EnqueueEvent(deval, (char *)this, 0);
    }
}

boolean xddimage::Process(graphic *dest) {
    if(cmap==NULL) return FALSE;
    xcolormap *xcmap=*(xcolormap **)cmap;
    Colormap map=xcmap->XColormap();
    xgraphic *xg=(xgraphic *)dest;
    if(xcmap==NULL) return FALSE;
    Display *disp;
    int scrn;
    Visual *visual; /* visual to use */
    Pixel *redvalue, *greenvalue, *bluevalue;
    unsigned int linelen, dpixlen, dbits;
    unsigned int x, y, a, b;
    unsigned short red, green, blue;
    class xcolor *xc;
    unsigned int ddepth; /* depth of the visual to use */
    boolean fit=FALSE;

    disp = xg->XDisplay();
    scrn = DefaultScreen(disp);

    visual = DefaultVisual(disp, scrn);
    ddepth = DefaultDepth(disp, scrn);

    redvalue = greenvalue = bluevalue = NULL;

    image *realsrc=source;
    
    ReleaseResources(TRUE);
    ReleaseData(TRUE);
    xgd=xg;
    xg->AddObserver(this);

    boolean do_old_code=TRUE;

    if(realsrc->type!=IBITMAP) {
	switch(visual->c_class) {
	    case TrueColor:
	    case DirectColor:
		ProcessTrueOrDirectColor(dest);
		break;
	    case StaticColor:
	    case StaticGray:
		ProcessWithStaticColors(dest);
		break;
	    case PseudoColor:
	    case GrayScale:
		if(UseColorManager<0) {
		    UseColorManager=environ::GetProfileSwitch("UseColorManager", TRUE);
		}
		if(UseColorCube<0) {
		    UseColorCube=environ::GetProfileSwitch("UseColorCube", TRUE);
		}
		if(map==DefaultColormap(disp, scrn) && UseColorManager) {
		    ProcessWithColorManager(dest);
		    if(xcmap->UsingColorManager()) do_old_code=FALSE;
		} else if(UseColorCube) ProcessWithColorCube(dest);
		else ProcessWithExactColors(dest);
	}
    }
    if(do_old_code) {
	waiting=FALSE;
	if(UseColorCube) maxcolors=ddimage_MAXQUOTA;
	else if(map==DefaultColormap(disp, scrn)) {
	    maxcolors=MIN(quota, visual->map_entries);
	} else maxcolors=visual->map_entries;



	unsigned long *indx=NULL;
	int numColorsInIndex=0;

	/* process image based on type of visual we're sending to
	 */

	switch (realsrc->type) {
	    case ITRUE:
		switch (visual->c_class) {
		    case TrueColor:
		    case DirectColor:
			/* goody goody */
			break;
		    default:
			realsrc = new image;
			(realsrc)->newRGBImage(source->Width(), source->Height(), ddepth);
			source->Duplicate(realsrc);
			if (visual->bits_per_rgb > 1 &&  visual->map_entries > 2 && maxcolors>2) {
			    realsrc = (realsrc)->Reduce(maxcolors);
			} else {
			    realsrc = (realsrc)->Dither();
			}
		}
		break;
	    case IGREYSCALE:
	    case IRGB:
		switch(visual->c_class) {
		    case TrueColor:
		    case DirectColor:
			/* no problem, we handle this just fine */
			break;
		    default:
			realsrc = new image;
			(realsrc)->newRGBImage(source->Width(), source->Height(), ddepth);
			source->Duplicate(realsrc);
			if (visual->bits_per_rgb > 1 && visual->map_entries > 2 && maxcolors>2)
			    realsrc = (realsrc)->Reduce(maxcolors);
			else
			    realsrc = (realsrc)->Dither();
			break;
		}
	    case IBITMAP:
		/* no processing ever needs to be done for bitmaps */
		break;
	}

	/* do color allocation */
	if(visual->map_entries > 2) {
	    switch (visual->c_class) {
		case TrueColor:
		    {
		    struct xcolormap_pixelmap pm;
		    redvalue = (Pixel *) malloc(sizeof(Pixel) * 256);
		    greenvalue = (Pixel *) malloc(sizeof(Pixel) * 256);
		    bluevalue = (Pixel *) malloc(sizeof(Pixel) * 256);

		    /* calculate number of distinct colors in each band */

		    xcolormap::ExplodePixelMasks(&pm, visual->red_mask, visual->green_mask, visual->blue_mask);

		    /* sanity check
		     */

		    if( ((int)pm.redcolors > visual->map_entries) ||
			((int)pm.greencolors > visual->map_entries) ||
			((int)pm.bluecolors > visual->map_entries)) {
			// error...
		    }

		    for(a=0;a<256;a++) {
			redvalue[a]=((pm.redcolors*a)/256)<<pm.firstredbit;
			greenvalue[a]=((pm.greencolors*a)/256)<<pm.firstgreenbit;
			bluevalue[a]=((pm.bluecolors)*a/256)<<pm.firstbluebit;
		    }
		    }
		    break;

		default:
		    retry: /* this tag is used when retrying because we couldn't get a fit */
		      if(indx) free(indx);
		    indx = (Pixel *) malloc(sizeof(Pixel) * realsrc->rgb->used);

		    /* either create a new colormap or fit the image into the one we
		     * have.  to create a new one, we create a private cmap and allocate
		     * the colors writable.  fitting the colors is harder, we have to:
		     *  1. grab the server so no one can goof with the colormap.
		     *  2. count the available colors using XAllocColorCells.
		     *  3. free the colors we just allocated.
		     *  4. reduce the depth of the image to fit.
		     *  5. allocate the colors again shareable.
		     *  6. ungrab the server and continue on our way.
		     * someone should shoot the people who designed X color allocation.
		     */

		    if (!UseColorCube || map!=DefaultColormap(disp, scrn)) {

			if (fit)
			    XGrabServer(disp);

			for (a = 0; a < realsrc->rgb->used; a++,  numColorsInIndex++) 
			    /* count entries we've got */
			    if (! XAllocColorCells(disp, map, False, NULL, 0, indx + a, 1))
				break;

			if (a > 0) {
			    XFreeColors(disp, map, indx, a, 0);
			    memset(indx, 0, numColorsInIndex * sizeof(Pixel));
			    numColorsInIndex = 0;
			}

			if (a == 0) {
			    free(indx);
			    indx = NULL;
			    numColorsInIndex = 0;
			    if(realsrc!=source) realsrc->Destroy();
			    return FALSE;
			}

			if (a <= 2 && realsrc->rgb->used>2) {
			    if(realsrc==source) {
				realsrc = new image;
				(realsrc)->newRGBImage(source->Width(), source->Height(), ddepth);
				source->Duplicate(realsrc);
			    }
			    realsrc = (realsrc)->Dither();
			    fit = 0;
			    free(indx);
			    indx = NULL;
			    numColorsInIndex = 0;
			    goto retry;
			}

			if (a < realsrc->rgb->used) {
			    if(realsrc==source) {
				realsrc = new image;
				(realsrc)->newRGBImage(source->Width(), source->Height(), ddepth);
				source->Duplicate(realsrc);
			    }
			    realsrc = (realsrc)->Reduce(a);
			}

			if (fit) {
			    for(a = 0; a < realsrc->rgb->used; a++, numColorsInIndex++)
				*(indx + a) = 
				  xcmap->CubePixel(*(realsrc->rgb->red + a),
						   *(realsrc->rgb->green + a),
						   *(realsrc->rgb->blue + a));
			    XUngrabServer(disp);
			}
			else {
			    for (b = 0; b < a; b++) {
				red = *(realsrc->rgb->red + b);
				green = *(realsrc->rgb->green + b);
				blue = *(realsrc->rgb->blue + b);
				xc = (class xcolor *)xcmap->Alloc( red, green, blue);
				if(xc)
				    *(indx + b) = (xc)->PixelRef();
				*(allocated.Append())=xc;
			    }
			}
		    }
		    else /* if ((visual == DefaultVisual(disp, scrn)) ||
			  (visual->c_class == StaticGray) ||
			  (visual->c_class == StaticColor)) */ {

			/* allocate colors shareable (if we can)
			 */
			image *newsrc;
			static boolean DitherToCube=environ::GetProfileSwitch("DitherToCube", TRUE);
			if(DitherToCube && realsrc->type!=IBITMAP) {
			    xcmap->CubePixel(0,0,0);
			    int reds=-1, greens=-1, blues=-1;
			    if(xcmap->map_info) {
				reds=xcmap->map_info->red_max+1;
				greens=xcmap->map_info->green_max+1;
				blues=xcmap->map_info->blue_max+1;
			    } else if(xcmap->rampsamples!=0 && xcmap->redramp && xcmap->greenramp && xcmap->blueramp) {
				reds=greens=blues=xcmap->rampsamples;
			    } else if(xcmap->colorCube) {
				reds=greens=blues=xcmap-> colorCube-> samplesPerComponent;
			    }
			    if(reds!=-1) {
				newsrc= realsrc->ColorDitherToCube(reds, greens, blues);
				if(realsrc!=source) realsrc->Destroy();
				realsrc=newsrc;
				if(indx) free(indx);
				indx = (Pixel *) malloc(sizeof(Pixel) * realsrc->rgb->used); 
			    }
			}
			for  (a = 0; a < realsrc->rgb->used; a++)
			    *(indx + a) = 
			      xcmap
			      ->CubePixel(
					  *(realsrc->rgb->red + a),
					  *(realsrc->rgb->green + a),
					  *(realsrc->rgb->blue + a));
		    }
		    break;
	    }
	}
	/* create an XImage and related colormap based on the image type
	 * we have.
	 */

	switch(realsrc->type) {
	    case IBITMAP:
		{ byte *data;
		int destlinelen = (realsrc->width / 8) + (realsrc->width % 8 ? 1 : 0);

		data = (byte*) malloc(destlinelen * realsrc->height);
		memmove(data, (byte*) realsrc->data, destlinelen * realsrc->height);
		ximage = XCreateImage(disp, visual, 1, XYBitmap, 0, (char *)data, realsrc->width, realsrc->height, 8, 0);
		ximage->bitmap_bit_order = MSBFirst;
		ximage->byte_order = MSBFirst;
		break;
		}

	    case IGREYSCALE:
	    case IRGB:
	    case ITRUE:

		/* modify image data to match visual and colormap
		 */

		dbits = bitsPerPixelAtDepth(disp, scrn, ddepth);
		dpixlen= (dbits + 7) / 8;

		switch (visual->c_class) {
		    case TrueColor:
			{ byte *data, *destptr, *srcptr;
			Pixel pixval, newpixval;

			ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0, NULL, realsrc->width, realsrc->height, 8, 0);
			data = (byte*) malloc(realsrc->width * realsrc->height * dpixlen);
			ximage->data = (char *)data;
			destptr = data;
			srcptr = realsrc->data;
			switch (realsrc->type) {
			    case ITRUE:
				for (y = 0; y < realsrc->height; y++)
				    for (x = 0; x < realsrc->width; x++) {
					pixval = memToVal(srcptr, realsrc->pixlen);
					newpixval = redvalue[TRUE_RED(pixval)] |
					  greenvalue[TRUE_GREEN(pixval)] | bluevalue[TRUE_BLUE(pixval)];
					valToMem(newpixval, destptr, dpixlen);
					srcptr += realsrc->pixlen;
					destptr += dpixlen;
				    }
				break;
			    case IGREYSCALE:
			    case IRGB:
				for (y = 0; y < realsrc->height; y++)
				    for (x = 0; x < realsrc->width; x++) {
					pixval = memToVal(srcptr, realsrc->pixlen);
					pixval = redvalue[realsrc->rgb->red[pixval] >> 8] |
					  greenvalue[realsrc->rgb->green[pixval] >> 8] |
					  bluevalue[realsrc->rgb->blue[pixval] >> 8];
					valToMem(pixval, destptr, dpixlen);
					srcptr += realsrc->pixlen;
					destptr += dpixlen;
				    }
				break;
			    default: /* something's broken */
				if(realsrc!=source) realsrc->Destroy();
				return FALSE;
			}
			ximage->byte_order = MSBFirst;
			break;
			}
		    default:

			/* only IRGB images make it this far.
			 */

			/* if our XImage doesn't have modulus 8 bits per pixel, it's unclear
			 * how to pack bits so we instead use an XYPixmap image.  this is
			 * slower.
			 */

			if (dbits % 8) {
			    byte *data, *destdata, *destptr, *srcptr, mask;
			    Pixel pixmask, pixval;

			    ximage = XCreateImage(disp, visual, ddepth, XYPixmap, 0, NULL, realsrc->width, realsrc->height, 8, 0);
			    data = (byte *)malloc(realsrc->width * realsrc->height * dpixlen);
			    ximage->data = (char *)data;
			    memset(data, 0, realsrc->width * realsrc->height * dpixlen);
			    ximage->bitmap_bit_order = MSBFirst;
			    ximage->byte_order = MSBFirst;
			    linelen = (realsrc->width + 7) / 8;
			    for (a = 0; a < dbits; a++) {
				pixmask = 1 << a;
				destdata = data + ((dbits - a - 1) * realsrc->height * linelen);
				srcptr = realsrc->data;
				for (y = 0; y < realsrc->height; y++) {
				    destptr = destdata + (y * linelen);
				    *destptr = 0;
				    mask = 0x80;
				    for (x = 0; x < realsrc->width; x++) {
					pixval= memToVal(srcptr, realsrc->pixlen);
					srcptr += realsrc->pixlen;
					if (indx[pixval] & pixmask)
					    *destptr |= mask;
					mask >>= 1;
					if (mask == 0) {
					    mask = 0x80;
					    destptr++;
					}
				    }
				}
			    }
			}
			else {
			    byte *data, *srcptr, *destptr;

			    ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0, NULL, realsrc->width, realsrc->height, 8, 0);
			    dpixlen = (ximage->bits_per_pixel + 7) / 8;
			    data = (byte*) malloc(realsrc->width * realsrc->height * dpixlen);
			    ximage->data = (char *)data;
			    ximage->byte_order = MSBFirst;
			    srcptr = realsrc->data;
			    destptr = data;
			    for (y = 0; y < realsrc->height; y++)
				for (x = 0; x < realsrc->width; x++) {
				    valToMem(indx[memToVal(srcptr, realsrc->pixlen)], destptr, dpixlen);
				    srcptr += realsrc->pixlen;
				    destptr += dpixlen;
				}
			}
			break;
		}
	}
	if (redvalue) {
	    free((byte *)redvalue);
	    free((byte *)greenvalue);
	    free((byte *)bluevalue);
	}
	if(indx) free(indx);
    }
    outdated=FALSE;
    if(realsrc!=source) realsrc->Destroy();
    released=FALSE;
    return TRUE;
}

xddimage::xddimage() {
    delayedeval=NULL;
    delayedrelease=NULL;
    histogram=NULL;
    xgd=NULL;
    waiting=FALSE;
    tsrc=NULL;
    ximage=NULL;
    released=TRUE;
    maxcolors=0;
    draw=NULL;
    pixok=FALSE;
    pixmap=None;
}

xddimage::xddimage(image *src, colormap **map) : iddimage(src, map) {
    delayedeval=NULL;
    delayedrelease=NULL;
    histogram=NULL;
    xgd=NULL;
    waiting=FALSE;
    tsrc=NULL;
    ximage=NULL;
    released=TRUE;
    maxcolors=0;
    draw=NULL;
    pixok=FALSE;
    pixmap=None;
}

xddimage::~xddimage() {
    if(delayedeval) {
	delayedeval->Cancel();
	delayedeval=NULL;
    }
    Release(FALSE);
    delete histogram;
}

void xddimage::ObservedChanged(observable *changed, long change) {
    if(changed==xgd && change==observable::OBJECTDESTROYED) {
	draw->Stop();
	xgd=NULL;
	return;
    }
    if(changed==source) {
	ReleaseData(FALSE);
    }
    if(cmap && *cmap==changed) {
        if(change==observable::OBJECTDESTROYED) {
            if(delayedeval) {
                delayedeval->Cancel();
                delayedeval=NULL;
            }
            cmap=NULL;
            allocated.Remove((size_t)0,allocated.GetN());
        } else {

            if(delayedrelease) {
                delayedrelease->Cancel();
                delayedrelease=NULL;
                Release(FALSE);
            }
            if(!released) {
                if(change==xcolormap_NEWIMAGECOLORS) {
                    ReProcess();
                    // trigger a redraw of this view...
                    NotifyObservers( observable::OBJECTCHANGED);
                } else if(change==xcolormap_NEWMANAGER) {
                    ReleaseData(FALSE);
                    // the resources are gone with the previous manager...
                    delete histogram;
                    histogram=NULL;
                    ReleaseResources(FALSE);
                    released=TRUE;
                    NotifyObservers(observable::OBJECTCHANGED);
                } else if(change==xcolormap_LOSTMANAGER) {
                    ReleaseData(FALSE);
                    // the resources are gone with the previous manager...
                    delete histogram;
                    histogram=NULL;
                    ReleaseResources(FALSE);
                    released=TRUE;
                    NotifyObservers(observable::OBJECTCHANGED);
                }
            }
        }
    }
    if(changed==draw && (change==WorkProc_Done || change==WorkProc_Exited || change==WorkProc_Stopped || change==observable::OBJECTDESTROYED)) {
	RemoveObserver(changed);
	draw=NULL;
    } else if(change==observable::OBJECTDESTROYED) {
	RemoveObserver(changed);
    }
    iddimage::ObservedChanged(changed, change);
}

static int UsePixmapsForImages=-1;
void xddimage::Draw(graphic *dest, long left, long top, long width, long height, long destLeft, long destTop) {
    xgraphic *xdest=(xgraphic *)dest;
    if(delayedrelease) {
	delayedrelease->Cancel();
	delayedrelease=NULL;
    }
    if(Outdated()) if(!Process(xdest)) {
	fprintf(stderr, "xgraphic: image processing failed!\n");
	return;
    }
    Display *disp = xdest->XDisplay();
    if(!waiting) {
#ifndef ATK_INCREMENTAL_IMAGE_LOADING
	if(ximage || pixok) {
	    unsigned int w=source->Width();
	    unsigned int h=source->Height();
	    int scrn = DefaultScreen(disp);

	    int ddepth = DefaultDepth(disp, scrn);
	    if(UsePixmapsForImages==-1) UsePixmapsForImages=environ::GetProfileSwitch("UsePixmapsForImages", FALSE);
	    if(pixmap==None && UsePixmapsForImages) {
		pixmap=XCreatePixmap(disp, xdest->XWindow(), (unsigned int)w, (unsigned int) h, ddepth);
		pixok=FALSE;
	    }
	    if(pixmap!=None) {
		if(ximage) {
		    XPutImage(disp, pixmap, DefaultGC(disp, scrn), ximage, 0, 0, 0, 0, (unsigned int)w, (unsigned int)h);
		    pixok=TRUE;
		    if(ximage) {
			XDestroyImage(ximage);
			ximage=NULL;
		    }
		}
		XCopyArea(disp, pixmap, xdest->XWindow(), xdest->XGC(), (int)left, (int)top, (unsigned int)width, (unsigned int)height, (int)physical::LogicalXToGlobalX(xgd, destLeft), (int)physical::LogicalYToGlobalY(xgd, destTop));
	    } else XPutImage(disp, xdest->XWindow(), xdest->XGC(), ximage, (int)left, (int)top, (int)physical::LogicalXToGlobalX(xgd, destLeft), (int)physical::LogicalYToGlobalY(xgd, destTop), (unsigned int)width, (unsigned int)height);
	}
#else 
	if(draw) {
	    draw->Stop();
	}
	if(ximage) draw=new xddimage_draw(this, left, top, destLeft, destTop, width, height);
#endif
    }
}

void xddimage::Draw(graphic *dest, const struct rectangle *src, long destLeft, long destTop) {
    xgraphic *xdest=(xgraphic *)dest;
    if(delayedrelease) {
	delayedrelease->Cancel();
	delayedrelease=NULL;
    }
    if(Outdated()) if(!Process(xdest)) {
	fprintf(stderr, "xgraphic: image processing failed!\n");
	return;
    }
    Display *disp=xdest->XDisplay();
    if(!waiting) {
#ifndef ATK_INCREMENTAL_IMAGE_LOADING
	if(ximage || pixok) {
	    int scrn = DefaultScreen(disp);

	    int ddepth = DefaultDepth(disp, scrn);

	    if(UsePixmapsForImages==-1) UsePixmapsForImages=environ::GetProfileSwitch("UsePixmapsForImages", FALSE);
	    if(pixmap==None && UsePixmapsForImages) {
		pixmap=XCreatePixmap(disp, xdest->XWindow(), source->Width(), source->Height(), ddepth);
		pixok=FALSE;
	    }
	    
	    if(pixmap!=None) {
		if(ximage) {
		    XPutImage(disp, pixmap, DefaultGC(disp, scrn), ximage, 0, 0, 0, 0, source->Width(), source->Height());
		    pixok=TRUE;
		    if(ximage) {
			XDestroyImage(ximage);
			ximage=NULL;
		    }
		}
		XCopyArea(disp, pixmap, xdest->XWindow(), xdest->XGC(), (int)src->left, (int)src->top, (unsigned int)src->width, (unsigned int)src->height, (int)physical::LogicalXToGlobalX(xgd, destLeft), (int)physical::LogicalYToGlobalY(xgd, destTop));
	    } else XPutImage(disp, xdest->XWindow(), xdest->XGC(), ximage, (int)src->left, (int)src->top, (int)physical::LogicalXToGlobalX(xgd, destLeft), (int)physical::LogicalYToGlobalY(xgd, destTop), (unsigned int)src->width, (unsigned int)src->height);
	}
#else	
	if(draw) {
	    draw->Stop();
	}

	if(ximage) draw=new xddimage_draw(this, src->left, src->top, destLeft, destTop, src->width, src->height);
#endif
    }
}
