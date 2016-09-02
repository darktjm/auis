/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

void DisplayBoxBlitOverlap(class rasterview  *self, class rasterimage  *pix);
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

extern boolean rasterview_debug;
#define Debug(s) {printf s ; fflush(stdout);}
#define DEBUG(s) {if (rasterview_debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), self))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), self))

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
	DrawHighlight((self), (G), (R), graphic_BLACK, graphic_WHITE);
#define DrawHighlightWhite(self, G, R) \
	DrawHighlight((self), (G), (R), graphic_WHITE, graphic_WHITE);
#define DrawHighlightBlack(self, G, R) \
	DrawHighlight((self), (G), (R), graphic_BLACK, graphic_BLACK);


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
