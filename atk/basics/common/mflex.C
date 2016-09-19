/* Copyright 1995 Carnegie Mellon University All rights reserved. */
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

