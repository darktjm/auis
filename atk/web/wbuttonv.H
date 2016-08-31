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

#ifndef _wbuttonv_H_
#define _wbuttonv_H_
ATK_INTER

#include <buttonV.H>
class wbuttonv: public buttonV{
public:
	virtual ATKregistryEntry *ATKregistry();
	wbuttonv();
	static boolean InitializeClass();

	void FullUpdate(enum view_UpdateType type, 
		long left, long top, long width, long right);
	void Update();

	virtual void SetLabel(const char *label);

	char *wlabel;
	int needinit;
};

#endif /* _wbuttonv_H_ */
