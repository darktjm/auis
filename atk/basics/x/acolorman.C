#include <andrewos.h>
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <stdlib.h>
#include <ahash.H>
#include <flex.H>
#include <quant.h>

static int MyGetProperty(
			 Display *display,
			 Window w,
			 Atom property,
			 Bool del,
			 Atom req_type,
			 Atom *actual_type_return,
			 int *actual_format_return,
			 unsigned int *nitems_return,
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
	    return status;
	}
	
	(*nitems_return)+=(unsigned int)nitems;
	size_t len=(*actual_format_return/8)*(*nitems_return);
	char *dest=prop.Insert(prop.GetN(), len);
	memcpy(dest, prop_return, len);
	start+=len;
	XFree(prop_return);
    } while(bytes_after_return>0);
    *prop.Append()='\0';
    return Success;
}

static char ErrMsg[120];
static long Nerrors = 0;
static boolean verbose=FALSE;

static int XErrorsToConsole(Display *DisplayPtr, XErrorEvent *ErrorBlock)
{
    Nerrors++;
    XGetErrorText(DisplayPtr, ErrorBlock->error_code, ErrMsg, sizeof(ErrMsg));
    if(ErrorBlock->error_code!=BadWindow || verbose) {
	fprintf(stderr, "acolorman: X error %d-%s.  Will ignore operation %d:%d on resource ID %lx.\n",
		ErrorBlock->error_code, ErrMsg, 
		ErrorBlock->request_code, ErrorBlock->minor_code,
		ErrorBlock->resourceid);
    }
    return 0;
}

class rgb {
  public:
    unsigned short r, g, b;
    inline rgb() {
	r=g=b=0;
    }
    inline rgb(const rgb &rhs) {
	r=rhs.r;
	g=rhs.g;
	b=rhs.b;
    }
    inline rgb &operator=(const rgb &rhs) {
	r=rhs.r;
	g=rhs.g;
	b=rhs.b;
	return *this;
    }
    inline size_t Hash() const {
	return (r<<16)^(g<<8)^b;
    }
    inline ~rgb() { }
    inline boolean operator==(const rgb &rhs){
	return rhs.r==r && rhs.g==g && rhs.b==b;
}
    
};

DEFINE_AMHASH_CLASS(rgblist,rgb,unsigned int);
DEFINE_AHASH_CLASS_ITER(rgblist,rgb,unsigned int);

typedef rgblist *rgblistp;
DEFINE_AMHASH_CLASS(windowlist_base,AHashLongKey,rgblistp);
DEFINE_AHASH_CLASS_ITER(windowlist_base,AHashLongKey,rgblistp);


class windowlist : public windowlist_base {
  public:
    ~windowlist() {
	windowlist_base_iter i(this);
	while(!i.done()) {
	    delete *i;
	    ++i;
	}
    }
};

rgblist colors;
boolean changed=FALSE;
boolean notify=FALSE;
Window manager;

static Atom supported[8];

#define XA_TARGETS supported[0]
#define XA_MULTIPLE supported[1]
#define TIMESTAMP supported[2]
#define ATK_ALLOCATE supported[3]
#define ATK_DEALLOCATE supported[4]
#define ATK_EVALUATE supported[5]
#define PIXEL supported[6]
#define ATK_COLORS supported[7]

DEFINE_AMHASH_CLASS(pixellist,rgb,CARD32);
DEFINE_AHASH_CLASS_ITER(pixellist,rgb,CARD32);

pixellist allocated;

static unsigned int maxcolors=64;

#define SCALEDOWN(x) ((x)>>(16-bpc))
#define SCALEUP(x) ((x)*65535U/((1<<bpc)-1))

static unsigned int bpc=6;
static int vclass=0;

static inline unsigned int ColorDistance(unsigned short r, unsigned short g, unsigned short b, unsigned short cred, unsigned short cgreen, unsigned short cblue) {
    int dr, dg, db;
    dr = (r - cred);	
    dg = (g - cgreen);	
    db = (b - cblue);	
    if(bpc>14) {
	dr = abs(dr)>>2;
	dg = abs(dg)>>2;
	db = abs(db)>>2;
    }
    return dr*dr + dg*dg + db*db;
}

