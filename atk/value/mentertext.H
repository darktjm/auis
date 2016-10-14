#ifndef _mentertext_H_
#define _mentertext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#define mentertext_BUFCHANGEDFLAG 10101
#include <text.H>
class mentertext : public  text {
public:
	TRACED_CLASS(mentertext);


     boolean InsertCharacters(long pos, const char *str, long len)  ;
     boolean DeleteCharacters(long pos, long len)  ;
     const char * ViewName()  ;

    virtual void updatebuf();
    virtual boolean Changed() ;
    virtual void SetChars(const char *str,int len);
    virtual void ClearLine(long pos);

    inline long buflen() { return (this->buflenval); }
    inline char * GetString() { return (this->buf); }
    inline const char * const * GetStringArray() { return (this->bufp); }
    inline long GetArraySize() { return (this->bufpcount); }
    inline const char * GetSrcString() { return (this->src); }

    
	mentertext();
    

    char *buf;
    const char *src;
    const char *bufp[128];
    long bufpcount;
    long buflenval,mod,realbuflen;
    class style *Style;
    boolean needswrap;
};
/** @} */
#endif /* _mentertext_H_ */
