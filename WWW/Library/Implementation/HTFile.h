/*                                                                      File access in libwww
                                       FILE ACCESS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   These are routines for local file access used by WWW browsers and servers.
   
   If the file is not a local file, then we pass it on to HTFTP in case it can be reached
   by FTP. However, as this is very time consuming when requesting a local file that
   actually doesn't exist, this redirection will be disabled in the next major release,
   www-bug@info.cern.ch June 1994.
   
   Note: All functions that deal with directory listings etc. have been moved to HTDirBrw
   Module.
   
   This module is implemented by HTFile.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTFILE_H
#define HTFILE_H

#include "HTFormat.h"
#include "HTAccess.h"
#include "HTML.h"               /* SCW */
#include "HTDirBrw.h"/*

Multiformat Handling

  SPLIT FILENAME TO SUFFIXES
  
 */
extern int HTSplitFilename (char *       s_str,
                                   char **      s_arr);/*

  GET CONTENT DESCRIPTION ACCORDING TO SUFFIXES
  
 */
extern HTContentDescription * HTGetContentDescription (char ** actual,
                                                              int         n);

#define MULTI_SUFFIX ".multi"   /* Extension for scanning formats */
#define MAX_SUFF 15             /* Maximum number of suffixes for a file *//*

Convert filenames between local and WWW formats

 */
extern char * HTLocalName (CONST char * name);/*

Make a WWW name from a full local path name

 */
extern char * WWW_nameOfFile (const char * name);/*

Generate the name of a cache file

 */
extern char * HTCacheFileName (CONST char * name);/*

Define the Representation for a File Suffix

   This defines a mapping between local file suffixes and file content types and
   encodings.
   
  ON ENTRY,
  
  suffix                  includes the "." if that is important (normally, yes!)
                         
  representation          is MIME-style content-type
                         
  encoding                is MIME-style content-transfer-encoding (8bit, 7bit, etc)
                         
  language               is MIME-style content-language
                         
  quality                 an a priori judgement of the quality of such files (0.0..1.0)
                         
 */
/*
** Example:  HTSetSuffix(".ps", "application/postscript", "8bit", NULL, 1.0);
*/

extern void HTSetSuffix (CONST char *    suffix,
                               CONST char *     representation,
                               CONST char *     encoding,
                               CONST char *     language,
                               double           quality);

extern void HTAddType (CONST char *      suffix,
                              CONST char *      representation,
                              CONST char *      encoding,
                              double            quality);

extern void HTAddEncoding (CONST char *  suffix,
                                  CONST char *  encoding,
                                  double                quality);

extern void HTAddLanguage (CONST char *  suffix,
                                  CONST char *  language,
                                  double                quality);


extern void HTFile_deleteSuffixes (void);/*

Get Representation and Encoding from file name

  ON EXIT,
  
  return                  The represntation it imagines the file is in
                         
  *pEncoding              The encoding (binary, 7bit, etc). See HTSetSuffix .
                         
  *pLanguage              The language.
                         
 */
extern HTFormat HTFileFormat (
                CONST char *    filename,
                HTAtom **       pEncoding,
                HTAtom **       pLanguage);

/*

Determine file value from file name

 */
extern double HTFileValue (
                CONST char * filename);

/*

Determine write access to a file

  ON EXIT,
  
  return value            YES if file can be accessed and can be written to.
                         
 */
/*

  BUGS
  
   Isn't there a quicker way?
   
 */
extern BOOL HTEditable (CONST char * filename);

/*

Determine a suitable suffix, given the representation

  ON ENTRY,
  
  rep                     is the atomized MIME style representation
                         
  ON EXIT,
  
  returns                 a pointer to a suitable suffix string if one has been found,
                         else NULL.
                         
 */
extern CONST char * HTFileSuffix (
                HTAtom* rep);


/*

The Protocols

 */
GLOBALREF HTProtocol HTFTP, HTFile;

#endif /* HTFILE_H */
/*

   end of HTFile */
