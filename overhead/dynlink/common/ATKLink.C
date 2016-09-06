/* Copyright 1994 Carnegie Mellon University All rights reserved.
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

#include "ATKLink.H"

int ATKLink::FirstObjectFile() {
    int i=0;
    ResetIterator();
    char *p;
    while((p=NextArgument())) {
	int len=strlen(p);
	if(p[len-1]=='o' && p[len-2]=='.') return i;
	i++;
    }
    return i;
}

int ATKLink::LastObjectFile() {
    int i=0, lastpos=0;
    ResetIterator();
    char *p;
    while((p=NextArgument())) {
	int len=strlen(p);
	if(p[len-1]=='o' && p[len-2]=='.') return lastpos=i;
	i++;
    }
    return i;
}


