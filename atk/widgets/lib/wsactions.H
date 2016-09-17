#ifndef _WSactions_H_
#define _WSactions_H_
ATK_INTER
/* ***************************************************
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'
 *  ***************************************************

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

	wsactions.H   -  actions for WidgetSelector

 *    $Log: wsactions.H,v $
 * Revision 1.1  1996/04/22  18:28:42  wjh
 * Initial revision
 *
 * Revision 1.1  1995/12/01   wjh
 * Initial revision
 */

#include <ATK.H>
class wsactions : public  ATK {
  public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	static void Init();	// put entries in proctable
};


#endif /* _WSactions_H_ */

