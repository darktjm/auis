/* Copyright 1996 Carnegie Mellon University All rights reserved. */

/* awgtfigview.c	

	view widgets built built atop figures
 */

#include <andrewos.h>
ATK_IMPL("awgtfigview.H")
#include <awgtfigview.H>

ATKdefineRegistryNoInit(AWgtFigView, AWidgetView);

#include <im.H>
#include <print.H>
#include <rect.h>
#include <texttroff.H>
#include <figure.H>
#include <figattr.H>
#include <figorect.H>
#include <figogrp.H>
#include <aaction.H>
#include <figview.H>
#include <awgtfig.H>
#include <aslot.H>
#include <ashadow.H>

ASlot_NAME(printImage);
ATK_CLASS(AWgtFigView);


static char  debug;      /* This debug switch is toggled with ESC-^D-D */
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), this))


#define LINELENGTH (260)
/* CloneFigure
	Copy src figure to a group at rectangle loc within dest figure.
		xxx ought to scale image to space
			or at least clip to the space
		xxx need a clone method in figure
	Returns TRUE for success
*/
	static boolean
CloneFigure(figview *viewer, figure *src, figure *dest, 
				rectangle *loc) {
	char buf[LINELENGTH+1];
	char namebuf[100];
	long ix, oref, tid;
	figogrp *o;
	struct point tmppt;

	FILE *fp = tmpfile();

	// write from src
	src->Write(fp, im::GetWriteID(), 0);
	rewind(fp);

	// read header
	if (fgets(buf, LINELENGTH, fp) == NULL
			|| sscanf(buf, "\\begindata{%[^,],%ld}", 
						namebuf, &tid) != 2
			|| !ATK::IsTypeByName(namebuf, "figure")) {
		// xxx error no fig in cutbuffer
		fclose(fp);
		return FALSE;
	}

	// read as a group under root of 'dest'
	o = new figogrp;
	oref = dest->AlwaysInsertObject(o, dest->RootObjRef(), -1);
	ix = dest->ReadPartial(fp, 0, oref, &tmppt);
	fclose(fp);
	if (ix != dataobject::NOREADERROR) {
		// xxx ERROR Unable to read figure from cut buffer
		return FALSE;
	}

	// reposition to 'loc'
	o->Reposition(
		viewer->ToFigX(rectangle_Left(loc)) - tmppt.x,
		viewer->ToFigY(rectangle_Top(loc)) - tmppt.y);

	dest->NotifyObservers(figure_DATACHANGED);
	return TRUE;
}


AWgtFigView::AWgtFigView() {
	viewer = new figview;
	currfig = NULL;
	currfigshared = FALSE;
	needToPrepImage = TRUE;
}

AWgtFigView::~AWgtFigView() {
	viewer->Destroy();
	if (currfig)
		currfig->Destroy();
	currfig = NULL;
}

	void
AWgtFigView::LinkTree(view *parent)  {
	view::LinkTree(parent);
	if (parent)
		viewer->LinkTree(this);
	else
		viewer->LinkTree(NULL);
	// if currfig is shared, give it up
	if (currfigshared) {
		viewer->SetDataObject(NULL);
		currfig->Destroy();
		currfig = NULL;
		currfigshared = FALSE;
	}
}

	void 
AWgtFigView::InitChildren() {
	if (GetIM() && viewer->GetIM()!=GetIM())
		viewer->LinkTree(this);
	// xxx this is bogus.  need to prepImage of widget
	viewer->InitChildren();
}


void AWgtFigView::FullUpdate(enum view::UpdateType   type, 
			long left, long top, long width, long height)  {
	DEBUG(("FullUpdate(%d, %ld, %ld, %ld, %ld)\n", 
					type, left, top, width, height));
	AWgtFig *dobj = (class AWgtFig *)GetDataObject();
	struct rectangle fullbounds, inner;
	boolean hadcurrfig = TRUE;

	if ( ! dobj->GetResourceClass())
		dobj->FetchInitialResources(
					atom_def("WidgetSelector"));

	if ( ! currfig) {
		hadcurrfig = FALSE;
		if (GetParent()->IsType(class_AWgtFigView) && ! currfig) {
			// use parents figure
			currfig = ((AWgtFigView *)GetParent())->currfig;
			currfig->Reference();
			currfigshared = TRUE;
		}
		else {
			// create figure
			currfig = new figure;
			currfigshared = FALSE;
		}
		viewer->SetDataObject(currfig);
	}
	if (currfigshared) 
		GetEnclosedBounds(&fullbounds);
	else   GetLogicalBounds(&fullbounds);

	if(border) 
		border->InteriorRect(&fullbounds, &inner);
	else inner = fullbounds;

	if (needToPrepImage) {
		struct rectangle scaled;
		rectangle_SetRectSize(&scaled,
			(long)(rectangle_Left(&inner) * uxscale),
			(long)(rectangle_Top(&inner) * uyscale),
			(long)(rectangle_Width(&inner) * uxscale),
			(long)(rectangle_Height(&inner) * uyscale));

		aaction *reviseImage = dobj->reviseImage;
		aaction *prepImage = dobj->prepImage;
		figure *initialImage = dobj->initialImage;
		boolean purple = TRUE;

		if (hadcurrfig && reviseImage) {
			// not 1st time and have a reviseImage().  Call it
			dobj->reviseImage(dobj, avalueflex() | this 
				| currfig | &scaled);
			purple = FALSE;
		}
		else {
			// 1st time, or no reviseImage
			// try initialImage and prepImage
			if (initialImage) {
				// copy initialImage to currfig
				purple = ! CloneFigure(viewer, initialImage, 
							currfig, &inner);
			}
			if (prepImage) {
				dobj->prepImage(dobj, avalueflex() | this 
						| currfig | &scaled);
				purple = FALSE;
			}
		}
		if (purple) {
			// nothing else worked.  Make a purple blob.
			figorect *fobj = figorect::Create(
				viewer->ToFigX(rectangle_Left(&inner)),
				viewer->ToFigY(rectangle_Top(&inner)),
				viewer->ToFigW(rectangle_Width(&inner)),
				viewer->ToFigH(rectangle_Height(&inner)));
			traced_ptr<figattr> fatr;
			fatr->SetColor("#b200c6");
			fobj->UpdateVAttributes(fatr, 
					(long) (1<<figattr_Color) );
			currfig->AlwaysInsertObject(fobj, 
					currfig->RootObjRef(), // put in outer group
					-1);		// in front of other contents
		}
		needToPrepImage = FALSE;
	}

	if (type != view::Remove) 
		viewer->InsertView(this, &inner); 

	viewer->FullUpdate(type, left, top, width, height);

	// do border
	AWidgetView::FullUpdate(type, left, top, width, height);

	LEAVE(AWgtFigView_FullUpdate);
}

