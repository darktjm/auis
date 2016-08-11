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




/* rawtext.ch: Text subclass specialized for dealing with initfiles and other style-less text.
*/

class rawtext: text {

  overrides:
    InsertCharacters(long pos, char *str, long len) returns boolean; /*RSK92overstrike*/
    SetAttributes(struct attributes *atts);
    Write(FILE *file, long writeID, int level) returns long;

  methods:
    JustInsertCharacters(long pos,char *str,long len) returns boolean; /*RSK92overstrike*/
    OverstrikeAChar(long pos); /*RSK92overstrike*/

  classprocedures:
    InitializeObject(struct rawtext *self) returns boolean;
    FinalizeObject(struct rawtext *self);

  macromethods:
    PutNewlineAtEOF() ((self)->NewlineEOF)
    IsInOverstrikeMode() ((self)->OverstrikeMode) /*RSK92overstrike*/
    ChangeOverstrikeMode(boolean newValue) (((self)->OverstrikeMode) = (newValue)) /*RSK92overstrike*/

  data:
    boolean NewlineEOF;
    boolean OverstrikeMode; /*RSK92overstrike*/ /*ideally, this should be associated with the view; unfortunately, the view is not known to srctext_InsertCharacters, which is where the overstriking is being done*/
};

