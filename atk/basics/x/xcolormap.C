/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <xim.H>
ATK_IMPL("xcolormap.H")
#include <xcolor.H>
#include <xcolormap.H>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <environ.H>
#include <util.h>
#include <atoms.h>
#undef DisplayString
#include <message.H>
static int xcolormap_debug=0;
extern long ATKXErrors;

ATKdefineRegistryNoInit(xcolormap, colormap);

void xcolormap::ExplodePixelMasks(xcolormap_pixelmap *pm, unsigned long red_mask, unsigned long green_mask, unsigned long blue_mask) {

    pm->red_mask=red_mask;
    pm->green_mask=green_mask;
    pm->blue_mask=blue_mask;
    pm->firstredbit=pm->firstgreenbit=pm->firstbluebit=(-1);
    pm->redcolors = pm->greencolors = pm->bluecolors = 1;
    pm->bits_per_pixel=0;
    int i=0;
    for (unsigned long pixval = 1; pixval; i++, pixval <<= 1) {
	if (pixval & red_mask) {
	    if(pm->firstredbit<0) pm->firstredbit=i;
	    pm->redcolors <<= 1;
	    pm->bits_per_pixel++;
	}
	if (pixval & green_mask) {
	    if(pm->firstgreenbit<0) pm->firstgreenbit=i;
	    pm->greencolors <<= 1;
	    pm->bits_per_pixel++;
	}
	if (pixval & blue_mask) {
	    if(pm->firstbluebit<0) pm->firstbluebit=i;
	    pm->bluecolors <<= 1;
	    pm->bits_per_pixel++;
	}
    }
}

static void DirectColorCube(xcolormap *xcmap) {
    unsigned int samples = 2;
    int cubesize;
    Display *disp;
    class xcolor *xc;
    if (xcmap == NULL)
	return;
    disp = (xcmap)->XDisplay();
    Visual *v=DefaultVisual(disp, DefaultScreen(disp));
    struct  xcolormap_pixelmap pm;
    xcolormap::ExplodePixelMasks(&pm, v->red_mask, v->green_mask, v->blue_mask);
    unsigned int mincolors=MIN(pm.redcolors, MIN(pm.greencolors, pm.bluecolors));
    unsigned int i;
    if(v->c_class==TrueColor) {
	i=mincolors;
    } else {
	cubesize = (int)environ::GetProfileInt("colorcubesize", -1);
	if(cubesize<0) {
	    i=mincolors/2;
	} else {
	    for(i=mincolors;i>=2;i--) {
		if(i*i*i<=(unsigned int)cubesize) break;
	    }
	}
    }
    if(i<2) goto error;
    samples=i;
    xcmap->pixels=(xcolor **)malloc(sizeof(xcolor *)*samples);
    xcmap->redramp=(unsigned long *)malloc(sizeof(unsigned long)*samples);
    xcmap->greenramp=(unsigned long *)malloc(sizeof(unsigned long)*samples);
    xcmap->blueramp=(unsigned long *)malloc(sizeof(unsigned long)*samples);
    xcmap->rampsamples=samples;
    if(xcmap->greenramp==NULL || xcmap->redramp==NULL || xcmap->blueramp==NULL) {
	error:
	  fprintf(stderr, "xcolormap: couldn't allocate color cube for TrueColor or DirectColor display.\n");
	if(xcmap->pixels) {
	    for(i=0;i<samples;i++) {
		if(xcmap->pixels[i]) xcmap->Free(xcmap->pixels[i]);
	    }
	    free(xcmap->pixels);
	}
	if(xcmap->redramp) free(xcmap->redramp);
	if(xcmap->greenramp) free(xcmap->greenramp);
	if(xcmap->blueramp) free(xcmap->blueramp);
	return;
    }
    unsigned int maxcolor=samples-1;
    for(i=0;i<samples;i++) {
	if((xc = xcmap->pixels[i] = (xcolor *)(xcmap)->AllocExact( i*65535/maxcolor, i*65535/maxcolor, i*65535/maxcolor, TRUE))) {
	    xcmap->redramp[i]=xc->PixelRef() & pm.red_mask;
	    xcmap->greenramp[i]=xc->PixelRef() & pm.green_mask;
	    xcmap->blueramp[i]=xc->PixelRef() & pm.blue_mask;
	} else goto error;
    }
}

