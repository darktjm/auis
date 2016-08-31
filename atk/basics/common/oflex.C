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

#include <oflex.H>

oflex::oflex(size_t typesize, size_t n) : mflex(typesize, n) {
}

oflex::oflex(const oflex &src) : mflex(src.typesize, src.GetN()) {
    char *pelts=flex::Insert(0, src.typesize*src.GetN());
    for(size_t i=0;i<src.GetN();i++) {
	memcpy(pelts+i*src.typesize, &src[i], typesize);
    }
}
	
    
oflex::~oflex() {
}

void oflex::MemCpy(char *dest, char *src, size_t bytelength) {
    MemMove(dest, src, bytelength);
}

void oflex::MemMove(char *dest, char *src, size_t bytelength) {
    size_t len=bytelength/typesize;
    if((size_t)(dest-src)>=bytelength) {
	/* copy forward */
	while(len-->0) {
	    CopyElement((void *)dest,(void *)src);
	    DestroyElement((void *)src);
	    dest+=typesize;
	    src+=typesize;
	}
    } else {
	/* copy backward */
	src+=bytelength-typesize;
	dest+=bytelength-typesize;
	while(len-->0) {
	    CopyElement((void *)dest,(void *)src);
	    DestroyElement((void *)src);
	    dest-=typesize;
	    src-=typesize;
	}
    }
}

void oflex::Invalidate(size_t i, size_t len) {
    size_t j;
    i/=typesize;
    len/=typesize;
    for(j=i;j<i+len;j++) {
	DestroyElement((void *)&(*this)[j]);
    }
}

char *oflex::Insert(size_t i, size_t len) {
    char *res=mflex::Insert(i, len), *p=res;
    while(len-->0) {
	ConstructElement((void *)p);
	p+=typesize;
    }
    return res;
}


void oflex::DestroyElement(void */* src */) {
}

void oflex::ConstructElement(void */* src */) {
}

void oflex::CopyElement(void */*dest */, void */*src*/) {
}
