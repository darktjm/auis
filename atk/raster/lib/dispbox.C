/***********************************************************
  Copyright IBM Corporation 1991

  All Rights Reserved

  ******************************************************************/

/* dispbox.c

  Support for the Display Box in Expanded Mode in rasterv.c

      */

#include <andrewos.h> /* strings.h */
#include <graphic.H>
#include <view.H>
#include <fontdesc.H>

#include <rect.h>

#include <rasterimage.H>
#include <raster.H>
#include <rasterview.H>

#include <dispbox.h>
#include <rastvaux.h>

#ifdef DB
static char debug;
#endif
void DisplayBoxBlitOverlap(class rasterview  *self, class rasterimage  *pix);
#if 0
void DisplayBoxDrawPanHighlight(class rasterview  *self);
#endif /* 0 */
void rasterview_DisplayBoxWritePixImageFull(class rasterview  *self, class graphic  *G, class rasterimage  *pix);
void rasterview_DisplayBoxWritePixImage(class rasterview  *self, class graphic  *G);
void rasterview_DisplayBoxHide(class rasterview  *self);
void rasterview_DisplayBoxDrawHighlight(class rasterview  *self, class graphic  *G);
void rasterview_DisplayBoxDrawHighlightGray(class rasterview  *self, class graphic  *G);
void rasterview_DisplayBoxHideHighlight(class rasterview  *self, class graphic  *G);
void rasterview_DrawHighlightBehindDisplayBox(class rasterview  *self, class graphic  *G, boolean  gray);
void rasterview_DisplayBoxHideOverlappingHighlight(class rasterview  *self, class graphic  *G, class rasterimage  *pix);
void rasterview_SetPixelBehindDisplayBox(class rasterview  *self, class rasterimage  *pix, long  x , long  y, boolean  bit);


void DisplayBoxBlitOverlap(class rasterview  *self, class rasterimage  *pix)
{
    struct rectangle SR;
    struct rectangle DB;
    struct rectangle overlap;

    if (self->MovingDisplayBox) return;

    if (self->DisplayBoxHidden) {
	SetLeftRect(&self->DisplayBox, -3*rectangle_Width(&self->DisplayBox));
	return; }

    ENTER(DisplayBoxBlitOverlap);
    DEBUG(("Offset: (%ld,%ld)\n", self->Xoff, self->Yoff));

    /* Note that the Display Box uses a double sized Border so that we can draw a frame around the box as well as leaving room to show the selected region if the selected region includes the edges of the expanded image. */
    DB = self->DisplayBox;
    InsetRect(&DB, -TWOBORDER, -TWOBORDER);
    /* Translate Display Box coordinates to PixelImage coordiantes. */
    OffsetRect(&DB, self->Xoff, self->Yoff);
    DEBUG(("DB: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DB), rectangle_Top(&DB),
	    rectangle_Width(&DB), rectangle_Height(&DB)));

    rectangle_SetRectSize(&SR, 0, 0,
			   (self->Expansion)->GetWidth(),
			   (self->Expansion)->GetHeight());
    DEBUG(("SR: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&SR), rectangle_Top(&SR),
	    rectangle_Width(&SR), rectangle_Height(&SR)));

    rectangle_IntersectRect(&overlap, &DB, &SR);
    if (IsNotEmptyRect(&overlap)) {
	/* The Display Box overlaps the pixelimg. Blit onto the image the portion of the Display Box which overlaps. */
	DEBUG(("Black: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&overlap), rectangle_Top(&overlap),
	       rectangle_Width(&overlap), rectangle_Height(&overlap)));
	(self->Expansion)->PaintSubraster( &overlap, BLACKBYTE);

	InsetRect(&DB, 1, 1);
	rectangle_IntersectRect(&overlap, &DB, &SR);
	if (IsNotEmptyRect(&overlap)) {
	    DEBUG(("White: (%ld,%ld,%ld,%ld)\n",
		   rectangle_Left(&overlap), rectangle_Top(&overlap),
		   rectangle_Width(&overlap), rectangle_Height(&overlap)));
	    (self->Expansion)->PaintSubraster( &overlap, WHITEBYTE); } }
    LEAVE(DisplayBoxBlitOverlap);
}

#if 0
void DisplayBoxDrawPanHighlight(class rasterview  *self)
{
    class graphic *G = (self)->GetDrawable();
    struct rectangle DS;
    struct rectangle DBS;

    DS = self->ViewSelection;
    DBS = self->DisplayBoxSelection;

    if (self->DisplayBoxHidden) return;

    DEBUG(("DBDrawPanHighlight: (%d,%d,%d,%d)\n       DBscroll: (%d,%d)\n",
	    rectangle_Left(&self->DisplayBox),
	    rectangle_Top(&self->DisplayBox),
	    rectangle_Width(&self->DisplayBox),
	    rectangle_Height(&self->DisplayBox),
	    self->DBXscroll, self->DBYscroll));

    InsetRect(&DBS, -BORDER, -BORDER);
    if (IsEnclosedBy(&DBS, &DS)) return;
    rectangle_IntersectRect(&DS, &DBS, &DS);
    if (IsEmptyRect(&DS)) return;

    /* Translate to Display Box coordinates. */
    SetLeftRect(&DS,
		 rectangle_Left(&DS) - rectangle_Left(&self->DisplayBoxSelection) +
		 rectangle_Left(&self->DisplayBox) - self->DBXscroll);
    SetTopRect(&DS,
		rectangle_Top(&DS) - rectangle_Top(&self->DisplayBoxSelection) +
		rectangle_Top(&self->DisplayBox) - self->DBYscroll);

    DEBUG(("DBHighlight: (%d,%d,%d,%d)\n",
	    rectangle_Left(&DS), rectangle_Top(&DS),
	    rectangle_Width(&DS), rectangle_Height(&DS)));

    DrawHighlightScreenCoordinates(self, G, DS, graphic::BLACK, graphic::WHITE);
}
#endif /* 0 */

