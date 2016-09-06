/*                                                                 Access manager  for libwww
                                      ACCESS MANAGER
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module keeps a list of valid protocol (naming scheme) specifiers with associated
   access code.  It allows documents to be loaded given various combinations of
   parameters.  New access protocols may be registered at any time.
   
   Note: HTRequest defined and request parameter added to almost all calls 18 Nov 1993.
   
   This module is implemented by HTAccess.c, and it is a part of the Library of Common
   Code.
   
   The module contains a lot of stuff but the main topics are:
   
      Initializing and Terminating the Library
      
      Management of access methods
      
      A lot of hard coded addresses
      
      The HTRequest structure
      
      Management of the HTRequest structure
      
      Functions for loading a document
      
      Help functions for clients to get started
      
      Functions for posting a document
      
      Access Method Registration
      
 */
#ifndef HTACCESS_H
#define HTACCESS_H

#include "HTList.h"
#include "HTChunk.h"/*

   Short Names
   
 */
#ifdef SHORT_NAMES
#define HTClientHost            HTClHost
#define HTSearchAbsolute        HTSeAbso
#define HTOutputStream          HTOuStre
#define HTOutputFormat          HTOuForm
#endif/*

Flags which may be set to control this module

 */
extern char * HTSaveLocallyDir;         /* Dir home for "save locally" files */
extern char * HTCacheDir;               /* Cache dir, default NULL: no cache */
extern char * HTClientHost;             /* Name or number of telnetting host */
extern FILE * HTlogfile;                /* File to output one-liners to */
extern BOOL HTSecure;                   /* Disable security holes? */
extern char * HTImServer;               /* If I'm cern_httpd */
extern BOOL HTImProxy;                  /* If I'm cern_httpd as a proxy */
extern BOOL HTForceReload;              /* Force reload from cache or net *//*

Initializing and Terminating the Library

   [IMAGE]These two functions initiates memory and settings for the Library and cleans up
   memory kept by the Library when about to exit the application. It is highly recommended
   that they are used!
   
 */
extern BOOL HTLibInit (void);
extern BOOL HTLibTerminate (void);/*

Method Management

   These are the valid methods, see HTTP Methods
   
 */
typedef enum {
        METHOD_INVALID  = 0,
        METHOD_GET      = 1,
        METHOD_HEAD,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
        METHOD_CHECKOUT,
        METHOD_CHECKIN,
        METHOD_SHOWMETHOD,
        METHOD_LINK,
        METHOD_UNLINK,
        MAX_METHODS
} HTMethod;/*

  GET METHOD ENUMERATION
  
   Gives the enumeration value of the method as a function of the (char *) name.
   
 */
extern HTMethod HTMethod_enum (char * name);/*

  GET METHOD STRING
  
   The reverse of HTMethod_enum()
   
 */
extern char * HTMethod_name (HTMethod method);/*

  VALID METHODS
  
   This functions searches the list of valid methods for a given anchor, see HTAnchor
   module If the method is found it returns YES else NO.
   
 */
extern BOOL HTMethod_inList (HTMethod    method,
                                    HTList *    list);/*

   
   ___________________________________
   This section might be move to the Access Authentication Module
   
    Match Template Against Filename
    
 */
/* extern                                               HTAA_templateMatch()
**              STRING COMPARISON FUNCTION FOR FILE NAMES
**                 WITH ONE WILDCARD * IN THE TEMPLATE
** NOTE:
**      This is essentially the same code as in HTRules.c, but it
**      cannot be used because it is embedded in between other code.
**      (In fact, HTRules.c should use this routine, but then this
**       routine would have to be more sophisticated... why is life
**       sometimes so hard...)
**
** ON ENTRY:
**      tmplate         is a template string to match the file name
**                      agaist, may contain a single wildcard
**                      character * which matches zero or more
**                      arbitrary characters.
**      filename        is the filename (or pathname) to be matched
**                      agaist the template.
**
** ON EXIT:
**      returns         YES, if filename matches the template.
**                      NO, otherwise.
*/
extern BOOL HTAA_templateMatch (CONST char * tmplate,
                                       CONST char * filename);/*

   
   ___________________________________
   The following have to be defined in advance of the other include files because of
   circular references.
   
 */
typedef struct _HTRequest HTRequest;
typedef struct _HTNetInfo HTNetInfo;

/*
** Callback to a protocol module
*/
typedef int (*HTLoadCallBack)   (HTRequest *     req);

#include "HTAnchor.h"
#include "HTFormat.h"
#include "HTAAUtil.h"           /* HTAAScheme, HTAAFailReason */
#include "HTAABrow.h"           /* HTAASetup *//*

