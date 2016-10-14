#ifndef _compress_H_
#define _compress_H_
/** \addtogroup libsrctext
 * @{ */
/* File compress.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   compress: a dataobject to store a hidden region of text. */

#include <text.H>
class compress: public text {
  public:
    TRACED_CLASS(compress);
    static boolean InitializeClass();

    compress();

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
/** @} */
#endif /* _compress_H_ */
