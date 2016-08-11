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



/*  compress.ch:  Compress code areas is source text views. */

class compress: text {

  classprocedures:
    InitializeObject(struct compress *self)returns boolean;
    FinalizeObject(struct compress *self);
    InitializeClass() returns boolean; 
    CurrentIndentation(struct text *txt,long pos) returns int;

  overrides:
    Read (FILE *file, long id) returns long;

  methods:
    GetStyle(struct text *txt);
    Compress(struct text *txt,long pos,long len);
    NextCompressLoc(struct text *txt,long startpos,int ind) returns long;
    Decompress(struct text *txt);
    IsHidden(struct text *txt,long pos) returns boolean;

  macromethods:
    GetLocatn() ((self)->locatn)
    SetLocatn(long newValue) ( ((self)->locatn) = (newValue) )
    GetCprsLength() ((self)->length)
    SetCprsLength(long newValue) ( ((self)->length) = (newValue) )
    GetLines() ((self)->lines)
    SetLines(long nlines) ( ((self)->lines) = (nlines) )
    GetParenttxt() ((self)->parenttext)

  data:
    struct mark *cpmark;
    long lines;
    struct text *parenttext;
    struct textview *parentview;
    struct envlist *envl;
    struct style *compressed_style;
};

