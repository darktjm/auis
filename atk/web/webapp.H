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
#ifndef _webapp_H_
#define _webapp_H_
ATK_INTER


#include <mflex.H>

class buffer;
DEFINE_MFLEX_CLASS(webapp_histlist, buffer *, 5);

struct webapp_webhist {
    webapp_histlist history;
    class im *im;
    struct webapp_webhist *next;
    size_t hpos;
};

#include <ezapp.H>
class webapp: public ezapp{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
    
	 boolean ParseArgs(int argc, char **argv)  ;
	 boolean Start()  ;
        	 int Run()  ;
        	 boolean Fork()  ;

	webapp();
	virtual ~webapp();

	static struct webapp_webhist * getwebhist(class view *view)  ;

	class value *bt;
	class wbuttonv *bv;
	class lpair *lpMain;
};

#endif /* _webapp_H_ */