static void NormalColorCube(xcolormap *xcmap) {
    unsigned short i, ri, gi, bi;
    long cubesize;
    unsigned short samples = 2;
    static int cubics[] = {216, 125, 64, 27, 8, -1};
    long realcubesize = -1;
    boolean approx=FALSE;
    Display *disp=xcmap->XDisplay();
    xcmap->colorCube = (struct xcolormap_cube*) malloc(sizeof(struct xcolormap_cube));
    if(xcmap->colorCube==NULL) return;
    xcolormap_cube *colorCube=xcmap->colorCube;
    memset(colorCube, 0, sizeof(struct xcolormap_cube));
    if((cubesize = environ::GetProfileInt("colorcubesize", -1)) != -1) {

	for(i = 0; cubics[i] != -1; i++)
	    if(cubesize >= cubics[i]) {
		realcubesize=cubics[i];
		break;
	    }
	if(realcubesize<0) fprintf(stderr, "xcolormap: bad ColorCubeSize preference:%ld,\nshould be a cubic value. (n^3)\n", cubesize);
    }

    if(realcubesize<0) {
	int	nc = CellsOfScreen(DefaultScreenOfDisplay(disp));
	int half_nc = nc/2;
	for(i = 0; cubics[i] != -1; i++)
	    if(half_nc >= cubics[i]) {
		realcubesize = cubics[i];
		break;
	    }
    }
    cubesize=realcubesize;
    retry:
      smaller:
      switch(realcubesize) {
	  case 8:	    
	      samples = 2; 
	      break;
	  case 27:    
	      samples = 3; 
	      break;
	  case 64:    
	      samples = 4; 
	      break;
	  case 125:
	      samples = 5;
	      break;
	  case 216:
	      samples = 6;
	      break;
	  default:
	      break;
      }

    colorCube->samplesPerComponent = samples;
    for(ri = 0; ri < samples; ri++) {
	for(gi = 0; gi < samples; gi++) {
	    for(bi = 0; bi < samples; bi++) {
		colorCube->cube[ri][gi][bi]=NULL;
	    }
	}
    }

    int maxcomp=samples-1;
    for(ri = 0; ri < samples; ri++) {
	for(gi = 0; gi < samples; gi++) {
	    for(bi = 0; bi < samples; bi++) {
		colorCube->cube[ri][gi][bi]=(xcolor *)xcmap->AllocExact((ri*65535)/maxcomp, (gi*65535)/maxcomp, (bi*65535)/maxcomp, !approx);
		if(colorCube->cube[ri][gi][bi]==NULL) {

		    for(ri = 0; ri < samples; ri++) {
			for(gi = 0; gi < samples; gi++) {
			    for(bi = 0; bi < samples; bi++) {
				if(colorCube->cube[ri][gi][bi]) xcmap->Free(colorCube->cube[ri][gi][bi]);
			    }
			}
		    }
		    if(samples>2) {
			realcubesize=cubics[7-samples];
			goto smaller;
		    } else {
			if(!approx) {
			    fprintf(stderr, "xcolormap: couldn't create exact color cube.  Using the closest available colors.\n");
			    approx=TRUE;
			    realcubesize=cubesize;
			    goto retry;
			} else goto leave;
		    }
		}
	    }
	}
    }
    leave:
    if(realcubesize!=cubesize) {
	fprintf(stderr, "xcolormap: couldn't create %ld color cube, using %ld color cube.\n", cubesize, realcubesize);
    }
}

void xcolormap::InitializeColorCube()
{
    Display *disp;
    
    disp = XDisplay();
    Visual *v=DefaultVisual(disp, DefaultScreen(disp));
    map_info=XAllocStandardColormap();
    Status status = XGetStandardColormap(disp, RootWindow(disp, DefaultScreen(disp)), map_info, XA_RGB_DEFAULT_MAP);
    if(!status || map_info->colormap!=this->XColorMap) {
	XFree((char *)map_info);
	map_info=NULL;
	if(v->c_class==DirectColor || v->c_class==TrueColor) {
	    DirectColorCube(this);
	    return;
	} else NormalColorCube(this);
    }
}

#define ATK_COLORS ac_ATK_COLORS
#define TIMESTAMP ac_TIMESTAMP
static void ClearOldManager(Display *display, Window manager) {
    if(manager!=None) {
	XEvent event;
	// disregard any further events from the old manager window...
	XSelectInput(display, manager, None);
	// sync up so we have all possible events for the old manager...
	XSync(display, False);
	// get rid of any old DestroyNotify events for the old manager.  Otherwise they might
	// be applied to the new manager....
	XCheckTypedWindowEvent(display, manager, DestroyNotify, &event);
    }
}

static Window FindManager(Display *display, Window mywin, Atom *AtomCache, Time *tstamp=NULL) {
    unsigned int retries=10;
    Time t=0;
    Window manager=None;
    Time timestamp;
    do {
	XEvent event;
	if(manager!=None) {
	    ClearOldManager(display, manager);
	    manager=None;
	}
	XSync(display, False);
	long nerr=ATKXErrors;
	XChangeProperty(display, mywin, ATK_COLORS, XA_ATOM, 32, PropModeAppend, (unsigned char *)&mywin, 0);
	XDeleteProperty(display, mywin, ATK_COLORS);
	XSync(display, False);
	// all the PropertyNotify events ought to be in the queue...
	while(XCheckTypedWindowEvent(display, mywin, PropertyNotify,
				     &event));
	timestamp=event.xproperty.time;
	manager=XGetSelectionOwner(display, ATK_COLORS);
	if(manager==None) break;
	XSelectInput(display, manager, StructureNotifyMask);
	XConvertSelection(display, ATK_COLORS, TIMESTAMP, XA_INTEGER, mywin, event.xproperty.time);
	while(nerr==ATKXErrors && !XCheckTypedWindowEvent(display, mywin, SelectionNotify,
						       &event)) XSync(display, False);
	if(nerr!=ATKXErrors) continue;
	if(event.type!=SelectionNotify) {
	    fprintf(stderr, "xcolormap: error getting timestamp on color manager selection... %d\n", __LINE__);
	    continue;
	}
	if(event.xselection.property==None) {
	    fprintf(stderr, "xcolormap: No color manager available...\n");
	    ClearOldManager(display, manager);
	    manager=None;
	    break;
	}
	Atom actual_type_return;
	int actual_format_return;
	unsigned long bytes_after_return=0;
	unsigned char *prop_return=NULL;
	unsigned long nitems;
	int status=XGetWindowProperty(display, mywin, XA_INTEGER, 0, 1, TRUE, XA_INTEGER, &actual_type_return, &actual_format_return, &nitems, &bytes_after_return, &prop_return);
	if(status!=Success || nitems==0 || actual_type_return!=XA_INTEGER || actual_format_return!=32 || prop_return==NULL) {
	    fprintf(stderr, "xcolormap: error getting timestamp on color manager selection... %d\n", __LINE__);
	    continue;
	}
	XSync(display, False);
	// All the PropertyNotify events ought to be in the queue...
	while(XCheckTypedWindowEvent(display, mywin, PropertyNotify,
				     &event));
	if(nerr!=ATKXErrors) continue;
	// chewed the PropertyNotify for the ChangeProperty call done by the s
	t=*(Time *)prop_return;
	XFree((char *)prop_return);
	if(t<timestamp) break;
    } while(--retries>0);
    if(retries==0) {
	manager=None;
    }
    if(tstamp) *tstamp=timestamp;
    return manager;
}



