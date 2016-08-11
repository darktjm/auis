/*                                                      The Format Manager in the WWW Library
                                    THE FORMAT MANAGER
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   Here we describe the functions of the HTFormat module which handles conversion between
   different data representations. (In MIME parlance, a representation is known as a
   content-type. In WWW the term format is often used as it is shorter). The content of
   this module is:
   
      Buffering for I/O
      
      Read from the Network
      
      Predefined Format Types
      
      Registration of Accepted Content Encodings
      
      Registration of Accepted Content Languages
      
      Registration of Accepted Converters and Presenters
      
      Ranking of Accepted Formats
      
      The Stream Stack
      
   This module is implemented by HTFormat.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTFORMAT_H
#define HTFORMAT_H

#include "HTUtils.h"
#include "HTStream.h"
#include "HTAtom.h"
#include "HTList.h"

#ifdef SHORT_NAMES
#define HTOutputSource HTOuSour
#define HTOutputBinary HTOuBina
#endif/*

Buffering for network I/O

   This routines provide buffering for READ (and future WRITE) to the network. It is used
   by all the protocol modules. The size of the buffer, INPUT_BUFFER_SIZE, is a compromis
   between speed and memory.
   
 */
#define INPUT_BUFFER_SIZE 8192

typedef struct _socket_buffer {
        char    input_buffer[INPUT_BUFFER_SIZE];
        char *  input_pointer;
        char *  input_limit;
        SOCKFD  input_file_number;
} HTInputSocket;/*

  CREATE AN INPUT BUFFER
  
   This function allocates a input buffer and binds it to the socket descriptor given as
   parameter.
   
 */
extern HTInputSocket* HTInputSocket_new PARAMS((SOCKFD file_number));/*

  FREE AN INPUT BUFFER
  
 */
extern void HTInputSocket_free PARAMS((HTInputSocket * isoc));/*

Read from the Network

   This function has replaced many other functions for doing read from the network. It
   automaticly converts from ASCII if we are on a NON-ASCII machine. This assumes that we
   do not use this function to read a local file on a NON-ASCII machine. The following
   type definition is to make life easier when having a state machine looking for a <CRLF>
   sequence.
   
 */
typedef enum _HTSocketEOL {
    EOL_ERR = -1,
    EOL_BEGIN = 0,
    EOL_FCR,
    EOL_FLF,
    EOL_DOT,
    EOL_SCR,
    EOL_SLF
} HTSocketEOL;

extern int HTInputSocket_read   PARAMS((HTInputSocket * isoc,
                                        HTStream * target));/*

Convert Net ASCII to local representation

   This is a filter stream suitable for taking text from a socket and passing it into a
   stream which expects text in the local C representation.  It does newline conversion.
   As usual, pass its output stream to it when creating it.
   
 */
extern HTStream * HTNetToText PARAMS ((HTStream * sink));/*

The HTFormat types

   We use the HTAtom object for holding representations. This allows faster manipulation
   (comparison and copying) that if we stayed with strings. These macros (which used to be
   constants) define some basic internally referenced representations.
   
 */
typedef HTAtom * HTFormat;/*

  INTERNAL ONES
  
   The www/xxx ones are of course not MIME standard.
   
   star/star is an output format which leaves the input untouched. It is useful for
   diagnostics, and for users who want to see the original, whatever it is.
   
 */
#define WWW_SOURCE      HTAtom_for("*/*")      /* Whatever it was originally *//*

   www/present represents the user's perception of the document.  If you convert to
   www/present, you present the material to the user.
   
 */
#define WWW_PRESENT     HTAtom_for("www/present")   /* The user's perception *//*

   The message/rfc822 format means a MIME message or a plain text message with no MIME
   header. This is what is returned by an HTTP server.
   
 */
#define WWW_MIME        HTAtom_for("www/mime")             /* A MIME message *//*

   www/print is like www/present except it represents a printed copy.
   
 */
#define WWW_PRINT       HTAtom_for("www/print")            /* A printed copy *//*

   www/unknown is a really unknown type.  Some default action is appropriate.
   
 */
#define WWW_UNKNOWN     HTAtom_for("www/unknown")/*

  MIME ONES
  
   These are regular MIME types defined. Others can be added!
   
 */
#define WWW_PLAINTEXT   HTAtom_for("text/plain")
#define WWW_POSTSCRIPT  HTAtom_for("application/postscript")
#define WWW_RICHTEXT    HTAtom_for("application/rtf")
#define WWW_AUDIO       HTAtom_for("audio/basic")
#define WWW_HTML        HTAtom_for("text/html")
#define WWW_BINARY      HTAtom_for("application/octet-stream")
#define WWW_VIDEO       HTAtom_for("video/mpeg")/*

   Extra types used in the library (EXPERIMENT)
   
 */
