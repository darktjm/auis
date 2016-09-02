#ifndef _compress_H_
#define _compress_H_
/* File compress.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $

   compress: a dataobject to store a hidden region of text. */

#include <text.H>
class compress: public text {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    compress();
    virtual ~compress();

    static void Compress(text *txt,long pos,long len);
    static void DecompressAll(text *txt);
    static void DecompressRange(text *txt,long pos,long len);
    static compress *IsThere(text *txt,long pos);

    long GetLineForPos(long pos);
    long GetModified();
    long GetPosForLine(long line);

    virtual void CompressInFront(text *txt,long pos,long len);
    virtual void CompressInBack(text *txt,long pos,long len);
    virtual void DecompressBox(text *txt);
    virtual void PartialDecompress(text *txt,long pos,long len);

    inline long GetLines() { return ((this)->lines); }
    inline void SetLines(long nlines) { ((this)->lines) = (nlines); }

    long lines, loc;
};

#endif /* _compress_H_ */
