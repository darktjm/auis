/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

extern NO_DLL_EXPORT void DisplayBoxBlitOverlap(class rasterview  *self, class rasterimage  *pix);
extern NO_DLL_EXPORT void DisplayBoxBlitOverlap(class rasterview  *self, class rasterimage  *pix);
#if 0
extern NO_DLL_EXPORT void DisplayBoxDrawPanHighlight(class rasterview  *self);
#endif /* 0 */
extern NO_DLL_EXPORT void rasterview_DisplayBoxWritePixImageFull(class rasterview  *self, class graphic  *G, class rasterimage  *pix);
extern NO_DLL_EXPORT void rasterview_DisplayBoxWritePixImage(class rasterview  *self, class graphic  *G);
extern NO_DLL_EXPORT void rasterview_DisplayBoxHide(class rasterview  *self);
extern NO_DLL_EXPORT void rasterview_DisplayBoxDrawHighlight(class rasterview  *self, class graphic  *G);
extern NO_DLL_EXPORT void rasterview_DisplayBoxDrawHighlightGray(class rasterview  *self, class graphic  *G);
extern NO_DLL_EXPORT void rasterview_DisplayBoxHideHighlight(class rasterview  *self, class graphic  *G);
extern NO_DLL_EXPORT void rasterview_DrawHighlightBehindDisplayBox(class rasterview  *self, class graphic  *G, boolean  gray);
extern NO_DLL_EXPORT void rasterview_DisplayBoxHideOverlappingHighlight(class rasterview  *self, class graphic  *G, class rasterimage  *pix);
extern NO_DLL_EXPORT void rasterview_SetPixelBehindDisplayBox(class rasterview  *self, class rasterimage  *pix, long  x , long  y, boolean  bit);

extern NO_DLL_EXPORT boolean rasterview_debug;
#define Debug(s) {printf s ; fflush(stdout);}
#define DEBUG(s) {if (rasterview_debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(%p)\n", Stringize(r), self))
#define LEAVE(r) DEBUG(("Leave %s(%p)\n", Stringize(r), self))

#define DisplayAndReturn(self, String) {message::DisplayString(self, 0, String); return;}

#define AskOrCancel(self, string, buf) \
{if (message::AskForString(self, 0, string, "", buf, sizeof buf - 1) < 0) \
{message::DisplayString(self, 0, "Cancelled."); \
	return;}}

#define ClipAndWritePixImage(clipw, cliph, G, x, y, pix, x1, y1, w, h) {	\
if ((x1) < (clipw) && (y1) < (cliph)) {						\
long width = ((x1) + (w) > (clipw)) ? (clipw) - (x1) : (w);			\
long height = ((y1) + (h) > (cliph)) ? (cliph) - (y1) : (h);			\
if (width > 0 && height > 0)							\
G->WritePixImage((x), (y), (pix), (x1), (y1), width, height); } }

#define DrawHighlight(self, Graphic, Rect, OuterBorder, InnerBorder)		\
{										\
    rectangle_SetRectSize(&(Rect),						\
			   rectangle_Left(&(Rect)) - (self)->Xoff - BORDER,	\
			   rectangle_Top(&(Rect)) - (self)->Yoff - BORDER,	\
			   rectangle_Width(&(Rect)) + TWOBORDER - 1,		\
			   rectangle_Height(&(Rect)) + TWOBORDER - 1);		\
    self->SetTransferMode( (OuterBorder));				\
    Graphic->DrawRect(&(Rect));					\
    if ((InnerBorder) >= 0) {							\
	(self)->SetTransferMode((InnerBorder));			\
	InsetRect(&(Rect), 1, 1);						\
	Graphic->DrawRect(&(Rect));					\
	rectangle_SetRectSize(&(Rect),						\
				rectangle_Left(&(Rect)) + (self)->Xoff + BORDER - 1,	\
				rectangle_Top(&(Rect)) + (self)->Yoff + BORDER - 1,	\
				rectangle_Width(&(Rect)) - TWOBORDER + 3,		\
				rectangle_Height(&(Rect)) - TWOBORDER + 3);		\
    }										\
    else {									\
	rectangle_SetRectSize(&(Rect),						\
				rectangle_Left(&(Rect)) + (self)->Xoff + BORDER,\
				rectangle_Top(&(Rect)) + (self)->Yoff + BORDER,	\
				rectangle_Width(&(Rect)) - TWOBORDER + 1,	\
				rectangle_Height(&(Rect)) - TWOBORDER + 1);	\
    }										\
}