#define WWW_NEWSLIST    HTAtom_for("text/newslist")/*

   We must include the following file after defining HTFormat, to which it makes
   reference.
   
The HTEncoding type

 */
typedef HTAtom* HTEncoding;/*

   The following are values for the MIME types:
   
 */
#define WWW_ENC_7BIT            HTAtom_for("7bit")
#define WWW_ENC_8BIT            HTAtom_for("8bit")
#define WWW_ENC_BINARY          HTAtom_for("binary")/*

   We also add
   
 */
#define WWW_ENC_COMPRESS        HTAtom_for("compress")/*

Registration of Accepted Content Encodings

   This function is not currently used.
   
 */
typedef struct _HTContentDescription {
    char *      filename;
    HTAtom *    content_type;
    HTAtom *    content_language;
    HTAtom *    content_encoding;
    int         content_length;
    double      quality;
} HTContentDescription;

extern void HTAcceptEncoding PARAMS((HTList *   list,
                                     char *     enc,
                                     double     quality));/*

Registration of Accepted Content Languages

   This function is not currently used.
   
 */
extern void HTAcceptLanguage PARAMS((HTList *   list,
                                     char *     lang,
                                     double     quality));/*

Registration of Accepted Converters and Presenters

   A converter is a stream with a special set of parameters and which is registered as
   capable of converting from a MIME type to something else (maybe another MIME-type). A
   presenter is a module (possibly an external program) which can present a graphic object
   of a certain MIME type to the user. That is, presenters are normally used to present
   graphic objects that the converters are not able to handle. Data is transferred to the
   external program using the HTSaveAndExecute stream which writes to a local file. This
   stream is actually a normal converter, e.g., at strem having the following set of
   parameters:
   
 */
#include "HTAccess.h"                   /* Required for HTRequest definition */

typedef HTStream * HTConverter  PARAMS((HTRequest *     request,
                                        void *          param,
                                        HTFormat        input_format,
                                        HTFormat        output_format,
                                        HTStream *      output_stream));/*

   Both converters and presenters are set up in a list which is used by the StreamStack
   module to find the best way to pass the information to the user.
   
   The HTPresentation structure contains both converters and presenters, and it is defined
   as:
   
 */
typedef struct _HTPresentation {
        HTAtom* rep;                         /* representation name atomized */
        HTAtom* rep_out;                         /* resulting representation */
        HTConverter *converter;       /* The routine to gen the stream stack */
        char *  command;                               /* MIME-format string */
        char *  test_command;                          /* MIME-format string */
        double  quality;                     /* Between 0 (bad) and 1 (good) */
        double   secs;
        double   secs_per_byte;
} HTPresentation;/*

  GLOBAL LIST OF CONVERTERS
  
   This module keeps a global list of converters. This can be used to get the set of
   supported formats.
   
   NOTE: There is also a conversion list associated with each request in the HTRequest
   structure.
   
 */
extern HTList * HTConversions;/*

  REGISTER A PRESENTER
  
  conversions            The list of conveters and presenters
                         
  representation         the MIME-style format name
                         
  command                the MAILCAP-style command template
                         
  quality                A degradation faction [0..1]
                         
  maxbytes               A limit on the length acceptable as input (0 infinite)
                         
  maxsecs                A limit on the time user will wait (0 for infinity)
                         
 */
extern void HTSetPresentation   PARAMS((HTList *        conversions,
                                        CONST char *    representation,
                                        CONST char *    command,
                                        CONST char *    test_command,
                                        double          quality,
                                        double          secs,
                                        double          secs_per_byte));/*

  REGISTER A CONVERTER
  
  conversions            The list of conveters and presenters
                         
  rep_in                 the MIME-style format name
                         
  rep_out                is the resulting content-type after the conversion
                         
  converter              is the routine to call which actually does the conversion
                         
  quality                A degradation faction [0..1]
                         
  maxbytes               A limit on the length acceptable as input (0 infinite)
                         
  maxsecs                A limit on the time user will wait (0 for infinity)
                         
 */
extern void HTSetConversion     PARAMS((HTList *        conversions,
                                        CONST char *    rep_in,
                                        CONST char *    rep_out,
                                        HTConverter *   converter,
                                        double          quality,
                                        double          secs,
                                        double          secs_per_byte));/*

  SET UP DEFAULT PRESENTERS AND CONVERTERS
  
   A default set of converters and presentersare defined in HTInit.c (or HTSInit.c in the
   server).
   
   NOTE:  No automatic initialization is done in the Library, so this is for the
   application to do
   
 */
