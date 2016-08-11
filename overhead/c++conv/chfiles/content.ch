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



struct content_chapentry {
    struct mark *rem,*loc;
    struct content_chapentry *next;
    int which,space;
};

class content : text {
overrides:
      ObservedChanged (struct observable *changed, long value);
      ViewName() returns char *;
methods:
      reinit();
      SetSourceText(struct text *txt);
      UpdateSource(long pos,long len);
      Enumerate(long pos,long len,char *start) returns long;
      Denumerate(long pos,long len);
      StringToInts(long pos,int *lev) returns long;
      locate(long pos) returns struct mark *;
      CopyEntry(long pos,long len,char *buf,long buflen) returns  struct content_chapentry*;
classprocedures:
      FinalizeObject(struct content *self);
      InitializeObject(struct content *self) returns boolean;
data:
      struct content_chapentry *entry,*indexentry;
      struct text *srctext;
      char *names[48];
      int namecount;
      int InUpdate;
      int enumerate;
      char *ChapNumber;
      boolean doindent,isindented;
      int chapcount;
};

 