Default WWW Addresses

   These control the home page selection. To mess with these for normal browses is asking
   for user confusion.
   
 */
#define LOGICAL_DEFAULT "WWW_HOME"            /* Defined to be the home page */

#ifndef PERSONAL_DEFAULT
#define PERSONAL_DEFAULT "WWW/default.html"             /* in home directory */
#endif

#ifndef LOCAL_DEFAULT_FILE
#define LOCAL_DEFAULT_FILE "/usr/local/lib/WWW/default.html"
#endif

/* If one telnets to an access point it will look in this file for home page */
#ifndef REMOTE_POINTER
#define REMOTE_POINTER  "/etc/www-remote.url"               /* can't be file */
#endif

/* and if that fails it will use this. */
#ifndef REMOTE_ADDRESS
#define REMOTE_ADDRESS  "http://info.cern.ch/remote.html"   /* can't be file */
#endif

/* If run from telnet daemon and no -l specified, use this file: */
#ifndef DEFAULT_LOGFILE
#define DEFAULT_LOGFILE "/usr/adm/www-log/www-log"
#endif

/* If the home page isn't found, use this file: */
#ifndef LAST_RESORT
#define LAST_RESORT     "http://info.cern.ch/default.html"
#endif

/* This is the default cache directory: */
#ifndef CACHE_HOME_DIR
#define CACHE_HOME_DIR          "/tmp/"
#endif

/* The default directory for "save locally" and "save and execute" files: */
#ifndef SAVE_LOCALLY_HOME_DIR
#define SAVE_LOCALLY_HOME_DIR   "/tmp/"
#endif/*

Protocol Specific Information

   This structure contains information about socket number, input buffer for reading from
   the network etc. The structure is used through out the protocol modules and is the
   refenrence point for introducing multi threaded execution into the library, see
   specifications on Multiple Threads.
   
 */
struct _HTNetInfo {
    SOCKFD              sockfd;                         /* Socket descripter */
    SockA               sock_addr;              /* SockA is defined in tcp.h */
    HTInputSocket *     isoc;                                /* Input buffer */
    HTStream *          target;                             /* Output stream */
    HTChunk *           transmit;                         /* Line to be send */
    int                 addressCount;        /* Attempts if multi-homed host */
    time_t              connecttime;             /* Used on multihomed hosts */
    struct _HTRequest * request;           /* Link back to request structure */
};/*

   Note: The AddressCount varaible is used to count the number of attempt to connect to a
   multi-homed host so we know when to stop trying new IP-addresses.
   
The Request structure

   When a request is handled, all kinds of things about it need to be passed along.  These
   are all put into a HTRequest structure.
   
   Note 1: There is also a global list of converters
   
   Note 2: If you reuse the request structure for more than one request then make sure
   that the request is re-initialized, so that no `old' data is reused, see functions to
   manipulate HTRequest Structure. The library handles its own internal information from
   request to request but the information set by the caller is untouched.
   
   The elements of the request structure are as follows.
   
 */
