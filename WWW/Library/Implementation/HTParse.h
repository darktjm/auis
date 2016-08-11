/*                                                             URI Parsing in the WWW Library
                                         HTPARSE
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module contains code to parse URIs and various related things such as:
   
       Parse a URI relative to another URI
      
       Get the URI relative to another URI
      
       Remove redundant data from the URI (HTSimplify and HTCanon)
      
       Expand a local host name into a full domain name (HTExpand)
      
       Search a URI for illigal characters in order to prevent security holes
      
   This module is implemented by HTParse.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTPARSE_H
#define HTPARSE_H

#include "HTEscape.h"/*

HTParse:  Parse a URI relative to another URI

   This returns those parts of a name which are given (and requested) substituting bits
   from the related name where necessary.
   
  CONTROL FLAGS FOR HTPARSE
  
   The following are flag bits which may be ORed together to form a number to give the
   'wanted' argument to HTParse.
   
 */
#define PARSE_ACCESS            16
#define PARSE_HOST               8
#define PARSE_PATH               4
#define PARSE_ANCHOR             2
#define PARSE_PUNCTUATION        1
#define PARSE_ALL               31/*

  ON ENTRY
  
  aName                   A filename given
                         
  relatedName             A name relative to which aName is to be parsed
                         
  wanted                  A mask for the bits which are wanted.
                         
  ON EXIT,
  
  returns                 A pointer to a malloc'd string which MUST BE FREED
                         
 */
extern char * HTParse  PARAMS(( const char * aName,
                                const char * relatedName,
                                int wanted));/*

HTStrip: Strip white space off a string

  ON EXIT
  
   Return value points to first non-white character, or to 0 if none.
   
   All trailing white space is OVERWRITTEN with zero.
   
 */
extern char * HTStrip PARAMS((char * s));/*

HTSimplify: Simplify a UTL

   A URI is allowed to contain the seqeunce xxx/../ which may be replaced by "" , and the
   seqeunce "/./" which may be replaced by "/". Simplification helps us recognize
   duplicate URIs. Thus, the following transformations are done:
   
       /etc/junk/../fred
      becomes
      /etc/fred
      
       /etc/junk/./fred
      becomes
      /etc/junk/fred
      
   but we should NOT change
   
       http://fred.xxx.edu/../.. or
      
       ../../albert.html
      
   In the same manner, the following prefixed are preserved:
   
       ./
      
       //
      
   In order to avoid empty URIs the following URIs become:
   
       /fred/..
      
      becomes /fred/..
      
       /fred/././..
      
      becomes /fred/..
      
       /fred/.././junk/.././
      becomes /fred/..
      
   If more than one set of `://' is found (several proxies in cascade) then only the part
   after the last `://' is simplified.
   
 */
extern char *HTSimplify PARAMS((char *filename));/*

HTRelative:  Make Relative (Partial) URI

   This function creates and returns a string which gives an expression of one address as
   related to another. Where there is no relation, an absolute address is retured.
   
  ON ENTRY,
  
   Both names must be absolute, fully qualified names of nodes (no anchor bits)
   
  ON EXIT,
  
   The return result points to a newly allocated name which, if parsed by HTParse relative
   to relatedName, will yield aName. The caller is responsible for freeing the resulting
   name later.
   
 */
extern char * HTRelative PARAMS((const char * aName, const char *relatedName));/*

HTExpand: Expand a Local Host Name Into a Full Domain Name

   This function expands the host name of the URI from a local name to a full domain name
   and converts the host name to lower case. The advantage by doing this is that we only
   have one entry in the host case and one entry in the document cache.
   
 */
extern char *HTCanon PARAMS ((  char ** filename,
                                char *  host));/*

Prevent Security Holes

   HTCleanTelnetString() makes sure that the given string doesn't contain characters that
   could cause security holes, such as newlines in ftp, gopher, news or telnet URLs; more
   specifically: allows everything between hexadesimal ASCII 20-7E, and also A0-FE,
   inclusive.
   
  str                     the string that is *modified* if necessary.  The string will be
                             truncated at the first illegal character that is encountered.
                         
  returns                 YES, if the string was modified.      NO, otherwise.
                         
 */
extern BOOL HTCleanTelnetString PARAMS((char * str));/*

 */
#endif  /* HTPARSE_H *//*

   End of HTParse Module */
