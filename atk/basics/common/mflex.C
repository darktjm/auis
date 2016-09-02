/* Copyright 1995 Carnegie Mellon University All rights reserved.
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
#include <andrewos.h>

#include <mflex.H>

mflex::mflex(size_t tsize, size_t n) : flex(n*tsize), typesize(tsize) {
}

mflex::~mflex() {
}

size_t mflex::RoundSize(size_t n) const {
    return ((n+typesize-1)/typesize)*typesize;
}


char *mflex::Insert(size_t i, size_t len) {
    return flex::Insert(i*typesize, len*typesize);
}

size_t mflex::Find(const char &o) const {
    int res=flex::Find(o);
    if(res<0) return res;
    else return res/typesize;
}

void mflex::Remove(size_t i, size_t len) {
    flex::Remove(i*typesize, len*typesize);
}


char *mflex::GetBuf(size_t i, size_t len, size_t *gotlenp) {
   size_t lgotlen;
   char *res=flex::GetBuf(i*typesize, len*typesize, &lgotlen);
   *gotlenp=lgotlen/typesize;
   return res;
}