/* Write Pixel Image of the contents of the Display Box. */
void rasterview_DisplayBoxWritePixImageFull(class rasterview  *self, class graphic  *G, class rasterimage  *pix)
{
    struct rectangle DB;

    if (self->MovingDisplayBox) return;

    DB = self->DisplayBox;
    DEBUG(("DBWriteFull: (%ld,%ld,%ld,%ld) Offset: (%ld,%ld)\n",
	    rectangle_Left(&DB), rectangle_Top(&DB),
	    rectangle_Width(&DB), rectangle_Height(&DB),
	    self->DBXscroll, self->DBYscroll));

    /* Write the entire Display Box back on the screen. */
    InsetRect(&DB, -TWOBORDER, -TWOBORDER);
    (self)->SetTransferMode( graphic::BLACK);
    (self)->FillRect( &DB, self->BlackPattern);
    InsetRect(&DB, 1, 1);
    (self)->SetTransferMode( graphic::WHITE);
    (self)->FillRect( &DB, self->WhitePattern);
    InsetRect(&DB, TWOBORDER - 1, TWOBORDER - 1);
    (self)->SetTransferMode( graphic::COPY);
    (G)->WritePixImage(
			   rectangle_Left(&DB) - self->DBXscroll,
			   rectangle_Top(&DB) - self->DBYscroll,
			   pix,
			   rectangle_Left(&self->DisplayBoxSelection),
			   rectangle_Top(&self->DisplayBoxSelection),
			   rectangle_Width(&self->DisplayBoxSelection),
			   rectangle_Height(&self->DisplayBoxSelection));
}

/* Write Pixel Image of the contents of the Display Box assuming a White area on which to write. */
void rasterview_DisplayBoxWritePixImage(class rasterview  *self, class graphic  *G)
{
    struct rectangle DB;

    if (self->MovingDisplayBox) return;

    DB = self->DisplayBox;

    DEBUG(("DBWritePixImage: (%ld,%ld,%ld,%ld)\n            DBS: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DB) - self->DBXscroll,
	    rectangle_Top(&DB) - self->DBYscroll,
	    rectangle_Width(&DB), rectangle_Height(&DB),
	    rectangle_Left(&self->DisplayBoxSelection),
	    rectangle_Top(&self->DisplayBoxSelection),
	    rectangle_Width(&self->DisplayBoxSelection),
	    rectangle_Height(&self->DisplayBoxSelection)));
    (self)->SetTransferMode( graphic::COPY);
    (G)->WritePixImage(
			   rectangle_Left(&DB) - self->DBXscroll,
			   rectangle_Top(&DB) - self->DBYscroll,
			   ((class raster *)self->GetDataObject())->GetPix(),
			   rectangle_Left(&self->DisplayBoxSelection),
			   rectangle_Top(&self->DisplayBoxSelection),
			   rectangle_Width(&self->DisplayBoxSelection),
			   rectangle_Height(&self->DisplayBoxSelection));
    /* Draw the Display Box Frame. */
    InsetRect(&DB, -BORDER, -BORDER);
    DEBUG(("        DBFrame: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DB), rectangle_Top(&DB),
	    rectangle_Width(&DB), rectangle_Height(&DB)));
    DrawHighlightScreenCoordinates(self, G, DB, graphic::BLACK, graphic::WHITE);
}

void rasterview_DisplayBoxHide(class rasterview  *self)
{
    class raster *ras = (class raster *)self->GetDataObject();
    class graphic *G = (self)->GetDrawable();
    struct rectangle VB;
    long clipw = (self->Expansion)->GetWidth();
    long cliph = (self->Expansion)->GetHeight();

    if ((ras == NULL) || ((ras)->GetPix() == NULL))  return;
    /* Don't try to hide if already hidden. */
    if (self->DisplayBoxHidden) return;

    (self)->GetVisualBounds( &VB);

    DEBUG(("Current Display Box: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&self->DisplayBox),
	    rectangle_Top(&self->DisplayBox),
	    rectangle_Width(&self->DisplayBox),
	    rectangle_Height(&self->DisplayBox)));

    /* This part covers over the existing Display Box (including the double frame) */
    if (IsNotEmptyRect(&self->DisplayBox)) {
	struct rectangle DB, SR, R;
	long x, y;
	long l, t, r, b;
	long Scale = self->Scale;

	DB = self->DisplayBox;
	rectangle_SetRectSize(&SR, 0, 0,
			      (self->Expansion)->GetWidth(),
			      (self->Expansion)->GetHeight());

	R = self->DisplayBox;
	InsetRect(&R, -TWOBORDER, -TWOBORDER);
	x = rectangle_Left(&R);
	x = ((x < -self->Xscroll) ? -self->Xoff : x);
	y = rectangle_Top(&R);
	y = ((y < -self->Yscroll) ? -self->Yoff : y);
	DEBUG(("WhiteOut Display Box: (%ld,%ld,%ld,%ld)\n   Offset: (%ld,%ld)\n",
	       rectangle_Left(&R), rectangle_Top(&R),
	       rectangle_Width(&R), rectangle_Height(&R),
	      self->Xoff, self->Yoff));
	(self)->SetTransferMode( graphic::WHITE);
	(self)->FillRect( &R, self->WhitePattern);

	/* convert the Display Boxes Screen coordinates to pixelimage coordinates. */
	OffsetRect(&R, self->Xoff, self->Yoff);
	rectangle_IntersectRect(&R, &R, &SR);
	rectangle_GetRectSize(&R, &l, &t, &r, &b);
	r += l;
	b += t;
	l -= (l % Scale);
	t -= (t % Scale);
	r += Scale - (r % Scale);
	b += Scale - (b % Scale);

	rectangle_SetRectSize(&SR,
			      l/Scale + rectangle_Left(&self->DisplayBoxSelection),
			      t/Scale + rectangle_Top(&self->DisplayBoxSelection),
			      (r-l)/Scale, (b-t)/Scale);

	/* Flag ReflectChangesInExpansion to NOT blit a Display Box into the image. */
	rectangle_EmptyRect(&self->DisplayBox);
	rasterview_ReflectChangesInExpansion(self, &SR);
	self->DisplayBox = DB;
	(self)->SetTransferMode( graphic::COPY);
	DEBUG(("Write Display Box: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&R),
	       rectangle_Top(&R),
	       rectangle_Width(&R),
	       rectangle_Height(&R)));
	ClipAndWritePixImage(clipw, cliph, G,
			     x, y,
			     self->Expansion,
			     rectangle_Left(&R), rectangle_Top(&R),
			     rectangle_Width(&R), rectangle_Height(&R));
    }
}

