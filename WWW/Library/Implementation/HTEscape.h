/*                                             Escape and Unescape Illegal Characters in URIs
                      ESCAPE AND UNESCAPE ILLEGAL CHARACTERS IN URIS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module have been spawned from HTParse, as it then can be used in utility programs
   without loading a whole bunch of the library code. It contains functions for escaping
   and unescaping a URI for reserved characters in URIs.
   
   This module is implemented by HTEscape.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTESCAPE_H
#define HTESCAPE_H
/*

Encode Unacceptable Characters using %xy

   This funtion takes a string containing any sequence of ASCII characters, and returns a
   malloced string containing the same infromation but with all "unacceptable" characters
   represented in the form %xy where x and y are two hex digits.
   
 */
extern char * HTEscape PARAMS((CONST char * str, unsigned char mask));/*

   The following are valid mask values. The terms are the BNF names in the URI document.
   
 */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2
#define URL_PATH        (unsigned char) 4/*

Decode %xy Escaped Characters

   This function takes a pointer to a string in which character smay have been encoded in
   %xy form, where xy is the acsii hex code for character 16x+y. The string is converted
   in place, as it will never grow.
   
 */
extern char * HTUnEscape PARAMS(( char * str));


#endif  /* HTESCAPE_H *//*

   End of HTEscape Module */
