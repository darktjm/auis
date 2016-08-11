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


/* Nick Williams, August 1990 */

/* tabs are a package. 
 * However, we want to be able to define methods and data,
 * so we have it as a basic class.
 */

class tabs {
classprocedures:
    InitializeObject(struct tabs *self) returns boolean;
    FinalizeObject(struct tabs *self);
    Create() returns struct tabs *;
    Death(struct tabs *self);
methods:
    OutputTroff(long indent, FILE *file);
    Different(struct tabs *b) returns int;
    Delete(int n) returns struct tabs *;
    Add(long pos, enum style_TabAlignment op) returns struct tabs *;
    Clear() returns struct tabs *;
    ApplyChange(struct tabentry *change) returns struct tabs *;
    Copy() returns struct tabs *;
data:
    long *Positions;
    long *Types;
    int number;
    int links;
};