struct _HTRequest {/*

  SET BY THE CALLER OF HTACCESS:
  
 */
    HTMethod    method;/*

   An enum used to specify the HTTP method used. The default value is GET
   
 */
    HTList *    conversions;/*

   NULL, or a list of conversions which the format manager can do in order to fulfill the
   request.  It typically points to a list set up an initialisation time for example by
   HTInit().
   
 */
    HTList *    encodings;/*

   The list of encodings acceptable in the output stream.
   
 */
    HTList *    languages;/*

   The list of (human) language values acceptable in the response. The default is all
   languages.
   
 */
    BOOL (* callback ) (struct _HTRequest* request,
                                                void *param);/*

   A function to be called back in the event that a file has been saved to disk by
   HTSaveAndCallBack for example.
   
 */
    void *      context;/*

   An arbitrary pointer passed to HTAccess and passed back as a parameter to the callback
   .
   
 */
    HTStream*   output_stream; /*

   The output stream to be used to put data down to as they come in from the network. The
   default value is NULL which means that the stream goes to the user (display).
   
 */
    HTStream*   output_flush; /*

   All object bodies sent from the server with status codes different from 200 OK will be
   put down this stream. This can be used as a debug window etc. If the value is NULL
   (default) then the stream used is HTBlackHole.
   
 */
    HTAtom *    output_format;/*

   The output format of the stream. This can be used to get unconverted data etc. from the
   library. If NULL, then WWW_PRESENT is default value.
   
 */
    BOOL BlockingIO;/*

   This flag can be set to override if a protocol module is registered as using
   non-blocking IO.
   
 */
    HTParentAnchor *parentAnchor;/*

   If this parameter is set then a `Referer: <parent address> is generated in the request
   to the server, see Referer field in a HTTP Request
   
  SET BY HTACCESS
  
   None of the bits below may be looked at by a client application except in the callback
   routine, when the anchor may be picked out.
   
 */
    HTParentAnchor*     anchor;/*

   The anchor for the object in question.  Set immediately by HTAcesss. Used by the
   protocol and parsing modules.  Valid thoughout the access.
   
 */
    HTChildAnchor *     childAnchor;    /* For element within the object  *//*

   The anchor for the sub object if any.  The object builder should ensure that htis is
   selected, highlighted, etc when the object is loaded.
   
 */
    void *              using_cache;/*

   Pointer to cache element if cache hit
   
  ERROR DIAGNOSTICS
  
 */
        BOOL            error_block;    /* YES if stream has been used    */
        HTList *        error_stack;    /* List of errors                 */
/*

  LIBRARY SIDE
  
 */
        HTNetInfo *     net_info;       /* Information about socket etc. */
        int             redirections;   /* Number of redirections *//*

  TEMPORARY UNTIL WE GET A GOOD MIME PARSER
  
 */
        char *          redirect;                      /* Location or URI */
        char *          WWWAAScheme;           /* WWW-Authenticate scheme */
        char *          WWWAARealm;             /* WWW-Authenticate realm */
        char *          WWWprotection;         /* WWW-Protection-Template *//*

  SERVER SIDE
  
 */
        HTAtom *        content_type;   /* Content-Type:                  */
        HTAtom *        content_language;/* Language                      */
        HTAtom *        content_encoding;/* Encoding                      */
        int             content_length; /* Content-Length:                */
        HTInputSocket * isoc;           /* InputSocket object for reading */
        char *          authorization;  /* Authorization: field           */
        HTAAScheme      scheme;         /* Authentication scheme used     *//*

  CLIENT SIDE
  
 */
        HTList *        valid_schemes;  /* Valid auth.schemes             */
        HTAssocList **  scheme_specifics;/* Scheme-specific parameters    */
        char *          authenticate;   /* WWW-authenticate: field */
        char *          prot_template;  /* WWW-Protection-Template: field */
        HTAASetup *     setup;          /* Doc protection info            */
        HTAARealm *     realm;          /* Password realm                 */
        char *          dialog_msg;     /* Authentication prompt (client) */
};
/*

Functions to Manipulate a HTRequest Structure

   Just to make things easier especially for clients, here are some functions to
   manipulate the request structure:
   
  CREATE BLANK REQUEST
  
   This request has defaults in -- in most cases it will need some information added
   before being passed to HTAccess, but it will work as is for a simple request.
   
 */
extern HTRequest * HTRequest_new (void);/*

  DELETE REQUEST STRUCTURE
  
   Frees also conversion list hanging from req->conversions.
   
 */
extern void HTRequest_delete (HTRequest * req);/*

  CLEAR A REQUEST STRUCTURE
  
   Clears a request structure so that it can be reused. The only thing that differs from
   using free/new is that the list of conversions is kept.
   
 */
extern void HTRequest_clear (HTRequest * req);/*

Functions for Loading a Document

   There are several different ways of loading a document. However, the major difference
   between them is whether the document is referenced by
   
      Relative URI
      
      Absolute URI
      
      Anchor element or
      
      Contains keywords for searching an relative URI
      
      Contains keywords for searching an absolute URI
      
   NOTE: From release 3.0 of the Library, the return codes from the loading functions are
   no mode BOOL, that is YES or NO. Insted they have been replaced with the following set
   of return codes defined in the Utility module:
   
  HT_WOULD_BLOCK         An I/O operation would block
                         
  HT_ERROR               Error has occured
                         
  HT_LOADED              Success
                         
  HT_NO_DATA             Success, but no document loaded. This might be the situation when
                         a  telnet sesssion is started etc.
                         
   However, a general rule about the return codes is that ERRORShave a negative value
   whereas SUCCESS has a positive value.
   
   There are also some functions to help the client getting started with the first URI.
   
  LOAD A DOCUMENT FROM RELATIVE NAME
  
 */
extern int HTLoadRelative       (CONST char *    relative_name,
                                        HTParentAnchor* here,
                                        HTRequest *     request);/*

  LOAD A DOCUMENT FROM ABSOLUTE NAME
  
 */
