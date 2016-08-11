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

class xcolor : color {
classprocedures:
  InitializeClass() returns boolean;
  InitializeObject( struct xcolor *self ) returns boolean;
  FinalizeObject( struct xcolor *self ) returns void;
  Create(char *name, unsigned short r, unsigned short g, unsigned short b ) returns struct xcolor *;
overrides:
  ObservedChanged(struct observable *observable, long value);
macromethods:
  Pixel() ((self)->color.pixel)
  SetColormap(colormap) ((self)->cmap = (colormap))
  GetColormap() ((self)->cmap)
  SetDisplay(display) ((self)->dpy = (display))
  GetDisplay() ((self)->dpy)

data:
  XColor color;
  Display *dpy;
  boolean used, haspixel;
  struct xcolormap *cmap;
};