static void ProcessColors(Display *dpy) {
    if(!changed) return;
    changed=FALSE;
    if(colors.GetN()==0) return;
    rgblist_iter iter(colors);
    colorhist_vector chv=(colorhist_vector)malloc(sizeof(colorhist_item)*colors.GetN());
    unsigned int i=0;
    unsigned int pixelcount=0;

    XColor *defs=NULL;
    Screen *s=DefaultScreenOfDisplay(dpy);
    int nc=CellsOfScreen(s);
    
    while(!iter.done()) {
	rgb &k=iter.Key();
	unsigned int count=*iter;
	if(count==0) {
	    colors.Remove(k);
	} else {
	    chv[i].value=count;
	    pixelcount+=count;
	    PPM_ASSIGN(chv[i].color, k.r, k.g, k.b);
	    ++i;
	}
	++iter;
    }
    static colorhist_vector oldcolors=NULL;
    static unsigned int oldbound=0;
    colorhist_vector newcolors=mediancut(chv, colors.GetN(), pixelcount, SCALEDOWN(65535), maxcolors);
    unsigned int upperbound=MIN(colors.GetN(),maxcolors);
    if(newcolors) {
	pixellist_iter iter4(allocated);
	rgb w;
	CARD32 *c;
	while(!iter4.done()) {
	    rgb &k=iter4.Key();
	    for(i=0;i<upperbound;i++) {
		if(k.r==PPM_GETR(newcolors[i].color) && k.g==PPM_GETG(newcolors[i].color) && k.b==PPM_GETB(newcolors[i].color)) break;
	    }
	    if(i>=upperbound) {
		unsigned long c = *iter4;
		XFreeColors(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &c, 1, 0);
		allocated.Remove(k);
		notify=TRUE;
	    }
	    ++iter4;
	}
	unsigned int failures=0;
	for(i=0;i<upperbound;i++) {
	    w.r=PPM_GETR(newcolors[i].color);
	    w.g=PPM_GETG(newcolors[i].color);
	    w.b=PPM_GETB(newcolors[i].color);
	    c=allocated.Fetch(w);
	    if(c==NULL) {
		XColor xc;
		xc.flags=DoRed|DoGreen|DoBlue;
		xc.red=SCALEUP(w.r);
		xc.green=SCALEUP(w.g);
		xc.blue=SCALEUP(w.b);
		notify=TRUE;
		Status status=XAllocColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &xc);
		if(!status) {
		    failures++;
		    if(nc<=1024) {
			int j;
			if(defs==NULL) {
			    defs=(XColor *)calloc(nc,sizeof(XColor));
			    if(defs) {
				for( j = 0; j < nc; j++ ) defs[j].pixel = j;
				XQueryColors(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), defs, nc);
			    }
			}
			if(defs) {
			    unsigned int diff, bestDiff=~0;
			    int bestDef=-1;
			    int flagged=0;
			    for(j=0;j<nc;j++) defs[j].flags=0;
			    retry:
			      for( j = 0; j < nc; j++ ) {
				  if(defs[j].flags==0) {
				      diff = ColorDistance(SCALEDOWN(defs[j].red), SCALEDOWN(defs[j].green), SCALEDOWN(defs[j].blue), w.r, w.g, w.b);
				      if( diff < bestDiff ) {
					  bestDiff = diff;
					  bestDef=j;
				      }
				  }
			      }
			    if(bestDef>0) {
				w.r= SCALEDOWN( defs[bestDef].red);
				w.g= SCALEDOWN( defs[bestDef].green);
				w.b= SCALEDOWN( defs[bestDef].blue);
				if(allocated.Fetch(w)==NULL) {
				    defs[bestDef].flags= DoRed|DoGreen|DoBlue;
				    status=XAllocColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &defs[bestDef]);
				    if(status) {
					CARD32 *p= allocated.Insert(w);
					*p=defs[bestDef].pixel;
				    } else {
					bestDef=-1;
					bestDiff=~0;
					goto retry;
				    }
				}
			    }
			}
		    }
		} else {
		    CARD32 *p=allocated.Insert(w);
		    *p=xc.pixel;
		}
	    }
		
	}
	if(failures>0) {
	    fprintf(stderr, "acolorman: %d color allocations failed.\n", failures);
	}
	free(newcolors);
	free(chv);
    }
    if(notify) {
	notify=FALSE;
	XClientMessageEvent cme;
	cme.type=ClientMessage;
	cme.window=manager;
	cme.message_type=ATK_COLORS;
	cme.format=32;
	cme.data.l[0]=0;
	cme.data.l[1]=0;
	cme.data.l[2]=0;
	cme.data.l[3]=0;
	cme.data.l[4]=0;
	Status status=XSendEvent(dpy, manager, False, StructureNotifyMask, (XEvent *)&cme);
    }
    if(defs) free(defs);

}