void rasterview_DisplayBoxDrawHighlight(class rasterview  *self, class graphic  *G)
{
    struct rectangle DS;
    struct rectangle DBS;

    DS = self->DesiredSelection;
    DBS = self->DisplayBoxSelection;

    if (self->DisplayBoxHidden) return;

    DEBUG(("DBDrawHighlight: (%ld,%ld,%ld,%ld)\n       DBscroll: (%ld,%ld)\n",
	    rectangle_Left(&self->DisplayBox),
	    rectangle_Top(&self->DisplayBox),
	    rectangle_Width(&self->DisplayBox),
	    rectangle_Height(&self->DisplayBox),
	    self->DBXscroll, self->DBYscroll));

    InsetRect(&DBS, -BORDER, -BORDER);
    if (IsEnclosedBy(&DBS, &DS)) return;
    rectangle_IntersectRect(&DS, &DBS, &DS);
    if (IsEmptyRect(&DS)) return;

    /* Translate to Display Box coordinates. */
    SetLeftRect(&DS,
		 rectangle_Left(&DS) - rectangle_Left(&self->DisplayBoxSelection) +
		 rectangle_Left(&self->DisplayBox) - self->DBXscroll);
    SetTopRect(&DS,
		rectangle_Top(&DS) - rectangle_Top(&self->DisplayBoxSelection) +
		rectangle_Top(&self->DisplayBox) - self->DBYscroll);

    DEBUG(("DBHighlight: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DS), rectangle_Top(&DS),
	    rectangle_Width(&DS), rectangle_Height(&DS)));

    DrawHighlightScreenCoordinates(self, G, DS, graphic::BLACK, graphic::WHITE);
}

void rasterview_DisplayBoxDrawHighlightGray(class rasterview  *self, class graphic  *G)
{
    struct rectangle DS;
    struct rectangle DBS;
    long l, t, w, h;

    DS = self->DesiredSelection;
    DBS = self->DisplayBoxSelection;

    InsetRect(&DBS, -BORDER, -BORDER);
    if (IsEnclosedBy(&DBS, &DS)) return;
    rectangle_IntersectRect(&DS, &DBS, &DS);
    if (IsEmptyRect(&DS)) return;

    /* Translate to Display Box coordinates. */
    SetLeftRect(&DS,
		 rectangle_Left(&DS) - rectangle_Left(&self->DisplayBoxSelection) +
		 rectangle_Left(&self->DisplayBox) - self->DBXscroll);
    SetTopRect(&DS,
		rectangle_Top(&DS) - rectangle_Top(&self->DisplayBoxSelection) +
		rectangle_Top(&self->DisplayBox) - self->DBYscroll);

    rectangle_GetRectSize(&DS, &l, &t, &w, &h);
    (self)->SetTransferMode( graphic::COPY);
    (self)->FillRectSize( l-2, t-2, w+4, 2, self->GreyPattern);
    (self)->FillRectSize( l-2, t+h, w+4, 2, self->GreyPattern);
    (self)->FillRectSize( l-2, t, 2, h, self->GreyPattern);
    (self)->FillRectSize( l+w, t, 2, h, self->GreyPattern);

    /* In case the above Gray overlapped the Display Box Frame, Redraw the Frame. */
    DS = self->DisplayBox;
    InsetRect(&DS, -BORDER, -BORDER);
    DrawHighlightScreenCoordinates(self, G, DS, graphic::BLACK, graphic::WHITE);
}

void rasterview_DisplayBoxHideHighlight(class rasterview  *self, class graphic  *G)
{
    class rasterimage *pix =
      ((class raster *)self->GetDataObject())->GetPix();
    struct rectangle DS, DBS;

    if (self->DisplayBoxHidden) return;

    DS = self->CurSelection;
    DBS = self->DisplayBoxSelection;

    InsetRect(&DBS, -BORDER, -BORDER);
    if (IsExclusivelyEnclosedBy(&DBS, &DS)) return;
    rectangle_IntersectRect(&DS, &DBS, &DS);
    if (IsEmptyRect(&DS)) return;

    DEBUG(("DBHiding: (%ld,%ld,%ld,%ld)\n", DS.left, DS.top, DS.width, DS.height));

    /* Translate to Display Box coordinates. */
    SetLeftRect(&DS,
		 rectangle_Left(&DS) - rectangle_Left(&self->DisplayBoxSelection) +
		 rectangle_Left(&self->DisplayBox) - self->DBXscroll);
    SetTopRect(&DS,
		rectangle_Top(&DS) - rectangle_Top(&self->DisplayBoxSelection) +
		rectangle_Top(&self->DisplayBox) - self->DBYscroll);
    DrawHighlightScreenCoordinates(self, G, DS, graphic::WHITE, -1);

    /* In case the above Hide overlapped the Display Box Frame, Redraw the Frame. */
    DS = self->DisplayBox;
    InsetRect(&DS, -BORDER, -BORDER);
    DrawHighlightScreenCoordinates(self, G, DS, graphic::BLACK, graphic::WHITE);

    /* Write the entire Display Box back on the screen. */
    (self)->SetTransferMode( graphic::COPY);
    (G)->WritePixImage(
			   rectangle_Left(&self->DisplayBox) - self->DBXscroll,
			   rectangle_Top(&self->DisplayBox) - self->DBYscroll,
			   pix,
			   rectangle_Left(&self->DisplayBoxSelection),
			   rectangle_Top(&self->DisplayBoxSelection),
			   rectangle_Width(&self->DisplayBoxSelection),
			   rectangle_Height(&self->DisplayBoxSelection));
}

