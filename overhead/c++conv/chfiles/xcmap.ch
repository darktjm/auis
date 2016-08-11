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

class xcolormap [xcmap] : colormap [cmap] {

classprocedures:
  InitializeClass() returns boolean;
  InitializeObject( struct xcolormap *self ) returns boolean;
  FinalizeObject( struct xcolormap *self ) returns void;
  Create( struct xim *xim ) returns struct xcolormap *;
overrides:
  SetColor(struct xcolor *xcolor, boolean needpixel) returns int;
  LookupColor(char *name, unsigned int red, unsigned int green, unsigned int blue, boolean needpixel) returns struct xcolor *;
  Copy(struct xcolormap *source) returns int;
  Merge(struct xcolormap *other) returns int;
  AllocColor(char *name, unsigned int red, unsigned int green, unsigned int blue, boolean needpixel) returns struct xcolor *;
  ChangeColor(struct xcolor *xc) returns int;
  DestroyColor(struct xcolor *xc);
macromethods:
  XDisplay() ((self)->display)
data:
  Colormap XColorMap;
  Display *display;
};