void AWgtFigView::Update() {

	ENTER(AWgtFigView_Update);
	updateRequested = FALSE;

	if (needToPrepImage) 
		FullUpdate(view::FullRedraw, 0, 0, 0, 0);
	else {
		viewer->Update();
		AWidgetView::Update();	// do border
	}

	LEAVE(AWgtFigView_Update);
}

view *AWgtFigView::Hit(enum view::MouseAction action, long x, long y, long n) {
	AWgtFig *dobj = (class AWgtFig *)GetDataObject();
	if((long)dobj->passMouse && viewer) viewer->Hit(action, x, y, n);
	return AWidgetView::Hit(action, x, y, n);
}

	view::DSattributes
AWgtFigView::DesiredSize(long  width, long  height, 
			enum view::DSpass  pass, 
			long  *dWidth, long  *dHeight)  {
	class AWidget *dobj = (class AWidget *)GetDataObject();
	view::DSattributes retval;
	long oldw, oldh;

	oldw = dobj->desiredWidth;
	oldh = dobj->desiredHeight;

	retval = AWidgetView::DesiredSize(width, height, pass,
		dWidth, dHeight);

	if (oldw != (long)dobj->desiredWidth  
			||  oldh != (long)dobj->desiredHeight)
		needToPrepImage = TRUE;

	return retval;
}


/* # # # # # # # # # # # # # # # # # 
 *	RECURSIVE SEARCH		
 *  # # # # # # # # # # # # # # # #  */

boolean
AWgtFigView::RecSearch(class search *pat, boolean toplevel) {
	return viewer->RecSearch(pat, toplevel);
}

boolean
AWgtFigView::RecSrchResume(class search *pat) {
	return viewer->RecSrchResume(pat);
}

boolean
AWgtFigView::RecSrchReplace(class dataobject *text, long pos, long len) {
	return viewer->RecSrchReplace(text, pos, len);
}

void
AWgtFigView::RecSrchExpose(const struct rectangle &logical, 
				struct rectangle &hit)  {
	viewer->RecSrchExpose(logical, hit);
}


/* # # # # # # # # # # # # # # 
 *		PRINTING		
 *  # # # # # # # # # # # # #  */

/* AWgtFigView::Print is the method that is used by the old printing mechanism. */
void
AWgtFigView::Print(FILE *file, const char *processor, const char	*format, boolean topLevel)  {
	class AWidget *dobj = (class AWidget *)GetDataObject();
	ASlot *pimage = dobj->Get(slot_printImage);

	if (pimage == NULL) 
		// use figureview to display image
		viewer->Print(file, processor, format, topLevel);
	else
		AWidgetView::Print(file, processor, format, topLevel);
}


void
AWgtFigView::PrintPSRect(FILE *file, long logwidth, long logheight, 
						 struct rectangle *visrect)  {
	AWidget *dobj = (AWidget *)GetDataObject();
	ASlot *pimage = dobj->Get(slot_printImage);

	if (pimage == NULL) 
		viewer->PrintPSRect(file, logwidth, logheight, visrect);
	else
		AWidgetView::PrintPSRect(file, logwidth, logheight, visrect);
}

	void 
AWgtFigView::ReceiveInputFocus()  {
	    AWidgetView::ReceiveInputFocus();
	    WantUpdate(this);
}
	void 
AWgtFigView::LoseInputFocus()  {
	    AWidgetView::LoseInputFocus();
	    WantUpdate(this);
}
/* 
 *   $Log: awgtfigview.C,v $
 * Revision 1.7  1996/06/14  17:14:23  robr
 * Added Receive/LoseInputFocus.
 * BUG
 *
 * Revision 1.6  1996/05/20  15:25:11  robr
 * /tmp/msg
 *
 * Revision 1.5  1996/05/13  17:14:08  wjh
 *  add to Update so AWidgetView::Update is called (to do border)
 *
 * Revision 1.4  1996/05/07  20:37:08  wjh
 * create CloneFigure
 * 	terrible: uses Write and Read
 * use CloneFigure to copy initialImage to views figure
 * introduce currfigshared if using parent's figure
 * rewrite FullUpdate, Update, and DesiredSize
 * remove printing; rely on AWidgetView.C
 *
 * Revision 1.3  1996/05/02  02:47:09  wjh
 * AFWidget --> AWgtFig; std hdr; reformatted ; reordered; removed changeScreenImage; revised FullUpdate and Update; added DesiredSize
 *
 */