void rasterview_DrawHighlightBehindDisplayBox(class rasterview  *self, class graphic  *G, boolean  gray)
{
    struct rectangle DBF;		/* Display Box Frame */
    struct rectangle DSF;		/* Desired Selection Frame */
    struct rectangle temp;
    long Scale = self->Scale;

    /* Set temp equal to something to avoid compiler warning. */
    temp = self->DisplayBox;

    /* The Display Box to avoid includes a double border. */
    DBF = self->DisplayBox;
    InsetRect(&DBF, -TWOBORDER, -TWOBORDER);
    DEBUG(("Screen DBF: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DBF),
	    rectangle_Top(&DBF),
	    rectangle_Width(&DBF),
	    rectangle_Height(&DBF)));

    /* Several conversions here: Expand from normal size, translate origin to screen coordinates (the coordinates of the Display Box), negative inset by BORDER. */
    DSF = self->DesiredSelection;
    rectangle_SetRectSize(&DSF,
			   (rectangle_Left(&DSF) - rectangle_Left(&self->DisplayBoxSelection))*Scale -
			   self->Xoff - BORDER,
			   (rectangle_Top(&DSF) - rectangle_Top(&self->DisplayBoxSelection))*Scale -
			   self->Yoff - BORDER,
			   rectangle_Width(&DSF)*Scale + TWOBORDER,
			   rectangle_Height(&DSF)*Scale + TWOBORDER);
    DEBUG(("Screen DSF: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&DSF),
	    rectangle_Top(&DSF),
	    rectangle_Width(&DSF),
	    rectangle_Height(&DSF)));

    if (IsEnclosedBy(&DSF, &DBF)) return;

    rectangle_IntersectRect(&temp, &DBF, &DSF);
    if (IsEmptyRect(&temp) || IsExclusivelyEnclosedBy(&DBF, &DSF) ) {
	/* No overlap. Just Draw the rectangles. */
	struct rectangle DS;
	DS = self->DesiredSelection;
	rectangle_SetRectSize(&DS,
			      (rectangle_Left(&DS) - rectangle_Left(&self->DisplayBoxSelection))*Scale,
			      (rectangle_Top(&DS) - rectangle_Top(&self->DisplayBoxSelection))*Scale,
			      rectangle_Width(&DS)*Scale,
			      rectangle_Height(&DS)*Scale);
	self->DisplayBoxOverlapped = FALSE;
	if (! gray) {
	    DrawHighlightBlackAndWhite(self, G, DS); }
	else {
	    long l, t, w, h;
	    rectangle_GetRectSize(&DS, &l, &t, &w, &h);
	    l -= self->Xoff;  /* convert to screen coords */
	    t -= self->Yoff;
	    (self)->SetTransferMode( graphic::COPY);
	    (self)->FillRectSize( l-2, t-2, w+4, 2, self->GreyPattern);
	    (self)->FillRectSize( l-2, t+h, w+4, 2, self->GreyPattern);
	    (self)->FillRectSize( l-2, t, 2, h, self->GreyPattern);
	    (self)->FillRectSize( l+w, t, 2, h, self->GreyPattern); } }
    else {
	/* Highlight overlaps the display box. There will be either four or five points to determine for the path to be drawn: four points for the case where an entire side (two corners) are inside the Display Box Frame and five points for the cases of a part of a side or a corner is inside the Display Box Frame. Note that the following completely ignores the problem of overlapping of the display box frame by redrawing the Display Box Frame after the Highlight has been drawn. Note that all of the below subtract 1 from the width and hight to conform to the drawing of all other highlights. */
	long x;
	struct point *path1 = self->DisplayBoxAvoidancePath1;
	struct point *path2 = self->DisplayBoxAvoidancePath2;
	struct point *path3 = self->DisplayBoxAvoidancePath3;
	struct point *path4 = self->DisplayBoxAvoidancePath4;
	long DBFleft, DBFtop, DBFwidth, DBFheight, DBFbottom, DBFright;
	long DSFleft, DSFtop, DSFwidth, DSFheight, DSFbottom, DSFright;

	self->DisplayBoxPathLength1 = 0;
	self->DisplayBoxPathLength2 = 0;
	self->DisplayBoxOverlapped = TRUE;

	/* Initialize all the points */
	for (x=0; x<4; x++) {
	    point_SetPt(&path1[x], 0, 0);
	    point_SetPt(&path2[x], 0, 0);
	    point_SetPt(&path3[x], 0, 0);
	    point_SetPt(&path4[x], 0, 0); }
	for (x=4; x<6; x++) {
	    point_SetPt(&path1[x], 0, 0);
	    point_SetPt(&path3[x], 0, 0); }

	rectangle_GetRectSize(&DBF, &DBFleft, &DBFtop, &DBFwidth, &DBFheight);
	DBFbottom = DBFtop + DBFheight;
	DBFright = DBFleft + DBFwidth;

	rectangle_GetRectSize(&DSF, &DSFleft, &DSFtop, &DSFwidth, &DSFheight);
	DSFheight--; DSFwidth--;
	DSFbottom = DSFtop + DSFheight;
	DSFright = DSFleft + DSFwidth;

	/* Test for Upper Left corner of Highlight in Display Box Frame */
	if ( DSFleft >= DBFleft && DSFleft <= DBFright
	    && DSFtop >= DBFtop && DSFtop <= DBFbottom ) {
	    /* Upper Left inside. Test Upper Right */
	    if ( DSFright <= DBFright ) {
		/* four points: Top overlaps */
		/* DEBUG(("\nTop overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		point_SetPt(&path1[0], DSFright, DBFbottom);
		point_SetPt(&path1[1], DSFright, DSFbottom);
		point_SetPt(&path1[2], DSFleft, DSFbottom);
		point_SetPt(&path1[3], DSFleft, DBFbottom);

		point_SetPt(&path3[0], DSFright - 1, DBFbottom);
		point_SetPt(&path3[1], DSFright - 1, DSFbottom - 1);
		point_SetPt(&path3[2], DSFleft + 1, DSFbottom - 1);
		point_SetPt(&path3[3], DSFleft + 1, DBFbottom);
	    }
	    /* Upper Left inside. Test Lower Left */
	    else if ( DSFbottom <= DBFbottom ) {
		/* four points: Left overlaps */
		/* DEBUG(("\nLeft overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		point_SetPt(&path1[0], DBFright, DSFtop);
		point_SetPt(&path1[1], DSFright, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFbottom);
		point_SetPt(&path1[3], DBFright, DSFbottom);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFright, DSFtop);
		point_SetPt(&path3[1], DSFright, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFbottom);
		point_SetPt(&path3[3], DBFright, DSFbottom);
	    }
	    else {
		/* five points: Upper Left overlaps*/
		/* DEBUG(("\nUpper Left overlaps")); */
		self->DisplayBoxPathLength1 = 5;
		point_SetPt(&path1[0], DBFright, DSFtop);
		point_SetPt(&path1[1], DSFright, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFbottom);
		point_SetPt(&path1[3], DSFleft, DSFbottom);
		point_SetPt(&path1[4], DSFleft, DBFbottom);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFright, DSFtop);
		point_SetPt(&path3[1], DSFright, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFbottom);
		point_SetPt(&path3[3], DSFleft, DSFbottom);
		point_SetPt(&path3[4], DSFleft, DBFbottom);
	    }
	}
	/* Test for Lower Right in Display Box Frame */
	else if ( DSFright >= DBFleft && DSFright <= DBFright
		 && DSFbottom >= DBFtop && DSFbottom <= DBFbottom ) {
	    /* Lower Right inside: Test Upper Right */
	    if ( DSFtop >= DBFtop ) {
		/* four points: Right overlaps */
		/* DEBUG(("\nRight overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		point_SetPt(&path1[0], DBFleft-1, DSFtop);
		point_SetPt(&path1[1], DSFleft, DSFtop);
		point_SetPt(&path1[2], DSFleft, DSFbottom);
		point_SetPt(&path1[3], DBFleft-1, DSFbottom);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFleft-1, DSFtop);
		point_SetPt(&path3[1], DSFleft, DSFtop);
		point_SetPt(&path3[2], DSFleft, DSFbottom);
		point_SetPt(&path3[3], DBFleft-1, DSFbottom);
	    }
	    /* Lower Right inside: Test Lower Left */
	    else if ( DSFleft >= DBFleft ) {
		/* four points: Bottom overlaps */
		/* DEBUG(("\nBottom overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		point_SetPt(&path1[0], DSFleft, DBFtop-1);
		point_SetPt(&path1[1], DSFleft, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFtop);
		point_SetPt(&path1[3], DSFright, DBFtop-1);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DSFleft, DBFtop-1);
		point_SetPt(&path3[1], DSFleft, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFtop);
		point_SetPt(&path3[3], DSFright, DBFtop-1);
	    }
	    else {
		/* five points: Lower Right overlaps */
		/* DEBUG(("\nLower Right overlaps")); */
		self->DisplayBoxPathLength1 = 5;
		point_SetPt(&path1[0], DBFleft-1, DSFbottom);
		point_SetPt(&path1[1], DSFleft, DSFbottom);
		point_SetPt(&path1[2], DSFleft, DSFtop);
		point_SetPt(&path1[3], DSFright, DSFtop);
		point_SetPt(&path1[4], DSFright, DBFtop-1);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFleft-1, DSFbottom);
		point_SetPt(&path3[1], DSFleft, DSFbottom);
		point_SetPt(&path3[2], DSFleft, DSFtop);
		point_SetPt(&path3[3], DSFright, DSFtop);
		point_SetPt(&path3[4], DSFright, DBFtop-1);
	    }
	}
	/* Test for Upper Right in Display Box Frame */
	else if ( DSFright >= DBFleft && DSFright <= DBFright
		 && DSFtop >= DBFtop && DSFtop <= DBFbottom ) {
	    /* five points: Upper Right overlaps */
	    /* DEBUG(("\nUpper Right overlaps")); */
	    self->DisplayBoxPathLength1 = 5;
	    point_SetPt(&path1[0], DSFright, DBFbottom);
	    point_SetPt(&path1[1], DSFright, DSFbottom);
	    point_SetPt(&path1[2], DSFleft, DSFbottom);
	    point_SetPt(&path1[3], DSFleft, DSFtop);
	    point_SetPt(&path1[4], DBFleft-1, DSFtop);

	    DSFleft++; DSFtop++; DSFright--; DSFbottom--;
	    point_SetPt(&path3[0], DSFright, DBFbottom);
	    point_SetPt(&path3[1], DSFright, DSFbottom);
	    point_SetPt(&path3[2], DSFleft, DSFbottom);
	    point_SetPt(&path3[3], DSFleft, DSFtop);
	    point_SetPt(&path3[4], DBFleft-1, DSFtop);
	}
	/* Test for Lower Left in Display Box Frame */
	else if ( DSFbottom >= DBFtop && DSFbottom <= DBFbottom
		 && DSFleft >= DBFleft && DSFleft <= DBFright ) {
	    /* five points: Lower Left overlaps */
	    /* DEBUG(("\nLower Left overlaps")); */
	    self->DisplayBoxPathLength1 = 5;
	    point_SetPt(&path1[0], DSFleft, DBFtop-1);
	    point_SetPt(&path1[1], DSFleft, DSFtop);
	    point_SetPt(&path1[2], DSFright, DSFtop);
	    point_SetPt(&path1[3], DSFright, DSFbottom);
	    point_SetPt(&path1[4], DBFright, DSFbottom);

	    DSFleft++; DSFtop++; DSFright--; DSFbottom--;
	    point_SetPt(&path3[0], DSFleft, DBFtop-1);
	    point_SetPt(&path3[1], DSFleft, DSFtop);
	    point_SetPt(&path3[2], DSFright, DSFtop);
	    point_SetPt(&path3[3], DSFright, DSFbottom);
	    point_SetPt(&path3[4], DBFright, DSFbottom);
	}
	/* Test for part of Top in Display Box Frame */
	else if ( DSFtop >= DBFtop && DSFtop <= DBFbottom ) {
	    /* Part of Top inside: Test for part of Bottom in Display Box Frame */
	    if ( DSFbottom <= DBFbottom ) {
		/* Two paths of four points each */
		/* DEBUG(("\nPart of top and part of bottom overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		self->DisplayBoxPathLength2 = 4;
		point_SetPt(&path1[0], DBFright, DSFtop);
		point_SetPt(&path1[1], DSFright, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFbottom);
		point_SetPt(&path1[3], DBFright, DSFbottom);
		point_SetPt(&path2[0], DBFleft-1, DSFtop);
		point_SetPt(&path2[1], DSFleft, DSFtop);
		point_SetPt(&path2[2], DSFleft, DSFbottom);
		point_SetPt(&path2[3], DBFleft-1, DSFbottom);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFright, DSFtop);
		point_SetPt(&path3[1], DSFright, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFbottom);
		point_SetPt(&path3[3], DBFright, DSFbottom);
		point_SetPt(&path4[0], DBFleft-1, DSFtop);
		point_SetPt(&path4[1], DSFleft, DSFtop);
		point_SetPt(&path4[2], DSFleft, DSFbottom);
		point_SetPt(&path4[3], DBFleft-1, DSFbottom);
	    }
	    else {
		/* One path of six points */
		/* DEBUG(("\nPart of top overlaps")); */
		self->DisplayBoxPathLength1 = 6;
		point_SetPt(&path1[0], DBFright, DSFtop);
		point_SetPt(&path1[1], DSFright, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFbottom);
		point_SetPt(&path1[3], DSFleft, DSFbottom);
		point_SetPt(&path1[4], DSFleft, DSFtop);
		point_SetPt(&path1[5], DBFleft-1, DSFtop);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DBFright, DSFtop);
		point_SetPt(&path3[1], DSFright, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFbottom);
		point_SetPt(&path3[3], DSFleft, DSFbottom);
		point_SetPt(&path3[4], DSFleft, DSFtop);
		point_SetPt(&path3[5], DBFleft-1, DSFtop);
	    }
	}
	/* Test for part of Right in Display Box Frame */
	else if ( DSFright >= DBFleft && DSFright <= DBFright ) {
	    /* Part of Right inside: Test for part of Left in Display Box Frame */
	    if ( DSFleft >= DBFleft && DSFleft <= DBFright ) {
		/* Two paths of four points each */
		/* DEBUG(("\nPart of Left and Part of Right overlaps")); */
		self->DisplayBoxPathLength1 = 4;
		self->DisplayBoxPathLength2 = 4;
		point_SetPt(&path1[0], DSFleft, DBFtop-1);
		point_SetPt(&path1[1], DSFleft, DSFtop);
		point_SetPt(&path1[2], DSFright, DSFtop);
		point_SetPt(&path1[3], DSFright, DBFtop-1);
		point_SetPt(&path2[0], DSFright, DBFbottom);
		point_SetPt(&path2[1], DSFright, DSFbottom);
		point_SetPt(&path2[2], DSFleft, DSFbottom);
		point_SetPt(&path2[3], DSFleft, DBFbottom);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DSFleft, DBFtop-1);
		point_SetPt(&path3[1], DSFleft, DSFtop);
		point_SetPt(&path3[2], DSFright, DSFtop);
		point_SetPt(&path3[3], DSFright, DBFtop-1);
		point_SetPt(&path4[0], DSFright, DBFbottom);
		point_SetPt(&path4[1], DSFright, DSFbottom);
		point_SetPt(&path4[2], DSFleft, DSFbottom);
		point_SetPt(&path4[3], DSFleft, DBFbottom);
	    }
	    else {
		/* One path of six points */
		/* DEBUG(("\nPart of Right overlaps")); */
		self->DisplayBoxPathLength1 = 6;
		point_SetPt(&path1[0], DSFright, DBFbottom);
		point_SetPt(&path1[1], DSFright, DSFbottom);
		point_SetPt(&path1[2], DSFleft, DSFbottom);
		point_SetPt(&path1[3], DSFleft, DSFtop);
		point_SetPt(&path1[4], DSFright, DSFtop);
		point_SetPt(&path1[5], DSFright, DBFtop-1);

		DSFleft++; DSFtop++; DSFright--; DSFbottom--;
		point_SetPt(&path3[0], DSFright, DBFbottom);
		point_SetPt(&path3[1], DSFright, DSFbottom);
		point_SetPt(&path3[2], DSFleft, DSFbottom);
		point_SetPt(&path3[3], DSFleft, DSFtop);
		point_SetPt(&path3[4], DSFright, DSFtop);
		point_SetPt(&path3[5], DSFright, DBFtop-1);
	    }
	}
	/* Test for part of Bottom in Display Box Frame */
	else if ( DSFbottom >= DBFtop && DSFbottom <= DBFbottom ) {
	    /* One path of six points */
	    /* DEBUG(("\nPart of Bottom overlaps")); */
	    self->DisplayBoxPathLength1 = 6;
	    point_SetPt(&path1[0], DBFleft-1, DSFbottom);
	    point_SetPt(&path1[1], DSFleft, DSFbottom);
	    point_SetPt(&path1[2], DSFleft, DSFtop);
	    point_SetPt(&path1[3], DSFright, DSFtop);
	    point_SetPt(&path1[4], DSFright, DSFbottom);
	    point_SetPt(&path1[5], DBFright, DSFbottom);

	    DSFleft++; DSFtop++; DSFright--; DSFbottom--;
	    point_SetPt(&path3[0], DBFleft-1, DSFbottom);
	    point_SetPt(&path3[1], DSFleft, DSFbottom);
	    point_SetPt(&path3[2], DSFleft, DSFtop);
	    point_SetPt(&path3[3], DSFright, DSFtop);
	    point_SetPt(&path3[4], DSFright, DSFbottom);
	    point_SetPt(&path3[5], DBFright, DSFbottom);
	}
	else {
	    /* Part of Left in Display Box Frame */
	    /* One path of six points */
	    /* DEBUG(("\nPart of Left overlaps")); */
	    self->DisplayBoxPathLength1 = 6;
	    point_SetPt(&path1[0], DSFleft, DBFtop-1);
	    point_SetPt(&path1[1], DSFleft, DSFtop);
	    point_SetPt(&path1[2], DSFright, DSFtop);
	    point_SetPt(&path1[3], DSFright, DSFbottom);
	    point_SetPt(&path1[4], DSFleft, DSFbottom);
	    point_SetPt(&path1[5], DSFleft, DBFbottom);

	    DSFleft++; DSFtop++; DSFright--; DSFbottom--;
	    point_SetPt(&path3[0], DSFleft, DBFtop-1);
	    point_SetPt(&path3[1], DSFleft, DSFtop);
	    point_SetPt(&path3[2], DSFright, DSFtop);
	    point_SetPt(&path3[3], DSFright, DSFbottom);
	    point_SetPt(&path3[4], DSFleft, DSFbottom);
	    point_SetPt(&path3[5], DSFleft, DBFbottom);
	}

	/* draw the path(s) */
	if (! gray) {
	    (self)->SetTransferMode( graphic::BLACK);
	    (G)->DrawPath( path1, self->DisplayBoxPathLength1);
	    (G)->DrawPath( path2, self->DisplayBoxPathLength2);
	    (self)->SetTransferMode( graphic::WHITE);
	    (G)->DrawPath( path3, self->DisplayBoxPathLength1);
	    (G)->DrawPath( path4, self->DisplayBoxPathLength2); }
	else {
	    long j;
	    long x1, y1, x2, y2, x, y, w, h;
	    for (j=0; j<self->DisplayBoxPathLength1-1; j++) {
		x1 = point_X(&path1[j]);
		y1 = point_Y(&path1[j]);
		x2 = point_X(&path3[j+1]);
		y2 = point_Y(&path3[j+1]);
		if (x1<x2) { x = x1; w = x2 - x1; }
		else { x = x2; w = x1 - x2; }
		if (y1<y2) { y = y1; h = y2 - y1; }
		else { y = y2; h = y1 - y2; }
		if (w==1) w = 2; else h = 2;
		(self)->FillRectSize( x, y, w, h, self->GreyPattern); }
	    for (j=0; j<self->DisplayBoxPathLength2-1; j++) {
		x1 = point_X(&path2[j]);
		y1 = point_Y(&path2[j]);
		x2 = point_X(&path4[j+1]);
		y2 = point_Y(&path4[j+1]);
		if (x1<x2) { x = x1; w = x2 - x1; }
		else { x = x2; w = x1 - x2; }
		if (y1<y2) { y = y1; h = y2 - y1; }
		else { y = y2; h = y1 - y2; }
		if (w==1) w = 2; else h = 2;
		(self)->FillRectSize( x, y, w, h, self->GreyPattern); } }
    }
}