windowlist windows;

class incrkey {
  public:
    Window requestor;
    Atom property;
    inline incrkey(Window r, Atom p) {
	requestor=r;
	property=p;
    }
    inline incrkey() {
	requestor=None;
	property=None;
    }
    inline ~incrkey() { }
    inline size_t Hash() const {
	return requestor^property;
    }
    inline boolean operator==(const incrkey &key) {
	return (key.requestor==requestor)&&(key.property==property);
    }
};

class incrdata {
    Atom type;
    int format;
    unsigned char *data, *ptr;
    unsigned int bytes_left;
    boolean bad;
  public:
    inline incrdata() {
	bad=FALSE;
	type=None;
	format=0;
	data=ptr=NULL;
	bytes_left=0;
    }
    inline incrdata(Atom t, int f, unsigned char *d, unsigned int nitems) : type(t), format(f), data(d), ptr(d), bytes_left((format/8)*nitems) {
	bad=FALSE;
    }
    inline ~incrdata() {
    }
    inline incrdata(const incrdata &src) {
	type=src.type;
	format=src.format;
	data=src.data;
	ptr=src.ptr;
	bytes_left=src.bytes_left;
    }
    inline incrdata &operator=(const incrdata &src) {
	if(data) free(data);
	type=src.type;
	format=src.format;
	data=src.data;
	ptr=src.ptr;
	bytes_left=src.bytes_left;
	return *this;
    }
    inline void FreeData() {
	if(data) free(data);
    }
    inline unsigned char *Ptr() {
	return ptr;
    }
    inline unsigned int NItems(unsigned int bytes) {
	if(bytes>bytes_left) bytes=bytes_left;
	return bytes/(format/8);
    }
    inline void Consume(unsigned int n) {
	unsigned int b=n*(format/8);
	if(b<bytes_left) {
	    bytes_left-=b;
	    ptr+=b;
	} else bytes_left=0;
    }
    inline Atom Type() {
	return type;
    }
    inline int Format() {
	return format;
    }
    inline boolean Bad() {
	return bad;
    }
    inline void SetBad() {
	bad=TRUE;
    }
};

DEFINE_AOHASH_CLASS(incrss, incrkey, incrdata);

incrss inprogress;
Time start=0;

static void RefuseRequest(Display *dpy, XSelectionRequestEvent *sre) {
    XSelectionEvent reply;
    reply.type=SelectionNotify;
    reply.requestor=sre->requestor;
    reply.selection=sre->selection;
    reply.target=sre->target;
    reply.property=None;
    XSendEvent(dpy, reply.requestor, False, NoEventMask, (XEvent *)&reply);
    // If we can't tell the requestor the request has been refused I don't see what we can do...
}

Atom INCR;
Atom MANAGER;