xcolormap::xcolormap(Display *dpy, Colormap def)
{
    managerpid=0;
    timestamp=0;
    delayedrestart=NULL;
    state=xcolormap_Dead;
    startmanager=xcolormap_ManualStart;
    colorprotocol=None;
    allocs=deallocs=NULL;
    allocreq=deallocreq=NULL;
    imagecolors=NULL;
    imagecolorcount=imagepixelcount=0;
    pending=FALSE;
    imagepixels=NULL;
    manager=None;
    
    rampsamples=0;
    redramp=NULL;
    greenramp=NULL;
    blueramp=NULL;
    pixels=NULL;
    XColorMap = def;
    display = dpy;
    colorCube=NULL;
    map_info=NULL;

    int scrn = DefaultScreen(dpy);

    if(XColorMap==None) XColorMap=DefaultColormap(dpy, scrn);

    AtomCache=xim::GetAtomCache(dpy);

    Visual *visual = DefaultVisual(dpy, scrn);

    if((XColorMap==DefaultColormap(dpy, scrn)) && environ::GetProfileSwitch("UseColorManager", TRUE)) {
	switch(visual->c_class) {
	    case DirectColor:
	    case TrueColor:
	    case StaticColor:
	    case StaticGray: break ;
	    case GrayScale:
	    case PseudoColor: {
		Window root=RootWindow(dpy, DefaultScreen(dpy));
		XSync(dpy, False);
		long nerr=ATKXErrors;
		colorprotocol=XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
		XSelectInput(dpy, colorprotocol, PropertyChangeMask);
		XSelectInput(dpy, root, StructureNotifyMask); /* XXX so far noplace else needs events from the root window... */
		XSync(dpy, False);
		if(nerr!=ATKXErrors) {
		    fprintf(stderr, "xcolormap: couldn't initialize communication with color manager.\n");
		    break;
		}
		manager=FindManager(dpy, colorprotocol, AtomCache, &timestamp);
		if(manager==None) startmanager=xcolormap_AutoStart;
		else {
		    state=xcolormap_Running;
		    startmanager=xcolormap_PromptStart;
		}
		}
	}
    }
}

xcolormap::~xcolormap( )
{
    int i;
    Flush();
    for(i =colors.GetN(); i>0;) {
	delete colors[--i];
    }
    if(colorprotocol) XDestroyWindow(display, colorprotocol);
    if(delayedrestart) delayedrestart->Cancel();
    delete allocreq;
    delete deallocreq;
    delete allocs;
    delete deallocs;
    if(imagecolors) free(imagecolors);
    if(imagepixels) free(imagepixels);
    
    if(map_info) XFree((char *)map_info);
    if(colorCube) free(colorCube);

    if(pixels) free(pixels);
    if(redramp) free(redramp);
    if(greenramp) free(greenramp);
    if(blueramp) free(blueramp);
    
    if(managerpid>0) {
	im::RemoveZombieHandler(managerpid);
    }
}

static inline unsigned int ColorDistance(unsigned short r, unsigned short g, unsigned short b, unsigned short cred, unsigned short cgreen, unsigned short cblue) {
    int dr, dg, db;
    dr = (r - cred); dr = abs(dr)>>8;	/* shift down to avoid overflow */
    dg = (g - cgreen); dg = abs(dg)>>8;
    db = (b - cblue);	db = abs(db)>>8;
    return dr*dr + dg*dg + db*db;
}

xcolor *xcolormap::FindClosest(unsigned short r, unsigned short g, unsigned short b)
{
    xcolor *tmp, *best = NULL;
    unsigned short R, G, B;
    Screen *s = DefaultScreenOfDisplay(XDisplay());
    int i = -1, nc = CellsOfScreen(s), bestDef = -1;
    unsigned int diff = 0, bestDiff = ~0;
    XColor *defs=NULL;

    if(nc<=1024) {
	if( ( defs = (XColor *) calloc(nc, sizeof(XColor)) ) ) {
	    for( i = 0; i < nc; i++ ) defs[i].pixel = i;
	    XQueryColors(XDisplay(), XColormap(), defs, nc);
	    for( i = 0; i < nc; i++ ) {
		diff = ColorDistance(defs[i].red, defs[i].green, defs[i].blue, r, g, b);
		if( diff < bestDiff ) {
		    bestDiff = diff;
		    bestDef=i;
		}
	    }
	}
    }
    retry:
    for( i = 0; i < (int)colors.GetN(); i++ ) {
	tmp = (class xcolor *) colors[i];
	(tmp)->GetRGB(R, G, B);
	diff = ColorDistance(R, G, B, r, g, b);
	if( diff < bestDiff ) {
	    bestDiff = diff;
	    best = tmp;
	    bestDef=(-1);
	}
    }
    if(bestDef>=0) {
	tmp=(xcolor *)AllocExact(defs[bestDef].red, defs[bestDef].green, defs[bestDef].blue, TRUE);
	if(tmp==NULL) {
	    bestDiff=~0;
	    goto retry;
	}
	return tmp;
    } else tmp=best;
    if(defs) free(defs);
    tmp->Reference();
    return tmp;
}


static boolean ParseColor(xcolormap *self, const atom *name, unsigned short &r, unsigned short &g, unsigned short &b) {
    XColor color;
    if(XParseColor(self->XDisplay(), self->XColormap(), name->Name(), &color)) {
	r=color.red;
	g=color.green;
	b=color.blue;
	return TRUE;
    }
    return FALSE;
}

ddcolor *xcolormap::Find(const atom *name) {
    size_t i;
    for(i=0;i<colors.GetN();i++) {
	if(colors[i]->iname==name) return colors[i];
    }
    return NULL;
}

