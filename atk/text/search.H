#ifndef _search_H_
#define _search_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <ATK.H>

/** Search for simple regular expressions.
 *  This class implements searching for most of ez/%ATK.  Patterns are
 *  similar to POSIX basic regular expressions:
 *    - `.`  Match any character
 *    - `*`  Match previous character or group any number of times.
 *    - `[` .. `]`  Match a simple character class.
 *          - '^' at start inverts class.
 *          - `-` in middle selects characters from previous to next.
 *          - any other char matches itself
 *    - `\` escapes any character, except `(` and `)`.
 *    - `\(` .. `\)` Characters within escaped parentheses form a group.
 *    - any other char matches itself
 */
class search : public ATK  {
  private:
    short size, used;
    unsigned char *body;
    int MatchLength;
  public:
    virtual ATKregistryEntry *ATKregistry();

    static const int VERSION = 1; /**< Version information for users */

    /** Default constructor.  Use CompilePattern() to initialize. */
    search() : size(0), used(0), body(NULL), MatchLength(-1) {} ;
    ~search();
    const char * CompilePattern(const char *string)  ;
    /**< Compile the pattern for use with the matching functions.
     *   See class summary for pattern syntax.
     *   \param string The pattern. */
    int MatchPattern(class simpletext *d, long pos)  ;
    /**< Find next occurrence of pattern in simple text.
     *   \param d The simple text.
     *   \param pos The start position.
     *   \return the position of the match (>=pos).  Use GetMatchLength() to
     *   retrieve its length.  If less than zero, there was no match or
     *   no compiled pattern.  */
    int MatchPatternReverse(class simpletext *d, long pos)  ;
    /**< Find previous occurrence of pattern in simple text.
     *   \param d The simple text.
     *   \param pos The start position.
     *   \return the position of the match (<pos).  Use GetMatchLength() to
     *   retrieve its length.  If less than zero, there was no match or
     *   no compiled pattern.  */
    int MatchPatternStr(unsigned char *str, long pos, long length)  ;
    /**< Find next occurrence of pattern in C string.
     *   \param str The string.
     *   \param pos The start position.
     *   \param length The length of the string.
     *   \return the position of the match (>=pos).  Use GetMatchLength() to
     *   retrieve its length.  If less than zero, there was no match or
     *   no compiled pattern.  */
    int MatchPatternStrReverse(unsigned char *str, long pos, long length)  ;
    /**< Find previous occurrence of pattern in C string.
     *   \param str The string.
     *   \param pos The start position.
     *   \return the position of the match (<pos).  Use GetMatchLength() to
     *   retrieve its length.  If less than zero, there was no match or
     *   no compiled pattern.  */
    inline int GetMatchLength() { return MatchLength; }
    /**< Find the length of the last match.
     *   \return the number of characters in the source which matched the
     *   pattern string.  -1 if no match function called yet */
    static char * GetQuotedSearchString(const char *string, char *resString, long resStrLen)  ;
    /**< Quote regular expression characters in string.
     *   Prefixes all special pattern characters in string with `\`.
     *   \param string Input.
     *   \param resString Output buffer.  If NULL, allocate a result
     *    using malloc(3).  The output is guaranteed to 0-terminated, but
     *    if excess space is required if non-NULL the result is silently
     *    truncated.
     *   \param resStrLen Output buffer length.
     *   \return The output buffer.  resString if non-NULL, or the allocated
     *   buffer.
     */
};
/** @} */
#endif /* _search_H_ */
