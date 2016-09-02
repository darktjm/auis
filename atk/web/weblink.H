/* Copyright 1995 Carnegie Mellon University -- All rights reserved

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
#ifndef _weblink_H_
#define _weblink_H_
ATK_INTER

class text;

#include <link.H>
class weblink: public link{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();

	const char * GetResolvedLink()  ;

	static weblink *GetNewWebLink(char *url)  ;
	static boolean isLinkString(char *s)  ; 
	static char *warpToURL(char *url, class view *v)  ;
	static void ReplaceTextWithLinks(class text *ptext, 
		long pos, long len) ;
	static void warpToURLinText(class text *ptext, 
		long pos, long len, class view *v) ;
};

#endif /* _weblink_H_ */