ddcolor *xcolormap::Find(unsigned short r, unsigned short g, unsigned short b) {
    size_t i;
    for(i=0;i<colors.GetN();i++) {
	unsigned short R, G, B;
	colors[i]->GetRGB(R, G, B);
	if((colors[i]->iname==NULL && colors[i]->ir==r && colors[i]->ig==g && colors[i]->ib==b) || (R==r && G==g && B==b)) return colors[i];
    }
    return NULL;
}

ddcolor *xcolormap::Alloc(const atom *name)
{
    static atom_def trans("transparent");
    unsigned short r, g, b;
    ddcolor *c=Find(name);
    if(c!=NULL) {
	c->Reference();
	c->iname=name;
	return c;
    } else  {
	if(name==trans) {
	    xcolor *xc=new xcolor(this, name, 0, 0, 0);
	    return xc;
	}
	if(!ParseColor(this, name, r, g, b)) {
	    fprintf(stderr, "xcolormap: bad color name %s.\n", (const char *)name);
	    return NULL;
	}
    }
    xcolor *xc=(xcolor *)AllocExact(r, g, b, FALSE);
    if(xc==NULL) return NULL;
    xc->iname=name;
    return xc;
}

ddcolor *xcolormap::AllocExact(unsigned short r, unsigned short g, unsigned short b, boolean exact) {
    if(xcolormap_debug) {
	fprintf(stderr, "xcolormap::AllocExact(%u, %u, %u, %s)\n", r, g, b, exact?"TRUE":"FALSE");
    }
    ddcolor *c=Find(r, g, b);
    if(c!=NULL) {
	c->Reference();
	c->ir=r;
	c->ig=g;
	c->ib=b;
	if(xcolormap_debug) {
	    unsigned short R, G, B;
	    c->GetRGB(R, G, B);
	    fprintf(stderr, "xcolormap::AllocExact returning existing color: %u, %u, %u.\n", R, G, B);
	}
	return c;
    }
    xcolor *xc=new xcolor(this, NULL, r, g, b);
    if(xc && xc->IsValid()) {
	unsigned short R, G, B;
	xc->GetRGB(R, G, B);
#define THIS_BREAKS_MONOCHROME 1
#if !THIS_BREAKS_MONOCHROME
	xcolor *xc2=(xcolor *)Find(R, G, B);
	if(xc2) {
	    delete xc;
	    xc=xc2;
	    xc2->Reference();
	}
#endif
	*colors.Append()=xc;
	if(xcolormap_debug) {
	    fprintf(stderr, "xcolormap::AllocExact returning new color: %u, %u, %u.\n", R, G, B);
	} 
	return xc;
    } else {
	delete xc;
	if(xcolormap_debug) {
	    fprintf(stderr, "xcolormap::AllocExact found no exact match.\n%s", exact?"":"Looking for closest match.\n");
	} 
	if(!exact) {
	    xc=FindClosest(r, g, b);
	    if(xc) {
		if(xcolormap_debug) {
		    unsigned short R, G, B;
		    xc->GetRGB(R, G, B);
		    fprintf(stderr, "xcolormap::AllocExact returning closest color: %u, %u, %u.\n", R, G, B);
		} 
		return xc;
	    }
	}
    }
    if(xcolormap_debug) {
	fprintf(stderr, "xcolormap::AllocExact couldn't find any color at all.\nThis should never happen.\n");
    }
    
    return NULL;
}

ddcolor *xcolormap::Alloc(unsigned short r, unsigned short g, unsigned short b) {
    return AllocExact(r, g, b, FALSE);
}

void xcolormap::ObservedChanged(observable  *changed, long  value)
{
    colormap::ObservedChanged(changed, value);
    if(value==observable::OBJECTDESTROYED) RemoveObserver(changed);
}

static void PromptAndStart(xcolormap *self);
static void dumbzombie(int /* pid */, long data, WAIT_STATUS_TYPE *zs) {
    xcolormap *self=(xcolormap *)data;
    if(self->state==xcolormap_Dead) return;
    if(!WIFEXITED(*zs)) return;
    self->state=xcolormap_Dead;
    PromptAndStart(self);
}

static const char * const RestartChoices[]={
    "Yes",
    "No",
    "Cancel",
    NULL
};

static void NotStarted(xcolormap *self) {
    if(self->state!=xcolormap_Running) message::DisplayString(NULL, 0, "The color manager has not been started.");
}

static void ForkManager(xcolormap *self) {
    if(self->managerpid>0) {
	im::RemoveZombieHandler(self->managerpid);
    }
    retry:
    const char *prog="acolorman";
    const char *aprog=environ::AndrewDir("/bin/acolorman");
    const char *colors=environ::GetProfile("ColorManagerQuota");
    char *dpystr=XDisplayString(self->display);
    const char *argv[6], **pa=argv;
    *(pa++)="acolorman";
    if(colors) {
        *(pa++)="-n";
        *(pa++)=colors;
    }
    if(dpystr) {
        *(pa++)="-display";
        *(pa++)=dpystr;
    }
    *(pa++)=NULL;
    self->managerpid=osi_vfork();
    if(self->managerpid==0) {
	execvp(prog, (char * const *)argv);
	execvp(aprog, (char * const *)argv);
	_exit(-1);
    } else if(self->managerpid>0) {
        im::AddZombieHandler(self->managerpid, dumbzombie, (long)self);
        self->startmanager=xcolormap_PromptStart;
        self->state=xcolormap_Starting;
    } else if(self->managerpid<0) {
	long answer=1;
	if(message::MultipleChoiceQuestion(NULL, 0, "The color manager could not be started. Try again?",
					   0, &answer, RestartChoices,
					   NULL)==-1) {
	}
	if(answer==1 || answer==2) {
	    NotStarted(self);
	    return;
	} else goto retry;
    }
    return;
}

