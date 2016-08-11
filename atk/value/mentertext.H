ATK_INTER
#ifndef _mentertext_H_
#define _mentertext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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


 
#define mentertext_BUFCHANGEDFLAG 10101
#include <text.H>
class mentertext : public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean InsertCharacters(long pos, char *str, long len)  ;
     boolean DeleteCharacters(long pos, long len)  ;
     char * ViewName()  ;

    virtual void updatebuf();
    virtual boolean Changed() ;
    virtual void SetChars(char *str,int len);
    virtual void ClearLine(long pos);

    inline long buflen() { return (this->buflenval); }
    inline char * GetString() { return (this->buf); }
    inline char ** GetStringArray() { return (this->bufp); }
    inline long GetArraySize() { return (this->bufpcount); }
    inline char * GetSrcString() { return (this->src); }

    
	mentertext();
    
	virtual ~mentertext();

    char *buf,*bufp[128],*src;
    long bufpcount;
    long buflenval,mod,realbuflen;
    class style *Style;
    boolean needswrap;
};

#endif /* _mentertext_H_ */