#define MAXBLOCK 1024
static void MyChangeProperty(Display *dpy, Window requestor, Atom property, Atom type, int format, unsigned char *data, unsigned int nitems) {
    if(nitems*(format/8)>MAXBLOCK) {
	incrkey k(requestor, property);
	//  data is now handed off to an incrdata, which will free it when the transfer is completed.
	inprogress.Insert(k, incrdata(type, format, data, nitems));
	unsigned int size=nitems*(format/8);
	XChangeProperty(dpy, requestor, property, INCR, 32, PropModeReplace, (unsigned char *)&size, 1);
    } else {
	XChangeProperty(dpy, requestor, property, type, format, PropModeReplace, data, nitems);
	free(data);
    }
}
// If the XSelectInput or an XChangeProperty fails in here HandleTarget should return FALSE.
static boolean HandleTarget(Display *dpy, Window requestor, Atom target, Atom property) {
    Nerrors=0;
    if(target==XA_TARGETS) {
	XChangeProperty(dpy, requestor, property, XA_ATOM, 32, PropModeReplace, (unsigned char *)supported, sizeof(supported)/sizeof(Atom));
    } else if(target==TIMESTAMP) {
	XChangeProperty(dpy, requestor, property, XA_INTEGER, 32, PropModeReplace, (unsigned char *)&start,1);
    } else if(target==ATK_ALLOCATE) {
	Atom type;
	int format;
	unsigned int nitems;
	flex fbuf;
	int result=MyGetProperty(dpy, requestor, property, False, ATK_ALLOCATE, &type, &format, &nitems, fbuf);
	if(result!=Success || type!=ATK_ALLOCATE) {
	    return FALSE;
	}
	if(nitems==0) return TRUE;
	XSelectInput(dpy, requestor, PropertyChangeMask|StructureNotifyMask);
	XSync(dpy, False);
	if(Nerrors!=0) return FALSE;
	unsigned short *list=(unsigned short *)&fbuf[0];
	size_t i;
	rgblist **wrgblistp=windows.Fetch(requestor);
	rgblist *wrgblist;
	if(wrgblistp==NULL) {
	    wrgblistp=windows.Insert(requestor);
	    *wrgblistp=new rgblist;
	}
	wrgblist=*wrgblistp;
	for(i=0;i<nitems-3;i+=4) {
	    // list[i+3] is the occurance count for the color.
	    // if the count is actually >65535 the color will be repeated
	    // with a count of 65535 as needed...
	    rgb c;
	    c.r=SCALEDOWN(list[i]);
	    c.g=SCALEDOWN(list[i+1]);
	    c.b=SCALEDOWN(list[i+2]);
	    if(vclass==GrayScale) {
		unsigned int lum=(299u*c.r)/1000+(587u*c.g)/1000+(114u*c.b)/1000;
		c.r=lum;
		c.g=lum;
		c.b=lum;
	    }
	    unsigned int h=list[i+3];
	    unsigned int *f=colors.Fetch(c);
	    if(f) (*f)+=h;
	    else {
		f=colors.Insert(c, h);
		changed=TRUE;
	    }
	    f=wrgblist->Fetch(c);
	    if(f) (*f)+=h;
	    else f=wrgblist->Insert(c, h);
	}
	XChangeProperty(dpy, requestor, property, ATK_ALLOCATE, 32, PropModeReplace, (unsigned char *)&requestor, 0);
    } else if(target==ATK_DEALLOCATE) {
	Atom type;
	int format;
	unsigned int nitems;
	flex fbuf;
	int result=MyGetProperty(dpy, requestor, property, False, ATK_DEALLOCATE, &type, &format, &nitems, fbuf);
	if(result!=Success || type!=ATK_DEALLOCATE) {
	    return FALSE;
	}
	if(nitems==0) return TRUE;
	unsigned short *list=(unsigned short *)&fbuf[0];
	size_t i;
	rgblist **wrgblistp=windows.Fetch(requestor);
	rgblist *wrgblist;
	if(wrgblistp==NULL) {
	    wrgblistp=windows.Insert(requestor);
	    *wrgblistp=new rgblist;
	}
	wrgblist=*wrgblistp;
	for(i=0;i<nitems-3;i+=4) {
	    rgb c;
	    c.r=SCALEDOWN(list[i]);
	    c.g=SCALEDOWN(list[i+1]);
	    c.b=SCALEDOWN(list[i+2]);
	    unsigned int h=list[i+3];
	    if(vclass==GrayScale) {
		unsigned int lum=(299u*c.r)/1000+(587u*c.g)/1000+(114u*c.b)/1000;
		c.r=lum;
		c.g=lum;
		c.b=lum;
	    }
	    unsigned int *f=wrgblist->Fetch(c);
	    if(f) {
		if(*f>h) (*f)-=h;
		else *f=0;
		if(*f==0) {
		    wrgblist->Remove(c);
		}
		f=colors.Fetch(c);
		if(f) {
		    if(*f>h) (*f)-=h;
		    else *f=0;
		    if(*f==0) {
			changed=TRUE;
		    }
		}
	    }
	}
	XChangeProperty(dpy, requestor, property, ATK_DEALLOCATE, 32, PropModeReplace, (unsigned char *)&requestor, 0);
    } else if(target==ATK_EVALUATE) {
	ProcessColors(dpy);
	XChangeProperty(dpy, requestor, property, ATK_EVALUATE, 32, PropModeReplace, (unsigned char *)&requestor, 0);
    } else if(target==PIXEL) {
	pixellist_iter iter(allocated);
	CARD32 *pixels=(CARD32 *)malloc(allocated.GetN()*sizeof(CARD32)), *p=pixels;
	while(!iter.done()) {
	    *p=*iter;
	    p++;
	    ++iter;
	}
	MyChangeProperty(dpy, requestor, property, PIXEL, 32, (unsigned char *)pixels, allocated.GetN());
    } else if(target==ATK_COLORS) {
	pixellist_iter iter(allocated);
	unsigned short *colors=(unsigned short *)malloc(allocated.GetN()*3*sizeof(unsigned short)), *c=colors;
	while(!iter.done()) {
	    rgb &rgbc=iter.Key();
	    c[0]=SCALEUP(rgbc.r);
	    c[1]=SCALEUP(rgbc.g);
	    c[2]=SCALEUP(rgbc.b);
	    c+=3;
	    ++iter;
	}
	MyChangeProperty(dpy, requestor, property, XA_CARDINAL, 16, (unsigned char *)colors, allocated.GetN()*3);
    }
    else {
	return FALSE;
    }
    XSync(dpy, False);
    if(Nerrors==0) return TRUE;
    else {
	Nerrors=0;
	return FALSE;
    }
}

