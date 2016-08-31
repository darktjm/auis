/* fontsamp.ch - font sample view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

class fontsample [fontsamp] : view {

    classprocedures:
      InitializeClass() returns boolean;
      InitializeObject(struct fontsample *self) returns boolean;
      FinalizeObject(struct fontsample *self);

    overrides:
      FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
      Update();
      Hit(enum view_MouseAction action, long x, long y, long n)	returns struct view *;
      ObservedChanged(struct fontsel *fontsel, long status);

    methods:
      SetString(char *val);
      GetFontDesc() returns struct fontdesc *;

    macromethods:
      GetString()  ((self)->teststring)

    data:
      char *teststring;
      boolean dirty;
      struct fontdesc *fdesc;
};