void rasterview_DisplayBoxHideOverlappingHighlight(class rasterview  *self, class graphic  *G, class rasterimage  *pix)
{
    long Xoff = self->Xoff;
    long Yoff = self->Yoff;
    long clipw = (pix)->GetWidth();
    long cliph = (pix)->GetHeight();

    struct point *path1 = self->DisplayBoxAvoidancePath1;
    struct point *path2 = self->DisplayBoxAvoidancePath2;
    struct point *path3 = self->DisplayBoxAvoidancePath3;
    struct point *path4 = self->DisplayBoxAvoidancePath4;
    long j;
    long x1, y1, x2, y2, x, y, w, h;

    for (j=0; j<self->DisplayBoxPathLength1-1; j++) {
	x1 = point_X(&path1[j]);
	y1 = point_Y(&path1[j]);
	x2 = point_X(&path3[j+1]);
	y2 = point_Y(&path3[j+1]);
	if (x1<x2) { x = x1; w = x2 - x1; }
	else { x = x2; w = x1 - x2; }
	if (y1<y2) { y = y1; h = y2 - y1; }
	else { y = y2; h = y1 - y2; }
	if (w==1) w = 2; else h = 2;
	(self)->FillRectSize( x, y, ++w, ++h, self->WhitePattern);
	(self)->SetTransferMode( graphic::COPY);
	if (x <= -Xoff) x = -Xoff;
	if (y <= -Yoff) y = -Yoff;
	ClipAndWritePixImage(clipw, cliph,
			     G, x, y, pix, x+Xoff, y+Yoff, w, h);
    }
    for (j=0; j<self->DisplayBoxPathLength2-1; j++) {
	x1 = point_X(&path2[j]);
	y1 = point_Y(&path2[j]);
	x2 = point_X(&path4[j+1]);
	y2 = point_Y(&path4[j+1]);
	if (x1<x2) { x = x1; w = x2 - x1; }
	else { x = x2; w = x1 - x2; }
	if (y1<y2) { y = y1; h = y2 - y1; }
	else { y = y2; h = y1 - y2; }
	if (w==1) w = 2; else h = 2;
	(self)->FillRectSize( x, y, ++w, ++h, self->WhitePattern);
	(self)->SetTransferMode( graphic::COPY);
	if (x <= -Xoff) x = -Xoff;
	if (y <= -Yoff) y = -Yoff;
	ClipAndWritePixImage(clipw, cliph,
			     G, x, y, pix, x+Xoff, y+Yoff, w, h);
    }
}

