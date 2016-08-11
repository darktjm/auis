ATK_INTER
#ifndef _simpletext_H_
#define _simpletext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
*/


 

/* Simple text conatins a string of text that can be manipulated.
 * The string is maintained in two section divided by a gap.
 * Position is always given referring to the next character.
 * Thus if position is set to 0 then an InsertString will insert characters
 * at the beginning of the string.
 */

#define simpletext_VERSION 1

#include <mark.H>
#include <dataobject.H>

class simpletext: public dataobject {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static boolean InitializeClass();
    simpletext();
    virtual ~simpletext();

    void SetAttributes(struct attributes *attributes);

    long Read(FILE *file, long id);
    long Write(FILE *file, long writeid, int level);

    /* Operations for Marks and Fence */
    virtual class mark * CreateMark(long pos, long length);
    virtual void RemoveMark(class mark *mark);
    virtual void SetReadOnly(boolean readOnly);
    virtual boolean GetReadOnly();
    virtual void RegionModified(long pos, long len);
    virtual void LengthChanged(long pos, long len);

    /* Operations for text */
    virtual void Clear();
    virtual boolean InsertCharacters(long pos, char *str, long len);  /* FALSE if read-only */
    virtual void AlwaysInsertCharacters(long position, char *string, long size);
    virtual void AddInCharacter(long pos, char c);   /* Does not modify marks */
    virtual boolean DeleteCharacters(long pos, long len);  /* FALSE if read-only */
    virtual void AlwaysDeleteCharacters(long position, long size);
    virtual boolean ReplaceCharacters(long pos, long len, char *repStr, long repLen);  /* FALSE if read-only */
    virtual void AlwaysReplaceCharacters(long pos, long len, char *repStr, long repLen);

    virtual long GetLength();
    virtual long GetChar(long pos);
    virtual long GetUnsignedChar(long pos);
    virtual char * GetBuf(long pos, long len, long *lenp);
    virtual char * GetBufEnd(long endpos, long len, long *lenp);
    virtual char * GetGap(long pos, long len);

    virtual long GetPosForLine(long line);
    virtual long GetLineForPos(long pos);
    virtual long GetBeginningOfLine(long pos);
    virtual long GetEndOfLine(long pos);

    virtual boolean CopyText(long pos, class simpletext *srctext, long srcpos, long len);  /* FALSE if read-only */
    virtual void AlwaysCopyText(long pos, class simpletext *srctext, long srcpos, long len);
    virtual long Index(long pos, char c, long len);
    virtual int Strncmp(long pos, char *str, long len);
    virtual int Textncmp(long pos, class simpletext *text, long pos2, long len);

    /* Operations for FILE I/O */
    virtual long ReadSubString(long pos, FILE *file, boolean quoteCharacters);
    virtual void WriteSubString(long pos, long len, FILE *file, boolean quoteCharacters);
    virtual void CopySubString(long pos, long len, char *buf, boolean quoteCharacters);
    virtual long HandleDataObject(long pos, class dataobject *dop, FILE *file);
    virtual long HandleKeyWord(long pos, char *keyword, FILE *file);
    virtual long HandleCloseBrace(long pos, FILE *file);
    virtual long HandleBegindata(long pos, FILE *file);

    virtual boolean CheckHighBit();
    virtual boolean CopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len);  /* FALSE if read-only */
    virtual void AlwaysCopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len);

    inline long GetFence() { return ((this)->fence)->GetLength(); }
    inline void SetFence(long pos) { ((this)->fence)->SetLength( pos); }
    inline void ClearFence() { ((this)->fence)->SetLength( 0L); }
    inline class mark * GetFenceMark() { return ((this)->fence); }
    inline boolean GetObjectInsertionFlag() { return ((this)->objectInsertionAllowed); }
    inline void SetObjectInsertionFlag(boolean flag) { ((this)->objectInsertionAllowed = (flag)); }

    char *string;
    long lowSize;
    long gapSize;
    long length;
    class mark *markList;
    class mark *fence;
    boolean pendingReadOnly;
    boolean objectInsertionAllowed;
    long Version;
    int highbitflag;
    int DOSfile;
    boolean checkDOSFormat;
};

#endif /* _simpletext_H_ */
