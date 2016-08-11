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


#define MODE822_NORMAL 1
#define MODE822_FORMAT 2
#define MODE822_FIXEDWIDTH 4
#define MODE822_ROT13 8

class text822: text {
    overrides:
      Read(FILE *file, long id) returns long;
      ReadSubString(long pos, FILE *file, boolean quoteCharacters) returns long;
      Write(FILE *file, long writeID, int level) returns long;
      ViewName() returns char *;
      Clear();
      ClearCompletely();
    methods:
      ReadAsText(FILE *file, long id) returns long;
    classprocedures:
      InitializeClass() returns boolean;
      ReadIntoText(struct text *d, FILE *fp, int Mode, char *ContentTypeOverride, int *len, boolean IsReallyTextObject, int *bodystart, int *ignorepos, struct text *auxheadtext) returns boolean;
      ResetGlobalStyle(struct text *t);
    data:
      struct style *InstructionsStyle, *BigBoldStyle;
};