void rasterview_SetPixelBehindDisplayBox(class rasterview  *self, class rasterimage  *pix, long  x , long  y, boolean  bit)
{
    class rasterimage *fullpix =
      ((class raster *)self->GetDataObject())->GetPix();

    /* Image is in Expanded Mode. */
    struct rectangle sub;
    struct rectangle *DB = &self->DisplayBox;
    class graphic *pattern = (bit) ? self->BlackPattern : self->WhitePattern;
    long byte = (bit) ? BLACKBYTE : WHITEBYTE;

    /* Determine the offset of the point to set within the Display Box Selection and the point within the Display Box itself to set. */
    long OffsetX = x - rectangle_Left(&self->DisplayBoxSelection);
    long OffsetY = y - rectangle_Top(&self->DisplayBoxSelection);
    long BitX = rectangle_Left(DB) + OffsetX - self->DBXscroll;
    long BitY = rectangle_Top(DB) + OffsetY - self->DBYscroll;

    /* Refer to the rectangle which is the expanded point by its upper left point. */
    long left = OffsetX*self->Scale;
    long top = OffsetY*self->Scale;
    long width = self->Scale;
    long height = self->Scale;
    long right = left + width;
    long bottom = top + height;

    /* Refer to the Display Box by including the (double) border around it, offset by BORDER. */
    long Xscroll = (self->Xscroll < 0) ? self->Xscroll : 0;
    long Yscroll = (self->Yscroll < 0) ? self->Yscroll : 0;
    long l = rectangle_Left(DB) - 3*BORDER + Xscroll;
    long t = rectangle_Top(DB) - 3*BORDER + Yscroll;
    long r = rectangle_Right(DB) + BORDER + Xscroll;
    long b = rectangle_Bottom(DB) + BORDER + Yscroll;

    /* Force bit to be either one or zero. */
    bit = (bit) ? 1 : 0;

    (self)->SetTransferMode( graphic::COPY);

    /* Set the pixel in the full rasterimage so that when contracted, the image will be correct. */
    (fullpix)->SetPixel(
			  rectangle_Left(&self->DisplayBoxSelection) + OffsetX,
			  rectangle_Top(&self->DisplayBoxSelection) + OffsetY,
			  bit);
    /* Set the pixel in rasterimage within the Display Box. */
    (pix)->SetPixel( BitX - BORDER, BitY - BORDER, bit);
    /* Draw the pixel on the screen within the Display Box. */
    rectangle_SetRectSize(&sub, BitX, BitY, 1, 1);
    (self)->FillRect( &sub, pattern);

    /* Draw the blob on the screen and in the rasterimage (so later updates will paint the right thing on the screen) making sure not to overlap the Display Box. */
    /* If blob is fully enclosed by Display Box then done.
      if Upper Left and Lower Right inside the inset then enclosed */
    if ((left >= l && left <= r && top >= t && top <= b
	  && right >= l && right <= r && bottom >= t && bottom <= b))
	return; 

    /* NOTE: within all of the SetRectSize's below
      is a conversion to screen coordinates */
    /* test for any intersection -- Any corner inside Display Box. */
    if (! ((left >= l && left <= r && top >= t && top <= b)
	    || (right >= l && right <= r && top >= t && top <= b)
	    || (left >= l && left <= r && bottom >= t && bottom <= b)
	    || (right >= l && right <= r && bottom >= t && bottom <= b)))
	/* No intersection -- draw whole blob on screen. */
	rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
			      width, height);
    else {
	/* could be overlapping in one of two ways: along a side or on a corner */

	/* test for Lower Right within the Display Box */
	if (right >= l && right <= r && bottom >= t && bottom <= b) {
	    /* test for Upper Right within the Display Box */
	    if (top >= t && top <= b) {
		/* Right Side overlaps Display Box */
		rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
				      l - left, height); }
	    /* test for Lower Left within Display Box */
	    else if (left >= l && left <= r) {
		/* Bottom overlaps Display Box */
		rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
				      width, t - top); }
	    else {
		/* Lower Right corner overlaps Display Box */
		/* adjust the right side and paint this */
		rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
				      l - left, height);
		(self)->FillRect( &sub, pattern);
		OffsetRect(&sub, self->Xoff, self->Yoff);
		(pix)->PaintSubraster( &sub, byte);
		/* Paint the Upper Right box which remains */
		rectangle_SetRectSize(&sub, l - self->Xoff, top - self->Yoff,
				      right - l, t - top); }
	}
	/* test for the Upper Left corner within the Display Box */
	else if (left >= l && left <= r && top >= t && top <= bottom) {
	    /* test for Lower Left corner within the Display Box */
	    if (bottom >= t && bottom <= b) {
		/* Left Side overlaps Display Box */
		rectangle_SetRectSize(&sub, r - self->Xoff, top - self->Yoff,
				      right - r, height); }
	    /* test for Upper Right within Display Box */
	    else if (right >= l && right <= r) {
		/* Top overlaps Display Box */
		rectangle_SetRectSize(&sub, left - self->Xoff, b - self->Yoff,
				      width, bottom - b); }
	    else {
		/* Upper Left corner overlaps Display Box */
		rectangle_SetRectSize(&sub, r - self->Xoff, top - self->Yoff,
				      right - r, height);
		(self)->FillRect( &sub, pattern);
		OffsetRect(&sub, self->Xoff, self->Yoff);
		(pix)->PaintSubraster( &sub, byte);
		rectangle_SetRectSize(&sub, left - self->Xoff, b - self->Yoff,
				      r - left, bottom - b); }
	}
	/* at this point either the Lower Left or the Upper Right corners overlap the Display Box. Test for the Lower Left corner. */
	else if (left >= l && left <= r) {
	    /* Lower Left corner overlaps Display Box */
	    rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
				  width, t - top);
	    (self)->FillRect( &sub, pattern);
	    OffsetRect(&sub, self->Xoff, self->Yoff);
	    (pix)->PaintSubraster( &sub, byte);
	    rectangle_SetRectSize(&sub, r - self->Xoff, t - self->Yoff,
				  right - r, bottom - t); }
	else {
	    /* Upper Right overlaps Display Box */
	    rectangle_SetRectSize(&sub, left - self->Xoff, top - self->Yoff,
				  l - left, height);
	    (self)->FillRect( &sub, pattern);
	    OffsetRect(&sub, self->Xoff, self->Yoff);
	    (pix)->PaintSubraster( &sub, byte);
	    rectangle_SetRectSize(&sub, l - self->Xoff, b - self->Yoff,
				  right - l, bottom - b); }
    }
    (self)->FillRect( &sub, pattern);
    OffsetRect(&sub, self->Xoff, self->Yoff);
    (pix)->PaintSubraster( &sub, byte);
}