static void HandleProperty(XPropertyEvent *pe) {
    if(pe->state!=PropertyDelete) {
	return;
    }
    Display *dpy=pe->display;
    incrkey k(pe->window, pe->atom);
    incrdata *src=inprogress.Fetch(k);
    if(src==NULL) {
	return;
    }
    unsigned int nitems;
    nitems=src->NItems(MAXBLOCK);
    XSync(dpy, False);
    Nerrors=0;
    if(nitems && !src->Bad()) {
	XChangeProperty(dpy, pe->window, pe->atom, src->Type(), src->Format(), PropModeAppend, (unsigned char *)src->Ptr(), nitems);
	src->Consume(nitems);
    } else {
	XChangeProperty(dpy, pe->window, pe->atom, src->Type(), src->Format(), PropModeAppend, (unsigned char *)src->Ptr(), 0);
	src->FreeData();
	inprogress.Remove(k);
	return;
    }
    XSync(dpy, False);
    if(Nerrors>0) {
	src->SetBad();
    }
}

static void HandleRequest(XSelectionRequestEvent *sre) {
    Display *dpy=sre->display;
    if(sre->time<start || (sre->target==XA_MULTIPLE && sre->property==None)) {
	RefuseRequest(dpy, sre);
	return;
    }
    if(sre->property==None) sre->property=sre->target;

    if(sre->target!=XA_MULTIPLE) {
	XSync(dpy, False);
	if(!HandleTarget(dpy, sre->requestor, sre->target, sre->property)) {
	    RefuseRequest(dpy, sre);
	    return;
	}
    } else {
	Atom type;
	int format;
	unsigned int nitems;
	flex fbuf;
	int result=MyGetProperty(dpy, sre->requestor, sre->property, False, XA_ATOM, &type, &format, &nitems, fbuf);
	if(result!=Success || type!=XA_ATOM || nitems==0) {
	    RefuseRequest(dpy, sre);
	    return;
	}
	Atom *list=(Atom *)&fbuf[0];
	boolean failed=FALSE;
	unsigned long i;
	XSync(dpy, False);
	for(i=0;i<nitems;i+=2) {
	    if(!HandleTarget(dpy, sre->requestor, list[i], list[i+1])) {
		list[i]=None;
		failed=TRUE;
	    }
	}
	if(failed) {
	    XChangeProperty(dpy, sre->requestor, sre->property, XA_ATOM, 32, PropModeReplace, (unsigned char *)&fbuf[0], nitems);
	}
    }
    XSelectionEvent reply;
    reply.type=SelectionNotify;
    reply.requestor=sre->requestor;
    reply.selection=sre->selection;
    reply.target=sre->target;
    reply.property=sre->property;
    Status status=XSendEvent(dpy, reply.requestor, False, NoEventMask, (XEvent *)&reply);
}

