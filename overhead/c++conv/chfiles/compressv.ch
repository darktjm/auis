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




/*   compressv.ch: View subclass specialized for compressing source text code. */

struct envlist {
    struct environment *envr;
    struct text *txt;
    struct compress *cp;
    struct envlist *nextenv,*prevenv;
    };

class compressv: view {

  classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct compressv *self) returns boolean;
    FinalizeObject(struct compressv *self);
    DecompressAll();
    MakeBox(struct compress *cprs);
    HideBoxes();
    UnhideBoxes();

  overrides:
    FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
    Update();
    Hit (enum view_MouseAction action, long x, long y, long numberOfClicks) returns struct view *;
    DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight) returns enum view_DSattributes;
    LinkTree(struct view *parent);
    GetOrigin(long width, long height, long *originX, long *originY);

  methods:

  data:
    struct fontdesc *fd;
    struct cursor *cursor;
    struct text *parenttext;
    struct textview *parentview;
    struct compressv *nextcompress,*prevcompress;
};

