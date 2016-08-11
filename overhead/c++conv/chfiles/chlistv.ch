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

class chlistview[chlistv]:textview[textv] {
    classprocedures:
      InitializeObject(struct chlistview *self) returns boolean;
      FinalizeObject(struct chlistview *self);
    overrides:
      Hit (enum view_MouseAction action, long x, long y, long numberOfClicks) returns struct view *;
      GetStyleInformation(struct text_statevector *sv, long pos, long *length) returns struct environment *;
    methods:
      ActivateItem(int pos);
      HighlightItem(long index);
      UnhighlightItem(long index);
      GetRegionStyle(long regionID, boolean highlighted) returns struct style *;
      SetRegionStyles(long regionID, struct style *normalStyle, struct style highlightStyle);

      SetUpdateRegion(long pos, long len);
    data:
      long highlightedItem;
      long numStylesAllocated;
      struct style **normalStyles;
      struct style **highlightedStyles;
};