static void ClearOldManager(Display *display, Window manager) {
    XEvent event;
    // disregard any further events from the old manager window...
    XSelectInput(display, manager, None);
    // sync up so we have all possible events for the old manager...
    XSync(display, False);
    // get rid of any old DestroyNotify events for the old manager.  Otherwise they might
    // be applied to the new manager....
    XCheckTypedWindowEvent(display, manager, DestroyNotify, &event);
}

static Window FindManager(Display *display, Window mywin, Time *tstamp=NULL) {
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
	long nerr=Nerrors;
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
	while(nerr==Nerrors && !XCheckTypedWindowEvent(display, mywin, SelectionNotify,
						       &event)) XSync(display, False);
	if(nerr!=Nerrors) continue;
	if(event.type!=SelectionNotify) {
	    fprintf(stderr, "acolorman: error getting timestamp on color manager selection... \n");
	    continue;
	}
	if(event.xselection.property==None) {
	    fprintf(stderr, "acolorman: No color manager available...\n");
	    ClearOldManager(display, manager);
	    manager=None;
	    break;
	}
	Atom actual_type_return;
	int actual_format_return;
	unsigned long bytes_after_return=0, start=0;
	unsigned char *prop_return=NULL;
	unsigned long nitems;
	int status=XGetWindowProperty(display, mywin, XA_INTEGER, 0, 1, TRUE, XA_INTEGER, &actual_type_return, &actual_format_return, &nitems, &bytes_after_return, &prop_return);
	if(status!=Success || nitems==0 || actual_type_return!=XA_INTEGER || actual_format_return!=32 || prop_return==NULL) {
	    fprintf(stderr, "acolorman: error getting timestamp on color manager selection...\n");
	    continue;
	}
	XSync(display, False);
	// All the PropertyNotify events ought to be in the queue...
	while(XCheckTypedWindowEvent(display, mywin, PropertyNotify,
				     &event));
	if(nerr!=Nerrors) continue;
	// chewed the PropertyNotify for the ChangeProperty call done by the s
	t=*(Time *)prop_return;
	XFree(prop_return);
	if(t<timestamp) break;
    } while(--retries>0);
    if(retries==0) {
	manager=None;
    }
    if(tstamp) *tstamp=timestamp;
    return manager;
}