static void PromptAndStart(xcolormap *self) {
    long answer=0;
    if(message::MultipleChoiceQuestion(NULL, 0, "The color manager has died, restart it?",
					   0, &answer, RestartChoices,
				       NULL)==-1) {
	self->startmanager=xcolormap_PromptStart;
	NotStarted(self);
	return;
    } else {
	if(self->state!=xcolormap_Dead  && self->state!=xcolormap_DelayedStart) {
	    message::DisplayString(NULL, 0, "The color manager is already running.");
	    self->startmanager=xcolormap_PromptStart;
	    return;
	} else if(answer!=0) NotStarted(self);
	if(answer==0) ForkManager(self);
	else if(answer==1) self->startmanager=xcolormap_ManualStart;
    }
}

static void DelayedStartManager(void *pd, long) {
    xcolormap *self=(xcolormap *)pd;
    self->delayedrestart=NULL;
    switch(self->startmanager) {
	case xcolormap_AutoStart:
	    ForkManager(self);
	    break;
	case xcolormap_PromptStart:
	    PromptAndStart(self);
	    break;
	case xcolormap_ManualStart:
	    break; /* tjm - FIXME: what should be done here? */
    }
}
    
static void StartManager(xcolormap *self) {
    switch(self->startmanager) {
        case xcolormap_AutoStart:
        case xcolormap_PromptStart:
            if(self->delayedrestart) return;
            self->state=xcolormap_DelayedStart;
            self->delayedrestart= im::EnqueueEvent( DelayedStartManager, (char *)self, 0);
            break;
	case xcolormap_ManualStart:
	    break;
    }
}

boolean xcolormap::AllocateImageColors(XCMapImageHist &hist) {
    if(state==xcolormap_Dead) {
	StartManager(this);
	return FALSE;
    }
    XCMapImageHist_iter j(hist);
    if(allocs==NULL) allocs=new XCMapImageHist;
    if(deallocs==NULL) deallocs=new XCMapImageHist;
    while(!j.done()) {
	XCMapImageRGB &rgb=j.Key();
	unsigned int count=*j;
	unsigned int *ac=allocs->Fetch(rgb);
	if(ac) (*ac)+=count; // an alloc entry is already there, just add the new count;
	else {
	    unsigned int *dc=deallocs->Fetch(rgb);
	    // is there an outstanding deallocation of this color?
	    if(dc) {
		if(*dc>=count) {
		    // more deallocations than allocations, offset the deallocations with the allocations
		    *dc-=count;
		    // if they cancel exactly remove the deallocations entirely.
		   if(*dc==0) deallocs->Remove(rgb);
		} else {
		    // more allocations than deallocations, enter the allocations, less the deallocations.
		   count-=*dc;
		   allocs->Insert(rgb, count);
		   // and remove the deallocations
		   deallocs->Remove(rgb);
	       }
	    }  else {
		// never seen before, enter the allocation
		allocs->Insert(rgb, count);
	    }
	}
	++j;
    }
    return TRUE;
}

boolean xcolormap::FreeImageColors(XCMapImageHist &hist) {
    if(state==xcolormap_Dead) {
	StartManager(this);
	return FALSE;
    }
    XCMapImageHist_iter j(hist);
    if(allocs==NULL) allocs=new XCMapImageHist;
    if(deallocs==NULL) deallocs=new XCMapImageHist;
    while(!j.done()) {
	XCMapImageRGB &rgb=j.Key();
	unsigned int count=*j;
	unsigned int *dc=deallocs->Fetch(rgb);
	if(dc) (*dc)+=count;
	else {
	    unsigned int *ac=allocs->Fetch(rgb);
	    if(ac) {
	       if(*ac>=count) {
		   (*ac)-=count;
		   if(*ac==0) allocs->Remove(rgb);
	       } else {
		   count-=*ac;
		   deallocs->Insert(rgb, count);
		   allocs->Remove(rgb);
	       }
	    }  else {
		deallocs->Insert(rgb, count);
	    }
	}
	++j;
    }
    return TRUE;
}


class incrkeyc {
    Window requestor;
    Atom property;
  public:
    inline incrkeyc(Window r, Atom p) {
	requestor=r;
	property=p;
    }
    inline incrkeyc() {
	requestor=None;
	property=None;
    }
    inline ~incrkeyc() {
    }
    inline size_t Hash() const {
	return requestor^property;
    }
    inline boolean operator==(const incrkeyc &key) {
	return (key.requestor==requestor)&&(key.property==property);
    }
};

class incrdatac {
    flex data;
    Atom type;
    int format;
    unsigned long nitems;
  public:
    inline incrdatac() {
	type=None;
	format=0;
	nitems=0;
    }
    inline incrdatac(const incrdatac &src) : data(src.data) {
	type=src.type;
	format=src.format;
	nitems=src.nitems;
    }
    inline incrdatac &operator=(const incrdatac &src) {
	type=src.type;
	format=src.format;
	nitems=src.nitems;
	data=src.data;
	return *this;
    }
    inline ~incrdatac() {
    }
    
    inline unsigned long NItems() {
	return nitems;
    }
    inline void AppendItems(unsigned long n) {
	nitems+=n;
    }
    inline Atom Type() {
	return type;
    }
    inline int Format() {
	return format;
    }
    inline flex &Data() {
	return data;
    }
    inline void SetFormat(int f) {
	format=f;
    }
    inline void SetType(Atom t) {
	type=t;
    }
    inline void AppendData(flex &src) {
	size_t len=src.GetN();
	if(len>1) {
	    unsigned char *d=(unsigned char *)data.Insert(data.GetN(), len-1);
	    size_t dummy;
	    unsigned char *s=(unsigned char *)src.GetBuf(0, len-1, &dummy);
	    memcpy(d, s, len-1);
	}
    }
};

