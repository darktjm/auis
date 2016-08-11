/* Copyright 1992 by Carnegie Mellon University All rights Reserved.
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
 *  $ */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/phelpv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */

class phelpv : textview [textv] {
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct prefval *self) returns boolean;
    FinalizeObject(struct prefval *self);
overrides:
    SetDotPosition(long pos);
    Hit(enum view_MouseAction action, long x, long y, long clicks) returns struct view *;
methods:
macros:
macromethods:
    SetPrefs(struct prefs *p) ((self)->prefs=(p))
    GetPrefs() ((self)->prefs)
data:
    struct prefs *prefs;
};