int main(int argc, char **argv) {
#ifdef NEED_ATKINIFINI
    ATK_DoIniFini();
#endif
    argc--;
    argv++;
    char *dpystr=NULL;
    char *colorq=getenv("ATKCOLORMANAGERQUOTA");
    if(colorq) maxcolors=atoi(colorq);
    while(argc>0) {
	if(argc>=2) {
	    if(strcmp(argv[0], "-n")==0) {
		maxcolors=atoi(argv[1]);
		argv++;
		argc--;
	    } else if(strcmp(argv[0], "-display")==0) {
		dpystr=argv[1];
		argv++;
		argc--;
	    }
	}
	argv++;
	argc--;
    }
    Display *dpy=XOpenDisplay(dpystr);
    if(dpy==NULL) {
	fprintf(stderr, "acolorman: Couldn't connect to X server.\n");
	exit(-1);
    }
    Visual *visual= DefaultVisual(dpy,DefaultScreen(dpy));
    if(visual==NULL) {
	fprintf(stderr, "acolorman: Couldn't find information about the default visual.\n");
	exit(-1);
    }
    if(visual->c_class!=PseudoColor  && visual->c_class!=GrayScale) {
	fprintf(stderr, "acolorman: The color manager is used only on PseudoColor or GrayScale displays.\n");
	exit(-1);
    }
    vclass=visual->c_class;
    bpc=visual->bits_per_rgb;
    if(visual->map_entries<maxcolors) {
	fprintf(stderr, "acolorman: Visual has only %d colors, a quota of %d was requested.\n", visual->map_entries, maxcolors);
	maxcolors=visual->map_entries;
    }
    
    XSetErrorHandler(XErrorsToConsole);
    supported[0]=XInternAtom(dpy, "TARGETS", FALSE);
    supported[1]=XInternAtom(dpy, "MULTIPLE", FALSE);
    supported[2]=XInternAtom(dpy, "TIMESTAMP", FALSE);
    ATK_ALLOCATE=XInternAtom(dpy, "ATK_ALLOCATE", FALSE);
    ATK_DEALLOCATE=XInternAtom(dpy, "ATK_DEALLOCATE", FALSE);
    ATK_EVALUATE=XInternAtom(dpy, "ATK_EVALUATE", FALSE);
    ATK_COLORS=XInternAtom(dpy, "ATK_COLORS", FALSE);
    PIXEL=XInternAtom(dpy, "PIXEL", FALSE);
    INCR=XInternAtom(dpy, "INCR", FALSE);
    MANAGER=XInternAtom(dpy, "MANAGER", FALSE);
    if(Nerrors!=0) {
	fprintf(stderr, "acolorman:Couldn't allocate all the X atoms needed.\n");
	exit(-1);
    }
    Window root=RootWindow(dpy, DefaultScreen(dpy));
    Window w=XCreateSimpleWindow(dpy, root, 99, 42, 42, 99, 0, 0, 0);
    manager=w;
    XSelectInput(dpy, w, PropertyChangeMask);
    Window oldmanager=FindManager(dpy, w);
    XSync(dpy, False);
    Nerrors=0;
    XChangeProperty(dpy, w, ATK_COLORS, XA_ATOM, 32, PropModeAppend, (unsigned char *)&w, 0);
    XEvent event;
    XWindowEvent(dpy, w, PropertyChangeMask, &event);
    start=event.xproperty.time;
    XSetSelectionOwner(dpy, ATK_COLORS, w, event.xproperty.time);
    XSync(dpy, False);
    if(Nerrors!=0) {
	fprintf(stderr, "acolorman: Couldn't set selection owner.\n");
	exit(-1);
    }
    if(oldmanager!=None) {
	fprintf(stderr, "acolorman: Waiting for old manager to exit.\n");
	while(Nerrors==0 && !XCheckTypedWindowEvent(dpy, oldmanager, DestroyNotify,
						    &event)) XSync(dpy, 0);
	if(Nerrors!=0) {
	    fprintf(stderr, "acolorman: Error encountered while waiting for old manager to exit.\n");
	    exit(-1);
	} else fprintf(stderr, "acolorman: Old manager exited, taking over color management.\n");
    }
    Window o=XGetSelectionOwner(dpy, ATK_COLORS);
    // this test ought to cover any failures up to this point
    if(o!=w) {
	fprintf(stderr, "acolorman: Lost selection during startup.\n");
	exit(-2);
    }
    XClientMessageEvent cme;
    cme.type=ClientMessage;
    cme.window=RootWindow(dpy, DefaultScreen(dpy));
    cme.message_type=MANAGER;
    cme.format=32;
    cme.data.l[0]=event.xproperty.time;
    cme.data.l[1]=ATK_COLORS;
    cme.data.l[2]=w;
    cme.data.l[3]=0;
    cme.data.l[4]=0;
    Status status=XSendEvent(dpy, RootWindow(dpy, DefaultScreen(dpy)), False, StructureNotifyMask, (XEvent *)&cme);
    XSync(dpy, False);
    if(Nerrors>0) {
	fprintf(stderr, "acolorman: Couldn't get selection, or couldn't notify clients.\n");
	exit(-1);
    }
    while(1) {
	fflush(stdout);
	fflush(stderr);
	XNextEvent(dpy, &event);
	if(event.xany.window==w) {
	    switch(event.type) {
		case SelectionClear: {
		    return 0;
		    }
		case SelectionRequest:
		    HandleRequest(&event.xselectionrequest);
		    break;
		default: ;
	    }
	} else {
	    switch(event.type) {
		case PropertyNotify:
		    HandleProperty(&event.xproperty);
		    break;
		case DestroyNotify: {
		    rgblist **wrgblistp=windows.Fetch(event.xany.window);
		    rgblist *wrgblist;
		    if(wrgblistp==NULL) continue;
		    wrgblist=*wrgblistp;
		    rgblist_iter iter(wrgblist);
		    while(!iter.done()) {
			rgb &c=iter.Key();
			unsigned int count=*iter;
			unsigned int *f=colors.Fetch(c);
			if(f) {
			    changed=TRUE;
			    if(count>=*f) {
				*f=0;
			    } else *f-=count;
			}
			++iter;
		    }
		    windows.Remove(event.xany.window);
		    delete wrgblist;
		    ProcessColors(dpy);
		    }
		    break;
	    }
	}

    }
    return 0;
}