DEFINE_AOHASH_CLASS(incrsc,incrkeyc,incrdatac); 
DEFINE_AHASH_CLASS_ITER(incrsc,incrkeyc,incrdatac);

static incrsc inprogress;

static void ClearInProgress(xcolormap */* self */) {
    incrsc_iter i(inprogress);
    while(!i.done()) {
	inprogress.Remove(i.Key());
	++i;
    }
}

static void ClearRequests(xcolormap *self) {
    if(self->allocreq==NULL || self->deallocreq==NULL) return;
    self->pending=FALSE;
    self->evaluate=FALSE;
    self->allocreq->Remove((size_t)0, self->allocreq->GetN());
    self->deallocreq->Remove((size_t)0, self->deallocreq->GetN());
}

static void DieDieDie(xcolormap *self) {
    ClearRequests(self);
    ClearInProgress(self);
    self->state=xcolormap_Dead;
    self->NotifyObservers(xcolormap_LOSTMANAGER);
}

static void TransferRequests(xcolormap *self) {
    if(self->state!=xcolormap_Running) return;
    if(self->allocreq==NULL || self->deallocreq==NULL || self->pending) return;
    if(self->allocreq->GetN()==0 && self->deallocreq->GetN()==0) {
	if(self->evaluate) {
	    self->NotifyObservers(xcolormap_NEWIMAGECOLORS);
	    self->evaluate=FALSE;
	}
	return;
    }
    size_t dummy;
    Display *display=self->display;
    Window colorprotocol=self->colorprotocol;
    size_t maxreq=512;
    /* one unit consists of r, g, b and the count.  (each 16bits) each request must
     consist of one or more whole units */
    maxreq=4*(maxreq/4);
    // set up the allocate and deallocate parameters
    size_t areqlen=MIN(self->allocreq->GetN(), maxreq);
    size_t dreqlen=MIN(self->deallocreq->GetN(), maxreq);
    size_t astart=self->allocreq->GetN()-areqlen;
    size_t dstart=self->deallocreq->GetN()-dreqlen;
    Atom list[10], *p=list;
    XSync(display, False);
    long nerr=ATKXErrors;
    if(areqlen) {
	XChangeProperty(display, colorprotocol, xim_ATK_ALLOCATE, xim_ATK_ALLOCATE, 16, PropModeReplace, (unsigned char *)(self->allocreq->GetBuf(astart, areqlen, &dummy)), areqlen);
	self->allocreq->Remove(astart, areqlen);
	*p++=xim_ATK_ALLOCATE;
	*p++=xim_ATK_ALLOCATE;
    }
    if(dreqlen) {
	XChangeProperty(display, colorprotocol, xim_ATK_DEALLOCATE, xim_ATK_DEALLOCATE, 16, PropModeReplace, (unsigned char *)(self->deallocreq->GetBuf(dstart, dreqlen, &dummy)), dreqlen);
	self->deallocreq->Remove(dstart, dreqlen);
	*p++=xim_ATK_DEALLOCATE;
	*p++=xim_ATK_DEALLOCATE;
    }
    // if no more allocate or deallocate operations are needed and evaluation has been requested
    // go for it.
    if(self->evaluate && self->deallocreq->GetN()==0 && self->allocreq->GetN()==0) {
	*p++=xim_ATK_EVALUATE;
	*p++=xim_ATK_EVALUATE;
	*p++=xim_PIXEL;
	*p++=xim_PIXEL;
	*p++=xim_ATK_COLORS;
	*p++=xim_ATK_COLORS;
    }
    if(p-list==2) {
	// only one request, might as well do it directly.
	XSync(display, False);
	if(nerr!=ATKXErrors) {
	    DieDieDie(self);
	    return;
	}
	nerr=ATKXErrors;
	XConvertSelection(display, xim_ATK_COLORS, list[0], list[1], colorprotocol, self->timestamp);
    } else {
	// two or more requests, use MULTIPLE protocol for efficiency.
	XChangeProperty(display, colorprotocol, xim_MULTIPLE, XA_ATOM, 32, PropModeReplace, (unsigned char *)list, p-list);
	XSync(display, False);
	if(nerr!=ATKXErrors) {
	    DieDieDie(self);
	    return;
	}
	nerr=ATKXErrors;
	XConvertSelection(display, xim_ATK_COLORS, xim_MULTIPLE, xim_MULTIPLE, colorprotocol, self->timestamp);
    }
    if(nerr==ATKXErrors) {
	self->pending=TRUE;
    } else DieDieDie(self);
}

boolean xcolormap::Evaluate() {
    if(state==xcolormap_Dead) {
	StartManager(this);
	return FALSE;
    } 
    if(allocs==NULL || deallocs==NULL) return TRUE;
    if(allocs->GetN()==0 && deallocs->GetN()==0) return TRUE;
    if(allocreq==NULL) allocreq=new XCMapColorReq;
    if(deallocreq==NULL) deallocreq=new XCMapColorReq;
    XCMapImageHist_iter i(allocs);
    while(!i.done()) {
	XCMapImageRGB &key=i.Key();
	unsigned int count=*i;
	if(count>65535*16) count=65535*16;
	while(count>0) {
	    CARD16 *c=allocreq->Insert(allocreq->GetN(), 4);
	    c[0]=key.r;
	    c[1]=key.g;
	    c[2]=key.b;
	    c[3]=MIN(count,65535);
	    count-=MIN(count,65535);
	}
	allocs->Remove(key);
	++i;
    }
    XCMapImageHist_iter j(deallocs);
    while(!j.done()) {
	XCMapImageRGB &key=j.Key();
	unsigned int count=*j;
	if(count>65535*16) count=65535*16;
	while(count>0) {
	    CARD16 *c=deallocreq->Insert(deallocreq->GetN(), 4);
	    c[0]=key.r;
	    c[1]=key.g;
	    c[2]=key.b;
	    c[3]=MIN(count,65535);
	    count-=MIN(count,65535);
	}
	deallocs->Remove(key);
	++j;
    }
    evaluate=TRUE;
    TransferRequests(this);
    return TRUE;
}