#define DrawHighlightScreenCoordinates(self, Graphic, Rect, OuterBorder, InnerBorder)	\
{										\
    rectangle_SetRectSize(&(Rect),						\
			   rectangle_Left(&(Rect)) - BORDER,			\
			   rectangle_Top(&(Rect)) - BORDER,			\
			   rectangle_Width(&(Rect)) + TWOBORDER - 1,		\
			   rectangle_Height(&(Rect)) + TWOBORDER - 1);		\
    self->SetTransferMode((OuterBorder));				\
    (Graphic)->DrawRect(&(Rect));					\
    if ((InnerBorder) >= 0) {							\
	self->SetTransferMode((InnerBorder));			\
	InsetRect(&(Rect), 1, 1);						\
	(Graphic)->DrawRect(&(Rect));					\
	rectangle_SetRectSize(&(Rect),						\
				rectangle_Left(&(Rect)) + BORDER - 1,		\
				rectangle_Top(&(Rect)) + BORDER - 1,		\
				rectangle_Width(&(Rect)) - TWOBORDER + 3,	\
				rectangle_Height(&(Rect)) - TWOBORDER + 3);	\
    }										\
    else {									\
	rectangle_SetRectSize(&(Rect),						\
				rectangle_Left(&(Rect)) + BORDER,		\
				rectangle_Top(&(Rect)) + BORDER,		\
				rectangle_Width(&(Rect)) - TWOBORDER + 1,	\
				rectangle_Height(&(Rect)) - TWOBORDER + 1);	\
    }										\
}

#define DrawHighlightBlackAndWhite(self, G, R) \
	DrawHighlight((self), (G), (R), graphic::BLACK, graphic::WHITE);
#define DrawHighlightWhite(self, G, R) \
	DrawHighlight((self), (G), (R), graphic::WHITE, graphic::WHITE);
#define DrawHighlightBlack(self, G, R) \
	DrawHighlight((self), (G), (R), graphic::BLACK, graphic::BLACK);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  *
  *	Rectangle Macroizations
  *	
  \* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define IsEmptyRect(TestRect) (TestRect)->width <= 0 || (TestRect)->height <= 0

#define IsNotEmptyRect(TestRect) (TestRect)->width > 0 && (TestRect)->height > 0

#define InsetRect(r, deltax, deltay) rectangle_SetRectSize((r), rectangle_Left((r)) + (deltax), rectangle_Top((r)) + (deltay), rectangle_Width((r)) - 2*(deltax), rectangle_Height((r)) - 2*(deltay));

#define OffsetRect(r, deltax, deltay) rectangle_SetRectSize((r), rectangle_Left((r)) + (deltax), rectangle_Top((r)) + (deltay), rectangle_Width((r)), rectangle_Height((r)));

#define IsEnclosedBy(a, b)				\
(rectangle_Left((b)) <= rectangle_Left((a))		\
 && rectangle_Top((b)) <= rectangle_Top((a))		\
 && rectangle_Right((b)) >= rectangle_Right((a))	\
 && rectangle_Bottom((b)) >= rectangle_Bottom((a)))

#define IsExclusivelyEnclosedBy(a, b)			\
(rectangle_Left((b)) < rectangle_Left((a))		\
 && rectangle_Top((b)) < rectangle_Top((a))		\
 && rectangle_Right((b)) > rectangle_Right((a))		\
 && rectangle_Bottom((b)) > rectangle_Bottom((a)))

#define SetLeftRect(r, l) (r)->left = (l);
#define SetTopRect(r, t) (r)->top = (t);
#define SetWidthRect(r, w) (r)->width = (w);
#define SetHeightRect(r, h) (r)->height = (h);