extern int HTLoadAbsolute       (CONST char *    addr,
                                        HTRequest *     request);/*

  LOAD A DOCUMENT FROM ABSOLUTE NAME TO A STREAM
  
 */
extern int HTLoadToStream       (CONST char *    addr,
                                        BOOL            filter,
                                        HTRequest *     request);/*

  LOAD IF NECESSARY, AND SELECT AN ANCHOR
  
   The anchor parameter may be a child anchor. The anchor in the request is set to the
   parent anchor. The recursive function keeps the error stack in the request structure so
   that no information is lost having more than one call. See also HTBindAnchor().
   
 */
extern int HTLoadAnchor         (HTAnchor  *     a,
                                        HTRequest *     request);
extern int HTLoadAnchorRecursive (HTAnchor *     a,
                                        HTRequest *     request);/*

  LOAD A DOCUMENT
  
   These are two internal routines fro loading a document which has an address AND a
   matching anchor.  (The public routines are called with one OR the other.)  This is
   recursively called from file load module to try ftp (though this will be obsolete in
   the next major release).
   
   If keep_error_stack is YES then the error (or info) stack is not cleared from the
   previous call.
   
 */
extern int HTLoad               (HTRequest * request,
                                        BOOL keep_error_stack);/*

 */
extern BOOL HTLoadTerminate     (HTRequest * request, int status);/*

  SEARCH USING RELATIVE URI
  
   Performs a search on word given by the user. Adds the search words to the end of the
   current address and attempts to open the new address.
   
 */
extern int HTSearch             (CONST char *    keywords,
                                        HTParentAnchor* here,
                                        HTRequest *     request);/*

  SEARCH USING ABSOLUTE URI
  
   Performs a keyword search on word given by the user. Adds the keyword to the end of the
   current address and attempts to open the new address.
   
 */
extern int HTSearchAbsolute     (CONST char *    keywords,
                                        CONST char *    indexname,
                                        HTRequest *     request);/*

Help Function for Clients to get started

   These function helps the client to load the first document. They are not mandatory to
   use - but they make life easier!
   
  BIND AN ANCHOR TO A REQUEST STRUCTURE WITHOUT LOADING
  
 */
extern BOOL HTBindAnchor (HTAnchor *anchor, HTRequest *request);/*

  GENERATE THE ANCHOR FOR THE HOME PAGE
  
   As it involves file access, this should only be done once when the program first runs.
   This is a default algorithm using the WWW_HOME environment variable.
   
 */
extern HTParentAnchor * HTHomeAnchor (void);/*

  FIND RELATED NAME
  
   Creates a local file URI that can be used as a relative name when calling HTParse() to
   expand a relative file name to an absolute one.
   
   The code for this routine originates from the Linemode browser and was moved here by
   howcome@dxcern.cern.ch in order for all clients to take advantage.
   
 */
extern char *  HTFindRelatedName (void);/*

Functions for Posting a Document

   This is not quite finished yet but more to come!
   
  GET A SAVE STREAM
  
    On Entry,
    
  request->anchor         is valid anchor which has previously beeing loaded
                         
    On exit,
    
  returns                 0 if error else a stream to save the object to.
                         
 */
extern HTStream * HTSaveStream (HTRequest * request);/*

Access Method Registration

   An access method is defined by an HTProtocol structure which point to the routines for
   performing the various logical operations on an object: in HTTP terms, GET, PUT, and
   POST. The access methods supported in the Library are initiated automaticly using the
   private function HTAccessInit() if not defined NO_INIT
   
   Each of these routine takes as a parameter a request structure containing details of
   the request. When the protocol class routine is called, the anchor element in the
   request is already valid (made valid by HTAccess).
   
 */
typedef enum _HTSocBlock {
    SOC_BLOCK,
    SOC_NON_BLOCK
} HTSocBlock;

typedef struct _HTProtocol {
    char *      name;
    HTSocBlock  block;
    int         (*load)         (HTRequest *     request);
    HTStream*   (*saveStream)   (HTRequest *     request);
    HTStream*   (*postStream)   (HTRequest *     request,
                                        HTParentAnchor* postTo);
} HTProtocol;

extern BOOL HTRegisterProtocol (HTProtocol * protocol);
extern void HTDisposeProtocols (void);/*

  USES PROTOCOL BLOCKING IO
  
   A small function to make life easier. Returns YES or NO. If the Library is run in
   NON-INTERACTIVE MODE then the function always returns YES;
   
 */
extern BOOL HTProtocolBlocking  (HTRequest *     request);/*

   end
   
 */
#endif /* HTACCESS_H *//*

   end of HTAccess */