CARD32 *xcolormap::MapImageColors(RGBMap *map) {
    if(pending || imagecolorcount!=imagepixelcount || imagecolors==NULL || imagepixels==NULL) return NULL;
    CARD32 *pixels=(CARD32 *)malloc(sizeof(CARD32)*map->used);
    if(pixels==NULL) return NULL;
    unsigned int i,j;
    for(i=0;i<map->used;i++) {
	CARD32 minpixel=0;
	unsigned long mindist=~0;
	for(j=0;j<imagecolorcount;j++) {
	    unsigned long dist=ColorDistance(map->red[i], map->green[i], map->blue[i], imagecolors[j*3], imagecolors[j*3+1], imagecolors[j*3+2]);
	    if(dist<mindist) {
		mindist=dist;
		minpixel=imagepixels[j];
	    }
	}
	pixels[i]=minpixel;
    }
    return pixels;
}

RGBMap *xcolormap::MapImageColorsToMap(RGBMap *map) {
    RGBMap *rgb=(RGBMap *)malloc(sizeof(RGBMap));
    if(rgb) {
	unsigned int size=map->size;
	rgb->used = map->used;
	rgb->size = size;
	rgb->compressed = 0;
	rgb->red = (Intensity *)malloc(sizeof(Intensity) * size);
	rgb->green = (Intensity *)malloc(sizeof(Intensity) * size);
	rgb->blue = (Intensity *)malloc(sizeof(Intensity) * size);
	unsigned int i,j;
	for(i=0;i<map->used;i++) {
	    unsigned long mindist=~0;
	    for(j=0;j<imagecolorcount;j++) {
		unsigned long dist=ColorDistance(map->red[i], map->green[i], map->blue[i], imagecolors[j*3], imagecolors[j*3+1], imagecolors[j*3+2]);
		if(dist<mindist) {
		    mindist=dist;
		    rgb->red[i]=imagecolors[j*3];
		    rgb->green[i]=imagecolors[j*3+1];
		    rgb->blue[i]=imagecolors[j*3+2];
		}
	    }
	}
    }
    return rgb;
}

static void HandleResult(xcolormap *self, Display */* dpy */, Window /* requestor */, Atom target, Atom type, int /* format */, flex &fbuf, unsigned long nitems, Atom *AtomCache) {
    if(target==ac_TARGETS) {
	if(type!=XA_ATOM) {
	    fprintf(stderr, "xcolormap: getwindowproperty failed!\n");
	    return;
	}
    } else if(target==ac_TIMESTAMP) {
    } else if(target==ac_ATK_ALLOCATE) {
    } else if(target==ac_ATK_DEALLOCATE) {
    }  else if(target==ac_ATK_EVALUATE) {
    } else if(target==ac_PIXEL) {
	size_t i;
	if(nitems==0) return;
	unsigned long *pixels=(unsigned long *)fbuf.GetBuf(0, fbuf.GetN(), &i);
	if(self->imagepixels) free(self->imagepixels);
	self->imagepixelcount=(size_t)nitems;
	self->imagepixels=(CARD32 *)malloc((size_t)(sizeof(CARD32)*nitems));
	for(i=0;i<nitems;i++) {
	    self->imagepixels[i]=pixels[i];
	}
	
    } else if(target==ac_ATK_COLORS) {
	size_t i;
	if(nitems==0) return;
	unsigned short *colors=(unsigned short *)fbuf.GetBuf(0, fbuf.GetN(), &i);
	if(self->imagecolors) free(self->imagecolors);
	self->imagecolors=(unsigned short *)malloc((size_t)(sizeof(unsigned short)*nitems*3));
	self->imagecolorcount=(size_t)(nitems/3);
	if(nitems>2) for(i=0;i<nitems-2;i+=3) {
	    self->imagecolors[i]=colors[i];
	    self->imagecolors[i+1]=colors[i+1];
	    self->imagecolors[i+2]=colors[i+2];
	}
    }
}

static int MyGetProperty(
			 Display *display,
			 Window w,
			 Atom property,
			 Bool del,
			 Atom req_type,
			 Atom *actual_type_return,
			 int *actual_format_return,
			 unsigned long *nitems_return,
			 flex &prop) {
    unsigned long bytes_after_return=0, start=0;
    unsigned char *prop_return=NULL;
    size_t olen=prop.GetN();
    (*nitems_return)=0;
    do {
	unsigned long nitems;
	int status=XGetWindowProperty(display, w, property, start/4, 256, del, req_type, actual_type_return, actual_format_return, &nitems, &bytes_after_return, &prop_return);
	if(status!=Success || nitems==0) {
	    if(prop.GetN()!=olen) prop.Remove(olen, prop.GetN()-olen);
	    *nitems_return=0;
	    if(prop_return) XFree((char *)prop_return);
	    return status;
	}
	
	(*nitems_return)+=nitems;
	size_t len=(size_t)((*actual_format_return/8)*(*nitems_return));
	char *dest=prop.Insert(prop.GetN(), len);
	memcpy(dest, prop_return, len);
	start+=len;
	XFree((char *)prop_return);
    } while(bytes_after_return>0);
    *prop.Append()='\0';
    return Success;
}



static void HandleTarget(xcolormap *self, Display *dpy, Window requestor, Atom target, Atom property, Atom *AtomCache) {
    Atom type;
    int format;
    unsigned long nitems;
    flex fbuf;
    int status=MyGetProperty(dpy, requestor, property, True, AnyPropertyType, &type, &format, &nitems, fbuf);
    if(status!=Success) {
	fprintf(stderr, "xcolormap: getwindowproperty failed!\n");
	return;
    }
    if(type==ac_INCR) {
	if(inprogress.Fetch(incrkeyc(requestor,property))) fprintf(stderr, "xcolormap: BOGUS: INCR operation already underway...\n");
	inprogress.Insert(incrkeyc(requestor,property), incrdatac());
    } else HandleResult(self, dpy, requestor, target, type, format, fbuf, nitems, AtomCache);
}


