/* Copyright 1995 Carnegie Mellon University  --All rights reserved

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
#ifndef _web_H_
#define _web_H_
ATK_INTER

#define web_Limit 1
#define web_IncludeContext 2
#define web_ISMAP 4

#include <htmltext.H>
class web: public htmltext {
	boolean imbedded;
public:
	virtual ATKregistryEntry *ATKregistry();

	web();
	~web();
	static boolean InitializeClass();

	virtual void SetWebcom(class webcom *c);
	virtual long ReadHTML(FILE* file, long id)  ; 
    
	long Read(FILE* file, long id)  ;
	inline class webcom  * getwebcom() 
			{ return ((this)->webcomp); }
	virtual void ImbedTextInTable(table *t, chunk *c);
	inline boolean Imbedded() {
	    return imbedded;
        }
        image *GetImage(const char *file, attlist *atts);
	webcom *webcomp;
};

#endif /* _web_H_ */
