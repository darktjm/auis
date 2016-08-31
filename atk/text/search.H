ATK_INTER
#ifndef _search_H_
#define _search_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#define search_VERSION 1

#include <ATK.H>

class search : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static const char * CompilePattern(const char *string, struct SearchPattern **result)  ;
    static int MatchPattern(class simpletext *d, long pos, struct SearchPattern *p)  ;
    static int MatchPatternReverse(class simpletext *d, long pos, struct SearchPattern *p)  ;
    static int MatchPatternStr(unsigned char *str, long pos, long length, struct SearchPattern *p)  ;
    static int MatchPatternStrReverse(unsigned char *str, long pos, long length, struct SearchPattern *p)  ;
    static int GetMatchLength()  ;
    static char * GetQuotedSearchString(char *string, char *resString, long resStrLen)  ;
};

#endif /* _search_H_ */