static void HandleProperty(xcolormap *self, XPropertyEvent *pe) {
    Display *dpy=pe->display;
    if(pe->state!=PropertyNewValue) {
	return;
    }
    incrkeyc k(pe->window, pe->atom);
    incrdatac *dc=inprogress.Fetch(k);
    if(dc==NULL) {
	return;
    }
    Atom type;
    int format;
    unsigned long nitems;
    flex fbuf;
    int status=MyGetProperty(dpy, pe->window, pe->atom, True, AnyPropertyType, &type, &format, &nitems, fbuf);
    if(status!=Success) {
	DieDieDie(self);
	fprintf(stderr, "xcolormap: getwindowproperty failed!\n");
	return;
    }
    if(dc->Format()==0) {
	dc->SetFormat(format);
	dc->SetType(type);
    } else {
	if(dc->Type()!=type) {
	    fprintf(stderr, "xcolormap: type mismatch between parts of INCR transfer...\n");
	    DieDieDie(self);
	    return;
	}
    }
    if(nitems==0) {
	XSync(dpy, False);
	long nerr=ATKXErrors;
	HandleResult(self, dpy, pe->window, pe->atom, dc->Type(), dc->Format(), dc->Data(), dc->NItems(), self->AtomCache);
	inprogress.Remove(k);
	XSync(dpy, False);
	if(nerr!=ATKXErrors) {
	    DieDieDie(self);
	    return;
	}
	if(inprogress.GetN()==0) {
	    self->pending=FALSE;
	    TransferRequests(self);
	}
    } else {
	dc->AppendData(fbuf);
	dc->AppendItems(nitems);
    }
}

static void HandleSelection(xcolormap *self, XSelectionEvent *se) {
    Display *dpy=se->display;
    XSync(dpy, False);
    long nerr=ATKXErrors;
    if(se->property==None) {
	DieDieDie(self);
	fprintf(stderr, "xcolormap: color manager refused request!\n");
	return;
    } else if(se->target==xim_MULTIPLE) {
	Atom type;
	int format;
	unsigned long nitems;
	flex fbuf;
	int status=MyGetProperty(dpy, se->requestor, se->property, False, XA_ATOM, &type, &format, &nitems, fbuf);
	if(status!=Success || type!=XA_ATOM) {
	    DieDieDie(self);
	    fprintf(stderr, "xcolormap: getwindowproperty failed!\n");
	    return;
	}
	Atom *list=(Atom *)&fbuf[0];
	unsigned long i;
	for(i=0;i<nitems;i+=2) {
	    if(list[i]!=None) HandleTarget(self, dpy, se->requestor, list[i], list[i+1], self->AtomCache);
	}
	XDeleteProperty(dpy, se->requestor, se->property);

    } else HandleTarget(self, dpy, se->requestor, se->target, se->property, self->AtomCache);
    XSync(dpy, False);
    if(nerr!=ATKXErrors) {
	DieDieDie(self);
	return;
    }
    if(inprogress.GetN()==0) {
	self->pending=FALSE;
	TransferRequests(self);
    }
}

void xcolormap::HandleXEvent(XEvent *e) {
    if(e->xany.window==RootWindow(display, DefaultScreen(display)) && e->type==ClientMessage && e->xclient.message_type==ac_MANAGER) {
	XClientMessageEvent *cme=(XClientMessageEvent *)e;
	if(((Atom)cme->data.l[1])!=ac_ATK_COLORS || ((Time)cme->data.l[0])<timestamp) return;
	ClearOldManager(display, manager);
	manager=(Window)cme->data.l[2];
	XSync(display, False);
	long nerr=ATKXErrors;
	XSelectInput(display, manager, StructureNotifyMask);
	XSync(display, False);
	if(nerr!=ATKXErrors) return;
	timestamp=(Time)cme->data.l[0];
	startmanager=xcolormap_PromptStart;
        state=xcolormap_Running;
	NotifyObservers(xcolormap_NEWMANAGER);
	return;
    }
    if(e->xany.window==manager) {
	switch(e->type) {
	    case DestroyNotify: 
		manager=None;
		DieDieDie(this);
		break;
	    case ClientMessage:
		{
		XClientMessageEvent *cme=(XClientMessageEvent *)e;
		if(cme->message_type==ac_ATK_COLORS && cme->data.l[0]==0) {
		    if(!pending) {
			Atom list[4], *p=list;
			*p++=ac_PIXEL;
			*p++=ac_PIXEL;
			*p++=ac_ATK_COLORS;
			*p++=ac_ATK_COLORS;
			XSync(display, False);
			long nerr=ATKXErrors;
			XChangeProperty(display, colorprotocol, ac_MULTIPLE, XA_ATOM, 32, PropModeReplace, (unsigned char *)list, p-list);
			XSync(display, False);
			if(nerr!=ATKXErrors) {
			    DieDieDie(this);
			    return;
			}
			XConvertSelection(display, ac_ATK_COLORS, ac_MULTIPLE, ac_MULTIPLE, colorprotocol, timestamp);
			XSync(display, False);
			if(nerr!=ATKXErrors) {
			    DieDieDie(this);
			    return;
			}
		    }
		    evaluate=TRUE;
		    pending=TRUE;
		}
		}
	}
	return;
    }
    if(e->xany.window!=colorprotocol) return;
    switch(e->type) {
	case SelectionNotify:
	    HandleSelection(this, &e->xselection);
	    break;
	case PropertyNotify:
	    HandleProperty(this, &e->xproperty);
	    break;
	default: ;
    }
}
