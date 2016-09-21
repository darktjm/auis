ATK_INTER
#ifndef _entertext_H_
#define _entertext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#define entertext_BUFCHANGEDFLAG 10101
#include <text.H>
class entertext : public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean InsertCharacters(long pos, const char *str, long len)  ;
     boolean DeleteCharacters(long pos, long len)  ;
     const char * ViewName()  ;

    virtual void updatebuf();
    virtual boolean Changed() ;
    virtual void SetChars(const char *str,int len);

    inline long buflen() { return (this->buflenval); }
    inline char * GetString() { return (this->buf); }

    
	entertext();
    
	virtual ~entertext();

    char *buf;
    long buflenval,mod,realbuflen;
    class style *Style;
    boolean needswrap;
};
/** @} */
#endif /* _entertext_H_ */