extern void HTFormatInit        PARAMS((HTList * conversions));/*

   This function also exists in a version where no presenters are initialized. This is
   intended for Non Interactive Mode, e.g., in the Line Mode Browser.
   
 */
extern void HTFormatInitNIM     PARAMS((HTList * conversions));/*

  REMOVE PRESENTATIONS AND CONVERSIONS
  
   This function deletes the LOCAL list of converters and presenters associated with each
   HTRequest structure.
   
 */
extern void HTFormatDelete      PARAMS((HTRequest * request));/*

   This function cleans up the GLOBAL list of converters. The function is called from
   HTLibTerminate.
   
 */
extern void HTDisposeConversions NOPARAMS;/*

Ranking of Accepted Formats

   This function is used when the best match among several possible documents is to be
   found as a function of the accept headers sent in the client request.
   
 */
extern BOOL HTRank PARAMS((HTList * possibilities,
                           HTList * accepted_content_types,
                           HTList * accepted_content_languages,
                           HTList * accepted_content_encodings));/*

HTStreamStack

   This is the routine which actually sets up the conversion. It currently checks only for
   direct conversions, but multi-stage conversions are forseen.  It takes a stream into
   which the output should be sent in the final format, builds the conversion stack, and
   returns a stream into which the data in the input format should be fed.
   
   If guess is true and input format is www/unknown, try to guess the format by looking at
   the first few bytes of the stream.
   
 */
extern HTStream * HTStreamStack PARAMS((HTFormat        rep_in,
                                        HTFormat        rep_out,
                                        HTStream *      output_stream,
                                        HTRequest *     request,
                                        BOOL            guess));/*

  FIND THE COST OF A FILTER STACK
  
   Must return the cost of the same stack which HTStreamStack would set up.
   
 */
#define NO_VALUE_FOUND  -1e20           /* returned if none found */

extern double HTStackValue      PARAMS((HTList *        conversions,
                                        HTFormat        format_in,
                                        HTFormat        format_out,
                                        double          initial_value,
                                        long int        length));/*

   
   ___________________________________
   [IMAGE] ATTENTION [IMAGE]
   
   THE REST OF THE FUNCTION DEFINED IN THIS MODULE ARE GOING TO BE OBSOLETE SO DO NOT USE
   THEM - THEY ARE NOT REENTRANT.
   ___________________________________
   
HTCopy:  Copy a socket to a stream

   This is used by the protocol engines to send data down a stream, typically one which
   has been generated by HTStreamStack. Returns the number of bytes transferred.
   
 */
extern int HTCopy       PARAMS((SOCKFD          file_number,
                                HTStream *      sink));/*

HTFileCopy:  Copy a file to a stream

   This is used by the protocol engines to send data down a stream, typically one which
   has been generated by HTStreamStack. It is currently called by HTParseFile
   
 */
extern void HTFileCopy PARAMS((
        FILE*                   fp,
        HTStream*               sink));

        /*

HTCopyNoCR: Copy a socket to a stream, stripping CR characters.

   It is slower than HTCopy .
   
 */
extern void HTCopyNoCR PARAMS((
        SOCKFD                  file_number,
        HTStream*               sink));

/*

HTParseSocket: Parse a socket given its format

   This routine is called by protocol modules to load an object.  usesHTStreamStack and
   the copy routines above.  Returns HT_LOADED if succesful, <0 if not.
   
 */
extern int HTParseSocket PARAMS((
        HTFormat        format_in,
        SOCKFD          file_number,
        HTRequest *     request));
/*

HTParseFile: Parse a File through a file pointer

   This routine is called by protocols modules to load an object. uses HTStreamStack and
   HTFileCopy .  Returns HT_LOADED if succesful, <0 if not.
   
 */
extern int HTParseFile PARAMS((
        HTFormat        format_in,
        FILE            *fp,
        HTRequest *     request));
/*

  GET NEXT CHARACTER FROM BUFFER
  
 */
extern int HTInputSocket_getCharacter PARAMS((HTInputSocket* isoc));/*

  READ BLOCK FROM INPUT SOCKET
  
   Read *len characters and return a buffer (don't free) containing *len characters ( *len
   may have changed). Buffer is not NULL-terminated.
   
 */
extern char * HTInputSocket_getBlock PARAMS((HTInputSocket * isoc,
                                                  int *           len));

extern char * HTInputSocket_getLine PARAMS((HTInputSocket * isoc));
extern char * HTInputSocket_getUnfoldedLine PARAMS((HTInputSocket * isoc));
extern char * HTInputSocket_getStatusLine PARAMS((HTInputSocket * isoc));
extern BOOL   HTInputSocket_seemsBinary PARAMS((HTInputSocket * isoc));

#endif
/*

   End of definition module  */
