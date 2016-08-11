/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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


 
#define mentertext_BUFCHANGEDFLAG 10101
class mentertext[menttext] : text {
overrides:
    InsertCharacters(long pos, char *str, long len) returns boolean;
    DeleteCharacters(long pos, long len) returns boolean;
    ViewName() returns char *;
methods:
    updatebuf();
    Changed()returns boolean;
    SetChars(char *str,int len);
    ClearLine(long pos);
macromethods:
    buflen() (self->buflen)
    GetString() (self->buf)
    GetStringArray() (self->bufp)
    GetArraySize() (self->bufpcount)
    GetSrcString() (self->src)
classprocedures:
    InitializeObject(struct entertext *self)returns boolean;
    FinalizeObject(struct entertext *self)returns boolean;
data:
    char *buf,*bufp[128],*src;
    long bufpcount;
    long buflen,mod,realbuflen;
    struct style *Style;
    boolean needswrap;
};
