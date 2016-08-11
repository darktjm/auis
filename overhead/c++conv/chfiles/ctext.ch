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



/* ctext.ch: Text subclass specialized for dealing with C code. */

/* kinds of styles that are used in hash table */
#define KEYWRD 1     /* keywords */

class ctext: srctext {

  overrides:
    Indentation(long pos) returns int;
    IsTokenChar(char ch) returns boolean;
    Quoted(long pos) returns boolean;
    RedoStyles();
    SetAttributes(struct attributes *atts);
    SetupStyles();

  methods:
    BackwardSkipDelimited(long pos,char begin,char end) returns long;
    BackwardSkipJunk(long pos) returns long;
    MaybeBackwardSkipCppLines(long pos) returns long;
    UnstyledCommentStart(long pos) returns long;

  classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct ctext *self) returns boolean;
 
  macromethods:

  data:
    /* Variables used to control the indenting style. */
    int braceIndent, switchLabelUndent, switchLevelIndent;
};

