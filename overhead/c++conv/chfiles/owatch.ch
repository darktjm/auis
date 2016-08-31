/* Copyright 1992 Carnegie Mellon University, All rights reserved.
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

struct owatch_data {
    struct observable *obj;
    boolean alive;
    struct owatch_data *next, *prev;
    long refs;
};

class owatch : observable[observe] {
methods:
overrides:      
    ObservedChanged(struct observable *changed, long val);
macros:
Check(struct owatch_data *tocheck) (tocheck?tocheck->alive:FALSE)
classprocedures:
    Create(struct observable *b) returns struct owatch_data *;
    Delete(struct owatch_data *d);
    CheckAndDelete(struct owatch_data *d) returns boolean;
data:
};
